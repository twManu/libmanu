#include "BufferList.h"
#include <string.h>

//todo
#define	ERROR	printf
#include <stdio.h>
#include <errno.h>

BufferList::BufferList()
	: m_count(0)
	, m_buffers(NULL)
	, m_usrAlloc(true)
{
	pthread_mutex_init(&m_freeMutex, NULL);
	pthread_cond_init(&m_freeCond, NULL);
	pthread_mutex_init(&m_usedMutex, NULL);
	pthread_cond_init(&m_usedCond, NULL);
}


BufferList::~BufferList()
{
	Clean();
	pthread_mutex_destroy(&m_freeMutex);
	pthread_cond_destroy(&m_freeCond);
	pthread_mutex_destroy(&m_usedMutex);
	pthread_cond_destroy(&m_usedCond);
}



/*
 * Allocate space to hold buffer headers.
 * In  : count - number of buffer headers
 */
bool BufferList::preInitBuffer(unsigned int count)
{
	if( m_buffers ) return false;
	if( !count ) return false;
	m_buffers = new Buffer *[count];
	if( m_buffers ) {
		memset(m_buffers, 0, sizeof(Buffer *)*count);
		m_count = count;
	} else {
		ERROR("fail to allocate buf array\n");
		return false;
	}
	return true;
}


void BufferList::moveToFreeList()
{
	for( unsigned int i=0; i<m_count; ++i ) {
		if( !m_buffers[i] ) break;
		PutFree(m_buffers[i]);
	}
}


bool BufferList::Init(unsigned int count, unsigned int size)
{
	if( 0==size || !preInitBuffer(count) ) return false;

	//m_count set and m_buffers allocated
	for( count=0; count<m_count; ++count ) {
		m_buffers[count] = new Buffer(true);
		if( !m_buffers[count] ) break;
		if( !m_buffers[count]->Init(NULL, size) ) {
			break;
		}
	}
	if( count!=m_count ) {
		ERROR("Error new or init buffer");
		Clean();
		return false;
	}
	moveToFreeList();
	m_usrAlloc = false;                //need to free buffer
	
	return true;
}


bool BufferList::Init(Buffer **bufs)
{
	int count=0;

	if( !bufs ) return false;
	for( ; bufs[count]; ++count);         //cal number of buffers

	if( count ) {                         //at least one buffer
		if( !preInitBuffer(count) ) return false;
		while(count--) m_buffers[count] = bufs[count];
		moveToFreeList();
		return true;
	}
	//failure
	Clean();
	return false;
}


void BufferList::Clean()
{
	Buffer **tmpBuf = m_buffers;
	m_buffers = NULL;
	Buffer *usedBuf;

	//move all used to free
	while( NULL!=(usedBuf=GetUsed(0)) )
		PutFree(usedBuf);
	//if buffers parked to this list, wait all freed
	if( tmpBuf ) {
		if( !m_usrAlloc ) {                                  //that means we allocated
			for( int i=0; i<(int)m_count; ++i ) {
				if( NULL==tmpBuf[i] ) break;         //stop on first NULL
				delete tmpBuf[i];
			}
		}
		pthread_mutex_lock(&m_freeMutex);
		while( m_freeList.size()!=m_count )
			pthread_cond_wait(&m_freeCond, &m_freeMutex);
		pthread_mutex_unlock(&m_freeMutex);
		//all buffers returned
		delete [] tmpBuf;
	}
}


/*
 * Helper function for cond wait to calculate due time
 * NOTE: no input check
 * @parm tt - input timespec
 * @param waitUS - delta time in micro sec, expected to be positive, non-zero
 */
void BufferList::calDueTime(timespec *tt, int waitUS)
{
	clock_gettime(CLOCK_REALTIME, tt);
	tt->tv_nsec += (waitUS*1000);
	if( tt->tv_nsec > (1000*1000*1000) ) {
		tt->tv_nsec -= (1000*1000*1000);
		tt->tv_sec += 1;
	}
}


#define WAIT_FOREVER	(waitUS<0)

Buffer *BufferList::GetFree(int waitUS)
{
	Buffer *tmpBuf = NULL;
	timespec tOut;
	int timedWaitFail = 0;

	pthread_mutex_lock(&m_freeMutex);
	if( waitUS ) {
		if( !m_freeList.size() ) {     //no buffer now...wait
			if( WAIT_FOREVER ) pthread_cond_wait(&m_freeCond, &m_freeMutex);
			else {                     //wait period set
				calDueTime(&tOut, waitUS);
				timedWaitFail = pthread_cond_timedwait(&m_freeCond, &m_freeMutex, &tOut);
			}
		}
	}

	if( !timedWaitFail ) {            //successful wait, no wait, or no need to wait
		if( m_freeList.size() ) {
			tmpBuf = *(m_freeList.begin());
			m_freeList.pop_front();
			tmpBuf->SetUsedSize(0);       //used = 0
		}
	} else {
		if( ETIMEDOUT!=timedWaitFail )
			ERROR("timedwait param error (%d)\n", timedWaitFail);
	}

	pthread_mutex_unlock(&m_freeMutex);
	return tmpBuf;
}


void BufferList::PutFree(Buffer *buf)
{
	if( buf ) {
		pthread_mutex_lock(&m_freeMutex);
		m_freeList.push_back(buf);
		pthread_cond_broadcast(&m_freeCond);
		pthread_mutex_unlock(&m_freeMutex);
	}
}


Buffer *BufferList::GetUsed(int waitUS, bool offList)
{
	Buffer *tmpBuf = NULL;
	timespec tOut;
	int timedWaitFail = 0;

	pthread_mutex_lock(&m_usedMutex);
	if( m_usedList.size() ) {
		//buffer ready
		tmpBuf = *(m_usedList.begin());
		if( offList ) m_usedList.pop_front();
	} else {
		//need to offList or wait, then need to wait
		if( offList && waitUS ) {
			if( WAIT_FOREVER ) pthread_cond_wait(&m_usedCond, &m_usedMutex);
			else {                     //wait period set
				calDueTime(&tOut, waitUS);
				timedWaitFail = pthread_cond_timedwait(&m_usedCond, &m_usedMutex, &tOut);
			}
			if( !timedWaitFail ) {            //successful wait
				if( m_usedList.size() ) {
					tmpBuf = *(m_usedList.begin());
					if( offList ) m_usedList.pop_front();
				}
			} else {
				if( ETIMEDOUT!=timedWaitFail )
					ERROR("timedwait param error (%d)\n", timedWaitFail);
			}
		}
	}
	pthread_mutex_unlock(&m_usedMutex);

	return tmpBuf;
}


void BufferList::PutUsed(Buffer *buf)
{
	if( buf ) {
		pthread_mutex_lock(&m_usedMutex);
		m_usedList.push_back(buf);
		pthread_cond_broadcast(&m_usedCond);
		pthread_mutex_unlock(&m_usedMutex);
	}
}


unsigned int BufferList::GetUsedCount()
{
	return m_usedList.size();
}

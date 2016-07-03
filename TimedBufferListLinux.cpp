#include "TimedBufferListLinux.h"
#include <errno.h>
#include <time.h>


TimedBufferListLinux::TimedBufferListLinux()
{
	Reset();
}


TimedBufferListLinux::~TimedBufferListLinux()
{
	Clean();
}


bool TimedBufferListLinux::Init(int count, int size, BaseLock *fLock, BaseLock *uLock)
{
	if( !BufferList::Init(count, size, fLock, uLock) ) return false;

	pthread_mutex_init(&m_freeMutex, NULL);
	pthread_cond_init(&m_freeCond, NULL);
	pthread_mutex_init(&m_usedMutex, NULL);
	pthread_cond_init(&m_usedCond, NULL);

	return true;
}


void TimedBufferListLinux::Clean()
{
	Buffer *tmpBuf;
	unsigned int size=0, count=0;

	//delete all in freeList
	pthread_mutex_lock(&m_freeMutex);
	while( !m_freeList.empty() ) {
		tmpBuf = *(m_freeList.begin());
		delete tmpBuf;
		m_freeList.pop_front();
	}
	pthread_mutex_unlock(&m_freeMutex);

	//delete all in usedList
	pthread_mutex_lock(&m_usedMutex);
	while( !m_usedList.empty() ) {
		tmpBuf = *(m_usedList.begin());
		++count;
		size += tmpBuf->GetUsedSize();
		delete tmpBuf;
		m_usedList.pop_front();
	}
	pthread_mutex_unlock(&m_usedMutex);

	pthread_mutex_destroy(&m_freeMutex);
	pthread_cond_destroy(&m_freeCond);
	pthread_mutex_destroy(&m_usedMutex);
	pthread_cond_destroy(&m_usedCond);
	if( count || size ) ERROR("%d buffer %d data cleaned\n", count, size);
	Reset();
}


Buffer *TimedBufferListLinux::GetFree(int waitMS, bool *timeout)
{
	Buffer *tmpBuf = NULL;
	timespec tOut;
	int timedWait = 0, timedWaitFail = 0;

	pthread_mutex_lock(&m_freeMutex);
	if( waitMS ) {		//need to wait
		if( !m_freeList.size() ) {	//no buffer now...wait
			if( !timeout )
				pthread_cond_wait(&m_freeCond, &m_freeMutex);
			else {			//wait period set
				timedWait = 1;
				*timeout = false;
				clock_gettime(CLOCK_REALTIME, &tOut);
				tOut.tv_nsec += (waitMS*1000*1000);
				if( tOut.tv_nsec > (1000*1000*1000) ) {
					tOut.tv_nsec -= (1000*1000*1000);
					tOut.tv_sec += 1;
				}
				timedWaitFail = pthread_cond_timedwait(&m_freeCond, &m_freeMutex, &tOut);
			}
		}
	}
	
	if( !timedWaitFail ) {
		if( m_freeList.size() ) {
			tmpBuf = *(m_freeList.begin());
			m_freeList.pop_front();
			SetBufferSize(tmpBuf, 0);	//used = 0
		}
	} else {
		if( ETIMEDOUT!=timedWaitFail )
			ERROR("timedwait param error (%d)\n", timedWaitFail);
		else *timeout = true;
	}

	pthread_mutex_unlock(&m_freeMutex);
	return tmpBuf;
}


void TimedBufferListLinux::PutFree(Buffer *buf)
{
	if( buf ) {
		pthread_mutex_lock(&m_freeMutex);
		m_freeList.push_back(buf);
		pthread_cond_broadcast(&m_freeCond);
		pthread_mutex_unlock(&m_freeMutex);
	}
}


Buffer *TimedBufferListLinux::GetUsed(int waitMS, bool *timeout)
{
	Buffer *tmpBuf = NULL;
	timespec tOut;
	int timedWait = 0, timedWaitFail = 0;

	pthread_mutex_lock(&m_usedMutex);
	if( waitMS ) {		//need to wait
		if( !m_usedList.size() ) {	//no buffer now...wait
			if( !timeout )
				pthread_cond_wait(&m_usedCond, &m_usedMutex);
			else {			//wait period set
				timedWait = 1;
				*timeout = false;
				clock_gettime(CLOCK_REALTIME, &tOut);
				tOut.tv_nsec += (waitMS*1000*1000);
				if( tOut.tv_nsec > (1000*1000*1000) ) {
					tOut.tv_nsec -= (1000*1000*1000);
					tOut.tv_sec += 1;
				}
				timedWaitFail = pthread_cond_timedwait(&m_usedCond, &m_usedMutex, &tOut);
			}
		}
	}
	
	if( !timedWaitFail ) {
		if( m_usedList.size() ) {
			tmpBuf = *(m_usedList.begin());
			m_usedList.pop_front();
			m_usedSize -= tmpBuf->GetUsedSize();
			++m_usedOutCount;
		}
	} else {
		if( ETIMEDOUT!=timedWaitFail )
			ERROR("timedwait param error (%d)\n", timedWaitFail);
		else *timeout = true;
	}

	pthread_mutex_unlock(&m_usedMutex);
	return tmpBuf;
}


void TimedBufferListLinux::PutUsed(Buffer *buf, unsigned int sz)
{
	if( buf ) {
		pthread_mutex_lock(&m_usedMutex);
		SetBufferSize(buf, sz);
		m_usedSize += sz;
		m_usedList.push_back(buf);
		++m_usedInCount;
		pthread_cond_broadcast(&m_usedCond);
		pthread_mutex_unlock(&m_usedMutex);
	}
}

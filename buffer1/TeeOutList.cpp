#include "TeeOutList.h"
#include "TeeInList.h"
#include <string.h>

//todo
#define	ERROR	printf
#include <stdio.h>
#include <errno.h>

TeeOutList::TeeOutList(TeeInList &inList)
	: BufferList()
{
	//todo ref-count to input
	inList.getUsedLock(&m_srcMutex, &m_srcCond);
}


TeeOutList::~TeeOutList()
{
	Clean();
}


//not supported
bool TeeOutList::Init(unsigned int count, unsigned int size)
{
	return false;
}


bool TeeOutList::Init(TeeBuffer **bufs)
{
	//todo
	return false;
}


void TeeOutList::Clean()
{
	if( m_usedList.size() ) ERROR("tout has used\n");
	if( m_freeList.size() ) ERROR("tout has free\n");
}


#define WAIT_FOREVER	(waitUS<0)

Buffer *TeeOutList::GetFree(int waitUS)
{
	Buffer *tmpBuf = NULL;
	timespec tOut;
	int timedWaitFail = 0;

	pthread_mutex_lock(m_srcMutex);
	if( waitUS ) {
		if( !m_freeList.size() ) {     //no buffer now...wait
			if( WAIT_FOREVER ) pthread_cond_wait(m_srcCond, m_srcMutex);
			else {                     //wait period set
				calDueTime(&tOut, waitUS);
				timedWaitFail = pthread_cond_timedwait(m_srcCond, m_srcMutex, &tOut);
			}
		}
	}

	if( !timedWaitFail ) {            //successful wait, no wait, or no need to wait
		if( m_freeList.size() ) {
			tmpBuf = *(m_freeList.begin());
			m_freeList.pop_front();
		}
	} else {
		if( ETIMEDOUT!=timedWaitFail )
			ERROR("timedwait param error (%d)\n", timedWaitFail);
	}

	pthread_mutex_unlock(m_srcMutex);
	return tmpBuf;
}


//wakeup that waiting on usedList of input
void TeeOutList::PutFree(Buffer *buf)
{
	if( buf ) {
		pthread_mutex_lock(m_srcMutex);
		m_freeList.push_back(buf);
		pthread_cond_broadcast(m_srcCond);
		pthread_mutex_unlock(m_srcMutex);
	}
}

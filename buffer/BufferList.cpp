#include "BufferList.h"
#include <string.h>


BufferList::BufferList(BaseLock *fLock, BaseLock *uLock)
	: m_count(0)
	, m_buffers(NULL)
	, m_freeLock(fLock)
	, m_usedLock(uLock)
	, m_usrAlloc(false)
{
}


BufferList::~BufferList()
{
	Clean();
}

#define	FLOCK(x)	do { if(m_freeLock) m_freeLock->Acquire(); } while(0)
#define	ULOCK(x)	do { if(m_usedLock) m_usedLock->Acquire(); } while(0)
#define	FUNLOCK(x)	do { if(m_freeLock) m_freeLock->Release(); } while(0)
#define	UUNLOCK(x)	do { if(m_usedLock) m_usedLock->Release(); } while(0)


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
		m_usrAlloc = true;
		return true;
	}
	return false;
}


void BufferList::Clean()
{
	Buffer **tmpBuf = m_buffers;
	m_buffers = NULL;
	
	FLOCK();
	while( m_freeList.size() )
		m_freeList.pop_front();
	FUNLOCK();
	ULOCK();
	while( m_usedList.size() )
		m_usedList.pop_front();
	UUNLOCK();

	if( tmpBuf ) {
		if( !m_usrAlloc ) {
			for( int i=0; i<(int)m_count; ++i ) {
				if( NULL==tmpBuf[i] ) break;         //stop on first NULL
				delete tmpBuf[i];
			}
		}
		delete [] tmpBuf;
	}
}


Buffer *BufferList::GetFree()
{
	Buffer *tmpBuf = NULL;

	FLOCK();
	if( m_freeList.size() ) {
		tmpBuf = *(m_freeList.begin());
		m_freeList.pop_front();
	}
	FUNLOCK();

	return tmpBuf;
}


void BufferList::PutFree(Buffer *buf)
{
	if( buf ) {
		FLOCK();
		m_freeList.push_back(buf);
		FUNLOCK();
	}
}


Buffer *BufferList::GetUsed(bool offList)
{
	Buffer *tmpBuf = NULL;

	ULOCK();
	if( m_usedList.size() ) {
		tmpBuf = *(m_usedList.begin());
		if( offList ) m_usedList.pop_front();
	}
	UUNLOCK();

	return tmpBuf;
}


void BufferList::PutUsed(Buffer *buf)
{
	if( buf ) {
		ULOCK();
		m_usedList.push_back(buf);
		UUNLOCK();
	}
}


unsigned int BufferList::GetUsedCount()
{
	return m_usedList.size();
}

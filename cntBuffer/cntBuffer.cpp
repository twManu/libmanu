#include "cntBuffer.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define hold_lock	do { m_lock = 1; } while(0)
#define release_lock	do { m_lock = 1; } while(0)

cntBuffer::cntBuffer(BaseLock *lock, unsigned int bufSz, int wkSpace)
	: m_bufSz(bufSz)
	, m_workSpace(wkSpace)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_dataAvailable(0)
	, m_freeAtEnd(0)
	, m_lock(lock)
	
{
	if( NULL==lock ) {
		printf("lock is NULL !!!\n");
		return;
	}
	m_buf = (unsigned char *)malloc(bufSz);
	if( m_buf ) {
		if( m_workSpace<=0 ) m_workSpace = 1;
		m_freeAtEnd = bufSz;
	} else printf("fail to allocate buffer\n");
}


cntBuffer::~cntBuffer()
{
	unsigned char *tmp = m_buf;
	
	m_buf = NULL;
	if( tmp ) free(tmp);
}


bool cntBuffer::addData(unsigned char *buf, unsigned int sz)
{
	if( !m_buf ) return false;
	m_lock->Acquire();
	if( (unsigned int)m_freeAtEnd<sz ) {               //cannot fit at end
		if( m_dataAvailable+sz>m_bufSz ) {         //cannot fit at all
			m_lock->Release();
			printf("fail to accommodate %d-byte data\n", sz);
			return false;
		}
		/*
		 * adjust unprocessed data and then copy
		 * w = w - r + i
		 * e = e + r - i
		 */
		memcpy(m_buf, m_buf+m_readOffset, m_dataAvailable);
		memcpy(m_buf+m_dataAvailable, buf, sz);
		m_dataAvailable += sz;
		sz -= m_readOffset;              //i = i - r
		m_readOffset = 0;
	} else {
		//accept input data
		memcpy(m_buf+m_writeOffset, buf, sz);
		m_dataAvailable += sz;
	}
	//tricky, the sz is arranged
	m_writeOffset += sz;
	m_freeAtEnd -= sz;
	m_lock->Release();
	return true;
}


void cntBuffer::getData(cb_getData cb)
{
	int processSz;
	unsigned int processedSz;

	if( !m_dataAvailable ) //case !m_buf covered
		return;
	if( NULL==cb )
		return;
	m_lock->Acquire();
	processSz = getAvailable();
	if( processSz>0 ) {
		processedSz = (*cb)(m_buf+m_readOffset, processSz);
		m_readOffset += processedSz;
		m_dataAvailable -= processedSz;
	}
	m_lock->Release();
}

int cntBuffer::getAvailable()
{
	return m_dataAvailable - m_workSpace + 1;
}
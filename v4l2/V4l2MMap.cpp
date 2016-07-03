#include "V4l2MMap.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

V4l2MMap::V4l2MMap(BaseLock *initLock, bufferCB bufCB, void *usrCntx)
	: V4l2Base()
	, LinuxThread(initLock)
	, m_qCount(0)
	, m_lockThread(initLock)
	, m_bufCB(bufCB)
	, m_usrCntx(usrCntx)
	
{
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_cond, NULL);
}


V4l2MMap::~V4l2MMap()
{
	streaming(false);       //blocked thread will wake up
	LinuxThread::Destroy();
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_mutex);
}


void V4l2MMap::ThreadMain()
{
	V4l2BaseBuffer *buf; 
	while( LinuxThread::isRunning() ) {
		buf = V4l2Base::getBuffer();
		if( !buf ) continue;
		//got a buffer
		if( m_bufCB ) (*m_bufCB)(m_usrCntx, buf);
		pthread_mutex_lock(&m_mutex);
		m_bufList.push_back(buf);
		++m_qCount;
		pthread_cond_signal(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}
	pthread_cond_signal(&m_cond);    //make sure no block of user
}


bool V4l2MMap::initV4l2(int devIndex, unsigned int bufCount, bool mmap, bool block)
{
	m_blockUser = block;
	if( !V4l2Base::initV4l2(devIndex, bufCount, mmap, true) )    //thread is always blocked
		return false;
	
	if( m_lockThread ) m_lockThread->Acquire();
	if( !LinuxThread::Start() ) {
		if( m_lockThread ) m_lockThread->Release();
		printf("fail to create thread\n");
		return false;
	}

	return true;
}


bool V4l2MMap::streaming(bool on)
{
	if( on ) {
		if( V4l2Base::streaming(on) ) {
			//let thread run
			if( m_lockThread ) m_lockThread->Release();
			return true;
		}
		return false;
	} else {
		V4l2Base::streaming(on);
		LinuxThread::Destroy();
		if( m_lockThread ) m_lockThread->Release();
	}
	return true;
}


V4l2BaseBuffer *V4l2MMap::getBuffer(bool offList)
{
	V4l2BaseBuffer *buf = NULL;

	if( m_streaming ) {
		pthread_mutex_lock(&m_mutex);
again:
		if( m_qCount ) {
			buf = *(m_bufList.begin());
			if( offList ) {
				m_bufList.pop_front();
				--m_qCount;
			}	
		} else if ( m_blockUser ) {
			pthread_cond_wait(&m_cond, &m_mutex);
			if( LinuxThread::isRunning() ) goto again;
		}
		pthread_mutex_unlock(&m_mutex);
	}

	return buf;
}


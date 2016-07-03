#ifndef __OSLINUX_H__
#define __OSLINUX_H__

#include <pthread.h>
#include <sys/time.h>
#include "BaseLock.h"
#include "BaseThread.h"
#include "TimeSec.hpp"


/*!
    @class LinuxLock
    Use pthread mutex to implement lock. This object blocks a thread until lock acquired
 */
class LinuxLock : public BaseLock
{
protected:
	pthread_mutex_t      m_mutex;

public:
	LinuxLock() {};
	virtual              ~LinuxLock();
	//! @brief to create m_lockObj by implementation, true means success
	virtual bool         Init();
	//! @brief to acquire lock, blocked until acquired
	virtual void         Acquire();
	//! @brief to release lock
	virtual void         Release();
};


class RateCtrl {
protected:
	struct timeval       m_Tstart;
	struct timeval       m_Tcurrent;
	unsigned int         m_rateKbps;            //kbps

public:
	RateCtrl() : m_rateKbps(0) {}
	//! @brief Init with same start time
	RateCtrl(RateCtrl &src) { this->m_Tstart = src.m_Tstart; }
	~RateCtrl() {}
	//! @brief Re-init the start time
	void                 timeReset() { gettimeofday(&m_Tstart, NULL); }
	//! @brief Allow rate change
	void                 setRate(unsigned int rate_kbps) { m_rateKbps=rate_kbps; }
	//! @brief Return size in byte it should be since reset
	unsigned long long   sizeSoFar(struct timeval *dTSoFar) {
		unsigned long long size, dTus;

		gettimeofday(&m_Tcurrent, NULL);
		dTus = m_Tcurrent.tv_usec - m_Tstart.tv_usec;
		dTus += (m_Tcurrent.tv_sec - m_Tstart.tv_sec) * (1000*1000);
		if( dTSoFar ) {
			dTSoFar->tv_sec = dTus / (1000*1000);
			dTSoFar->tv_usec = dTus % (1000*1000);
		}
		size = dTus * m_rateKbps / 8000;	//kbps*us = 10^(3)bps * 10^(-6)s = 10^(-3)bit = 1/8*10^(-3)B
		return size;
	}
	RateCtrl &operator=(const RateCtrl &src) {
		this->m_Tstart = src.m_Tstart;
		this->m_rateKbps = src.m_rateKbps;
		return *this;
	}
};


/*
 * Implement the thread requires join
 * 
 * The child class MUST implement ThreadMain() which is triggered by Start
 *    LinuxThread.Start->BaseThread.ThreadEntry->LinuxThread.Proc->ThreadMain
 *
 * The parent thread can check with BaseThread.isRunning and wait the child stops
 * or delete the LinuxThread to force it stop
 */
class LinuxThread : public BaseThread
{
protected:
	pthread_t            m_thread;

	/*! @detail
	 * The entrance of Linux thread which call ThreadMian of implementation
	 * In most case the derived class no need to implement this routine
	 */
	virtual void         Proc();
	//!@brief The main implementation and should honor m_toStop
	virtual void         ThreadMain() = 0;

	/*! @detail
	 * To destroy created
	 * The derived class free its resource here
	 */
	virtual void         Destroy();

public:
	LinuxThread(BaseLock *lock=NULL);
	virtual              ~LinuxThread();
	/*! @detail
	 * To create and start the thread
	 * In most case the derived class no need to implement this routine
	 * @return 0 - failure
	 *          otherwise - successful
	 */
	virtual int          Start();
};


class LinuxTimeSec : public TimeSec {
protected:
	virtual unsigned int getCurTime();

public:
	LinuxTimeSec(int periodSec=0);
	~LinuxTimeSec();
};

#endif	//__OSLINUX_H__

#ifndef	__BASETHREAD_H__
#define	__BASETHREAD_H__

#include "libcommon.h"
#include "BaseLock.h"

#ifdef	DEF_DEBUG_THREAD
	#define	DBG_THREAD 	ERROR
#else	//DEF_DEBUG_THREAD
	#if	IF_LINUX_TYPE_DEBUG
		#define	DBG_THREAD(a, arg...) do {} while(0)
	#else	//IF_LINUX_TYPE_DEBUG
		#define DBG_THREAD(a, ...) do {} while(0)
	#endif	//IF_LINUX_TYPE_DEBUG
#endif	//DEF_DEBUG_THREAD

/*! @class
 * The Proc() runs as soon as Start().
 * Stop() won't kill thread. That is done in Destroy
 *
 * If lock is provided in construct and acquired before hand, the thread won't
 * start nor exit before the lock is released.
 * 
 * The implementation is in OSxxx.
 *
 * Usage 1:
 *  constructor
 *  start
 *  while(isRunning)
 *  destroy
 *  --- delete lock if lock is provided to constructor
 *
 * Usage 2:
 *  constructor
 *  start
 *  stop (optionally)
 *  destroy
 *  --- delete lock if lock is provided to constructor
 *
 */
class BaseThread {
protected:
	int                  m_toStop;
	unsigned long long   m_tid;
	BaseLock             *m_initLock;        //Acquire before running
	/*! @detail
	 * The routine a thread should implemente its main task.
	 * It MUST honor m_toStop
	 */
	virtual void         Proc() = 0;

	/*! @detail
	 * To destroy created thread
	 */
	virtual void         Destroy()= 0;

	/*! @detail
	 * Entry of all thread routine then call to Proc.
	 * All Start() should take this as thread entry
	 * @param cntxt - "this" point as thread context
	 */
	static void          ThreadEntry(void *cntxt) {
		BaseThread *obj = (BaseThread *)cntxt;
		if( !obj ) ERROR("Null thread context\n");
		if( obj->m_initLock ) {                  /* the parent should avoid delete m_initLock before this */
			obj->m_initLock->Acquire();
			//possibly blocked here
			obj->m_initLock->Release();
			obj->m_initLock = NULL;
			if( obj->m_toStop ) {
				ERROR("Thread not running\n");
				return;
			}
		}
		obj->Proc();
	}

public:
	BaseThread() : m_toStop(0), m_tid(0), m_initLock(NULL) {}
	/*! @detail
	 * This method provide a way to control the time of running. The
	 * Acquire() will be called before Proc() called if the m_toStop
	 * stays false. The lock will be Release() after Acquire().
	 *
	 * The lock is used once and for all.
	 * Caller should pay attention not to delete the lock before.
	 * This lock is not deleted by BaseThread, caller should do that...
	 *
	 * @param lock - If lock presents, to sync with (mostly) parent thread
	 *               actually it should use Start() if not present
	 */
	BaseThread(BaseLock *lock) : m_toStop(0), m_tid(0), m_initLock(lock) {}
	virtual ~BaseThread() {
		Stop();
		DBG_THREAD("In Destructor of BaseThread\n");
	}
	/*! @detail
	 * To create and start the thread. It should call ThreadEntry
	 * to initialize the thread task and then Proc will be invoked
	 * @param m_tid - should be updated if successful
	 * @return 0 - failure
	 *          otherwise - successful
	 */
	virtual int          Start() = 0;

	//!@brief Proc or LinuxThread.ThreadMain is expected to honor this flag
	void                 Stop() { m_toStop = 1; }
	//!@brief Default of thread statsu report. Proc should set m_toStop correctly
	virtual int          isRunning() { return !m_toStop; }
	//!@brief Implementation should update m_tid in Start()
	unsigned long long   getID() { return m_tid; }
};

#endif	//__BASETHREAD_H__

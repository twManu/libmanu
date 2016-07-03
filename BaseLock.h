#ifndef	__BASELOCK_H__
#define	__BASELOCK_H__

#include "stdlib.h"
#include "libcommon.h"

/*!
   @class BaseLock
   To define abstraction class for lock facility. It aimed at simple exclusive
   access and perform no check of repeat acquisitation or release. The calling
   secquence follows
   
   bl = new BaseLock();
   bl.Init();      ---> delete bl if false returned
   bl.Acquire();
   bl.Release();
   bl.Acquire();
   bl.Release();
      :
   delete bl;
 */
class BaseLock {
protected:
	//! @brief a lock object holder by implementation
	void                 *m_lockObj;

public:
	BaseLock() { m_lockObj = NULL; }
	virtual              ~BaseLock() { }
	//! @brief to create m_lockObj by implementation, true means success
	virtual bool         Init() = 0;
	//! @brief to acquire lock, blocked until acquired
	virtual void         Acquire() = 0;
	//! @brief to release lock
	virtual void         Release() = 0;
};

#endif	//__BASELOCK_H__

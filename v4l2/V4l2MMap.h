#ifndef	__V4L2_MMAP_H__
#define	__V4L2_MMAP_H__

#include "V4l2Base.h"
#include "../BaseLock.h"
#include "../OSLinux.h"
#include <pthread.h>
#include <list>

/* support memory map of v4l2 streaming */

using namespace std;

/* callback for usr if they want to process anything before a buffer queued */
extern "C" typedef void (*bufferCB)(void *usrCntx, V4l2BaseBuffer *buffer);

/*
   usage:
	V4l2MMap()
	setFormat()
	initV4l2()
	streaming(on) ...option starts, ok for single threading
	getBuffer()
	putBuffer()
	streaming(off) ...option ends
	~V4l2MMap()
 */
class V4l2MMap : public V4l2Base, public LinuxThread {
protected:
	virtual void ThreadMain();
	//! @brief clean mapped buffer and space allocated, expected to be stop in advance
	void V4l2Unmap();
	list<V4l2BaseBuffer *>         m_bufList;          //protected by m_mutex
	unsigned int                   m_qCount;           //protected by m_mutex
	pthread_mutex_t                m_mutex;
	pthread_cond_t                 m_cond;
	bool                           m_blockUser;
	BaseLock                       *m_lockThread;
	bufferCB                       m_bufCB;
	void                           *m_usrCntx;

public:
	/* !@details
	 * @param lock - for buffer list protection. each free buffer is submit to driver,
	 * so we maintain used list only
	 * @param initLock - for internal thread contrl
	 * @param lock - to avoid race between class and user thread
	 * @param bufCB - for buffer process before queued to used list. if not NULL,
	 * called when a buffer arrival. In this case, a dedicated thread is forked to
	 * dequeue. Otherwise, queue and deQueue a buffer is conducted by user.
	 * @param usrCntx - the argument used for bufCB. Can be NULL
	 */
	V4l2MMap(BaseLock *initLock, bufferCB bufCB=NULL, void *usrCntx=NULL);
	virtual ~V4l2MMap();
	/* !@details
	 * open file, query capability, request buffer and do mappings
	 * @param devIndex - <0 means auto search, otherwise specifi /dev/videoN used
	 * @param bufCount - max buffer count to request
	 * @param mmap - whether to do alloc and mmap. valid if bufCount!=0
	 *           if no mmap, all buffers needs to be assign of at least addr
	 * @param block - true if getBuffer will be blocked
	 */
	virtual bool initV4l2(int devIndex, unsigned int bufCount=0, bool mmap=true, bool block=true);
	virtual bool streaming(bool on);
	/* !@details
	 * get a buffer. no lock operation...suitable for single threading
	 * it might sleep if blockIO
	 * @return NULL if error
	 */
	virtual V4l2BaseBuffer *getBuffer(bool offList=true);
};

#endif	//__V4L2_MMAP_H__

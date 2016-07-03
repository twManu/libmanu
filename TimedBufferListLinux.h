#ifndef	__TIMEDBUFFERLISTLINUX_H__
#define	__TIMEDBUFFERLISTLINUX_H__

#include "BufferList.h"
#include "OSLinux.h"	//LinuxLock


/*!
   @class TimedBufferListLinux
   Use Lock for doubly linked list exclusive access. It implements two FIFO
   queues. One for freed and one for occupied buffers. All buffers are inited
   to the free queue.
   
   This class ignores the FreeLock and UsedLock input when Init(). Instead, it
   use internal mutex to achieve timed wait.

   bl = new BufferList();
   bl.Init(10, 1024);   ---> delete bl if false returned

   [consumer]
   buf = bl.GetFree();
   if buf {
	<copy to buf w/ size>
	bl.PutUsed(buf, size);
   }

   [producer]
   buf = bl.GetUsed();
   if buf {
	<copy from buf w/ GetSize()>
	bl.PutFree(buf);
   }
      :
   delete bl;
 */
class TimedBufferListLinux : public BufferList {
protected:
	pthread_mutex_t      m_freeMutex;
	pthread_cond_t       m_freeCond;
	//-------- freeXXX, including parent's member are protected by m_freeMutex

	pthread_mutex_t      m_usedMutex;
	pthread_cond_t       m_usedCond;
	//-------- freeXXX, including parent's member are protected by m_freeMutex

	//! @brief free all buffers in freed and used queues
	virtual void         Clean();

public:
	TimedBufferListLinux();
	//! @brief Caller has to make sure all buffers are in either freed or used Q before deletion
	virtual ~TimedBufferListLinux();

	//! @brief initialize. Return true if successful
	/*!
	    @details
	    This routine MUST be called before using other function.
	    @param count - number of buffers to create
	    @param size - data size of each buffer
	    @param fLock - not actually use but cannot be NULL
	    @param uLock - not actually use but cannot be NULL
	    @return true - successful
	             false - failure
	 */
	virtual bool         Init(int count, int size, BaseLock *fLock, BaseLock *uLock);

	/*! @detail
	 * Get a free buffer from FIFO queue (head), size is cleared
	 * The following defines the mode implementation should action and the interpretation of parameters
	 * 
	 * type\(WAIT, TO) | (0, NULL) | ( 0, !NULL )  | (!0, NULL)  | (!0, !NULL)
	 * ----------------+-----------+---------------+-------------+-------------
	 * Q_BLOCK_TIMED   | n-wait    | n-wait        | wait        | time'd wait
	 *
	 * @param waitMS - millisecond to wait
	 * @param timeout - report whether timeout if TYPE_TIMED supported.
	 * @return buffer to be returned, When TYPE_TIMED supported and timeout, NULL should be returned.
	 */
	virtual Buffer       *GetFree(int waitMS, bool *timeout);

	//! @brief return a free buffer to FIFO queue (tail)
	virtual void         PutFree(Buffer *);

	/*! @detail
	 * Get a used buffer from FIFO queue (head),
	 * The following defines the mode implementation should action and the interpretation of parameters
	 * 
	 * type\(WAIT, TO) | (0, NULL) | ( 0, !NULL )  | (!0, NULL)  | (!0, !NULL)
	 * ----------------+-----------+---------------+-------------+-------------
	 * Q_BLOCK_TIMED   | n-wait    | n-wait        | wait        | time'd wait
	 *
	 * @param waitMS - millisecond to wait
	 * @param timeout - report whether timeout if TYPE_TIMED supported.
	 * @return buffer to be returned, When TYPE_TIMED supported and timeout, NULL should be returned.
	 */
	virtual Buffer       *GetUsed(int waitMS, bool *timeout);

	//! @brief return a used buffer w/ size to FIFO queue (tail)
	virtual void         PutUsed(Buffer *buf, unsigned int size);

	//! @brief return BLOCK way (or'ed) suppored
	virtual int          GetBufBlockType() { return Q_BLOCK_TIMED; }
};


#endif	//__TIMEDBUFFERLISTLINUX_H__

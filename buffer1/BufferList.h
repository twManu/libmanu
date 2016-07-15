#ifndef	__BUFFERLIST_H__
#define	__BUFFERLIST_H__

#include "Buffer.h"
#include <pthread.h>
#include <list>
#include <time.h>

using namespace std;


/*!
   @class BufferList
   Use Lock for doubly linked list exclusive access. It implements two FIFO
   queues. One for freed and one for occupied buffers. All buffers are inited
   to the free queue.
   
   This class is used as default implementation. It works as non-block list and
   is suitable for polling model. The derived class can override virtual function
   for other implementations.
   
   The destructor will delete the buffer objects allocated this class

case 1: lib allocates buffers
   bl = new BufferList();            
   bl.Init(10, 1024);   ---> delete bl if false returned
   // now all buffer in freeList no matter lock provided or not
   buf=bl.GetFree();
   // process buffer
   bl.PutUsed(buf);
      :
   delete bl;
   
case 2: no allocation, buffers initialized from outside
   bl = new BufferList();
   b1.Init(bufs)
   // now all buffer in freeList no matter lock provided or not
   buf=bl.GetFree();
   // process buffer
   bl.PutUsed(buf);
      :
   delete bl;

case 3: no allocation, buffers just pass in and out
	bl = new BufferList();
	// no buffer in either queue
	bl.PutUsed(buf);
	  :
	buf=bl.GetFree(); 
 */
class BufferList {
protected:
	//! @brief Nr. of buffer managed by this list
	unsigned int         m_count;
	Buffer               **m_buffers;

	pthread_mutex_t      m_freeMutex;
	pthread_cond_t       m_freeCond;
	//! @brief list of free buffers
	list<Buffer *>       m_freeList;
	//-------- above protected by m_freeMutex

	pthread_mutex_t      m_usedMutex;
	pthread_cond_t       m_usedCond;
	//! @brief list of occupied buffers
	list<Buffer *>       m_usedList;
	//-------- above protected by m_usedMutex
	//! @brief we must free that we allocated, when false
	bool                 m_usrAlloc;

	/*! @detail
	    undo init and remove all buffers from both queues 
	 */
	virtual void         Clean();
	//! @brief allocate buffer array and avoid duplicated allocation
	virtual bool         preInitBuffer(unsigned int count);
	//! @brief queue all to free list, it is used in init stage
	virtual void         moveToFreeList();
	//! @brief calculate time out time
	void                 calDueTime(timespec *tt, int waitUS);

public:
	/*! @detail
	 */
	BufferList();
	//! @brief Caller has to make sure all buffers are in either freed or used Q before deletion
	virtual ~BufferList();

	/*!
	    @details
	    The caller has to new locks before Init and delete locks after destructor called
	    The library will allocate the buffer as size provided
	    
	    After Init done, buffers are free queue
	    @param count - number of buffers to create
	    @param size - data size of each buffer
	    @param fLock - lock for freeList
	    @param uLock - lock for usedList
	    @return true - successful
	             false - failure
	 */
	virtual bool         Init(unsigned int count, unsigned int size);
	/*! @details
	 * The caller allocated buffer and inject to the list. In this case, buffers won't be
	 * deleted by list.
	 * @param bufs - an array, ended with NULL,  of Buffer pointer
	 * @return true - successful
	 *         false - failure
	 */
	virtual bool         Init(Buffer **bufs);

	/*! @detail
	 * Get a free buffer from FIFO queue (head), size is cleared
	 * The following defines the mode implementation should action and the interpretation of parameters
	 *
	 * @param waitUS - micro sec to wait. negtive value means wait forever
	 * @return buffer to be returned, When TYPE_TIMED supported and timeout, NULL should be returned.
	 */
	virtual Buffer       *GetFree(int waitUS);

	//! @brief return a free buffer to FIFO queue (tail)
	virtual void         PutFree(Buffer *buf);

	/*! @detail
	 * Get a used buffer from FIFO queue (head),
	 * The following defines the mode implementation should action and the interpretation of parameters
	 * @param - offList, true if to delete from the list, otherwise keep in the list
	 * @param waitUS - micro sec to wait. negtive value means wait forever. wait only when offList
	 * @return buffer to be returned or NULL.
	 */
	virtual Buffer       *GetUsed(int waitUS, bool offList=true);

	//! @brief return a used buffer w/ size to FIFO queue (tail)
	virtual void         PutUsed(Buffer *buf);

	//! @brief return used buffer count
	unsigned int         GetUsedCount();
};


#endif	//__BUFFERLIST_H__

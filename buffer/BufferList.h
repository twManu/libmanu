#ifndef	__BUFFERLIST_H__
#define	__BUFFERLIST_H__

#include "../BaseLock.h"
#include "Buffer.h"
#include <list>

using namespace std;


/*!
   @class BufferList
   Use Lock for doubly linked list exclusive access. It implements two FIFO
   queues. One for freed and one for occupied buffers. All buffers are inited
   to the free queue.
   
   This class is used as default implementation. It works as non-block list and
   is suitable for polling model. The derived class can override virtual function
   for other implementations.
   
   The destructor will delete the buffer as well

case 1:
   bl = new BufferList();
   bl.Init(10, 1024);   ---> delete bl if false returned
   ==== now all buffer in freeList no matter lock provided or not ====

      :
   delete bl;
   
case 2:
   bl = new BufferList(false);        //no allocation, buffer is injected from outside
   b1.Init(10, buf)
   b1.Init(10, buf)                   //each placed in order
   b1.Init(10, buf)                   //total times
   :
   b1.Init(10, buf)                   //till "count" times injection ...finish the init
      :
   ==== now all buffer in freeList no matter lock provided or not ====
 */
class BufferList {
protected:
	//! @brief Nr. of buffer managed by this list
	unsigned int         m_count;
	unsigned int         m_waitCount;
	Buffer               **m_buffers;

	//! @brief exclusive access of free list
	BaseLock             *m_freeLock;
	//! @brief list of free buffers
	list<Buffer *>       m_freeList;
	//-------- above protected by m_freeLock

	//! @brief exclusive access of occupied list
	BaseLock             *m_usedLock;
	//! @brief list of occupied buffers
	list<Buffer *>       m_usedList;
	//-------- above protected by m_usedLock

	/*! @detail
	    Free all buffer in free and used queues. Make sure no one is accessing.
	 */
	virtual void         Clean();
	//! @brief allocate buffer array and avoid duplicated allocation
	virtual bool         preInitBuffer(unsigned int count);
	//! @brief queue all to free list, it is used in init stage
	virtual void         moveToFreeList();

public:
	/*! @detail
	 * if no lock provided, that queue is not protected...nor does the statistics
	 * @param allocMem - whether buffer object allocate memory itself
	 */
	BufferList(BaseLock *fLock=NULL, BaseLock *uLock=NULL);
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
	virtual bool         Init(unsigned int count, Buffer *buf);

	/*! @detail
	 * Get a free buffer from FIFO queue (head), size is cleared
	 * The following defines the mode implementation should action and the interpretation of parameters
	 *
	 * @return buffer to be returned, When TYPE_TIMED supported and timeout, NULL should be returned.
	 */
	virtual Buffer       *GetFree();

	//! @brief return a free buffer to FIFO queue (tail)
	virtual void         PutFree(Buffer *buf);

	/*! @detail
	 * Get a used buffer from FIFO queue (head),
	 * The following defines the mode implementation should action and the interpretation of parameters
	 * @param - offList, true if to delete from the list, otherwise keep in the list
	 * @return buffer to be returned or NULL.
	 */
	virtual Buffer       *GetUsed(bool offList);

	//! @brief return a used buffer w/ size to FIFO queue (tail)
	virtual void         PutUsed(Buffer *buf);

	//! @brief return used buffer count
	unsigned int         GetUsedCount();
};


#endif	//__BUFFERLIST_H__

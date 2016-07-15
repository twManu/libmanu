#ifndef	__TEEOUTLIST_H__
#define	__TEEOUTLIST_H__

#include "BufferList.h"
#include "TeeBuffer.h"

class TeeInList;


/*!
   @class TeeOutList
   The tee is composed of one input list and many output lists. The VideoBuffers from input side is to
   be cloned to 0-N outputs as TeeBuffer.
   This class handle output side activities and deal with TeeBuffer. Since buffers coming in from
   TeeInList and stay until all clone done, we design to us usedLock/Mutex of input as that of the
   freeList 
 */
class TeeOutList : public BufferList {
protected:
	pthread_mutex_t          *m_srcMutex;
	pthread_cond_t           *m_srcCond;
	virtual void             Clean();

public:
	/*! @detail
	 * if no lock provided, that queue is not protected...nor does the statistics
	 * @param allocMem - whether buffer object allocate memory itself
	 */
	TeeOutList(TeeInList &inList);
	//! @brief Caller has to make sure all buffers are in either freed or used Q before deletion
	virtual ~TeeOutList();

	/*!
	    @details
		not expected to be used as self-alloc, fail it
	 */
	virtual bool         Init(unsigned int count, unsigned int size);
	/*! @details
	 * The caller allocated buffer and inject to the list. In this case, buffers won't be
	 * deleted by list.
	 * As init, the m_maxTee outputs are created but all disabled.
	 * 
	 * @param bufs - an array, ended with NULL, of TeeBuffer pointer
	 *        NULL - none is parked to the list
	 * @return true - successful
	 *         false - failure
	 */
	virtual bool         Init(TeeBuffer **bufs);


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
};


#endif	//__TEEOUTLIST_H__

#ifndef	__TEEINLIST_H__
#define	__TEEINLIST_H__

#include "BufferList.h"
#include "VideoBuffer.h"

class TeeOutList;

/*!
   @class TeeList
   The tee is composed of one input list and many output lists. The VideoBuffers from input side is to
   be cloned to 0-N outputs as TeeBuffer. And the output side list are always used a pass through.
   
   Outputs of a tee can only be used between get- and put- TeeOutput so buffers must be kept in
   list before put operation.
 
   Usage: buffer starts with this list
   	construct(n)
	Init(buffers)
	list0=getTeeOutput(0)
	//use of buffer in tee0
	list2=getTeeOutput(2)
	//use of buffer in tee2
	//return all buffer to tee2
	putTeeOutput(list2)
	//return all buffer to tee0
	putTeeOutput(list0)
	destruct ... WAITING for free buffers

 */
class TeeInList : public BufferList {
friend TeeOutList;
protected:
	//! @brief Nr. of buffer managed by this list
	int                  m_teeCount;              //0 - (teeCount-1)
	unsigned int         m_activeTee;             //bit0 -> 1st tee, bit1, 2nd tee
	TeeOutList           **m_outputs;
	list<VideoBuffer *>  m_lingerList;            //cloning buffer by teeThread
	bool                 m_stopThread;
	pthread_t            m_thread;

	/*! @detail
	    Free all buffer in free and used queues. Make sure no one is accessing.
	 */
	virtual void         Clean();
	//! @brief allocate buffer array and avoid duplicated allocation
	//! @brief to pass lock to sink side
	void                 getUsedLock(pthread_mutex_t **srcMutex, pthread_cond_t **srcCond);
	//! @brief for all freed in output pin, maintain ref of source buffer and delete clone
	void                 cleanOutFree();
	//! @brief to move linger buffer to freedIn, force will clean all buffer, otherwise only 0 ref of on top
	void                 popLinger(bool force);

public:
	/*! @detail
	 * if no lock provided, that queue is not protected...nor does the statistics
	 * @param allocMem - whether buffer object allocate memory itself
	 */
	TeeInList(unsigned int teeCount);
	//! @brief Caller has to make sure all buffers are in either freed or used Q before deletion
	virtual ~TeeInList();

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
	 * @param bufs - an array, ended with NULL, of VideoBuffer pointer
	 *        NULL - none is parked to the list
	 * @return true - successful
	 *         false - failure
	 */
	virtual bool         Init(VideoBuffer **bufs);
	//! @brief thread listen for freeList of TeeOut and usedList of TeeIn
	virtual void         threadRun();
	//! @brief acquire the output
	BufferList           *getTeeOutput(int idx);
	//! @brief release the output
	void                 putTeeOutput(BufferList *output);
};


#endif	//__TEEINLIST_H__

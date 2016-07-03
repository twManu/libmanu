#ifndef	__CNT_BUFFER_HPP__
#define	__CNT_BUFFER_HPP__

//for lock
#include "BaseLock.h"

/*
 * A client is told of buffer and size available. And the client
 * returns size processed
 */
extern "C" typedef unsigned int (cb_getData)(unsigned char *buf, unsigned int sz);

/*
 * Continuous buffer
 * It accepts data accommodates between writeOffset and end of buffer,
 * otherwise it copies non-processed data to front before insert into buffer.
 *
 * Multi-thread might fail
 */
class cntBuffer {
protected:
	unsigned int                   m_bufSz;
	unsigned char                  *m_buf;
	int                            m_workSpace;           //need this space to work
	int                            m_writeOffset;         //offset to buffer
	int                            m_readOffset;          //offset to buffer
	int                            m_dataAvailable;       //unprocessed data size
	int                            m_freeAtEnd;           //free space at the end of buffer
	BaseLock                       *m_lock;


public:
	//lock is supposedly init'ed
	cntBuffer(BaseLock *lock, unsigned int bufSz, int wkSpace=1);
	~cntBuffer();
	bool                           addData(unsigned char *buf, unsigned int sz);
	//caller process data in cb, an internal lock is hold during callback
	void                           getData(cb_getData cb);
	int                            getAvailable();
};

#endif	//__CNT_BUFFER_HPP__
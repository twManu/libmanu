#ifndef	__BASESTREAM_H__
#define	__BASESTREAM_H__

#include "libcommon.h"

/*! @class
 * It's blocked or nonblocked by implementation. Don't return EOS in case of eagain
 */
class BaseStream
{
public:
	static const int EOS = 0;
	BaseStream() {}
	~BaseStream() {}
	//0 means fail
	virtual int open() = 0;
	//0 means fail
	virtual void close() = 0;
	
	/*! @detail
	 *  Read data from stream into buffer with size as long as no error happened.
	 *  May not implemented in write case.
	 *  @param buffer [IN] - data to read from stream
	 *  @param size [IN] - size request to read
	 *  @return 0/EOS - end of stream
	 *           <1 - error
	 *           otherwise - size actually read. 
	 */
	virtual int read(void *buffer, int size) { return 0; }
	
	/*! @detail
	 *  Write data from buffer to stream with size as long as no error happened.
	 *  May not implemented in read case.
	 *  @param buffer [IN] - data to write to stream
	 *  @param size [IN] - size request to write
	 *  @return <1 - error
	 *           otherwise - size actually write.
	 */
	virtual int write(const void *buffer, int size) { return 0; }
};

#endif	//__BASESTREAM_H__
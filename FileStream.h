#ifndef	__FILESTREAM_H__
#define __FILESTREAM_H__

#include <unistd.h>
#include "BaseStream.h"

/*! @class
 * File is created when WRONLY set. Previous file content is lost.
 */
class FileStream : public BaseStream
{
public:
	static const int MAX_FNAME_SIZE = 255;
	static const int BM_RDONLY = 1;
	static const int BM_WRONLY = 2;

	FileStream(const char *fname, int mode);
	virtual ~FileStream();
	virtual int open();
	virtual void close();

	/*! @detail
	 *  Read data from stream into buffer with size as long as no error happened.
	 *  May not implemented in write case.
	 *  @param buffer [IN] - data to read from stream
	 *  @param size [IN] - size request to read
	 *  @return 0/EOS - end of stream
	 *           <1 - error
	 *           otherwise - size actually read. 
	 */
	virtual int read(void *buffer, int size);

	/*! @detail
	 *  Write data from buffer to stream with size as long as no error happened.
	 *  May not implemented in read case.
	 *  @param buffer [IN] - data to write to stream
	 *  @param size [IN] - size request to write
	 *  @return <1 - error
	 *           otherwise - size actually write.
	 */
	virtual int write(const void *buffer, int size);

protected:
	char                 m_fname[MAX_FNAME_SIZE+1];
	int                  m_fd;
	int                  m_mode;	//BM_RDONLY, BM_WRONLY, both means RW
};

#endif	//__FILESTREAM_H__
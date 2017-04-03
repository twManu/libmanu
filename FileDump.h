#ifndef	__FILEDUMP_H__
#define __FILEDUMP_H__

#include "FileStream.h"

/*! @class
 * File is created when WRONLY set. Previous file content is lost.
 * File hader is
 *  hdrSize - 4byte
 *  maxDataSize [0] - 4byte
 *  data size [1] - 4byte
 *    :
 *  data size [((size-8)/4)-1]
 * Then data [0] follows
 *
 * Usage:
 *	dd=constructor()
 *	dd.open()    - don't do further if failure
 *	dd.write()/dd.read   - n times
 *	dd.close()      - optional
 *	dd.destructor()
 */
class FileDump : public FileStream
{
public:
	FileDump(const char *fname, int mode, int count=0);     //mode is either BM_RDONLY or BM_WRONLY, count is ignore for read
	virtual ~FileDump();
	virtual int open();
	virtual void close();
	//!@brief, return failure if 0
	virtual int writeData(const void *data, int size);
	//!@brief, return size read, failure if -1
	virtual int readData(void *data);
	void readHdr(int &hdrSize, int &maxDataSize, int &count);


protected:
	int                  m_hdrSize;             //size of header
	int                  m_maxDataSize;         //max data size
	int                  m_count;               //number of data count
	int                  m_index;               //current index
	unsigned int         *m_sizeTable;          //size of each data chunk
};


#endif	//__FILESDUMP_H__

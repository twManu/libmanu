#include "FileDump.h"
#include <stdlib.h>
#include <string.h>

FileDump::FileDump(const char *fname, int mode, int count)
	: FileStream(fname, mode)
	, m_hdrSize(0)
	, m_maxDataSize(0)
	, m_count(count)
	, m_index(0)
	, m_sizeTable(NULL)
{
}


FileDump::~FileDump()
{
	close();
}


int FileDump::writeData(const void *data, int size)
{
	int sz;
	if( BM_WRONLY!=m_mode ) return 0;
	if( size<=0 ) return 0;

	if( m_index>=m_count ) {
		ERROR("data overflow\n");
		return 0;
	}
	sz = FileStream::write(data, size);
	if( sz ) m_sizeTable[m_index++] = sz;
	if( sz>m_maxDataSize ) m_maxDataSize = sz;
	if( sz!=size )
		ERROR("request %d but only %d written\n", size, sz);
	return sz;
}


int FileDump::readData(void *data)
{
	int reqSz, sz;
	if( BM_RDONLY!=m_mode ) return -1;

	if( m_index>=m_count ) {
		ERROR("data overflow\n");
		return -1;
	}
	reqSz = m_sizeTable[m_index++];
	if( reqSz>m_maxDataSize ) {
		ERROR("request larger than limit\n");
		return -1;
	}
	//could be 0
	sz = FileStream::read(data, reqSz);
	if( sz!=reqSz )
		ERROR("request %d but only %d read\n", reqSz, sz);
	return sz;
}


/*
 * Ret:
 * 0 - failure
 * WO - fd
 * RO - max data size
 */
int FileDump::open()
{
	int tmpSz;

	if( BM_WRONLY!=m_mode && BM_RDONLY==m_mode ) {
		ERROR("either WO nor RD\n");
		return 0;
	}

	if( !FileStream::open() ) return 0;
	if( BM_WRONLY==m_mode ) {
		tmpSz = m_count*sizeof(m_sizeTable[0]);
		m_hdrSize = sizeof(m_hdrSize) +
			sizeof(m_maxDataSize) +
			tmpSz;
		//prepare for data write
		lseek(m_fd, m_hdrSize, SEEK_SET);
		m_sizeTable = (unsigned int *)malloc(tmpSz);
		if( !m_sizeTable ) {
			ERROR("fail to allocate sizeTable\n");
			goto failure;
		}
		memset(m_sizeTable, 0, tmpSz);
	} else {
		if( sizeof(m_hdrSize)!=FileStream::read(&m_hdrSize, sizeof(m_hdrSize)) ) {
			ERROR("fail to read header\n");
			goto failure;
		}
		if( sizeof(m_maxDataSize)!=FileStream::read(&m_maxDataSize, sizeof(m_maxDataSize)) ) {
			ERROR("fail to read max data size\n");
			goto failure;
		}
		m_count = m_hdrSize - sizeof(m_hdrSize) - sizeof(m_maxDataSize);
		m_sizeTable = (unsigned int *)malloc(m_count);
		if( !m_sizeTable ) {
			ERROR("fail to alloc sizeTable\n");
			goto failure;
		}
		if( m_count!=FileStream::read(m_sizeTable, m_count) ) {
			ERROR("fail to load sizeTable\n");
			free(m_sizeTable);
			m_sizeTable = NULL;
			goto failure;			
		}
		m_count /= sizeof(m_sizeTable[0]);
		return m_maxDataSize;
	}

	return m_fd;
failure:
	m_mode = 0;
	FileStream::close();
	return 0;
}


void FileDump::close()
{
	if( m_sizeTable ) {
		if( BM_WRONLY==m_mode ) {
			int sz = m_count * sizeof(m_sizeTable[0]);
			lseek(m_fd, 0, SEEK_SET);
			if( sizeof(m_hdrSize)!=FileStream::write(&m_hdrSize, sizeof(m_hdrSize)) )
				ERROR("fail to write hdrSize\n");
			if( sizeof(m_maxDataSize)!=FileStream::write(&m_maxDataSize, sizeof(m_maxDataSize)) )
				ERROR("fail to write maxDataSize\n");
			if( sz!=FileStream::write(m_sizeTable, sz) )
				ERROR("fail to write sizeTable\n");
		}
		free(m_sizeTable);
		m_sizeTable = NULL;
	}
	FileStream::close();
}


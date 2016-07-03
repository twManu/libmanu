#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "FileStream.h"

FileStream::FileStream(const char *fname, int mode)
	: m_fd( 0 )
{
	if( !fname ) {
		ERROR("missing file name\n");
		return;
	}
	if( (int)(strlen(fname))>MAX_FNAME_SIZE ) {
		ERROR("file name %s exeeds max. size\n", fname);
		return;
	}
	if( !mode ) {
		ERROR("missing file mode\n");
		return;
	}
	strcpy(m_fname, fname);
	m_mode = mode;
}


FileStream::~FileStream()
{
	close();
}


int FileStream::open()
{
	if( m_fd ) {
		ERROR("file already opened\n");
		return 0;
	}
	if( BM_RDONLY==m_mode ) m_fd = ::open(m_fname, O_RDONLY);
	else if( BM_WRONLY==m_mode ) m_fd = ::open(m_fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	else m_fd = ::open(m_fname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	
	if( m_fd<0 ) m_fd = 0;
	return m_fd;
}


void FileStream::close()
{
	int tmpFd = m_fd;
	m_fd = 0;
	if( tmpFd ) ::close(tmpFd);
}


int FileStream::read(unsigned char *buffer, int size)
{
	int totalRead, curRead, rest;
	unsigned char *ptr = buffer;

	if( !(m_mode&BM_RDONLY) ) {
		ERROR("read not allowed\n");
		return -1;
	}
	if( !ptr || !m_fd ) {
		ERROR("read to NULL address\n");
		return -1;
	}
	if( !size ) {
		ERROR("read zero size\n");	//0 is EOS
		return -1;
	}
	for( totalRead=0, rest=size; rest; ) {
		curRead = ::read(m_fd, ptr, rest);
		if( curRead<0 ) break;
		totalRead += curRead;	//update read size as long as no error
		if( !curRead ) break;
		ptr += curRead;
		rest -= curRead;
	}

	if( 0==curRead ) {	//EOS happen
		if( 0==totalRead ) return EOS;   //EOS
		return totalRead;                //ok
	}
	if( curRead<0 ) return curRead;          //error
	return totalRead;                        //ok
}


int FileStream::write(const unsigned char *buffer, int size)
{
	int totoalWrite, curWrite, rest;
	unsigned char *ptr = (unsigned char *)buffer;

	if( !(m_mode&BM_WRONLY) ) {
		ERROR("write not allowed\n");
		return -1;
	}
	if( !ptr || !m_fd ) {
		ERROR("write to NULL address\n");
		return -1;
	}
	for( curWrite=totoalWrite=0, rest=size; rest; ) {
		curWrite = ::write(m_fd, ptr, rest);
		if( curWrite<0 ) break;
		totoalWrite += curWrite;	//update read size as long as no error
		ptr += curWrite;
		rest -= curWrite;
	}

	if( curWrite<0 ) return -1;             //error
	return totoalWrite;                     //ok
}

#ifndef	__FILE_WRITER_LNX_THREAD_H__
#define	__FILE_WRITER_LNX_THREAD_H__

#include "FileStream.h"
#include "OSLinux.h"
#include "TimedBufferListLinux.h"

/*! @class
 */
class FileWriterLnxThread : public LinuxThread
protected:
	FileStream               *m_fileStrea;
	virtual void             ThreadMain();

public:
	FileWriterLnxThread(char *fname, buffer *);
	~FileWriterLnxThread();
};

#endif	//__FILE_WRITER_LNX_THREAD_H__
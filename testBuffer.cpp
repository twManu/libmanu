#include "TimedBufferListLinux.h"
#include "OSLinux.h"
#include <unistd.h>

	#define BUF_COUNT	32
	#define BUF_SIZE	64
	#define MAX_WAIT_MS	300

class consumer : public LinuxThread
{
protected:
	TimedBufferListLinux *m_bufList;
	void                 ThreadMain();

public:
	consumer(TimedBufferListLinux *bufList, BaseLock *lock=NULL) : LinuxThread(lock), m_bufList(bufList) {}
	~consumer() {}
};


void consumer::ThreadMain()
{
	bool timeout;
	Buffer *tmpBuf;
	int curIndex = -1, tmp;

	if( m_bufList ) {
		while( !m_toStop ) {
			tmpBuf = m_bufList->GetUsed(MAX_WAIT_MS, &timeout);
			if( !tmpBuf ) {
				ERROR("fail to get a buffer within %d ms\n", MAX_WAIT_MS);
				continue;
			}
			tmp = *((int *)tmpBuf->GetData());
			if( curIndex>0 ) {
				if( (curIndex-1)!=tmp ) ERROR("counter skipped %d becomes %d\n", curIndex-1, tmp);
			}
			curIndex = tmp;
			ERROR("Index %d got, there is %d buffer in usedQ\n", curIndex, m_bufList->GetUsedCount());
			m_bufList->PutFree(tmpBuf);
			usleep(15*1000);
		}
	}
}


int main()
{
	TimedBufferListLinux  *bufList;
	Buffer *tmpBuf;
	LinuxLock dummyLock;	//not generic
	consumer *chThread;
	int *ptr, i;
	bool timeOut;

	dummyLock.Init();
	bufList = new TimedBufferListLinux();
	if( !bufList->Init(BUF_COUNT, BUF_SIZE, &dummyLock, &dummyLock) ) {
		ERROR("fail init buffer list\n");
		return -1;
	}
	if( bufList->GetBufBlockType()<BufferList::Q_BLOCK_TIMED ) {
		ERROR("No wait mode support\n");
		goto freeBuf;
	}
	dummyLock.Acquire();
	chThread = new consumer(bufList, &dummyLock);
	if( !chThread ) {
		ERROR("fail create thread\n");
		goto freeBuf;
	}
	if( !chThread->Start() ) goto freeTh;
#define	DATA_COUNT	30
	i = DATA_COUNT;
	do {
		tmpBuf = bufList->GetFree(MAX_WAIT_MS, &timeOut);
		if( !tmpBuf ) {
			ERROR("fail to get a buffer within %d ms\n", MAX_WAIT_MS);
			break;
		}
		ptr = (int *)tmpBuf->GetData();
		*ptr = i;
		bufList->PutUsed(tmpBuf, sizeof(int));
		ERROR("Inserting %d\n", i);
		if( (DATA_COUNT-5+1)==i ) {
			ERROR("Release comsumer thread now...\n");
			dummyLock.Release();
		}
		usleep(5*1000);
	} while( --i );

	sleep(1);

freeTh:
	delete chThread;
freeBuf:
	ERROR("child thread deleted\n");
	delete bufList;

	return 0;
}

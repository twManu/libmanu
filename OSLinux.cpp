#include "OSLinux.h"
#include <sys/types.h>
//#include <unistd.h>

//lock implementation
bool LinuxLock::Init()
{
	pthread_mutex_init(&m_mutex, NULL);
	m_lockObj = &m_mutex;
	return true;
}


LinuxLock::~LinuxLock()
{
	if( m_lockObj )
		pthread_mutex_destroy(&m_mutex);
}


void LinuxLock::Acquire()
{
	if( m_lockObj )
		pthread_mutex_lock((pthread_mutex_t *)m_lockObj);
}


void LinuxLock::Release()
{
	if( m_lockObj )
		pthread_mutex_unlock((pthread_mutex_t *)m_lockObj);
}


LinuxThread::LinuxThread(BaseLock *lock)
	: BaseThread(lock)
	, m_thread(0)
{
}


LinuxThread::~LinuxThread()
{
	DBG_THREAD("In Destructor of LinuxThread\n");
	Destroy();
}


void LinuxThread::Destroy()
{
	pthread_t tmpThread = m_thread;
	DBG_THREAD("In Destroy of LinuxThread\n");
	m_thread = 0;
	if( tmpThread ) {
		Stop();
		pthread_join(tmpThread, NULL);
		m_tid = 0;
	}
}


void LinuxThread::Proc()
{
	m_tid = pthread_self();	//it actually m_thread
	DBG_THREAD("Entering class defined thread main\n");
	ThreadMain();
	pthread_exit(NULL);
}



int LinuxThread::Start()
{
	pthread_attr_t attr;

	if( m_thread ) {
		ERROR("Thread already created\n");
		return 0;
	}
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if( pthread_create(&m_thread, &attr, (void *(*)(void *))ThreadEntry, this) ) {
		ERROR("Fial to create thread\n");
		m_thread = 0;
		return 0;
	}

	return 1;
}

/*
 * Given value(mask), return the position of first bit set
 *
 * In  : mask - bit mask to test with
 * Out : len - number of continuous bit since(including) first bit
 * Ret : -1 - mask is 0
 *       otherwise - 0 based index
 */
inline int posOfU32Mask(unsigned int mask, int *len)
{
	int pos = -1;  //error
	for( ; mask; mask>>=1 ) {
		++pos;
		if( mask&1 ) {
			if( len ) {
				int tmpLen;
				for( tmpLen=0; mask&1; mask>>=1, ++tmpLen );
				*len = tmpLen;
			}
			break;
		}
	}
	return pos;
}


/*
 * Build a mask from bit position 'start' to 'start+length-1' after left shifted
 * 
 * In  : start - 0 based bit position
 *       length - length of mask
 * Ret : 0 - of length =0
 *       otherwise - left shifted mask
 */
inline unsigned int makeU32Mask(unsigned char start, unsigned char length)
{
	unsigned int mask;
	for( mask=0; length; mask<<=1, mask|=1, --length);
	mask <<= start;

	return mask;
}


/*
 * Given data, extra the value from bit 'start' to 'start+length-1' after right shifted
 *
 * In  : start - 0 based position of the first bit
 *       length (>=1) - the bits represents the value 
 * Ret : shifted value or -1 (error)
 */
unsigned int getBit(unsigned int data, unsigned char start, unsigned char length)
{
	unsigned int result = -1;
	unsigned char mask = makeU32Mask(start, length);
	if( !mask ) {
		ERROR("wrong getBit param\n");
	} else {
		result &= mask;
		result >>= start;
	}

	return result;
}


unsigned int LinuxTimeSec::getCurTime()
{
	return time(NULL);
}


LinuxTimeSec::LinuxTimeSec(int periodSec)
	: TimeSec(periodSec)
{
}

LinuxTimeSec::~LinuxTimeSec()
{
}

#include "TeeInList.h"
#include "TeeOutList.h"
#include <string.h>

//todo
#define	ERROR	printf
#include <stdio.h>
#include <errno.h>

TeeInList::TeeInList(unsigned int teeCount)
	: BufferList()
	, m_teeCount(teeCount)
	, m_activeTee(0)
	, m_outputs(NULL)
	, m_stopThread(false)
	, m_thread(0)
{
}


TeeInList::~TeeInList()
{
	Clean();
}


//not supported
bool TeeInList::Init(unsigned int count, unsigned int size)
{
	return false;
}

static void *cThreadEntry(void *cntx)
{
	TeeInList *inList = (TeeInList *)cntx;
	inList->threadRun();
	return NULL;
}


/*
 * for all output
 * 1. delete free clone buffer
 * 2. maintain ref count of source buffer
 */
void TeeInList::cleanOutFree()
{
	int i;
	TeeBuffer *cloneBuf;

	for(i=0; i<m_teeCount; ++i) {          //each output side
		while( NULL!=(cloneBuf=static_cast <TeeBuffer *> (m_outputs[i]->GetFree(0))) ) {
			--cloneBuf->getSrcBuffer().m_cloneCount;
			delete cloneBuf;     //todo
		}
	}
}


void TeeInList::popLinger(bool force)
{
	VideoBuffer *srcBuf;
	
	while( m_lingerList.size() ) {
		srcBuf = *(m_lingerList.begin());
		//stop on first non-zero if not force
		if(srcBuf->m_cloneCount) { 
			if( !force ) break;
			else ERROR("cloneCount not zero\n");
		}
		m_lingerList.pop_front();
		srcBuf->SetUsedSize(0); 
		PutFree(srcBuf);
	}
}

void TeeInList::threadRun()
{
	int i;
	TeeBuffer *cloneBuf;
	VideoBuffer *srcBuf;

	while( !m_stopThread ) {
		cleanOutFree();
		popLinger(0==m_activeTee);               //clean top buffers w/ ref=0
		srcBuf = static_cast <VideoBuffer*> (GetUsed(-1));    //block
		if( NULL==srcBuf ) continue;
		if( m_stopThread ) {
			PutFree(srcBuf);
			break;
		}
		int cloneCount = 0, mask=1;
		/* 
		   clone each activated
		   hold lock to avoid race w/ putTeeOutput
		 */
		pthread_mutex_lock(&m_usedMutex);
		if( m_activeTee ) {
			for( i=0; i<m_teeCount; ++i, mask<<=1 ) {
				if( m_activeTee & mask ) {
					cloneBuf = new TeeBuffer(*srcBuf);
					if( NULL==cloneBuf ) {
						ERROR("skip cloning since fail to allocate buffer\n");
						continue;
					}
					++cloneCount;
					m_outputs[i]->PutUsed(cloneBuf);
				}
			}
		}
		pthread_mutex_unlock(&m_usedMutex);
		//clone or send it back
		if( cloneCount ) {
			srcBuf->m_cloneCount = cloneCount;
			m_lingerList.push_back(srcBuf);
		} else {
			//avoid out of order todo
			PutFree(srcBuf);
		}		
	}
	/*
	 * expect down stream has returned all clone
	 * clean free and then linger
	 */
	cleanOutFree();
	popLinger(true);
	pthread_exit(NULL);
}


bool TeeInList::Init(VideoBuffer **bufs)
{
	if( m_outputs || m_thread ) return false;
	//if needed, allocate space for buffer parking
	if( bufs ) {
		if( !BufferList::Init((Buffer **)bufs) )
			goto cleanup;
	}

	//allocate output holder
	if( m_teeCount ) {
		m_outputs = new TeeOutList* [m_teeCount];
		if( NULL==m_outputs ) goto cleanup;
	}

	//allocate outputs
	for(int i=0; i<m_teeCount; ++i ) {
		m_outputs[i] = new TeeOutList(*this);
		if( !m_outputs[i] ) {
			ERROR("failed to alloc output lists\n");
			goto cleanup;
		}
	}
	//create threads
	pthread_attr_t attr;
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if( pthread_create(&m_thread, &attr, (void *(*)(void *))cThreadEntry, this) ) {
		ERROR("Fial to create thread\n");
		m_thread = 0;
		goto cleanup;
	}

	return true;

cleanup:
	Clean();
	return false;
}


/*
    Graph builder needs to make sure no down stream user
	1. stop thread
	2. free TeeOutList and holder
	3. clear lingers
	4. parent clean
 */
void TeeInList::Clean()
{
	int i;
	
	m_buffers = NULL;
	pthread_t tmpThread = m_thread;

	m_thread = 0;
	pthread_mutex_lock(&m_usedMutex);
	if( m_activeTee ) ERROR("tee is busy %x\n", m_activeTee);
	m_stopThread = true;
	pthread_mutex_unlock(&m_usedMutex);
	//stop thread
	if( tmpThread ) {
		pthread_cond_broadcast(&m_usedCond);
		pthread_join(tmpThread, NULL);
	}
	//delete output lists and holder
	if( m_outputs ) {
		for(i=0; i<m_teeCount; ++i)
			if( m_outputs[i] )
				delete m_outputs[i];
		delete [] m_outputs;
	}
}


void TeeInList::getUsedLock(pthread_mutex_t **srcMutex, pthread_cond_t **srcCond)
{
	if( srcMutex ) *srcMutex = &m_usedMutex;
	if( srcCond ) *srcCond = &m_usedCond; 
}


BufferList* TeeInList::getTeeOutput(int idx)
{
	BufferList *outList = NULL;

	if( idx<m_teeCount && NULL!=m_outputs ) {
		int mask = 1<<idx;
		pthread_mutex_lock(&m_usedMutex);
		if( !(m_activeTee & mask) ) {
			//not yet occupied
			m_activeTee |= mask;
			outList = m_outputs[idx];
		}
		pthread_mutex_unlock(&m_usedMutex);
	}
	return outList;
}


void TeeInList::putTeeOutput(BufferList *outList)
{
	int i, mask;
	int thisRelease = 0;

	//maintain active mask
	pthread_mutex_lock(&m_usedMutex);
	if( m_outputs ) {
		for(i=0, mask=1; i<m_teeCount; ++i, mask<<=1) {
			if( m_outputs[i]==static_cast <TeeOutList *> (outList) ) {
				thisRelease = (m_activeTee & mask);
				m_activeTee &= ~mask;
				break;
			}
		}
	}
	pthread_mutex_unlock(&m_usedMutex);
	//move all used to free, they are handled and freed by thread
	if( thisRelease ) {
		Buffer *buf;
		while( NULL!=(buf=outList->GetUsed(0)) )
			outList->PutFree(buf);
	}
}
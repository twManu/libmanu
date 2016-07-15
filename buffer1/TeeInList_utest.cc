#include "gtest/gtest.h"
#include <pthread.h>
#include "TeeInList.h"

#define	BUF_COUNT	10

class BLTest : public testing::Test {
protected:
	int                  m_teeCount;
	TeeInList            *m_list;
	VideoBuffer          *m_bufs[BUF_COUNT+1];      //may not used in certain test
	BufferList           **m_tee;                   //hold output tees
	//ret ： successful or failure
	virtual bool         newList(int teeCount) {
		m_teeCount = teeCount;
		if( m_list ) return false;
		m_list = new TeeInList(teeCount);
		if( NULL==m_list ) return false;
		
		m_tee = new BufferList *[teeCount];
		return NULL!=m_tee;
	}

public:
	BLTest() : m_list(NULL), m_tee(NULL) {}
	~BLTest() {}

	virtual void SetUp() {
		
		for( int i=0; i<BUF_COUNT; ++i) {
			m_bufs[i] = new VideoBuffer(1920, 1080, 0);
			ASSERT_TRUE( NULL!=m_bufs[i] ) << "Fails to allocate buffer\n";
		}
		m_bufs[BUF_COUNT] = NULL;    //ended with NULL
	}

	virtual void TearDown() {
		int i;
		
		if( m_list ) {
			if( m_tee ) {
				for( i=0; i<m_teeCount; ++i )
					m_list->putTeeOutput(m_tee[i]);
				delete [] m_tee;
				m_tee = NULL;
			}

			delete m_list;
			m_list = NULL;
		}
		for( i=0; i<BUF_COUNT;++i ) {
			if( NULL==m_bufs[i] ) break;
			delete m_bufs[i];
			m_bufs[i] = NULL;
		}
	}
};


//only test userAlloc, parked or passthru case
TEST_F(BLTest, bufferRetAsTeeNoOut) {
	VideoBuffer *thisBuf;
	ASSERT_TRUE(newList(0)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	thisBuf = (VideoBuffer *) m_list->GetFree(0);
	ASSERT_TRUE(thisBuf!=NULL) << "Tee should have one free buffer ready";
	/* reflect used buffer count */
	m_list->PutUsed(thisBuf);
	usleep(100*1000);
	ASSERT_EQ((unsigned int)0, m_list->GetUsedCount()) <<"Tee should immediate return used to free since no outputs";
}


TEST_F(BLTest, getTeeOut) {
	ASSERT_TRUE(newList(1)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	usleep(100*1000);
	/* get non-existing teeouput should be handled */
	m_tee[0] = m_list->getTeeOutput(1);
	ASSERT_TRUE(NULL==m_tee[0]) << "Tee index out of range should return NULL";
	m_tee[0] = m_list->getTeeOutput(0);
	ASSERT_TRUE(NULL!=m_tee[0]) << "Tee index within range shouldn't return NULL";
}


TEST_F(BLTest, teeRegot) {
	ASSERT_TRUE(newList(2)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	usleep(100*1000);
	m_tee[0] = m_list->getTeeOutput(0);
	ASSERT_TRUE(NULL!=m_tee[0]) << "Tee index within range shouldn't return NULL";
	m_tee[1] = m_list->getTeeOutput(1);
	ASSERT_TRUE(NULL!=m_tee[1]) << "Tee index within range shouldn't return NULL";
	/* put and get output again */
	m_list->putTeeOutput(m_tee[1]);
	m_tee[1] = m_list->getTeeOutput(1);
	ASSERT_TRUE(NULL!=m_tee[1]) << "Tee re-get shouldn't return NULL";
}

TEST_F(BLTest, cloneCheck) {
	Buffer *buf;
	int i;
	ASSERT_TRUE(newList(3)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	usleep(100*1000);
	m_tee[0] = m_list->getTeeOutput(0);
	ASSERT_TRUE(NULL!=m_tee[0]) << "Tee index within range shouldn't return NULL";
	m_tee[1] = m_list->getTeeOutput(1);
	ASSERT_TRUE(NULL!=m_tee[1]) << "Tee index within range shouldn't return NULL";
	m_tee[2] = m_list->getTeeOutput(2);
	ASSERT_TRUE(NULL!=m_tee[2]) << "Tee index within range shouldn't return NULL";
	/*
	 * put to in side, check tee side
	 */
	for( i=0; i<BUF_COUNT;++i ) buf = m_list->GetFree(0);          //pop all free
	ASSERT_TRUE(NULL!=buf)<< "There should be buffers available";
	ASSERT_EQ(NULL, m_list->GetFree(0)) << "There should be no buffers in free side";
	m_list->PutUsed(buf);            //insert used buffer
	usleep(100*1000);
	//   check used and free count in input side
	ASSERT_EQ((unsigned int)0, m_list->GetUsedCount()) <<"Tee should immediate processing clone when there is output";
	for( i=0; i<m_teeCount; i++ ) {
		ASSERT_TRUE(NULL!=m_tee[i]->GetUsed(0, false)) << "There should be one clone in each tee" << i;
	}
	m_list->putTeeOutput(m_tee[2]);
	ASSERT_EQ((unsigned int)0, m_list->GetUsedCount()) <<"Tee should keep original buffer in used since there are outputs";
	ASSERT_EQ(NULL, m_list->GetFree(0)) << "There should be no buffers in free side";
	//leave buffers in used list of clone see if destruct lock
}

TEST_F(BLTest, cloneFree) {
	Buffer *buf;
	int i;
	ASSERT_TRUE(newList(3)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	usleep(100*1000);
	m_tee[0] = m_list->getTeeOutput(0);
	ASSERT_TRUE(NULL!=m_tee[0]) << "Tee index within range shouldn't return NULL";
	m_tee[1] = m_list->getTeeOutput(1);
	ASSERT_TRUE(NULL!=m_tee[1]) << "Tee index within range shouldn't return NULL";
	m_tee[2] = m_list->getTeeOutput(2);
	ASSERT_TRUE(NULL!=m_tee[2]) << "Tee index within range shouldn't return NULL";
	/*
	 * each tee return, check in side
	 */
	for( i=0; i<BUF_COUNT;++i ) buf = m_list->GetFree(0);          //pop all free
	m_list->PutUsed(buf);            //insert used buffer
	usleep(100*1000);
	//   tee return and check free of input side
	for( i=0; i<m_teeCount; ++i) {
		ASSERT_TRUE(NULL!=(buf=m_tee[i]->GetUsed(0))) << "There should be one clone in tee" <<i;
		m_tee[i]->PutFree(buf);
	}
	usleep(100*1000);
	ASSERT_TRUE(NULL!=(buf=m_list->GetFree(0))) << "There should be one free in input side";
	m_list->PutFree(buf);
}


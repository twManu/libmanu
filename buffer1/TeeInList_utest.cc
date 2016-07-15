#include "gtest/gtest.h"
#include <pthread.h>
#include "TeeInList.h"

#define	BUF_COUNT	10
class BLTest : public testing::Test {
protected:
	TeeInList            *m_list;
	VideoBuffer          *m_bufs[BUF_COUNT+1];      //may not used in certain test
	//ret ： successful or failure
	virtual bool         newList(int teeCount) {
		if( m_list ) return false;
		m_list = new TeeInList(teeCount);
		return NULL!=m_list;
	}
public:
	BLTest() : m_list(NULL) {}
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
TEST_F(BLTest, tee0) {
	VideoBuffer *thisBuf;

	ASSERT_TRUE(newList(0)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	thisBuf = (VideoBuffer *) m_list->GetFree(0);
	ASSERT_TRUE(thisBuf!=NULL) << "Tee should have one free buffer ready";
	m_list->PutUsed(thisBuf);
	usleep(100*1000);
	ASSERT_EQ((unsigned int)0, m_list->GetUsedCount()) <<"Tee should immediate return used to free since no outputs";
}


TEST_F(BLTest, teeGet) {
	BufferList *tee;

	ASSERT_TRUE(newList(1)) << "Failed to allocate tee object";
	/* buffers park to list */
	ASSERT_TRUE(m_list->Init(m_bufs)) << "\tTee init should be successful";
	usleep(100*1000);
	tee = m_list->getTeeOutput(1);
	ASSERT_TRUE(NULL==tee) << "Tee index out of range should return NULL";
	tee = m_list->getTeeOutput(0);
	ASSERT_TRUE(NULL!=tee) << "Tee index within range shouldn't return NULL";
	m_list->putTeeOutput(tee);
}

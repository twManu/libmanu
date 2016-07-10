#include "gtest/gtest.h"
#include <pthread.h>
#include "BufferList.h"

#define	BUF_COUNT	10
class BLTest : public BufferList, public testing::Test {
protected:
	BufferList           *m_bufList;

public:
	BLTest() {}
	~BLTest() {}
	virtual void SetUp() {
		m_bufList = new BufferList(NULL, NULL);
	}

	virtual void TearDown() {
		if( m_bufList ) {
			delete m_bufList;
			m_bufList = NULL;
		}
	}
};



TEST_F(BLTest, selfAlloc) {
	int i;
	Buffer buffer;
	Buffer *bufs[2] = { &buffer, NULL };
	
	ASSERT_TRUE(NULL!=m_bufList) << "\tFail to alloc buffer list";

	ASSERT_TRUE(m_bufList->Init(BUF_COUNT, 1024)) << "\tFail to init buffer list";
	ASSERT_TRUE(0==m_bufList->GetUsedCount()) << "\tused count should be 0";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(false)) << "\tused list should be empty";
	for( i=0; i<BUF_COUNT; ++i )
		ASSERT_TRUE(NULL!=m_bufList->GetFree()) << "\tbuffer " << i << " should be in free list";
	ASSERT_FALSE(m_bufList->Init(bufs)) << "selfALloc list cannot be user configured again";
}


TEST_F(BLTest, userAlloc) {
	int i;
	Buffer *bufs[BUF_COUNT+1] = { NULL };
	
	ASSERT_TRUE(NULL!=m_bufList) << "\tFail to alloc buffer list";

	for( i=0; i<=BUF_COUNT; ++i ) {
		bufs[i] = new Buffer(false);
		ASSERT_TRUE( NULL!=bufs[i] ) << "Fails to allocate buffer\n";
		goto cleanup;
		
		if( BUF_COUNT==i ) {
			bufs[i] = NULL;              //end
			ASSERT_FALSE(m_bufList->Init(bufs)) << "\tShould be init'ed twice";
		}
	}
	for( i=0; i<BUF_COUNT; ++i )
		ASSERT_TRUE(NULL!=m_bufList->GetFree()) << "\tbuffer " << i << " should be in free list";

cleanup:
	for( i=0; i<BUF_COUNT; ++i )
		if( bufs[i] ) delete bufs[i];
	return;
	
}

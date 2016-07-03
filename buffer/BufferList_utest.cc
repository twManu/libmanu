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
	
	ASSERT_TRUE(NULL!=m_bufList) << "\tFail to alloc buffer list";

	ASSERT_TRUE(m_bufList->Init(BUF_COUNT, 1024)) << "\tFail to init buffer list";
	ASSERT_TRUE(0==m_bufList->GetUsedCount()) << "\tused count should be 0";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(false)) << "\tused list should be empty";
	for( i=0; i<BUF_COUNT; ++i )
		ASSERT_TRUE(NULL!=m_bufList->GetFree()) << "\tbuffer " << i << " should be in free list";
	ASSERT_FALSE(m_bufList->Init(BUF_COUNT, &buffer)) << "selfALloc list cannot be user configured again";
}


TEST_F(BLTest, userAlloc) {
	int i;
	Buffer *buffer;
	
	ASSERT_TRUE(NULL!=m_bufList) << "\tFail to alloc buffer list";

	for( i=0; i<=BUF_COUNT; ++i ) {
		buffer = new Buffer(false);
		if( !buffer ) printf("fails to allocate buffer\n");
		if( BUF_COUNT==i ) {
			ASSERT_FALSE(m_bufList->Init(BUF_COUNT, buffer)) << "\tCannot allocate more then configured buffers (" << BUF_COUNT << ")";
			delete buffer;
		} else ASSERT_TRUE(m_bufList->Init(BUF_COUNT, buffer)) << "\tFail to init buffer list";
	}
	ASSERT_FALSE(m_bufList->Init(BUF_COUNT, 1024)) << "\tuserALloc list connt be self configured again";
	for( i=0; i<BUF_COUNT; ++i )
		ASSERT_TRUE(NULL!=m_bufList->GetFree()) << "\tbuffer " << i << " should be in free list";
	
}

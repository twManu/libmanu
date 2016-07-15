#include "gtest/gtest.h"
#include <pthread.h>
#include "BufferList.h"

#define	BUF_COUNT	10
class BLTest : public BufferList, public testing::Test {
protected:
	BufferList           *m_bufList;
	Buffer               *m_bufs[BUF_COUNT+1];      //may not used in certain test

public:
	BLTest() {}
	~BLTest() {}
	virtual void SetUp() {
		m_bufList = new BufferList();
		ASSERT_TRUE(NULL!=m_bufList) << "Setup fails to allocate BufferList\n";
		for( int i=0; i<BUF_COUNT; ++i) {
			m_bufs[i] = new Buffer(false);
			ASSERT_TRUE( NULL!=m_bufs[i] ) << "Fails to allocate buffer\n";
		}
		m_bufs[BUF_COUNT] = NULL;    //ended with NULL
	}

	virtual void TearDown() {
		int i;

		if( m_bufList ) {
			delete m_bufList;
			m_bufList = NULL;
		}

		for( i=0; i<BUF_COUNT;++i ) {
			if( NULL==m_bufs[i] ) break;
			delete m_bufs[i];
			m_bufs[i] = NULL;
		}
	}
};



TEST_F(BLTest, selfAllocRetFree) {
	int i;
	Buffer buffer;
	
	ASSERT_TRUE(NULL!=m_bufList) << "\tFail to alloc buffer list";
	ASSERT_TRUE(m_bufList->Init(BUF_COUNT, 1024)) << "\tFail to init buffer list";
	ASSERT_TRUE(0==m_bufList->GetUsedCount()) << "\tused count should be 0";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(0, false)) << "\tused list, no wait, should be empty";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(5000, false)) << "\tused list, wait 5ms, should be empty";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(-2, false)) << "\tused list, wait forever, should be empty w/o block";
	for( i=0; i<BUF_COUNT; ++i )
		ASSERT_TRUE(NULL!=m_bufList->GetFree(-1)) << "\tbuffer " << i << " should be in free list";
	ASSERT_FALSE(m_bufList->Init(m_bufs)) << "selfALloc list cannot be user configured again";
	//queue back to free before deleting list
	for( i=0; m_bufs[i]; ++i )
		m_bufList->PutFree(m_bufs[i]);
}


TEST_F(BLTest, selfAllocRetUsed) {
	int i;
	Buffer buffer;
	
	ASSERT_TRUE(NULL!=m_bufList) << "\tFail to alloc buffer list";
	ASSERT_TRUE(m_bufList->Init(BUF_COUNT, 1024)) << "\tFail to init buffer list";
	ASSERT_TRUE(0==m_bufList->GetUsedCount()) << "\tused count should be 0";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(0, false)) << "\tused list, no wait, should be empty";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(5000, false)) << "\tused list, wait 5ms, should be empty";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(-2, false)) << "\tused list, wait forever, should be empty w/o block";
	for( i=0; i<BUF_COUNT; ++i )
		ASSERT_TRUE(NULL!=m_bufList->GetFree(-1)) << "\tbuffer " << i << " should be in free list";
	ASSERT_FALSE(m_bufList->Init(m_bufs)) << "selfALloc list cannot be user configured again";
	//queue back to free before deleting list
	for( i=0; m_bufs[i]; ++i )
		m_bufList->PutUsed(m_bufs[i]);
}


TEST_F(BLTest, userAlloc) {
	int i;
	Buffer *thisBuf;
	/* buffers park to list */
	ASSERT_TRUE(m_bufList->Init(m_bufs)) << "\tShould be init'ed at least once";
	ASSERT_TRUE(0==m_bufList->GetUsedCount()) << "\tused count should be 0";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(false, 0)) << "\tused list, no wait, should be empty";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(false, 5000)) << "\tused list, wait 5ms, should be empty";
	ASSERT_TRUE(NULL==m_bufList->GetUsed(false, -2)) << "\tused list, wait forever, should be empty w/o block";

	/* buffers move from free to used */
	for( i=0; i<BUF_COUNT; ++i ) {
		thisBuf = m_bufList->GetFree(-2);        //wait forever
		ASSERT_TRUE(NULL!=thisBuf) << "\tbuffer " << i << " should be in free list";
		//processing
		m_bufList->PutUsed(thisBuf);
		ASSERT_EQ((int)m_bufList->GetUsedCount(), i+1) <<"Nr. of used buffer mismatches that inserted\n";
	}
	/* buffers move from used to free */
	for( i=0; i<BUF_COUNT; ++i ) {
		Buffer *buf2;
		thisBuf = m_bufList->GetUsed(-2, false); //peek
		ASSERT_TRUE(NULL!=thisBuf) << "\tbuffer " << i << " should be in used list";
		ASSERT_EQ((int)m_bufList->GetUsedCount(), BUF_COUNT-i) <<"Nr. of used buffer mismatches that inserted before peek\n";
		buf2 =  m_bufList->GetUsed(-2);          //wait forever
		ASSERT_EQ(thisBuf, buf2) << "\tbuffer " << i << " should be same a peeked";
		ASSERT_EQ((int)m_bufList->GetUsedCount(), BUF_COUNT-1-i) <<"Nr. of used buffer mismatches that inserted after got\n";
		m_bufList->PutFree(thisBuf);
	}

}


TEST_F(BLTest, asPassThru) {
	int i;
	Buffer *thisBuf;

	for( i=0; i<BUF_COUNT; ++i ) {
		m_bufList->PutUsed(m_bufs[i]);
		ASSERT_EQ((int)m_bufList->GetUsedCount(), i+1) <<"Nr. of used buffer mismatches that injected\n";
	}

	for( i=0; i<BUF_COUNT; ++i ) {
		Buffer *buf2;

		thisBuf = m_bufList->GetUsed(-2);        //wait forever
		ASSERT_TRUE(NULL!=thisBuf) << "\tbuffer " << i << " should be in used list";
		//processing
		m_bufList->PutFree(thisBuf);
		buf2 = m_bufList->GetFree(-1);           //wait forever
		ASSERT_EQ(buf2, thisBuf) <<"Buffer mismatch\n";
	}
}
#include "gtest/gtest.h"
#include <pthread.h>
#include "Buffer.h"

//to export SetData and SetSize
class testBuffer : public Buffer {
public:
	testBuffer(bool selfAlloc=true) : Buffer(selfAlloc) {}
	~testBuffer() {}
	virtual bool Init(unsigned char *data, unsigned int bufSize){
		return Buffer::Init(data, bufSize);
	}
	void SetUsedSize(unsigned int usedSize) {
		Buffer::SetUsedSize(usedSize);
	}
};


class selfAllocTest : public testing::Test {
protected:
	testBuffer           *m_buffer;

public:
	selfAllocTest() {}
	~selfAllocTest() {}
	virtual void SetUp() {
		m_buffer = new testBuffer();
		ASSERT_TRUE(NULL!=m_buffer) << "\tFail to buffer object";
	}

	virtual void TearDown() {
		if( m_buffer ) {
			delete m_buffer;
			m_buffer = NULL;
		}
	}
};


class usrAllocTest : public testing::Test {
protected:
	testBuffer           *m_buffer;

public:
	usrAllocTest() {}
	~usrAllocTest() {}
	virtual void SetUp() {
		m_buffer = new testBuffer(false);
		ASSERT_TRUE(NULL!=m_buffer) << "\tFail to buffer object";
	}

	virtual void TearDown() {
		if( m_buffer ) {
			delete m_buffer;
			m_buffer = NULL;
		}
	}
};


TEST_F(selfAllocTest, constructor) {
	if( !m_buffer ) return;

	ASSERT_TRUE(NULL==m_buffer->GetData()) << "\tdata pointer is not init'ed to NULL";
	ASSERT_TRUE(0==m_buffer->GetSize()) << "\tsize is not init'ed to 0";
	ASSERT_TRUE(0==m_buffer->GetUsedSize()) << "\tused size is not init'ed to 0";
}


TEST_F(selfAllocTest, init) {
	unsigned int sz = 5;
	unsigned char *dataAddr;
	if( !m_buffer ) return;
	ASSERT_TRUE(m_buffer->Init((unsigned char *)m_buffer, sz)) << "fails to allocate data";
	dataAddr = m_buffer->GetData();
	ASSERT_FALSE(NULL==dataAddr) << "\tbuffer should be allocated";
	ASSERT_FALSE((unsigned char *)m_buffer==dataAddr) << "\tbuffer should not be as the parameter of init'ed";
	dataAddr[2] = 'a';
	unsigned char *tmp = m_buffer->GetData();
	ASSERT_TRUE(tmp[2]==dataAddr[2]) << "\tdata integrity issue";
	ASSERT_TRUE(sz==m_buffer->GetSize()) << "\tsize should be configured";
	ASSERT_TRUE(0==m_buffer->GetUsedSize()) << "\tused size should not be altered";
	
	ASSERT_FALSE(m_buffer->Init((unsigned char *)m_buffer, sz)) << "2nd allocation should fail";
}
#define DATA_SZ	5
TEST_F(selfAllocTest, setUsed) {
	unsigned int sz = DATA_SZ;
	if( !m_buffer ) return;
	m_buffer->SetUsedSize(sz);
	
	ASSERT_FALSE(sz==m_buffer->GetUsedSize()) << "\tused size should be 0 before init";
	m_buffer->Init(NULL, sz);
	m_buffer->SetUsedSize(sz);
	ASSERT_TRUE(sz==m_buffer->GetUsedSize()) << "\tused size should be " << sz;
}


TEST_F(usrAllocTest, constructor) {
	if( !m_buffer ) return;

	ASSERT_TRUE(NULL==m_buffer->GetData()) << "\tdata pointer is not init'ed to NULL";
	ASSERT_TRUE(0==m_buffer->GetSize()) << "\tsize is not init'ed to 0";
	ASSERT_TRUE(0==m_buffer->GetUsedSize()) << "\tused size is not init'ed to 0";
}


TEST_F(usrAllocTest, init) {
	unsigned int sz = DATA_SZ;
	unsigned char dataAddr[DATA_SZ] = {'a', 'b', 'c', 'd', 'e' };
	unsigned char dataAddr1[DATA_SZ-1] = {'1', '2', '3', '4' };
	if( !m_buffer ) return;
	ASSERT_TRUE(m_buffer->Init(dataAddr, DATA_SZ)) << "fails to allocate data";
	ASSERT_TRUE(m_buffer->GetData()==dataAddr) << "data address miss match";
	ASSERT_TRUE(sz==m_buffer->GetSize()) << "\tsize should be configured";
	ASSERT_TRUE(0==m_buffer->GetUsedSize()) << "\tused size should not be altered";
	
	ASSERT_TRUE(m_buffer->Init(dataAddr1, sz-1)) << "2nd allocation should be successful";
	ASSERT_TRUE(m_buffer->GetData()==dataAddr1) << "data address miss match";
	ASSERT_TRUE((sz-1)==m_buffer->GetSize()) << "\tsize should be configured";
}

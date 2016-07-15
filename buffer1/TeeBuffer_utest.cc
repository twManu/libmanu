#include "gtest/gtest.h"
#include <pthread.h>
#include "TeeBuffer.h"


#define WIDTH   1920
#define HEIGHT	1080
#define FORMAT  2

#define NEW_WIDTH   1280
#define NEW_HEIGHT	720
#define FORMAT  2

class myTeeBuffer : public TeeBuffer {
public:
	myTeeBuffer(VideoBuffer &srcBuffer) : TeeBuffer(srcBuffer) {}
	virtual ~myTeeBuffer() {}
	//to enable access of srcBuffer
	VideoBuffer &getSrcBuffer() { return TeeBuffer::getSrcBuffer(); }
};


class simpleTest : public testing::Test {
protected:
	VideoBuffer              *m_vBuffer;
	myTeeBuffer              *m_buffer;
	unsigned char            *m_data0;
	unsigned char            *m_data1;

public:
	simpleTest() : m_vBuffer(NULL), m_buffer(NULL) {}
	~simpleTest() {}
	virtual void SetUp() {
		//every TeeBuffer needs a VideoBuffer to contructed with
		m_vBuffer = new VideoBuffer(WIDTH,HEIGHT, FORMAT);
		ASSERT_TRUE(NULL!=m_vBuffer) << "\tFail to alloc video buffer";
		m_data0 = (unsigned char *)"this is src data";
	    m_data1 = (unsigned char *)"this is new data";
	}

	virtual void TearDown() {
		VideoBuffer *tmpVBuffer = m_vBuffer;
		myTeeBuffer *tmpTeeBuffer = m_buffer;

		m_vBuffer = NULL;
		m_buffer = NULL;
		if( tmpVBuffer ) delete tmpVBuffer;
		if( tmpTeeBuffer ) delete tmpTeeBuffer;
	}
};


TEST_F(simpleTest, constructor) {
	unsigned short w, h;
	unsigned int fmt;

	ASSERT_TRUE(m_vBuffer->Init(m_data0, sizeof(m_data0))) << "\t Shouldn't fail to set data";
	m_buffer = new myTeeBuffer(*m_vBuffer);
	ASSERT_TRUE(NULL!=m_buffer) << "\tFail to alloc video buffer";
	m_buffer->getParam(&w, &h, &fmt);
	ASSERT_EQ(w, WIDTH) << "\tWidth of Tee should be the same as Src";
	ASSERT_EQ(h, HEIGHT) << "\tHeight of Tee should be the same as Src";
	ASSERT_EQ((int)fmt, FORMAT) << "\tFormat of Tee should be the same as Src";
	ASSERT_EQ(m_data0, m_buffer->GetData())<< "\tData object of Tee should be the same as Src";
}


TEST_F(simpleTest, sourceCheck) {
	unsigned short w, h;
	unsigned int fmt;

	ASSERT_TRUE(m_vBuffer->Init(m_data0, sizeof(m_data0))) << "\t Shouldn't fail to set data";
	m_buffer = new myTeeBuffer(*m_vBuffer);
	ASSERT_TRUE(NULL!=m_buffer) << "\tFail to alloc video buffer";
	VideoBuffer &srcBuffer = m_buffer->getSrcBuffer();
	srcBuffer.getParam(&w, &h, &fmt);
	ASSERT_EQ(w, WIDTH) << "\tWidth of Tee should be the same as Src";
	ASSERT_EQ(h, HEIGHT) << "\tHeight of Tee should be the same as Src";
	ASSERT_EQ((int)fmt, FORMAT) << "\tFormat of Tee should be the same as Src";
	ASSERT_EQ(m_data0, srcBuffer.GetData())<< "\tData object of Tee should be the same as Src";
}


TEST_F(simpleTest, cloneChange) {
	unsigned short w, h;
	unsigned int fmt;

	ASSERT_TRUE(m_vBuffer->Init(m_data0, sizeof(m_data0))) << "\t Shouldn't fail to set data";
	m_buffer = new myTeeBuffer(*m_vBuffer);
	ASSERT_TRUE(NULL!=m_buffer) << "\tFail to alloc video buffer";
	//make a change
	ASSERT_FALSE(m_buffer->Init(m_data1, sizeof(m_data1))) << "\tTee shouldn't be able to change data object";
	ASSERT_EQ(m_data0, m_buffer->GetData())<< "\tData object of Tee should remain as data0";
	m_buffer->getParam(&w, &h, &fmt);
	m_buffer->setParam(NEW_WIDTH, NEW_HEIGHT, fmt);
	m_buffer->getParam(&w, &h, &fmt);
	ASSERT_EQ(w, WIDTH) << "\tWidth of Tee should not be changed";
	ASSERT_EQ(h, HEIGHT) << "\tHeight of Tee should not be changed";
}
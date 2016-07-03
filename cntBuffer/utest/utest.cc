#include "gtest/gtest.h"
#include <pthread.h>
#include "../cntBuffer.hpp"
#include "../../OSLinux.h"

class simpleTest : public testing::Test {
protected:
	LinuxLock            *m_lock;

public:
simpleTest() : m_lock(NULL) {
}

~simpleTest() {
	if( m_lock ) {
		delete m_lock;
		m_lock = NULL;
	}
}

virtual void SetUp() {
	m_lock = new LinuxLock();
	if( m_lock && !m_lock->Init() ) {
		delete m_lock;
		m_lock = NULL;
	}
}

virtual void TearDown() {
	if( m_lock ) {
		delete m_lock;
		m_lock = NULL;
	}
}

};

extern "C" {
	static unsigned char g_val;
	static unsigned int g_avail;
	/*
	 * Make sure there is available buffer g_avail in size and 1st byte is expected as g_val
	 * Consume the byte if everything is correct
	 */
	unsigned int proc1byte(unsigned char *buf, unsigned int sz)
	{
		if( g_avail==sz ) {
			if( g_val!=*buf ) {
				printf("not a %u detected\n", g_val);
				return 0;
			}
			return 1;
		} else printf("available size is not as expected %u, %u instead\n", g_avail, sz);
		return 0;
	}
}


TEST_F(simpleTest, wkSpc0) {
	unsigned char data[4]={ 1, 2, 3, 4 };
	unsigned char data1[4]={ 9, 8, 7, 6 };
	cntBuffer bufObj(m_lock, 4, 0);

	//+4
	ASSERT_TRUE(bufObj.addData(data, 4)) << "\tFail to add initial 4-bytes data+4";
	//dummy +1
	ASSERT_FALSE(bufObj.addData(data, 1)) << "\tFail to reject adding 1-byte data+4+1";
	//-1
	g_val = 1;
	g_avail = 4;
	bufObj.getData(proc1byte);
	//+1
	ASSERT_TRUE(bufObj.addData(data, 1)) << "\tFail to add 1-byte data+4-1+1";
	//-1
	g_val = 2;
	g_avail = 4;
	bufObj.getData(proc1byte);
	//-1
	g_val = 3;
	g_avail = 3;
	bufObj.getData(proc1byte);
	//-1
	g_val = 4;
	g_avail = 2;
	bufObj.getData(proc1byte);
	//-1
	g_val = 1;
	g_avail = 1;
	bufObj.getData(proc1byte);
	ASSERT_TRUE(0==bufObj.getAvailable()) << "\tData not balanced\n";
	ASSERT_TRUE(bufObj.addData(data1, 4)) << "\tFail to add initial 4-bytes data+4-1+1-1-1-1-1+4";

	//-1
	g_val = 9;
	g_avail = 4;
	bufObj.getData(proc1byte);
}


TEST_F(simpleTest, wkSpc3) {
	unsigned char data[4]={ 1, 2, 3, 4 };
	unsigned char data1[4]={ 9, 8, 7, 6 };
	cntBuffer bufObj(m_lock, 4, 3);

	//+4
	ASSERT_TRUE(bufObj.addData(data, 4)) << "\tFail to add initial 4-bytes data+4";
	//-1
	g_val = 1;
	g_avail = 2;
	bufObj.getData(proc1byte);
	//-1
	g_val = 2;
	g_avail = 1;
	bufObj.getData(proc1byte);
	ASSERT_TRUE(0==bufObj.getAvailable()) << "\tData not balanced\n";
}
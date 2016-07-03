#include "gtest/gtest.h"
#include "NetAddr.hpp"


#define TEST_IP	MK_ADDR(192, 168, 1, 1)
#define MY_PORT		3


class TestNull : public testing::Test {
protected:
	V4addr m_addr;
	// Remember that SetUp() is run immediately before a test starts.
	// This is a good place to record the start time.
	void reset() {
		V4addr tmp;
		m_addr = tmp;
	}
};


TEST_F(TestNull, init) {
	unsigned int addr;
	unsigned short port;
	string str;

	reset();
	m_addr.getAddr(&addr, &port);
	ASSERT_EQ(addr, 0) << "construct of IP out of expect\n";
	ASSERT_EQ(port, 0) << "construct of port out of expect\n";
	ASSERT_FALSE(m_addr.isConfiged()) << "empty object should not be configured\n"; 
	m_addr.getIPString(str);
	ASSERT_EQ(0, strcmp("0.0.0.0", str.c_str())) << "empty object address mismatch\n";
	m_addr.getIPPortString(str);
	ASSERT_EQ(0, strcmp("0.0.0.0:0", str.c_str())) << "empty object address:port mismatch\n";
}


TEST_F(TestNull, configIP) {
	unsigned int ip = TEST_IP;
	reset();
	m_addr.setAddr(&ip, NULL);
	ASSERT_TRUE(m_addr.isConfiged()) << "setting IP is considered configured\n";
}


TEST_F(TestNull, configPort) {
	unsigned short port = MY_PORT;
	reset();
	m_addr.setAddr(NULL, &port);
	ASSERT_TRUE(m_addr.isConfiged()) << "setting Port is considered configured\n";
}


TEST_F(TestNull, configBoth) {
	unsigned int ip = TEST_IP;
	unsigned short port = MY_PORT;
	V4addr addr(TEST_IP, MY_PORT);
	reset();
	m_addr.setAddr(&ip, &port);
	ASSERT_TRUE(addr==m_addr) << "error configured object comparison\n";
}



class TestNumber : public testing::Test {
protected:
	V4addr *m_addr;
	// Remember that SetUp() is run immediately before a test starts.
	// This is a good place to record the start time.
	virtual void SetUp() {
		m_addr = new V4addr(TEST_IP, MY_PORT);
	}

	// TearDown() is invoked immediately after a test finishes.  Here we
	// check if the test was too slow.
	virtual void TearDown() {
		if( m_addr ) {
			delete m_addr;
			m_addr = NULL;
		}
	}
};


TEST_F(TestNumber, init) {
	unsigned int addr;
	unsigned short port;
	
	ASSERT_TRUE(NULL!=m_addr) << "V4addr initialization fails\n";
	m_addr->getAddr(&addr, &port);
	ASSERT_EQ(addr, TEST_IP) << "construct of IP out of expect\n";
	ASSERT_EQ(port, MY_PORT) << "construct of port out of expect\n";
}


TEST_F(TestNumber, initObject) {
	unsigned int addr;
	unsigned short port;
	
	ASSERT_TRUE(NULL!=m_addr) << "V4addr initialization fails\n";
	V4addr dupObj(*m_addr);
	dupObj.getAddr(&addr, &port);
	ASSERT_EQ(addr, TEST_IP) << "error object duplication in IP\n";
	ASSERT_EQ(port, MY_PORT) << "error object duplication in port\n";
}


TEST_F(TestNumber, initAssign) {
	unsigned int addr;
	unsigned short port;
	
	ASSERT_TRUE(NULL!=m_addr) << "V4addr initialization fails\n";
	V4addr dupObj = *m_addr;
	dupObj.getAddr(&addr, &port);
	ASSERT_EQ(addr, TEST_IP) << "error assignment in IP\n";
	ASSERT_EQ(port, MY_PORT) << "error assignment in port\n";
	ASSERT_TRUE(dupObj==*m_addr) << "error assignment object comparison\n";
}


TEST_F(TestNumber, initIPPortString) {
	unsigned int addr;
	unsigned short port;
	char ipStr[LEN_ADDR_STR];

	ASSERT_TRUE(NULL!=m_addr) << "V4addr initialization fails\n";
	sprintf(ipStr, "%s:%d", "192.168.1.1", MY_PORT);
	V4addr dupObj(ipStr);
	dupObj.getAddr(&addr, &port);
	ASSERT_EQ(addr, TEST_IP) << "error ip:port string constructed object in IP\n";
	ASSERT_EQ(port, MY_PORT) << "error ip:port string constructed object in port\n";
	//operator ==
	ASSERT_TRUE(dupObj==*m_addr) << "error in object comparison\n";
}


class TestString : public testing::Test {
};


TEST_F(TestString, add1) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 0, 0, 0)) << "error ip string1 constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string1 object in port\n";
}


TEST_F(TestString, add1d) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192.");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 0, 0, 0)) << "error ip string1d constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string1d object in port\n";
}


TEST_F(TestString, add2) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192.168");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 0, 0)) << "error ip string2 constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string2 object in port\n";
}


TEST_F(TestString, add2d) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192.168.");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 0, 0)) << "error ip string2d constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string2d object in port\n";
}


TEST_F(TestString, add3) {
	unsigned int addr;
	unsigned short port;
	string str;

	V4addr saddr("192.168.2");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 2, 0)) << "error ip string3 constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string3 object in port\n";
	saddr.getIPPortString(str);
	ASSERT_EQ(0, strcmp("192.168.2.0:0", str.c_str())) << "add3 object address:port mismatch\n";
}


TEST_F(TestString, add3d) {
	unsigned int addr;
	unsigned short port;
	string str;

	V4addr saddr("192.168.2.");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 2, 0)) << "error ip string3d constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string3d object in port\n";
	saddr.getIPPortString(str);
	ASSERT_EQ(0, strcmp("192.168.2.0:0", str.c_str())) << "add3d object address:port mismatch\n";
}

TEST_F(TestString, add4) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192.168.2.1");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 2, 1)) << "error ip string4 constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string4 object in port\n";
}


TEST_F(TestString, add4c) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192.168.2.1:");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 2, 1)) << "error ip string4c constructed object in IP\n";
	ASSERT_EQ(port, 0) << "error ip string4c object in port\n";
}


TEST_F(TestString, add4cp) {
	unsigned int addr;
	unsigned short port;

	V4addr saddr("192.168.2.1:4321");
	saddr.getAddr(&addr, &port);
	ASSERT_EQ(addr, MK_ADDR(192, 168, 2, 1)) << "error ip string4cp constructed object in IP\n";
	ASSERT_EQ(port, 4321) << "error ip string4cp object in port\n";
}


#ifndef	__NET_ADDR_H__
#define	__NET_ADDR_H__

using namespace std;

#include "debug.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>


//length for address string abc.def.ghi.jkl:65535
#define	LEN_ADDR_STR	32
#define MK_ADDR(a, b, c, d)	 ((unsigned int)\
	(\
		((unsigned char) (a))<<24|\
		((unsigned char) (b))<<16|\
		((unsigned char) (c))<< 8|\
		((unsigned char) (d))     \
	)\
)

/*! @class
 * construct by one of 4 ways
 * 1. null argument - 0.0.0.0:0
 * 2. by addr and port number - a.b.c.d:p
 * 3. by V4addr - copy the addr, port
 * 4. by V4addr assignment - copy the addr, port
 * 5. by addr and port string - 'a.b.c.d:p'
 */
class V4addr {
	static const unsigned int LOOPBACK_IP = MK_ADDR(127,0,0,1);

public:
	V4addr() : m_addr(0), m_port(0) {
		updateStr();
	}

	
	/*! @detail
	 * @param addr - 10.1.9.200 is 10<<24 | 1<<16 | 9<<8 | 200. i.e. host endian
	 * @param port - host endian
	 */
	V4addr(unsigned int addr, unsigned short port)
		: m_addr(addr), m_port(port) {
		updateStr();
	}
	

	//! @brief constructor/assignment
	V4addr(V4addr &host) { copyObj(host); }
	V4addr &operator=(V4addr &addr) { copyObj(addr); return *this; }
	//! @brief by string
	V4addr(const char *str) : m_addr(0), m_port(0) {
		char *cur, *saved;
		unsigned char addr[4] = { 0 };
		int i;
		
		//copy str string
		if( !str ) return;
		strncpy(m_strAddr, str, LEN_ADDR_STR-1);
		m_strAddr[LEN_ADDR_STR-1] = 0;
		
		//a.b.c.
		cur = strtok_r(m_strAddr, ".", &saved);
		for( i=0; i<3 && cur; ++i ) {
			addr[i] = atoi(cur) & 0xff;
			cur = strtok_r(NULL, ".", &saved);
		}
		if( 3==i && cur ) { //all 3 '.' got
			addr[3] = atoi(cur) & 0xff;
			char *ptr = strstr(cur, ":");
			if( ptr ) { //found ':'
				ptr = strtok_r(cur, ":", &saved);
				ptr = strtok_r(NULL, ":", &saved);
				if( ptr ) m_port = atoi(ptr) & 0xffff;
			}
		}
		m_addr = MK_ADDR(addr[0], addr[1], addr[2], addr[3]);
		updateStr();
	}
	

	//! @brief check if addr has same address and port
	bool operator==(V4addr &addr) {
		unsigned int ip;
		unsigned short port;
		addr.getAddr(&ip, &port);
		if( ip==m_addr && port==m_port ) return true;
		return false;
	}


	//! @brief where address is 127.0.0.1
	bool isLoopback(V4addr &host) {
		unsigned int addr;
		host.getAddr(&addr, NULL);
		return LOOPBACK_IP==addr;
	}

	
	/*! @detail
	 * Get address string either ADDR_ONLY or ADDR_PORT
	 * @return the string is returned but user is NOT supposed to modify it
	 */
	void getIPString(string &ipstr) { ipstr = (const char *)m_strAddr; }
	void getIPPortString(string &ippstr) { ippstr = (const char *)m_strAddrPort; }
	
	
	//! @brief if parameter provided, the corespondent value is copied
	void getAddr(unsigned int *addr, unsigned short *port) {
		if( addr ) *addr = m_addr;
		if( port ) *port = m_port;
	}


	//! @brief if parameter provided, the corespondent value is set
	void setAddr(unsigned int *addr, unsigned short *port) {
		if( addr ) m_addr = *addr;
		if( port ) m_port = *port;
		updateStr();
	}


	//! @brief check if address or port ever set
	bool isConfiged() { return (m_addr || m_port); }


protected:
	unsigned int         m_addr;                       //address in host endian, see LOOPBACK_IP
	unsigned short       m_port;                       //same endian as m_addr
	char                 m_strAddr[LEN_ADDR_STR];      //addr only
	char                 m_strAddrPort[LEN_ADDR_STR];  //addr and port
	
	//! @brief copy from object
	void copyObj(V4addr &host) {
		host.getAddr(&m_addr, &m_port);
		updateStr();
	}
	
	/*! @detail
	 * Construct the string of address and address-port as well.
	 * @param m_addr [in] - host endian ip
	 * @param m_port [in] - host endian port
	 * @param m_strAddr/m_strAttrPort - updated according m_addr/m_port
	 */
	void updateStr() {
		sprintf(m_strAddr, "%d.%d.%d.%d", (m_addr>>24)&0xff, (m_addr>>16)&0xff, (m_addr>>8)&0xff, (m_addr)&0xff);
		sprintf(m_strAddrPort, "%s:%d", m_strAddr, m_port);
	}
};


#endif	//__NET_ADDR_H__
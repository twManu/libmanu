#include "LinuxTCP.hpp"
#include <errno.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <resolv.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/wait.h>
#include <arpa/inet.h>

using namespace std;

//init static member
socklen_t LinuxTCP::m_lenSaddrIN = sizeof(struct sockaddr_in);

LinuxTCP::LinuxTCP(unsigned int la, unsigned short lp)
	: BaseTCP(la, lp), m_backlog(128)
{
}


LinuxTCP::LinuxTCP(V4addr &host)
	: BaseTCP(host), m_backlog(128)
{
}

LinuxTCP::LinuxTCP(int fd) : BaseTCP()
{
	V4addr addr;

	if( fd<=0 ) {
		ERROR("invalid file descriptor\n");
		return;
	}

	m_fd = fd;           //updateLocalAddress needs it
	if( !updateLocalAddress(addr) ) {
		m_fd = 0;
		return;
	}
	m_LA = addr;
	//peer address
	if( updatePeerAddress(addr) ) m_PA = addr;
	return;
}


bool LinuxTCP::Close()
{
	return doClose();
}


bool LinuxTCP::updateLocalAddress(V4addr &addr)
{
	socklen_t len = m_lenSaddrIN;
	struct sockaddr_in localAddr;
	unsigned int laddr;
	unsigned short lport;
	if( getsockname(m_fd, (sockaddr *)&localAddr, &len) ) return false;
	laddr = (unsigned int)ntohl(localAddr.sin_addr.s_addr);
	lport = (unsigned short)ntohs(localAddr.sin_port);

	m_LA.setAddr(&laddr, &lport);
	addr = m_LA;
	return true;
}


bool LinuxTCP::updatePeerAddress(V4addr &addr)
{
	socklen_t len = m_lenSaddrIN;
	struct sockaddr_in peerAddr;
	unsigned int paddr;
	unsigned short pport;
	
	if( getpeername(m_fd, (sockaddr *)&peerAddr, &len) ) return false;
	paddr =	(unsigned int)ntohl(peerAddr.sin_addr.s_addr);
	pport =  (unsigned short)ntohs(peerAddr.sin_port);
	m_PA.setAddr(&paddr, &pport);
	addr = m_PA;
	return true;
}


bool LinuxTCP::doConnect(V4addr &peerAddr)
{
	V4addr localAddr;
	struct sockaddr_in local_addr;
	struct sockaddr_in peer_addr;   //for peer
	unsigned int ip;
	unsigned short port;
	char const *errstr = NULL;

	//m_fd created mean we cannot open socket anymore
	if( m_fd ) return false;
	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( m_fd<0 ) {
		m_fd = 0;
		ERROR("socket already created for connect\n");
		return false;
	}
	//do binding if port specified
	getLocalAddr(localAddr);
	localAddr.getAddr(&ip, &port);
	if( port ) {
		memset(&local_addr, 0, sizeof(local_addr));
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(port);
		if( ip ) local_addr.sin_addr.s_addr = htonl(ip);
		else local_addr.sin_addr.s_addr = INADDR_ANY;
		if( bind(m_fd, (struct sockaddr *) &local_addr, m_lenSaddrIN) < 0 )
			ERROR("failure binding for connection !!!"); //keep going
	}
	peerAddr.getAddr(&ip, &port);
	if( 0==ip ) {
		errstr = "server ip not specified\n";
		goto errClose;
	}
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(port);
	peer_addr.sin_addr.s_addr = htonl(ip);
	if( connect(m_fd, (struct sockaddr *)&peer_addr, m_lenSaddrIN)<0 ) {
		errstr = "failure connection\n";
		goto errClose;
	}
	updateLocalAddress(localAddr);
	updatePeerAddress(peerAddr);
	return true;
	
errClose:
	close(m_fd);
	m_fd = 0;
	ERROR("%s\n", errstr);
	return false;
}


/*
 * create socket if not yet created
 * do binding if address specified
 */
bool LinuxTCP::doListen(int max)
{
	V4addr localAddr;
	struct sockaddr_in serv_addr;
	unsigned int ip;
	unsigned short port;
	char const *errstr = NULL;

	//m_fd created mean we cannot open socket anymore
	if( m_fd ) return false;
	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( m_fd<0 ) {
		m_fd = 0;
		ERROR("socket already created\n");
		return false;
	}
	//do binding if address specified
	getLocalAddr(localAddr);
	localAddr.getAddr(&ip, &port);
	if( 0==port ) {
		errstr = "port not specified\n";
		goto errClose;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if( ip ) serv_addr.sin_addr.s_addr = htonl(ip);
	else serv_addr.sin_addr.s_addr = INADDR_ANY;
	if( bind(m_fd, (struct sockaddr *) &serv_addr, m_lenSaddrIN) < 0 ) {
		errstr = "binding fails\n";
		goto errClose;
	}
	if( listen(m_fd, max) ) {
		errstr = "failure listen\n";
		perror("");
		goto errClose;

	}
	m_backlog = max;
	return true;

errClose:
	close(m_fd);
	m_fd = 0;
	ERROR("%s\n", errstr);
	return false;
}


BaseTCP *LinuxTCP::doAccept()
{
	struct sockaddr_in cli_addr;
	int newfd;
	socklen_t clilen = m_lenSaddrIN;
	
	newfd = accept(m_fd, (struct sockaddr *) &cli_addr, &clilen);
	if( newfd<0 ) {
		ERROR("failure accept\n");
		return NULL;
	}

	return new LinuxTCP(newfd);
}


int LinuxTCP::doRead(char *buf, int size) {
	int thisRead, rest = size;
	while( rest ) {
		thisRead = read(m_fd, buf, rest);
		if( thisRead<0 ) break;	//error
		buf += thisRead;
		rest -= thisRead;
	}
	return size-rest;

}


int LinuxTCP::doWrite(char *buf, int size) {
	int thisWrite, rest = size;
	while( rest ) {
		thisWrite = write(m_fd, buf, rest);
		if( thisWrite<=0 ) break;	//error
		buf += thisWrite;
		rest -= thisWrite;
	}
	return size-rest;
}


bool LinuxTCP::doClose()
{
	int tmpfd = m_fd;
	m_fd = 0;
	if( tmpfd ) {
		shutdown(tmpfd, SHUT_RDWR);
		close(tmpfd);
	}
	return true;
}

bool LinuxTCP::doSetNBlock(bool block) { return true; }                  //(nonblock), (0): block, (otherwise): nonblock
bool LinuxTCP::doSetTout(int readMS, int writeMS) { return true; }       //(read MS, write MS), 0 means forever
bool LinuxTCP::doSetKeepALive(int KAL1_ms, int KALn_ms, int count)       //(1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
			{ return true; }
bool LinuxTCP::doSetLinger(bool on, int waitMS) { return true; }         //(linger on, linger wait MS), (0, x) means disable
bool LinuxTCP::doSetBuffer(int rcvSz, int sendSz) { return true; }       //(receive B, send B)
bool LinuxTCP::doSetResue(bool on) { return true; }                      //(reuse), 0 means disable
bool LinuxTCP::doGetNBlock(bool *block) {                                //(nonblock), (0): block, (otherwise): nonblock
		*block = m_nBLK;
		return true;
	}
bool LinuxTCP::doGetTout(int *readMS, int *writeMS) {                    //(read MS, write MS), 0 means forever
		if( readMS ) *readMS = m_readTOms;
		if( writeMS ) *writeMS = m_writeTOms;
		return true;
	}
bool LinuxTCP::doGetKeepALive(int *KAL1_ms, int *KALn_ms, int *count) {  //(1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
		if( KAL1_ms ) *KAL1_ms = m_KAL1stMS;
		if( KALn_ms ) *KALn_ms = m_KALsubMS;
		if( count ) *count = m_KALcount;
		return true;
	}
bool LinuxTCP::doGetLinger(bool *on, int *waitMS) {                      //(linger on, linger wait MS), (0, x) means disable
		if( on ) *on = m_linger;
		if( waitMS ) *waitMS = m_lingerMS;
		return true;
	}
bool LinuxTCP::doGetBuffer(int *rcvSz, int *sendSz) {                    //(receive B, send B)
		if( rcvSz ) *rcvSz = m_rcvBufSize;
		if( sendSz ) *sendSz = m_sndBufSize;
		return true;
	}
bool LinuxTCP::doGetResue(bool *on) {                                    //(reuse), 0 means disable
		if( on ) *on = m_reuseAddr;
		return true;
	}

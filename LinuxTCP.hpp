#ifndef	__LINUX_TCP_H__
#define	__LINUX_TCP_H__

#include "BaseTCP.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

/*
 * active open
 *	con -> set option (optional) -> connect ( binding if LA/LP provided ) -> set option (optional)
 * passive open
 *       con -> set option (optional) -> listen ( binding and LP MUST be provided ) -> accept
 */
class LinuxTCP : public BaseTCP {
public:

	/*! @detail
	 * If local address or port is provided, it will be used during for binding.
	 * The option is applied when SetOption invoked. Previous settings are ignored.
	 * @param addr - local address in network order, default to 0 which is determined by interface bind (passive) or default routing (active)
	 * @param port - local port, default to 0 which means any
	 */
	LinuxTCP(unsigned int la=0, unsigned short lp=0);
	LinuxTCP(V4addr &host);
	LinuxTCP(int fd);
	virtual ~LinuxTCP() { Close(); }
	virtual bool Close();

protected:
	static socklen_t m_lenSaddrIN;
	int m_backlog;
	//! @brief get the remote and local address/port of the socket (m_fd)
	bool getPeerAP(unsigned int *la, unsigned short *lp);
	bool getLocalAP(unsigned int *la, unsigned short *lp);
	//! @brief get ip:port from m_fd with getsockname
	bool updateLocalAddress(V4addr &addr);
	//! @brief get ip:port from m_fd with getpeername
	bool updatePeerAddress(V4addr &addr);
	/*! @detail
	 * Listen on socket every configured. The implementation should invoke like this
	 *        bool TCP::Listen(unsigned int addr, unsigned short port, int maxcon) {
	 *	        if( !BaseTCP::Listen(addr, port, maxcon) ) return false;
	 *               ...
	 *        }
	 * @param maxcon - maximum connection
	 * @return false - failure
	 *          true - successful
	 */

	virtual bool doListen(int maxcon);
		
	/*! @detail
	 * If local address or port is provided, it will be used during for binding.
	 * @param pa - peer address
	 * @param pp - peer port
	 * @return false - failure
	 *          true - successful
	 */
	virtual bool doConnect(V4addr &host);

	/*! @detail
	 * @param newSkt - new created socket if success
	 * @return false - failure
	 *          true - successful
	 */
	virtual BaseTCP *doAccept();
	//! @brief it can be open'ed again after close if the LA/LP remains
	virtual bool doClose();

	/*! @detail
	 * Read/write as many size unless timeout happens
	 * @param timeoutMS [in/out] - input as timeout value in MS
	 *                              output as error happens
	 * @return the byte read or write no matter error happens or not
	 */
	virtual int doRead(char *buf, int size);
	virtual int doWrite(char *buf, int size);

	virtual bool doSetNBlock(bool block);                                  //(nonblock), (0): block, (otherwise): nonblock
	virtual bool doSetTout(int readMS, int writeMS);                       //(read MS, write MS), 0 means forever
	virtual bool doSetKeepALive(int KAL1_ms, int KALn_ms, int count);      //(1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
	virtual bool doSetLinger(bool on, int waitMS);                         //(linger on, linger wait MS), (0, x) means disable
	virtual bool doSetBuffer(int rcvSz, int sendSz);                       //(receive B, send B)
	virtual bool doSetResue(bool on);                                      //(reuse), 0 means disable
	virtual bool doGetNBlock(bool *block);                                 //(nonblock), (0): block, (otherwise): nonblock
	virtual bool doGetTout(int *readMS, int *writeMS);                     //(read MS, write MS), 0 means forever
	virtual bool doGetKeepALive(int *KAL1_ms, int *KALn_ms, int *count);   //(1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
	virtual bool doGetLinger(bool *on, int *waitMS);                       //(linger on, linger wait MS), (0, x) means disable
	virtual bool doGetBuffer(int *rcvSz, int *sendSz);                     //(receive B, send B)
	virtual bool doGetResue(bool *on);                                     //(reuse), 0 means disable
};


#endif	//__LINUX_TCP_H__
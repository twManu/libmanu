#ifndef	__BASE_TCP_H__
#define	__BASE_TCP_H__

#include "NetAddr.hpp"
#include <sys/socket.h>
#include <unistd.h>


/*
 * active open
 *	con -> set option (optional) -> connect ( PA must be provided, binding if LA ) -> set option (optional)
 * passive open
 *       con -> set option (optional) -> listen ( binding if LA ) -> accept
 */
class BaseTCP {
public:
	typedef enum {
		  TYPE_NONE
		, FILE_nBLK            //(nonblock), 0: block, otherwise: nonblock
		, TIMEOUT_MS           //(read MS, write MS), 0 means forever
		, KEEPALIVE_MS         //(1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
		, LINGER_MS            //(linger on, linger wait MS), (0, x) means disable
		, BUFFER_B             //(receive B, send B)
		, REUSE_ADDR           //(reuse), 0 means disable
		, TYPE_LAST
	} sktOpt;

	typedef struct {
		sktOpt       type;           
		int          value[4];
	} attr;

	/*! @detail
	 * If local address or port is provided, it will be used during for binding.
	 * The option is applied when SetOption invoked. Previous settings are ignored.
	 * @param addr - local address in network order, default to 0 which is determined by interface bind (passive) or default routing (active)
	 * @param port - local port, default to 0 which means any
	 */
	BaseTCP(unsigned int la=0, unsigned short lp=0) : m_LA(la, lp) { Init(); }
	BaseTCP(V4addr &host) : m_LA(host) { Init(); }
	/*! @defail
	 * used when accept creates a socket. if a socket was opened, it will be closed
	 * call has to make sure file description valid
	 */
	BaseTCP(int fd) {
		V4addr addr;

		if( fd<=0 ) {
			ERROR("invalid file descriptor\n");
			return;
		}
		if( m_fd ) Close();
		Init();

		m_fd = fd;           //updateLocalAddress needs it
	/*
		if( !updateLocalAddress(addr) ) {
			m_fd = 0;
			return;
		}

		m_LA = addr;
		if( updatePeerAddress(addr) ) m_PA = addr;
	 */
		return;
	}
	virtual ~BaseTCP() {
		Close();
	}

	
#define curVAL(x) (opts[i].value[x])
	/*! @detail
	 * Set multiple options.
	 * @param size - number of options to set
	 * @param option - array of options with type provided
	 * @return 0 - success
	 *          -1 - fail during 1st option
	 *          -2 - fail during 2nd option
	 *                 :
	 */
	int nSetOption(int size, attr *opts) {
		int i;
		bool flagOn;
	
		for( i=0; i<size; ++i ) {
			flagOn = (0!=curVAL(0));   //flagOn==false only when 0
			switch( opts[i].type ) {
			case FILE_nBLK:
				if( m_fd && !doSetNBlock(flagOn) ) goto err;
				else m_nBLK = flagOn;
				break;
			case TIMEOUT_MS:
				if( m_fd && !doSetTout(curVAL(0), curVAL(1)) ) goto err;
				else {
					m_readTOms = curVAL(0);
					m_writeTOms = curVAL(1);
				}
				break;
			case KEEPALIVE_MS:
				if( m_fd && !doSetKeepALive(curVAL(0), curVAL(1), curVAL(2)) ) goto err;
				else {
					m_KAL1stMS = curVAL(0);
					m_KALsubMS = curVAL(1);
					m_KALcount = curVAL(2);
				}
				break;
			case LINGER_MS:
				if( m_fd && !doSetLinger(flagOn, curVAL(1)) ) goto err;
				else {
					m_linger = flagOn;
					m_lingerMS = curVAL(1);
				}
				break;
			case BUFFER_B:
				if( m_fd && !doSetBuffer(curVAL(0), curVAL(1)) ) goto err;
				else {
					m_rcvBufSize = curVAL(0);
					m_sndBufSize = curVAL(1);
				}
				break;
			case REUSE_ADDR:
				if( m_fd && !doSetResue(flagOn) ) goto err;
				else m_reuseAddr = flagOn;
				break;
			default:
				ERROR("Unknown socket option %d\n", opts[i].type);
				goto err;
			}
		}
		return 0;
	err:
		return (++i)*-1;
	}

	/*! @detail
	 * Retrieve multiple options.
	 * @param size - number of options to set
	 * @param option - array of options with type provided
	 * @return 0 - success
	 *          -1 - fail during 1st option
	 *          -2 - fail during 2nd option
	 *                 :
	 */
	int nGetOption(int size, attr *opts) {
		int i, v1, v2, v3;
		bool flagOn;
	
		for( i=0; i<size; ++i ) {
			flagOn = (0!=curVAL(0));   //flagOn==false only when 0
			switch( opts[i].type ) {
			case FILE_nBLK:
				if( m_fd && doGetNBlock(&flagOn) ) m_nBLK = flagOn;
				curVAL(0) = m_nBLK;
				break;
			case TIMEOUT_MS:
				if( m_fd && doGetTout(&v1, &v2) ) {
					m_readTOms = v1;
					m_writeTOms = v2;
				}
				curVAL(0) = m_readTOms;
				curVAL(1) = m_writeTOms;
				break;
			case KEEPALIVE_MS:
				if( m_fd && doGetKeepALive(&v1, &v2, &v3) ) {
					m_KAL1stMS = v1;
					m_KALsubMS = v2;
					m_KALcount = v3;
				}
				curVAL(0) = m_KAL1stMS;
				curVAL(1) = m_KALsubMS;
				curVAL(2) = m_KALcount;
				break;
			case LINGER_MS:
				if( m_fd && doGetLinger(&flagOn, &v1) ) {
					m_linger = flagOn;
					m_lingerMS = v1;
				}
				curVAL(0) = m_linger;
				curVAL(1) = m_lingerMS;
				break;
			case BUFFER_B:
				if( m_fd && doGetBuffer(&v1, &v2) ) {
					m_rcvBufSize = v1;
					m_sndBufSize = v2;
				}
				curVAL(0) = m_rcvBufSize;
				curVAL(1) = m_sndBufSize;
				break;
			case REUSE_ADDR:
				if( m_fd && doGetResue(&flagOn) ) m_reuseAddr = flagOn;
				curVAL(0) = m_reuseAddr;
				break;
			default:
				ERROR("Unknown socket option %d\n", opts[i].type);
				break;
			}
		}
		return 0;
	}
	 
	/*! @detail
	 * If local address or port is provided, it will be used during for binding.
	 * @param pa - peer address
	 * @param pp - peer port
	 * @return false - failure
	 *          true - successful
	 */
	bool Connect(unsigned int pa, unsigned short pp) {
		V4addr host(pa, pp);
		return Connect(host);
	}
	bool Connect(V4addr &paddr) {
		bool success = doConnect(paddr);
		if( success ) m_PA = paddr;
		return success;
	}

	/*! @detail
	 * Listen on socket. The implementation should invoke like this
	 *        bool TCP::Listen(unsigned int addr, unsigned short port, int maxcon) {
	 *	        if( !BaseTCP::Listen(addr, port, maxcon) ) return false;
	 *               ...
	 *        }
	 * @param host - imply the interface to binding. if not configured, any interface applied
	 * @param maxcon - maximum connection
	 * @return false - failure
	 *          true - successful
	 */
	bool Listen(int maxcon) { return doListen(maxcon); }
	bool Listen(V4addr &host, int maxcon) {
		m_LA = host;
		return Listen(maxcon);
	}

	/*! @detail
	 * The created BaseTCP must later be freed
	 */
	BaseTCP *Accept() { return doAccept(); }

	/*! @detail
	 * Read/write as many size unless timeout happens
	 * @param buf/size - buffer and size to read/write
	 * @return the byte read or write, <0 means error occured
	 */
	int Read(char *buf, int size) { return doRead(buf, size); }
	int Write(char *buf, int size) { return doWrite(buf, size); }

	//! @brief it can be open'ed again after close if the LA/LP remains
	virtual bool Close() {
		if( m_fd ) {
			ERROR("error: fd is not closed\n");
			return false;
		}
		return true;
	}

	/*! @detail
	 * The address is available after successful connect or accept.
	 * @param localAddr - copy local address to
	 */
	void getLocalAddr(V4addr &localAddr) { localAddr = m_LA; }

	/*! @detail
	 * @param peerAddr - copy peer address to
	 */
	void getPeerAddr(V4addr &peerAddr) { peerAddr = m_PA; }
	
protected:
	V4addr               m_LA;              //local address, it should be updated as it can be determined
	V4addr               m_PA;              //peer address, it should be updated as it can be determined
	int                  m_fd;	        //file descriptor
	bool                 m_closed;
	void Init() {
		m_closed = false;
		m_fd = 0;
		m_nBLK = m_KAL = m_linger = m_reuseAddr = 0;
		m_readTOms = m_writeTOms = m_KAL1stMS = m_KALsubMS = 0;
		m_KALcount = m_lingerMS = m_rcvBufSize = m_sndBufSize = 0;
	}
	
	//pure virtual
	virtual bool doListen(int maxcon) = 0;
	virtual bool doConnect(V4addr &host) = 0;
	virtual BaseTCP *doAccept() = 0;
	virtual bool doClose() = 0;
	virtual int doRead(char *buf, int size) = 0;
	virtual int doWrite(char *buf, int size) = 0;
	
	//options for implementation to overwrite
	//! @brief (nonblock), (0): block, (otherwise): nonblock
	virtual bool doSetNBlock(bool block) { return true; }
	//! @brief (read MS, write MS), 0 means forever
	virtual bool doSetTout(int readMS, int writeMS) { return true; }
	//! @brief (1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
	virtual bool doSetKeepALive(int KAL1_ms, int KALn_ms, int count)
			{ return true; }
	//! @brief (linger on, linger wait MS), (0, x) means disable
	virtual bool doSetLinger(bool on, int waitMS) { return true; }
	//! @brief (receive B, send B)
	virtual bool doSetBuffer(int rcvSz, int sendSz) { return true; }
	//! @brief (reuse), 0 means disable
	virtual bool doSetResue(bool on) { return true; }
	//! @brief (nonblock), (0): block, (otherwise): nonblock
	virtual bool doGetNBlock(bool *block) {
		*block = m_nBLK;
		return true;
	}
	//! @brief (read MS, write MS), 0 means forever
	virtual bool doGetTout(int *readMS, int *writeMS) {
		if( readMS ) *readMS = m_readTOms;
		if( writeMS ) *writeMS = m_writeTOms;
		return true;
	}
	//! @brief (1st KAL MS after ACK, sub-KAL MS after prev KAL, count), (0, x, x) means disable
	virtual bool doGetKeepALive(int *KAL1_ms, int *KALn_ms, int *count) {
		if( KAL1_ms ) *KAL1_ms = m_KAL1stMS;
		if( KALn_ms ) *KALn_ms = m_KALsubMS;
		if( count ) *count = m_KALcount;
		return true;
	}
	//! @brief (linger on, linger wait MS), (0, x) means disable
	virtual bool doGetLinger(bool *on, int *waitMS) {
		if( on ) *on = m_linger;
		if( waitMS ) *waitMS = m_lingerMS;
		return true;
	}
	//! @brief (receive B, send B)
	virtual bool doGetBuffer(int *rcvSz, int *sendSz) {
		if( rcvSz ) *rcvSz = m_rcvBufSize;
		if( sendSz ) *sendSz = m_sndBufSize;
		return true;
	}
	//! @brief (reuse), 0 means disable
	virtual bool doGetResue(bool *on) {
		if( on ) *on = m_reuseAddr;
		return true;
	}
	
	//! @brief a helper function to apply earlier options to socket
	void applyOption() {
		if( 0==m_fd ) return;
		//TYPE_LAST
		
	}

	//options
	int                  m_nBLK:1;          //nonBlock
	int                  m_KAL:1;           //keep alive on
	int                  m_linger:1;        //linger on
	int                  m_reuseAddr:1;     //reuse address
	int                  m_readTOms;        //read timeout
	int                  m_writeTOms;       //write timeout
	int                  m_KAL1stMS;        //1st KAL
	int                  m_KALsubMS;        //sub KAL
	int                  m_KALcount;
	int                  m_lingerMS;
	int                  m_rcvBufSize;      //receive buffer size
	int                  m_sndBufSize;      //send buffer size
};


#endif	//__BASE_TCP_H__
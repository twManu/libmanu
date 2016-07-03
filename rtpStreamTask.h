#ifndef	__RTPSTREAMTASK_H__
#define	__RTPSTREAMTASK_H__

#include "BaseStream.h"
#include "rtp.h"
#include "OSLinux.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


/*! @class
 * The class takes a stream as its rtp source input.
 * And UDP send to a socket according to the time stamp
 */
class rtpStreamTask : public LinuxThread
{
public:
	static const int MAX_TYPE_LEN = 16;
	static const int ECHO_PERIOD_SEC = 5;
	/*! @detail
	 * The input stream MUST be opened on invokation and close after destructure
	 * A rateCtrl MUST be proviede so that we know when to start time counting
	 * A sleep duration has to be provided. It sleeps for the period when the
	 * media is not yet to be sent
	 */
	rtpStreamTask(BaseStream *stream, RateCtrl *rateCtrl, int sleepUS);

	virtual              ~rtpStreamTask();
	/*! @detail
	 * The Init() has to be call before Start()
	 * And only when it is successful
	 * @return
	 *	0 - failure
	 *	otherwise - success
	 */
	virtual int          Init(const char *remote_ip, unsigned short remote_port, const char *type, int cps);

protected:
	BaseStream           *m_stream;
	RTPparser            *m_parser;
	RateCtrl             *m_rateCtrl;
	struct sockaddr_in   m_saddr;
        int                  m_port;
	int                  m_socket;
	int                  m_sleepUS;
	int                  m_countPS;	//sample count per sec
	char                 m_type[MAX_TYPE_LEN];

	virtual void         Destroy();
	virtual void         ThreadMain();
	//! @brief To open UDP socket for output. A part of init
	static const int     UDP_BUFFER_SIZE = 1024*1024;
	int                  openUDP(char *remote_ip);
	
};

#endif	//__RTPSTREAMTASK_H__
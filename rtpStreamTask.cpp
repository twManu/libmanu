#include "rtpStreamTask.h"
#include <string.h>

rtpStreamTask::rtpStreamTask(BaseStream *stream, RateCtrl *rc, int sleepUS)
	: m_stream(stream)
	, m_parser(NULL)
	, m_rateCtrl(rc)
	, m_port(0)
	, m_socket(0)
	, m_sleepUS(sleepUS)
{
}

rtpStreamTask::~rtpStreamTask()
{
	DBG_THREAD("In Destructor of rtpStreamTask\n");
	Destroy();	//undo Init
}


//undo Init
void rtpStreamTask::Destroy()
{
	DBG_THREAD("In Destroy of %s Task\n", m_type);
	LinuxThread::Destroy();	//stop thread
	if( m_socket ) {
		shutdown(m_socket, SHUT_RDWR);
		close(m_socket);
		m_socket = 0;
	}
	m_stream = NULL;
	m_rateCtrl = NULL;
	TEST_THEN_DELETE(m_parser);
}


void rtpStreamTask::ThreadMain()
{
	char gotPkt;
	unsigned int dTS, rtpPktSize;
	unsigned long long mediaTimeMS, curTimeMS, mediaTime = 0;	//90000 means 1s
	struct timeval curTV;
	unsigned char    *rtpPkt = NULL;
	long long curSec = 0;

	DBG_THREAD("%s thread running, m_tid=%llu\n", m_type, m_tid);
	for( gotPkt=0; !m_toStop; dTS=0) {
		if( !gotPkt ) {
			//get src
			rtpPkt = m_parser->getPacket(&rtpPktSize, &dTS);
			if( !rtpPkt ) {
				ERROR("fail to get rtp packet\n");
				m_toStop = 1;
				break;
			} else gotPkt = 1;
		}

		mediaTime += dTS;	//mediaTimeMS = mediaTime/(90000/1000);
		mediaTimeMS = mediaTime*1000/m_countPS;
		m_rateCtrl->sizeSoFar(&curTV);
		//for debug msg +++
		if( curSec+(ECHO_PERIOD_SEC-1)<curTV.tv_sec ) {
			curSec = curTV.tv_sec;
			ERROR("%s %llu sec pass, media %llu ms pass\n", m_type, curSec, mediaTimeMS);
		}
		//for debug msg ---
		curTimeMS = curTV.tv_sec * 1000 + curTV.tv_usec / 1000;
		if( curTimeMS<mediaTimeMS ) {
			usleep(m_sleepUS);
			continue;
		}
		if( sendto(m_socket, rtpPkt, rtpPktSize, 0, (struct sockaddr *)&m_saddr, sizeof(m_saddr))<0 ) {
              		ERROR("%s fail sendto\n", m_type);
			break;
		}
		gotPkt = 0;
	}
}


int rtpStreamTask::Init(const char *remote_ip, unsigned short remote_port, const char *type, int cps)
{
	char ip[32];

	ERROR("addr=%s, port=%d, type=%s, sample count=%d/s waiting %d.%d ms\n", remote_ip, remote_port, type, cps, m_sleepUS/1000, m_sleepUS%1000);
	strncpy(m_type, type, sizeof(m_type));

	if( !m_stream || !m_rateCtrl ) {
		ERROR("Null stream or rc provided\n");
		return 0;
	}
	//init output socket
	if( !strcasecmp("localhost", remote_ip) ) strcpy(ip, "127.0.0.1");
	else strncpy(ip, remote_ip, sizeof(ip));
	m_port = remote_port;
	m_countPS = cps;
	if( !openUDP(ip) ) goto error;
	//init input
	m_parser = new RTPparser();
	if( !m_parser ) {
		ERROR("fail allocating parser objects\n");
		goto error;
	}
	if( !m_parser->Init(65536, m_stream) ) {
		ERROR("fail to init parser object\n");
		goto error;
	}

	return 1;
error:
	Destroy();
	return 0;
}


/*
 * create socket and bind interface with given port
 * In    : remote_ip
 *         m_port - remote port
 * Out   : m_socket - file descriptor opened if success.
 *       : m_saddr - update for sendto
 * Ret   : 0 - fail
 *         1 - success
 */
int rtpStreamTask::openUDP(char *remote_ip)
{
        int skt_size = UDP_BUFFER_SIZE;

	if( m_socket ) {
		ERROR("socket already opened\n");
		return 0;
	}
	if( (m_socket = socket(AF_INET, SOCK_DGRAM, 0) )==-1 ) {
		ERROR("Error creating socket\n");
		m_socket = 0;
                return 0;
	}

	memset(&m_saddr, 0, sizeof(m_saddr));
	m_saddr.sin_family = AF_INET;
        m_saddr.sin_port = htons(m_port+10);
        m_saddr.sin_addr.s_addr = inet_addr(remote_ip);
	if( setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&skt_size, sizeof(int)) < 0)
		ERROR("fail to set socket size to %d\n", skt_size);
	if( bind(m_socket, (struct sockaddr *)&m_saddr, sizeof(struct sockaddr))==-1 ) {
		ERROR("fail to bind to %s:%d\n", remote_ip, m_port+10);
		close(m_socket);
		m_socket = 0;
		return 0;
        }
	//for send
	m_saddr.sin_port = htons(m_port);
        return 1;
}
#include "rtp.h"
#include <string.h>
#include <arpa/inet.h>



RTP::RTP()
{
}


void RTP::Init(unsigned char *buffer, unsigned int size)
{
	m_data = buffer;
	m_size = size;
	m_pl_data = NULL;
	m_pl_size = 0;
	memset(&m_attr, 0, sizeof(m_attr));
}

unsigned int RTP::parseHeader()
{
	unsigned int byteProcessed = 0;
	unsigned int reset_size = m_size;	//byteProcess + reset_size always eq m_size
	unsigned char *curByte;

	if( NULL==m_data || !m_size ) return byteProcessed;

#define	RET_IF_NOT_ENOUGH(cur_size, next_size)	\
	do {\
		byteProcessed += (cur_size);\
		reset_size -= (cur_size);\
		if( reset_size<(next_size) ) return byteProcessed;\
		curByte += (cur_size);\
	} while( 0 )

	curByte = m_data;	//byte 1, m_data[0]
	m_attr.version = getBit(*curByte, 6, 2);
	m_attr.padding = getBit(*curByte, 5, 1);
	m_attr.extension = getBit(*curByte, 4, 1);
	m_attr.ccount = getBit(*curByte, 0, 4);
	RET_IF_NOT_ENOUGH(1, 1);
	//to byte 2, m_data[1]
	m_attr.marker = getBit(*curByte, 7, 1);
	m_attr.pl_type = getBit(*curByte, 0, 7);
	RET_IF_NOT_ENOUGH(1, 2);
	//to byte 3, m_data[2]
	m_attr.seq = ntohs(*((unsigned short *)(curByte)));	//m_data[2], m_data[3]
	RET_IF_NOT_ENOUGH(2, 4);
	//to byte 5, m_data[4]
	m_attr.ts = ntohl(*((unsigned long *)(curByte)));	//m_data[4-7]
	RET_IF_NOT_ENOUGH(4, 4);
	//to byte 9, m_data[8]
	m_attr.ssrc = ntohl(*((unsigned long *)(curByte)));	//m_data[8-11]
	RET_IF_NOT_ENOUGH(4, (unsigned int)(m_attr.ccount*4));
	//to byte 12, m_data[11]
	/***** the following is a guess *****/
	m_pl_data = curByte;
	if( reset_size ) {
		unsigned int pdSize = 0;
		if( m_attr.padding ) pdSize = m_pl_data[reset_size-1];
		if( pdSize>reset_size ) {
			m_pl_data = NULL;	//it's error, unknown payload info
			return byteProcessed;
		}
		m_pl_size = reset_size - pdSize;
	}

	return m_size;
}


void RTP::getAttr(attrib *attr)
{
	if( attr ) *attr = m_attr;
}


unsigned char *RTP::getPayload(unsigned int *size)
{
	if( size ) *size = m_pl_size;
	return m_pl_data;	//maybe NULL
}


RTPparser::RTPparser() : m_rtp(NULL)
{
	memset(&m_1stAttrib, 0, sizeof(m_1stAttrib));
	memset(m_buffer, 0, sizeof(m_buffer));
}


//ret 0 mean failure
int RTPparser::Init(int size, BaseStream *stream)
{
	if( size<12 ) {
		ERROR("fail to work w/ small size buffer\n");
		return 0;
	}
	if( !stream ) {
		ERROR("NULL input stream\n");
		return 0;
	}
	if( m_buffer[0] || m_buffer[1] ) {
		ERROR("buffer already allocated\n");
		return 0;
	}
	m_buffer[0] = new unsigned char[size];
	if( !m_buffer[0] ) {
		ERROR("fail to allocate buffer\n");
		return 0;
	}
	m_buffer[1] = new unsigned char[size];
	if( !m_buffer[1] ) {
		ERROR("fail to allocate buffer\n");
		freeResource();
		return 0;
	}
	m_rtp = new RTP();
	if( !m_rtp ) {
		ERROR("fail to allocate rtp\n");
		freeResource();
		return 0;
	}
	m_bufSize = size;
	m_curBuf = m_EOF = 0;
	m_writeOffset = m_readOffset = 0;
	m_pktCount = m_streamOffset = 0;
	m_stream = stream;
	return 1;
}


void RTPparser::freeResource()
{
	TEST_THEN_DELETE(m_buffer[0]);
	TEST_THEN_DELETE(m_buffer[1]);
	TEST_THEN_DELETE(m_rtp);
	m_stream = NULL;
}


RTPparser::~RTPparser()
{
	freeResource();
}


int RTPparser::isPktMatch(unsigned char *buf)
{
	int result = 0;

	if( buf[8]==m_ssrc[0] && buf[9]==m_ssrc[1] && buf[10]==m_ssrc[2] && buf[11]==m_ssrc[3] ) {
		m_rtp->Init(buf, 8);	//upto ts
		if( m_rtp->parseHeader() ) {
			RTP::attrib attr;
			m_rtp->getAttr(&attr);
			if( 2==attr.version ) {		//found
				++m_1stAttrib.seq;
				result = 1;
				//ERROR("manutest seq=%u, ts=%u\n", attr.seq, attr.ts);
				if( attr.seq!=m_1stAttrib.seq ) {
					ERROR("the seq# is not continuous @ ofst=%llu(%llu-th) %u should be %u\n", m_streamOffset, m_pktCount, attr.seq, m_1stAttrib.seq);
					m_1stAttrib.seq = attr.seq;
				}
				m_curTS = attr.ts;
			}
		}
	}
	return result;
}


int RTPparser::get1stPacket()
{
	int sizeProduced = produceData();
	unsigned char *ptr;
	if( sizeProduced<RTP_HDR_SIZE ) return 0;
	sizeProduced -= (RTP_HDR_SIZE-1);	//this many iterations
	//check one pass, if not found. The packet doesn't exist
	for( ptr = m_buffer[m_curBuf]+m_readOffset; sizeProduced; ++ptr, --sizeProduced ) {
		m_rtp->Init(ptr, RTP_HDR_SIZE);	//upto ssrc
		++m_readOffset;
		if( m_rtp->parseHeader() ) {
			m_rtp->getAttr(&m_1stAttrib);
			if( 2==m_1stAttrib.version ) {
				ERROR("1st rtp: seq# = %u, ts=%u ssrc = 0x%08x\n", m_1stAttrib.seq, m_1stAttrib.ts, m_1stAttrib.ssrc);
				break;
			}
		}
	}

	if( 2!=m_1stAttrib.version ) return 0;	//no found till end

	m_curPktOffset = m_readOffset-1;
	m_streamOffset = m_curPktOffset;
	ERROR("1st rtp pkt found @ %d\n", m_curPktOffset);
	m_nxtPktOffset = -1;
	//m_readOffset points to 1st byte of hdr, pop whole hdr before locating 2nd pkt hdr
	m_readOffset += (RTP_HDR_SIZE-1);
	ptr += RTP_HDR_SIZE;
	if( m_readOffset>=m_bufSize ) {
		ERROR("No room for 2nd pkt\n");
		return 0;
	}
	{
		unsigned long tmpSsrc = htonl(m_1stAttrib.ssrc);
		for( int i=0; i<4; ++i, tmpSsrc>>=8 ) m_ssrc[i] = tmpSsrc;
	}
	for( ; m_readOffset+RTP_HDR_SIZE<m_bufSize; ++m_readOffset, ++ptr ) {
		if( isPktMatch(ptr) ) {	//fist time curTS is set (as TS of 2nd packet)
			m_nxtPktOffset = m_readOffset;
			m_readOffset += RTP_HDR_SIZE;
			m_pktCount = 1;
			return 1;
		}
	}
	return 0;
}


int RTPparser::switchBuffer()
{
	unsigned char oldBuf;

	//copy from m_curPktOffset old buffer to new buffer
	if( m_EOF ) return 0;
	//LOG_DEBUG("switching buffer while curPktOffset = %d, readOffset = %d\n", m_curPktOffset, m_readOffset);
	oldBuf = m_curBuf;
	m_curBuf ^= 1;
	m_writeOffset = m_bufSize - m_curPktOffset;
	//LOG_DEBUG("copy %d byte data to the other buffer\n", m_writeOffset);
	memcpy(m_buffer[m_curBuf], m_buffer[oldBuf]+m_curPktOffset, m_writeOffset);
	m_readOffset -= m_curPktOffset;
	m_curPktOffset = 0;
	return produceData();
}


/*
 * try to get a packet
 * Out   : ts - the delta TS to previous packet reported
 */
unsigned char *RTPparser::getPacket(unsigned int *size, unsigned int *dTS)
{
	unsigned char *ptr = NULL;
	unsigned int prevTS = 0;
	if( !m_1stAttrib.version ) {	//not yet started
		if( get1stPacket() ) {
			*size = m_nxtPktOffset - m_curPktOffset;
			ptr = m_buffer[m_curBuf] + m_curPktOffset;
			m_curPktOffset = m_nxtPktOffset;
			m_nxtPktOffset = -1;
			*dTS = 0;		//the diff to previous TS is 0
		} else {
			m_EOF = 1;
			ERROR("stop further file read\n");
		}
		return ptr;
	}

	prevTS = m_curTS;
	do {
		ptr = m_buffer[m_curBuf]+m_readOffset;
		while( m_readOffset+RTP_HDR_SIZE<=m_bufSize ) {
			if( isPktMatch(ptr) ) {	//found next, return current
				ptr = m_buffer[m_curBuf] + m_curPktOffset;
				*size = m_readOffset - m_curPktOffset;
				m_curPktOffset = m_readOffset;
				m_readOffset += RTP_HDR_SIZE;
				*dTS = m_curTS - prevTS;
				++m_pktCount;
				m_streamOffset += *size;
				return ptr;
			}
			++ptr;
			++m_readOffset;
		}
	} while( switchBuffer() );
	if( m_EOF ) ERROR("%d bytes left behind latest pkt\n", m_bufSize - m_curPktOffset);
	return NULL;
}


/*
 * Read into buffer 'm_curBuf' from m_inputFile. Data loaded into
 * offset m_bufSize-m_restByte
 *
 * In    : m_EOF, m_bufSize, m_writeOffset, m_curBuf, m_inputFile
 * Out   : m_EOF updated when necessary
 *         m_writeOffset updated
 *         m_bufSize updated if necessary
 * Ret   : size read
 */
int RTPparser::produceData()
{
	int	readOff, size, sizeRead, totalRead = 0;
	if( m_EOF ) return 0;
	//determine the size to read
	readOff = m_writeOffset;
	size = m_bufSize - m_writeOffset;

	sizeRead = m_stream->read(m_buffer[m_curBuf]+readOff, size);
	if( BaseStream::EOS==sizeRead ) {
		m_EOF = 1;
		sizeRead = 0;
	} else if( sizeRead<0 ) {
		ERROR("read fail\n");
		m_EOF = 1;
		sizeRead = 0;
	}
	readOff += sizeRead;
	size -= sizeRead;
	totalRead += sizeRead;

	m_writeOffset += totalRead;
	if( m_EOF ) {		//this should be the last read
		m_bufSize = m_writeOffset;
		ERROR("m_bufSize shirnks to %d\n", m_bufSize);
	}
	return totalRead;
}


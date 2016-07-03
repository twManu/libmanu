#ifndef	__RTP_H__
#define	__RTP_H__

#include "BaseStream.h"

/*
 * It works for Little Endian (or maybe x86) only
 */
//1st DWORD, defined bit field in (byte) memory
#ifdef	CPU_BIG_ENDIAN
	#define	BM_VERSION             0x03             //bit0, 1
	#define BM_PADDING             0x04             //bit 2
	#define BM_EXTENSION           0x08             //bit 3
	#define BM_CSRC_COUNT          0xF0             //bit 4-7
	#define BM_MARKER              0x01             //bit 8
	#define BM_PAYLOAD_TYPE        0xFE             //bit 9-15
#else	//CPU_BIG_ENDIAN
	#define	BM_VERSION             0xC0             //bit7, 6
	#define BM_PADDING             0x20             //bit 5
	#define BM_EXTENSION           0x10             //bit 4
	#define BM_CSRC_COUNT          0x0F             //bit 3-1
	#define BM_MARKER              0x80             //bit 15
	#define BM_PAYLOAD_TYPE        0x7F             //bit 14-8
#endif	//CPU_BIG_ENDIAN

#define	RTP_HDR_SIZE             12

/*! @class
    Used when RTP packet boundary is known
 */
class RTP {
public:
	//! @brief construct with buffer and it's size
	RTP();
	~RTP() {}

	typedef struct attrib {
		unsigned int           version: 2;
		unsigned int           padding: 1;
		unsigned int           extension: 1;
		unsigned int           ccount: 4;
		unsigned int           marker: 1;
		unsigned int           pl_type: 7;
		unsigned short         seq;
		unsigned int           ts;
		unsigned int           ssrc;
	} attrib;
	//! @brief init/reset member var
	void                 Init(unsigned char *buffer, unsigned int size);
	//! @brief parse header as possible. Return bytes of header byte processed, 0 means failure
	unsigned int         parseHeader();
	//! @brief get attribute
	void                 getAttr(attrib *attr);
	//! @brief get payload buffer and size, null means unknow payload
	unsigned char        *getPayload(unsigned int *size);


protected:
	unsigned char        *m_data;
	unsigned char        *m_pl_data;     //payload data
	unsigned int         m_size;
	unsigned int         m_pl_size;      //payload size
	attrib               m_attr;
};


/*! @class
    Used when RTP packet boundary is unknown
 */
class RTPparser {
protected:
	RTP                  *m_rtp;		//to use RTP function
	unsigned char        *m_buffer[2];	//ping pong buffer
	unsigned char        m_curBuf;		//index
	unsigned char        m_EOF;
	BaseStream           *m_stream;		//input stream
	RTP::attrib          m_1stAttrib;
	int                  m_bufSize;		//init'ed as buffer size but it shirnks in the last read
	                               		//so the m_readoffset == m_bufSize-1 means
	int                  m_readOffset;	//where next read happen
	int                  m_writeOffset;	//where next write happen
	int                  m_curPktOffset;    //point to current pkt hdr
	int                  m_nxtPktOffset;    //point to next pkt hdr
	unsigned int         m_curTS;           //the TS of the packet reported
	unsigned long long   m_pktCount;        //1 based packet counter
	unsigned long long   m_streamOffset;    //cumulative size returned so far
	unsigned char        m_ssrc[4];		//4byte network endian for searching
	
	//! @brief Copy from m_curPktOffset of current buffer to the other buffer and load data upto end of buffer
	int                  switchBuffer();
	
	//! @brief Read data from inputFile and m_writeoffset updated
	int                  produceData();

	/*! @detail
	 * Read up to buffer size and find the version=2 as start of 1st pkt.
	 * The check is one pass of the whole buffer. It fails if it cannot find a
	 * 2nd ssrc meets the first one.
	 *
	 * @param m_1stAttrib [OUT] : updated if success
	 * @param many [OUT] : m_readOffset, m_curPktOffset ...
	 * @return
	 *	0 - failed
	 *	1 - successful
	 */
	int                  get1stPacket();

	//!@brief free resource object if allocated
	void                 freeResource();
	
	/*! @detail
	 * Check if buf[0-11] a matched pkt header as 1st packet
	 * @param buf [IN] : must be at least 12B
	 * @param m_ssrc [IN] : 4B char to search (order in mem)
	 * @param m_1stAttrib
	 * @param m_1stAttrib.seq [OUT] : inc'ed or updated when incontinuous
	 * @param m_curTS [OUT] : updated
	 * @return
	 *	0 - mismatch
	 *       1 - ssrc match and version = 2
	 *       -1 - resource error
	 */
	int                  isPktMatch(unsigned char *buf);

public:
	RTPparser();
	~RTPparser();
	//! @brief Allocate resource and init var. It must be called before working.
	int                  Init(int size, BaseStream *stream);
	/*! @detail
	 * Try to guess the next packet with matching ssrc and v=2
	 * @param size [OUT] - size of packet returned
	 * @param dTS [OUT] - the diff of timestamp to previous packet
	 * @return packet address if found
	 */
	unsigned char        *getPacket(unsigned int *size, unsigned int *dTS);
};

#endif	//__RTP_H__

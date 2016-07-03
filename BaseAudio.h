#ifndef	__BASEAUDIO_H__
#define __BASEAUDIO_H__

typedef enum {
	  eRATE_8k
	, eRATE_16k
	, eRATE_32k
	, eRATE_44k_1
	, eRATE_48k
} eRate;


typedef enum {
	  eType_CAP
	, eType_BP
} eType;


typedef enum {
	  eChannel_1 = 1               //mono
	, eChannel_2                   //stereo
} eChannel;


typedef enum {
	  eFormat_U8
	, eFormat_S8
	, eFormat_U16_LE
	, eFormat_U16_BE
	, eFormat_S16_LE
	, eFormat_S16_BE
} eFormat;


class audioBase {
protected:
	eType                          m_type;
	eChannel                       m_channel;          //number of channel, configured indicator
	eFormat                        m_format;           //data format
	eRate                          m_rate;             //sampling rate
	int                            m_frameSz;          //number of bytes an audio clock generates
	/*
	 * update frame size, use after m_channel and m_format determined
	 * In  : m_channel and m_format
	 * Out : m_frameSz updated
	 * Ret : m_frameSz;
	 */
	int calFrameSize() {
		char bytes = 0;
		switch( m_format ) {
		case eFormat_U8:
		case eFormat_S8: bytes = 1;
			break;
		case eFormat_U16_LE:
		case eFormat_U16_BE:
		case eFormat_S16_LE:
		case eFormat_S16_BE: bytes = 2;
		default:
			break;
		}
		m_frameSz = bytes * m_channel;
		return m_frameSz;
	}

public:
	audioBase(eType type)
		: m_type(type)
		, m_channel((eChannel)0) {}
	~audioBase() {}
	int getFrameSize() { return m_frameSz; }
	//0 is invalid
	int getRate() {
		switch(m_rate) {
		case eRATE_8k:         return 8000;
		case eRATE_16k:        return 16000;
		case eRATE_32k:        return 32000;
		case eRATE_44k_1:      return 44100;
		case eRATE_48k:        return 48000;
		default:               break;
		}
		return 0;
	}
	/*
	 * Convert time to size
	 * m_channel, m_rate, and m_frameSz must be set
	 */
	int timeUsToSize(int timeUS) {
		int size = getRate() /100 * m_frameSz;
		size *= timeUS;
		size /= 10000;
		return size;
	}
	/*
	 * calculate number of frame by given time in us
	 * m_rate must be set
	 */
	int timeUsToFrameCount(int timeUS) {
		int count = getRate()/100;
		count *= timeUS;
		count /= 10000;
		return count;
	}
};

#endif	//__BASEAUDIO_H__

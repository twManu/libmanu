#ifndef	__VIDEOBUFFER_H__
#define	__VIDEOBUFFER_H__

#include "Buffer.h"

class TeeInList;

/*!
   @class VideoBuffer
   a wrapper of Buffer holding more parameter like width, height

   Usage as user allocated buffer
	construct(w,h,f)
	Init(addr, size)
	destructor()
 */
class VideoBuffer : public Buffer {
friend TeeInList;        //to use protected function
protected:
	//! @brief w & h allow to be set so as to resue buffer
	unsigned short       m_width;
	unsigned short       m_height;
	unsigned int         m_format;
	//! @brief for Tee to use as issue clone count, expected to be accessed by teeThread
	int                  m_cloneCount;


public:
	//! @brief never allocate buffer by lib
	VideoBuffer(unsigned short w, unsigned short h, unsigned int fmt);
	//! @brief used as a clone
	VideoBuffer(VideoBuffer &srcBuffer);
	virtual void setParam(unsigned short w, unsigned short h, unsigned int fmt);
	void getParam(unsigned short *w, unsigned short *h, unsigned int *fmt);
	virtual ~VideoBuffer();
};

#endif	//__VIDEOBUFFER_H__

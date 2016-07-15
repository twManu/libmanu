#ifndef	__TEEBUFFER_H__
#define	__TEEBUFFER_H__

#include "VideoBuffer.h"

class TeeInList;

/*!
   @class TeeBuffer
   Used as sink side of a TeeList duplicated from the source
 */
class TeeBuffer : public VideoBuffer {
friend TeeInList;        //to use protected function
protected:
	/*!@brief
		Used as a clone in TeeList
		NOTE: TeeBuffer is not expected to flow through more than one TeeList
	 */
	VideoBuffer                        &m_srcBuffer;
	VideoBuffer &getSrcBuffer();

public:
	TeeBuffer(VideoBuffer &srcBuffer);
	virtual ~TeeBuffer();
	//nop since it is not allowed
	virtual bool Init(unsigned char *data, unsigned int bufSize);
	virtual void setParam(unsigned short w, unsigned short h, unsigned int fmt);
};

#endif	//__TEEBUFFER_H__

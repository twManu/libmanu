#ifndef	__V4L2_BASE_H__
#define	__V4L2_BASE_H__

#include <stdlib.h>
#include <linux/videodev2.h>
#include "../buffer/Buffer.h"

class V4l2Base;
using namespace std;

/* 
 * V4l2Base echo the activity of buffer queue and deqeue.
 * I/O block or not is determined by the user.
 * It is designed for SINGLE THREADING programming
 */
class V4l2BaseBuffer : public Buffer {
friend class V4l2Base;
protected:
	int                  m_index;

public:
	V4l2BaseBuffer(bool selfAlloc, int index)
		: Buffer(selfAlloc)
		, m_index(index)
		{}
	~V4l2BaseBuffer() {};
	int getIndex() { return m_index; }
};


#define MAX_SUPPORT_FORMT	16
/*
   usage:
	V4l2Base()
	setFormat()
	initV4l2()
	streaming(on) ...option starts, ok for single threading
	getBuffer()
	putBuffer()
	streaming(off) ...option ends
	~V4l2Base()
 */
class V4l2Base {
private:
	/* !@brief set m_capability */
	void loadCapability();
	void printCapability();
	void checkVideoFormat();
	bool reqBuf(int);
	void deallocBuf();
	bool allocBuf();
	bool applyFormat();
	void objectInit();

protected:
	/* !@brief m_fd <0 if invalid */
	int                            m_fd;
	/* !@brief /dev/videoN to operate with, <0 means get first */
	int                            m_devIndex;
	int                            m_bufCount;
	bool                           m_blockIO;
	bool                           m_queryOnly;
	/* !@brief alloc and map ... bad, the are separate */
	bool                           m_mmap;
	bool                           m_streaming;
	unsigned int                   m_supportFormat[MAX_SUPPORT_FORMT];
	struct v4l2_capability         m_capability;
	unsigned int                   m_width;
	unsigned int                   m_height;
	unsigned int                   m_curFormat;
	V4l2BaseBuffer                 **m_buffers;
	struct v4l2_buffer             m_v4l2_buffer;        //template

public:
	V4l2Base();
	//!@brief index got from enumV4L2Format
	V4l2Base(unsigned int indexFormat);
	//!@brief child class need to call that of parent
	virtual ~V4l2Base();
	/* !@details
	 * open file and query capability
	 * child class needs to call initV4l2 of parent class before its own
	 * @param index - <0 means auto search, otherwise specifi /dev/videoN used
	 * @param bufCount - max buffer count to request, max=VIDEO_MAX_FRAME
	 * @param mmap - whether to do alloc and mmap. valid if bufCount!=0
	 * @param block - whether getBuffer will be blocked. This class implement single threading.
	 *                so this flag directly reflect to file open. That is, getBuffer will be
	 *                blocked if true. False otherwise.
	 */
	virtual bool initV4l2(int devIndex, bool queryOnly, unsigned int bufCount=0, bool mmap=true, bool block=true);
	/* !@details
	 * remember format set apply when init
	 * @param pixelformat - v4l2_fourcc encoded pixelformat
	 * @return reflect the sucessfulness of ioctl, check exact format by getFormat
	 */
	bool setFormat(unsigned int w, unsigned int h, unsigned int pixelformat);
	void getFormat(unsigned int *w, unsigned int *h, unsigned int *pixelformat);
	//!@brief report V4L2_PIX_FMT_XXX in fmtArray and the size, 0 means none
	int  enumFormat(unsigned int **fmtArray);
	const char *pixFormatGetName(unsigned int pixformat, unsigned int *fmtIndex=NULL);
	//!@brief set frame rate, if no parameter, use what is got from device
	void setFrmRate(unsigned int num=0, unsigned int denom=0);
	//!@brief from 0 to max_support_format index
	unsigned int enumV4L2Format(int index);
	bool enumFrameSz(unsigned int pixfmt);

	/*!@detail 
	 * streaming on or off, return true if success
	 * all buffer is queued to driver and before that, getBuffer is not successful
	 */
	virtual bool streaming(bool on);
	/* !@details
	 * get a buffer. no lock operation...suitable for single threading
	 * it might sleep if blockIO
	 * @return NULL if error
	 */
	virtual V4l2BaseBuffer *getBuffer();
	/* !@details
	 * Return a buffer to driver
	 * @buffer - buffer to queue to driver
	 * @checkStreaming - true, to queue if streaming is true
	 * @retrun true - if successful queued.
	 */
	virtual bool putBuffer(V4l2BaseBuffer *buffer, bool checkStreaming=true);
	/* !@brief
	 *  print v4l2 definitions
	 */
	void printRaw();
};

#endif	//__V4L2_BASE_H__

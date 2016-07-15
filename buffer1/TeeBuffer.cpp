#include "TeeBuffer.h"

#define ERROR printf

//used as a sink of TeeList
TeeBuffer::TeeBuffer(VideoBuffer &srcBuffer)
	: VideoBuffer(srcBuffer)
	, m_srcBuffer(srcBuffer)
{
}


TeeBuffer::~TeeBuffer()
{
}

bool TeeBuffer::Init(unsigned char *data, unsigned int bufSize)
{
	return false;
}

VideoBuffer &TeeBuffer::getSrcBuffer()
{
	return m_srcBuffer;
}

void TeeBuffer::setParam(unsigned short w, unsigned short h, unsigned int fmt)
{
	//ignore
}
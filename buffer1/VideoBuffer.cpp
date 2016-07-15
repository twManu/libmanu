#include "VideoBuffer.h"
#include <stdlib.h>


VideoBuffer::VideoBuffer(unsigned short w, unsigned short h, unsigned int fmt)
	: Buffer(false)
	, m_width(w)
	, m_height(h)
	, m_format(fmt)
	, m_cloneCount(0)
{
}


VideoBuffer::VideoBuffer(VideoBuffer &srcBuffer)
	: Buffer(srcBuffer)
	, m_width(srcBuffer.m_width)
	, m_height(srcBuffer.m_height)
	, m_format(srcBuffer.m_format)
	, m_cloneCount(0)
{
}


VideoBuffer::~VideoBuffer()
{
	m_data = NULL;
}


void VideoBuffer::setParam(unsigned short w, unsigned short h, unsigned int fmt)
{
	m_width = w;
	m_height = h;
	m_format = fmt;
}


void VideoBuffer::getParam(unsigned short *w, unsigned short *h, unsigned int *fmt)
{
	if( w ) *w = m_width;
	if( h ) *h = m_height;
	if( fmt ) *fmt = m_format;
}
#include "Buffer.h"
#include <stdlib.h>

//todo
#include <stdio.h>
#define ERROR	printf

Buffer::Buffer(bool selfAlloc)
	: m_selfAlloc(selfAlloc)
	, m_data(NULL)
	, m_size(0)
	, m_used(0)
{
}


Buffer::Buffer(Buffer &srcBuffer)
	: m_selfAlloc(false)
	, m_data(srcBuffer.m_data)
	, m_size(srcBuffer.m_size)
	, m_used(srcBuffer.m_used)
{
}


Buffer::~Buffer()
{
	if( m_selfAlloc && m_data )
		delete m_data;
}


bool Buffer::Init(unsigned char *data, unsigned int bufSize)
{
	if( !m_selfAlloc ) {
		m_data = data;
		m_size = bufSize;
	} else {
		if( m_data ) {
			ERROR("no 2nd alloc allowed\n");
			return false;
		}
		m_size = bufSize;
		if( bufSize ) {
			m_data = new unsigned char [bufSize];
			if( m_data ) return true;
			m_size = 0;
		}
		return false;
	}
	return true;
}


unsigned char *Buffer::GetData()
{
	return m_data;
}


unsigned int Buffer::GetUsedSize()
{
	return m_used;
}


unsigned int Buffer::GetSize()
{
	return m_size;
}

void Buffer::SetUsedSize(unsigned int usedSize)
{
	m_used = usedSize>m_size ? m_size : usedSize;
}

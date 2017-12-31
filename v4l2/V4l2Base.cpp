#include "V4l2Base.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../libcommon.h"

V4l2Base::V4l2Base()
	: m_fd(-1)
	, m_queryOnly(false)
	, m_streaming(false)
	, m_width(0)
	, m_height(0)
	, m_curFormat(0)
	, m_buffers(NULL)
{
	objectInit();
}


void V4l2Base::objectInit()
{
	memset(&m_v4l2_buffer, 0, sizeof(m_v4l2_buffer));
	m_v4l2_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   //hard coding for now
	//leave to init m_v4l2_buffer.memory = V4L2_MEMORY_MMAP;
	//must leave a 0 in the last entry
	memset(m_supportFormat, 0, sizeof(m_supportFormat));
}


V4l2Base::~V4l2Base()
{
	streaming(false);
	deallocBuf();
	if( m_fd>=0 ) close(m_fd);
}


void V4l2Base::deallocBuf()
{
	int i;
	if( m_buffers ) {
		for( i=0; i<(int)m_bufCount; ++i ) {
			if( m_buffers[i] ) {
				if( m_mmap )
					munmap(m_buffers[i]->m_data, m_buffers[i]->m_size);
				delete m_buffers[i];
				m_buffers[i] = NULL;
			}
		}
		delete [] m_buffers;
		m_buffers = NULL;
		reqBuf(0);
	}
}


//do clean-up right here if fails
//m_fd and m_bufCount not checked
bool V4l2Base::allocBuf()
{
	int i;
	struct v4l2_buffer buf;

	if( m_buffers ) {
		printf("duplicate allocation\n");
		return false;
	}
	m_buffers = new V4l2BaseBuffer*[m_bufCount];
	if( !m_buffers ) {
		printf("fail to allocate buffer array\n");
		return false;
	} else memset(m_buffers, 0, sizeof(V4l2BaseBuffer *)*m_bufCount);

	for( i=0; i<(int)m_bufCount; ++i ) {
		buf = m_v4l2_buffer;
		buf.index = i;
		if( 0!=ioctl(m_fd, VIDIOC_QUERYBUF, &buf) ) {
			printf("fail to query buffer[%d]\n", i);
			deallocBuf();
			return false;
		}
		if( m_mmap ) {
			void *mmapped_buffer = mmap(
				NULL /* start anywhere */,
				buf.length,
				PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */,
				m_fd, buf.m.offset
			);
			if( MAP_FAILED==mmapped_buffer ) {
				printf("fail to map buffer[%d]\n", i);
				deallocBuf();
				return false;
			}
			m_buffers[i] = new V4l2BaseBuffer(false, i);
			if( !m_buffers[i] ) {
				printf("fail to new buffer[%d]\n", i);
				deallocBuf();
				return false;
			}
			m_buffers[i]->Init((unsigned char *)mmapped_buffer, buf.length);
		} else {        //USERPTR
			m_buffers[i] = new V4l2BaseBuffer(true, i);
			if( !m_buffers[i]->Init(NULL, buf.length) ) {
				printf("fail to alloc buf for user ptr\n");
				deallocBuf();
				return false;
			}
		}
	}

	return true;
}


/*
 * m_fd and m_bufCount not checked
 * In : m_v4l2_buffer.type - used for REQBUFS
 *      m_v4l2_buffer.memory - used for REQBUFS
 *
 */
bool V4l2Base::reqBuf(int count)
{
	struct v4l2_requestbuffers request;
	bool nResult;
	

	memset(&request, 0, sizeof(request));
	request.count = count;
	request.type = m_v4l2_buffer.type;
	request.memory = m_v4l2_buffer.memory;
	nResult = ioctl(m_fd, VIDIOC_REQBUFS, &request);
	if( nResult ) {
		perror("REQBUFS");
	}
	printf("%s buf count = %d\n",\
		V4L2_MEMORY_USERPTR==request.memory?"user ptr":"mmap", count);
	return !nResult;
}


bool V4l2Base::initV4l2(int devIndex, bool queryOnly, unsigned int bufCount, bool mmap, bool block)
{
#define	MAX_LEN_FNAME	32
	char fname[MAX_LEN_FNAME];
	bool keepFinding = true;
	struct stat buff;

	m_queryOnly = queryOnly;
	if( 0<=m_fd ) {
		printf("object in use !!!\n");
		return false;
	}
	
	m_mmap = mmap;
	if( mmap ) {
		printf("use memory mapping\n");
		m_v4l2_buffer.memory = V4L2_MEMORY_MMAP;
	} else {
		printf("use user pointer\n");
		m_v4l2_buffer.memory = V4L2_MEMORY_USERPTR;
	}
	m_bufCount = bufCount>VIDEO_MAX_FRAME ? VIDEO_MAX_FRAME : (int) bufCount;
	if( 1==m_bufCount ) m_bufCount = 2;
	m_blockIO = block;
	//find first available dev node
	m_devIndex = devIndex;
	if( devIndex>=0 ) keepFinding = false;
	else m_devIndex = 0;     //search from 0

	do {
		snprintf(fname, MAX_LEN_FNAME, "/dev/video%d", m_devIndex);
		if( 0==stat(fname, &buff) ) {
			if( S_ISCHR(buff.st_mode) ) {
				m_fd = open(fname, O_RDWR | (block ? 0:O_NONBLOCK), 0);
				if( 0<=m_fd ) {
					keepFinding = false;
					loadCapability();
				}
			}
		}
	} while( keepFinding && (++m_devIndex<16) );

	if( keepFinding )
		return false;
	if( !m_queryOnly ) {
		if( !applyFormat() ) return false;
		//m_fd valid
		if( m_bufCount ) {
			if( !reqBuf(m_bufCount) ) return false;        //fail to request buffer for mapping
			if( !allocBuf() ) return false;
		} else printf("no buffer allocated\n");
	}

	return (m_fd>=0);
}


//expect m_fd valid
void V4l2Base::loadCapability()
{
	memset(&m_capability, 0, sizeof(m_capability));
	if( ioctl(m_fd, VIDIOC_QUERYCAP, &m_capability)<0 ) {
		printf("fails to get capability\n");
		return;
	}
	printf("/dev/video%d connects to %s using the %s driver\n",
		m_devIndex, m_capability.card, m_capability.driver);
	printCapability();
	if( V4L2_CAP_VIDEO_CAPTURE & m_capability.capabilities )
		checkVideoFormat();
	//todo vbi
}


//! @breif for capabilities field
static val_name_t g_capability_vn[] = {
	  DEF_VAL_NAME(V4L2_CAP_VIDEO_CAPTURE)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_OUTPUT)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_OVERLAY)
	, DEF_VAL_NAME(V4L2_CAP_VBI_CAPTURE)
	, DEF_VAL_NAME(V4L2_CAP_VBI_OUTPUT)
	, DEF_VAL_NAME(V4L2_CAP_SLICED_VBI_CAPTURE)
	, DEF_VAL_NAME(V4L2_CAP_SLICED_VBI_OUTPUT)
	, DEF_VAL_NAME(V4L2_CAP_RDS_CAPTURE)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
	, DEF_VAL_NAME(V4L2_CAP_HW_FREQ_SEEK)
	, DEF_VAL_NAME(V4L2_CAP_RDS_OUTPUT)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_CAPTURE_MPLANE)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_OUTPUT_MPLANE)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_M2M_MPLANE)
	, DEF_VAL_NAME(V4L2_CAP_VIDEO_M2M)
	, DEF_VAL_NAME(V4L2_CAP_TUNER)
	, DEF_VAL_NAME(V4L2_CAP_AUDIO)
	, DEF_VAL_NAME(V4L2_CAP_RADIO)
	, DEF_VAL_NAME(V4L2_CAP_MODULATOR)
	, DEF_VAL_NAME(V4L2_CAP_READWRITE)
	, DEF_VAL_NAME(V4L2_CAP_ASYNCIO)
	, DEF_VAL_NAME(V4L2_CAP_STREAMING)
	, DEF_VAL_NAME(V4L2_CAP_DEVICE_CAPS)
};


static val_name_t g_format_vn[] = {
	  DEF_VAL_NAME(V4L2_PIX_FMT_RGB332)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB444)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB555)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB565)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB555X)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB565X)
	, DEF_VAL_NAME(V4L2_PIX_FMT_BGR666)
	, DEF_VAL_NAME(V4L2_PIX_FMT_BGR24)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB24)
	, DEF_VAL_NAME(V4L2_PIX_FMT_BGR32)
	, DEF_VAL_NAME(V4L2_PIX_FMT_RGB32)
	, DEF_VAL_NAME(V4L2_PIX_FMT_GREY)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y4)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y6)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y10)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y16)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y10BPACK)
	, DEF_VAL_NAME(V4L2_PIX_FMT_PAL8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_UV8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YVU410)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YVU420)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUYV)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YYUV)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YVYU)
	, DEF_VAL_NAME(V4L2_PIX_FMT_UYVY)
	, DEF_VAL_NAME(V4L2_PIX_FMT_VYUY)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV422P)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV411P)
	, DEF_VAL_NAME(V4L2_PIX_FMT_Y41P)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV444)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV555)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV565)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV32)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV410)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV420)
	, DEF_VAL_NAME(V4L2_PIX_FMT_HI240)
	, DEF_VAL_NAME(V4L2_PIX_FMT_HM12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_M420)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV21)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV16)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV61)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV24)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV42)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV12M)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV21M)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV12MT_16X16)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YUV420M)
	, DEF_VAL_NAME(V4L2_PIX_FMT_YVU420M)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SBGGR8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGBRG8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGRBG8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SRGGB8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SBGGR10)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGBRG10)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGRBG10)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SRGGB10)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SBGGR12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGBRG12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGRBG12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SRGGB12)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SBGGR10ALAW8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGBRG10ALAW8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGRBG10ALAW8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SRGGB10ALAW8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SBGGR10DPCM8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGBRG10DPCM8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SGRBG10DPCM8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SRGGB10DPCM8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SBGGR16)
	, DEF_VAL_NAME(V4L2_PIX_FMT_MJPEG)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV16M)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV61M)
	, DEF_VAL_NAME(V4L2_PIX_FMT_NV12MT)
	, DEF_VAL_NAME(V4L2_PIX_FMT_JPEG)
	, DEF_VAL_NAME(V4L2_PIX_FMT_DV)
	, DEF_VAL_NAME(V4L2_PIX_FMT_MPEG)
	, DEF_VAL_NAME(V4L2_PIX_FMT_H264)
	, DEF_VAL_NAME(V4L2_PIX_FMT_H264_NO_SC)
	, DEF_VAL_NAME(V4L2_PIX_FMT_H264_MVC)
	, DEF_VAL_NAME(V4L2_PIX_FMT_H263)
	, DEF_VAL_NAME(V4L2_PIX_FMT_MPEG1)
	, DEF_VAL_NAME(V4L2_PIX_FMT_MPEG2)
	, DEF_VAL_NAME(V4L2_PIX_FMT_MPEG4)
	, DEF_VAL_NAME(V4L2_PIX_FMT_XVID)
	, DEF_VAL_NAME(V4L2_PIX_FMT_VC1_ANNEX_G)
	, DEF_VAL_NAME(V4L2_PIX_FMT_VC1_ANNEX_L)
	, DEF_VAL_NAME(V4L2_PIX_FMT_VP8)
	, DEF_VAL_NAME(V4L2_PIX_FMT_CPIA1)
	, DEF_VAL_NAME(V4L2_PIX_FMT_WNVA)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SN9C10X)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SN9C20X_I420)
	, DEF_VAL_NAME(V4L2_PIX_FMT_PWC1)
	, DEF_VAL_NAME(V4L2_PIX_FMT_PWC2)
	, DEF_VAL_NAME(V4L2_PIX_FMT_ET61X251)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SPCA501)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SPCA505)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SPCA508)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SPCA561)
	, DEF_VAL_NAME(V4L2_PIX_FMT_PAC207)
	, DEF_VAL_NAME(V4L2_PIX_FMT_MR97310A)
	, DEF_VAL_NAME(V4L2_PIX_FMT_JL2005BCD)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SN9C2028)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SQ905C)
	, DEF_VAL_NAME(V4L2_PIX_FMT_PJPG)
	, DEF_VAL_NAME(V4L2_PIX_FMT_OV511)
	, DEF_VAL_NAME(V4L2_PIX_FMT_OV518)
	, DEF_VAL_NAME(V4L2_PIX_FMT_STV0680)
	, DEF_VAL_NAME(V4L2_PIX_FMT_TM6000)
	, DEF_VAL_NAME(V4L2_PIX_FMT_CIT_YYVYUY)
	, DEF_VAL_NAME(V4L2_PIX_FMT_KONICA420)
	, DEF_VAL_NAME(V4L2_PIX_FMT_JPGL)
	, DEF_VAL_NAME(V4L2_PIX_FMT_SE401)
	, DEF_VAL_NAME(V4L2_PIX_FMT_S5C_UYVY_JPG)
};


static val_name_t g_ioctl_vn[] = {
	  DEF_VAL_NAME(VIDIOC_QUERYBUF)
	, DEF_VAL_NAME(VIDIOC_REQBUFS)
	, DEF_VAL_NAME(VIDIOC_QUERYCAP)
	, DEF_VAL_NAME(VIDIOC_G_PARM)
	, DEF_VAL_NAME(VIDIOC_S_PARM)
	, DEF_VAL_NAME(VIDIOC_ENUM_FMT)
	, DEF_VAL_NAME(VIDIOC_S_FMT)
	, DEF_VAL_NAME(VIDIOC_ENUM_FRAMESIZES)
};


V4l2Base::V4l2Base(unsigned int indexFormat)
	: m_fd(-1)
	, m_streaming(false)
	, m_width(0)
	, m_height(0)
	, m_buffers(NULL)
{
	if( indexFormat<SIZE_ARRAY(g_format_vn) )
		m_curFormat = g_format_vn[indexFormat].val;
	else
		m_curFormat = 0;
	objectInit();
}


void V4l2Base::printCapability()
{
	for( int i=0; i<SIZE_ARRAY(g_capability_vn); ++i) {
		if( m_capability.capabilities & g_capability_vn[i].val )
			printf("\t%s support\n", g_capability_vn[i].name);
	}
}


void V4l2Base::printRaw()
{
	int i;
	PRINT_VAL_NAME(g_ioctl_vn);
	for( i=0; i<SIZE_ARRAY(g_ioctl_vn); ++i)
		printf("\t, '%s': %s\n", g_ioctl_vn[i].name, g_ioctl_vn[i].name);
	PRINT_VAL_NAME(g_format_vn);
	for( i=0; i<SIZE_ARRAY(g_format_vn); ++i)
		printf("\t, '%s': %s\n", g_format_vn[i].name, g_format_vn[i].name);
	PRINT_VAL_NAME(g_capability_vn);
	for( i=0; i<SIZE_ARRAY(g_capability_vn); ++i)
		printf("\t, '%s': %s\n", g_capability_vn[i].name, g_capability_vn[i].name);
}


void V4l2Base::setFrmRate(unsigned int num, unsigned int denom)
{
	struct v4l2_streamparm sparm;

	memset(&sparm, 0, sizeof(sparm));
	sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(m_fd, VIDIOC_G_PARM, &sparm) ) {
		printf("Unable to get parm: %s (%d).\n", strerror(errno), errno);
        } else {
		if( num )
			sparm.parm.capture.timeperframe.numerator = num;
		else
			printf("cur numerator = %d\n", sparm.parm.capture.timeperframe.numerator);
		if( denom )
			sparm.parm.capture.timeperframe.denominator = denom;
		else
			printf("cur denominator = %d\n", sparm.parm.capture.timeperframe.denominator);
		if( ioctl(m_fd, VIDIOC_S_PARM, &sparm) ) {
			printf("Unable to set parm: %s (%d).\n", strerror(errno), errno);
		}
	}
}


/*
 *  Out : index - if valid pointer, return index found
 *                -1 means not found
 */
const char *V4l2Base::pixFormatGetName(unsigned int pixformat, unsigned int *index)
{
	const char *name = NULL;

	if( index ) *index = -1;                        //not found
	for( int i=0; i<SIZE_ARRAY(g_format_vn); ++i ) {
		if( g_format_vn[i].val==pixformat ) {
			name = g_format_vn[i].name;
			if( index ) *index = i;
			break;
		}
	}
	return name;
}


unsigned int V4l2Base::enumV4L2Format(int index)
{
	if( index>=SIZE_ARRAY(g_format_vn) )
		return 0;
	return g_format_vn[index].val;	
}


void V4l2Base::checkVideoFormat()
{
	struct v4l2_fmtdesc format;
	const char *formatName;
	unsigned int fmtIndex;

	printf("Source supplies the following formats:\n");

	for( int indx=0; 1; ++indx ) {
		memset(&format, 0, sizeof(format));
		format.type = m_v4l2_buffer.type;
		format.index = indx;

		if( 0==ioctl(m_fd, VIDIOC_ENUM_FMT, &format) ) {
			formatName = pixFormatGetName(format.pixelformat, &fmtIndex);
			if( formatName ) {
				printf("\t%s (%d) support\n", formatName, fmtIndex);
				m_supportFormat[indx] = format.pixelformat;
				if( m_queryOnly )
					enumFrameSz(format.pixelformat);
			} else printf("\tunknow format 0x%08u\n", format.pixelformat);
		} else return;
	}
}


bool V4l2Base::setFormat(unsigned int w, unsigned int h, unsigned int pixelformat)
{

	if( !w || !h ) return false;
	
	m_curFormat = pixelformat;
	m_width = w;
	m_height = h;
	return true;
}



bool V4l2Base::applyFormat()
{
	struct v4l2_format fmt;
	int i;

	if( 0>m_fd ) {
		printf("invalid file\n");
		return false;
	}
	
	if( m_streaming ) {
		printf("no format setting during streaming\n");
		return false;
	}
	//make sure format supported
	for( i=0; i<MAX_SUPPORT_FORMT; ++i ) {
		if( 0==m_supportFormat[i] ) return false;
		if( m_curFormat==m_supportFormat[i] )
			break;
	}

	memset(&fmt, 0, sizeof(fmt));
	if( m_capability.capabilities & V4L2_CAP_VIDEO_CAPTURE ) {
		fmt.type = m_v4l2_buffer.type;
		fmt.fmt.pix.width = m_width;
		fmt.fmt.pix.height = m_height;
		fmt.fmt.pix.pixelformat = m_curFormat;
		//fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
		fmt.fmt.pix.field = V4L2_FIELD_ANY;
		if( ioctl(m_fd, VIDIOC_S_FMT, &fmt) ) {
			perror("S_FMT");
			//manutest fall thru return false;
		}

		if( (m_width!=fmt.fmt.pix.width) || (m_height!=fmt.fmt.pix.height) ) {
			printf("device modify dimension: ");
			m_width = fmt.fmt.pix.width;
			m_height =fmt.fmt.pix.height;
		}
		printf("set %s @ %ux%u\n", pixFormatGetName(m_curFormat), m_width, m_height);
		setFrmRate(1, 30);
	} else if( m_capability.capabilities & V4L2_CAP_VBI_CAPTURE ) {
		fmt.type = V4L2_BUF_TYPE_VBI_CAPTURE;
		printf("vbi not yet support\n");
		return false;
	} else {
		printf("no video nor vbi support\n");
		return false;
	}

	return true;
}


int V4l2Base::enumFormat(unsigned int **fmtArray)
{
	int count = 0;
	if( fmtArray ) {
		while(count<MAX_SUPPORT_FORMT && m_supportFormat[count])
			++count;
		*fmtArray = m_supportFormat;
	}
	return count;
}


void V4l2Base::getFormat(unsigned int *w, unsigned int *h, unsigned int *pixelformat)
{
	if( w ) *w = m_width;
	if( h ) *h = m_height;
	if( pixelformat ) *pixelformat = m_curFormat;
}


bool V4l2Base::streaming(bool on)
{
	int type = m_v4l2_buffer.type;
	if( 0>m_fd ) return false;
	if( !(m_capability.capabilities & V4L2_CAP_STREAMING) ) {
		printf("streaming not support\n");
		return false;
	}
	if( on ) {           //streamon
		if( !m_streaming ) {
			int i;
			m_streaming = true;
			for( i=0; i<(int)m_bufCount; ++i ) {
				if( !putBuffer(m_buffers[i]) ) {
					m_streaming = false;
					return false;
				}
			}
			return !ioctl(m_fd, VIDIOC_STREAMON, &type);
		} else return false;
	} else {
		if( m_streaming ) {
			m_streaming = false;
			return !ioctl(m_fd, VIDIOC_STREAMOFF, &type);
		} else return false;
	}
	return true;
}


V4l2BaseBuffer *V4l2Base::getBuffer()
{
	V4l2BaseBuffer *buf = NULL;
	struct v4l2_buffer buffer;
	if( 0<=m_fd ) {
		buffer = m_v4l2_buffer;
		buffer.index = 0;
		if( 0==ioctl(m_fd, VIDIOC_DQBUF, &buffer) ) {
			buf = m_buffers[buffer.index];
			buf->m_used = buffer.bytesused;
		}
	}
	return buf;
}


bool V4l2Base::putBuffer(V4l2BaseBuffer *buffer, bool checkStreaming)
{
	bool result = false;

	if( buffer && (0<=m_fd) ) {
		if( !checkStreaming || m_streaming ) {
			buffer->m_used = 0;
			m_v4l2_buffer.index = buffer->m_index;
			if( !m_mmap ) {
				m_v4l2_buffer.m.userptr = (unsigned long)buffer->GetData();
				m_v4l2_buffer.length = buffer->GetSize();
			}
			if( 0==ioctl(m_fd , VIDIOC_QBUF, &m_v4l2_buffer) )
				result = true;
		}
	}
	return result;
}

bool V4l2Base::enumFrameSz(unsigned int pixfmt)
{
	int ret;
	struct v4l2_frmsizeenum fsize;
	
	if( m_fd<0 )
		return false;
	memset(&fsize, 0, sizeof(fsize));
	fsize.index = 0;
	fsize.pixel_format = pixfmt;
	while ((ret = ioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
		if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			printf("\t\t{ discrete: %ux%u }\n",	fsize.discrete.width, fsize.discrete.height);
#if 0
			ret = enum_frame_intervals(m_fd, pixfmt, fsize.discrete.width, fsize.discrete.height);
			if (ret != 0)
				printf("  Unable to enumerate frame sizes.\n");
#endif
		} else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
			printf("\t\t{ continuous: (%ux%u) .. (%ux%u)}\n",
				fsize.stepwise.min_width, fsize.stepwise.min_height,
				fsize.stepwise.max_width, fsize.stepwise.max_height);
			//printf("  Refusing to enumerate frame intervals.\n");
			break;
		} else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			printf("\t\t{ stepwise: min (%ux%u) .. (%ux%u) /stepsize (%u, %u)}\n",
				fsize.stepwise.min_width, fsize.stepwise.min_height,
				fsize.stepwise.max_width, fsize.stepwise.max_height,
				fsize.stepwise.step_width, fsize.stepwise.step_height);
			//printf("  Refusing to enumerate frame intervals.\n");
			break;
		}
		fsize.index++;
	}
	if( 0==fsize.index && ret<0 )
		return false;
	return true;
}

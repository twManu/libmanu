#include "V4l2Base.h"
#include <stdio.h>

#define BUF_COUNT	4
int main(int argc, char **argv)
{
	V4l2Base v4l2;
	V4l2BaseBuffer *buf;
	int i;

	v4l2.setFormat(640, 480, V4L2_PIX_FMT_YUYV);
	if( !v4l2.initV4l2(-1, BUF_COUNT) ) return -1;
//	v4l2.setFormat(1920, 1080, V4L2_PIX_FMT_MJPEG);
//	v4l2.setFormat(1920, 1080, V4L2_PIX_FMT_H264);
	if( v4l2.streaming(true) ) {
		for( i=0; i<16; ++i ) {
			buf = v4l2.getBuffer();
			if( !buf ) {
				printf("fail to get buffer\n");
				break;
			}
			printf("got buffer[%d], use size=%u\n", buf->getIndex(), buf->GetUsedSize());
			v4l2.putBuffer(buf);
		}
		v4l2.streaming(false);
	}

	return 0;
}

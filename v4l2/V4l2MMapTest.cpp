#include "V4l2MMap.h"
#include <stdio.h>
#include "../OSLinux.h"


static void gotBuffer(void *usrCntx, V4l2BaseBuffer *buffer)
{
	printf("before queue buffer[%d], %p\n", buffer->getIndex(), buffer);
}

#define BUF_COUNT	4
int main(int argc, char **argv)
{
	LinuxLock lock;
	V4l2BaseBuffer *buf;
	int i;

	lock.Init();
	V4l2MMap v4l2(&lock, gotBuffer, NULL);      //block user

	v4l2.setFormat(640, 480, V4L2_PIX_FMT_YUYV);
	if( !v4l2.initV4l2(-1, BUF_COUNT) ) return -1;
	if( v4l2.streaming(true) ) {
		for( i=0; i<16; ++i ) {
			buf = v4l2.getBuffer();
			if( !buf ) {
				printf("fail to get buffer\n");
				break;
			}
			printf("got buffer[%d], %p, use size=%u\n", buf->getIndex(), buf, buf->GetUsedSize());
			v4l2.putBuffer(buf);
		}
		v4l2.streaming(false);
	}

	return 0;
}

#include "V4l2Base.h"
#include <stdio.h>
#include <getopt.h>

#define BUF_COUNT	4


void usage() {
	printf("Test streaming of given video device\n");
	printf("Usage: V4l2BaseTest [-n DEVNR]\n");
	printf("      DEVNR - /dev/videoX which defaults to 0\n");
	exit(-1);
}

static int g_devNr = 0;

bool checkParam(int argc, char *argv[])
{
	int opt;

	while( (opt = getopt(argc, argv, "hn:")) != -1) {
		switch (opt) {
		case 'n': {
			g_devNr = atoi(optarg);
			printf("to open /dev/video%d\n", g_devNr);
			break;
		}
		default:
			usage();
		}
	}
	return true;
}


int main(int argc, char **argv)
{
	V4l2Base v4l2;
	V4l2BaseBuffer *buf;
	int i;

	checkParam(argc, argv);
	v4l2.setFormat(640, 360, V4L2_PIX_FMT_YUYV);
	if( !v4l2.initV4l2(g_devNr, BUF_COUNT) ) return -1;
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
	} else perror("STREAMON");

	return 0;
}

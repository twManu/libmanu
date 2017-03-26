#include "V4l2Base.h"
#include <stdio.h>
#include <getopt.h>

#define BUF_COUNT	4

static int g_cap_count = 16;

void usage() {
	printf("Test streaming of given video device\n");
	printf("Usage: V4l2BaseTest [-n DEVNR] [-c CAP_COUNT\n");
	printf("      DEVNR - /dev/videoX which defaults to 0\n");
	printf("      CAP_COUNT - default to %d\n", g_cap_count);
	exit(-1);
}

static int g_devNr = 0;

bool checkParam(int argc, char *argv[])
{
	int opt;

	while( (opt = getopt(argc, argv, "hn:c:")) != -1) {
		switch (opt) {
		case 'n':
			g_devNr = atoi(optarg);
			printf("to open /dev/video%d\n", g_devNr);
			break;
		case 'c':
			g_cap_count = atoi(optarg);
			printf("to capture %d buffer\n", g_cap_count);
			break;
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
	int **fmts;
	int i, count;

	checkParam(argc, argv);
	v4l2.setFormat(640, 360, V4L2_PIX_FMT_YUYV);
	//v4l2.setFormat(640, 360, V4L2_PIX_FMT_MJPEG);
	//v4l2.setFormat(1920, 1080, V4L2_PIX_FMT_H264);
	if( !v4l2.initV4l2(g_devNr, BUF_COUNT) ) return -1;
	/*
	count = v4l2.enumFormat(fmts);
	if( !count ) {
		printf("no video format enumerated\n");
		return -1;
	} */


	if( v4l2.streaming(true) ) {
		for( i=0; i<g_cap_count; ++i ) {
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

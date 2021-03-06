#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include "OSLinux.h"
#include "V4l2Base.h"
#include "FileDump.h"

#define BUF_COUNT	4

static int g_cap_count = 16;

void usage() {
	int index;
	V4l2Base v4l2;
	printf("Test streaming of given video device\n");
	printf("Usage: V4l2BaseTest [-n DEVNR] [-c CAP_COUNT] [-f FORMAT] [-s WIDTHxHEIGHT] [-d SAVE_FILE] [-q] [-u]\n");
	printf("      DEVNR - /dev/videoX which defaults to 0\n");
	printf("      CAP_COUNT - default to %d\n", g_cap_count);
	printf("      FORMAT - definded as following\n");
	printf("      -q - just query\n");
	printf("      -u - use user pointer\n");
	for( index=0; 1; ++index ) {
		const char *name;
		unsigned int fmt;

		fmt = v4l2.enumV4L2Format(index);
		if( !fmt ) break;
		name = v4l2.pixFormatGetName(fmt);
		if( name )
			printf("  %s : %d\n", name, index);
		else
			printf("%d-th missing foramt\n", index);
	}
	exit(-1);
}

static int g_devNr = 0;
static int g_format = -1;
static int g_width = 640;
static int g_height = 360;
static const char *g_savefile = NULL;
static bool g_mmap = true;
static bool g_query = false;


bool checkParam(int argc, char *argv[])
{
	int opt;
	int i, w, h;

	while( (opt = getopt(argc, argv, "hn:c:f:s:d:u:q")) != -1) {
		switch (opt) {
		case 'n':
			g_devNr = atoi(optarg);
			printf("to open /dev/video%d\n", g_devNr);
			break;
		case 'c':
			g_cap_count = atoi(optarg);
			printf("to capture %d buffer\n", g_cap_count);
			break;
		case 'f':
			g_format = atoi(optarg);
			break;
		case 's':
			i = sscanf(optarg, "%dx%d", &w, &h);
			if( 2!=i ) usage();
			g_width = w;
			g_height = h;
			break;
		case 'd':
			g_savefile = optarg;
			break;
		case 'u':
			g_mmap = false;
			break;
		case 'q':
			g_query = true;
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
	FileDump *dump = NULL;
	swClock swclk;
	long long int diffUsec;
	int i;

	checkParam(argc, argv);
	v4l2.setFormat(g_width, g_height,
		g_format<0 ? V4L2_PIX_FMT_YUYV : v4l2.enumV4L2Format(g_format));
	if( !v4l2.initV4l2(g_devNr, g_query, BUF_COUNT, g_mmap) ) return -1;
	if( g_query ) {
		v4l2.printRaw();
		return 0;
	}
	if( g_savefile ) {
		dump = new FileDump(g_savefile, FileStream::BM_WRONLY, g_cap_count);
		if( dump ) {
			if( !dump->open() ) {
				delete dump;
				dump = NULL;
			}
		}
	}

	swclk.reset();
	if( v4l2.streaming(true) ) {
		for( i=0; i<g_cap_count; ++i ) {
			buf = v4l2.getBuffer();
			if( !buf ) {
				printf("fail to get buffer\n");
				break;
			}
			if( dump ) {
				int sz;
				sz = dump->writeData(buf->GetData(), buf->GetUsedSize());
				printf("write %d(%s) bytes\n", sz, sz==(int)buf->GetUsedSize()?"OK":"NG");
			} else
				printf("got buffer[%d], use size=%u\n", buf->getIndex(), buf->GetUsedSize());
			v4l2.putBuffer(buf);
		}
		diffUsec = swclk.diff();
		v4l2.streaming(false);
		if( i )
			printf("count=%d, fps=%0.2f\n", i,(double)(i*1000*1000)/diffUsec);
	} else perror("STREAMON");

	if( dump ) {
		dump->close();
		delete dump;
	}

	return 0;
}

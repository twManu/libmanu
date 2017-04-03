#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include "V4l2Base.h"

#define BUF_COUNT	4

static int g_cap_count = 16;

void usage() {
	int index;
	V4l2Base v4l2;
	printf("Test streaming of given video device\n");
	printf("Usage: V4l2BaseTest [-n DEVNR] [-c CAP_COUNT] [-f FORMAT] [-s WIDTHxHEIGHT] [-d SAVE_FILE]\n");
	printf("      DEVNR - /dev/videoX which defaults to 0\n");
	printf("      CAP_COUNT - default to %d\n", g_cap_count);
	printf("      FORMAT - definded as following\n");
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
static int g_savefd = -1;
struct header {
	unsigned int         index;
	unsigned int         size;
	unsigned int         data[0];
} *g_hdr = NULL;


bool checkParam(int argc, char *argv[])
{
	int opt;
	int i, w, h;

	while( (opt = getopt(argc, argv, "hn:c:f:s:d:")) != -1) {
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
			g_savefd = open(optarg, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			if( g_savefd<0 )
				perror("open");
			break;	
		default:
			usage();
		}
	}

       if( g_savefd>=0 ) {
                g_hdr = (struct header *)                 //one for index, the other of size
			calloc(g_cap_count+2, sizeof(unsigned int));
                if( !g_hdr ) {
                        close(g_savefd);
                        printf("failed to allocate header\n");
                        g_savefd = -1;
                } else {
                        g_hdr->index = 0;
                        g_hdr->size = (g_cap_count+1)*sizeof(unsigned int);
		}
	}

	return true;
}


int main(int argc, char **argv)
{
	V4l2Base v4l2;
	V4l2BaseBuffer *buf;
	int i, count;
	unsigned int write_sz;

	checkParam(argc, argv);
	v4l2.setFormat(g_width, g_height,
		g_format<0 ? V4L2_PIX_FMT_YUYV : v4l2.enumV4L2Format(g_format));
	if( !v4l2.initV4l2(g_devNr, BUF_COUNT) ) return -1;
	if( g_savefd>=0 ) lseek(g_savefd, g_hdr->size, SEEK_SET);
	if( v4l2.streaming(true) ) {
		for( i=0; i<g_cap_count; ++i ) {
			buf = v4l2.getBuffer();
			if( !buf ) {
				printf("fail to get buffer\n");
				break;
			}
			if( g_savefd>=0 ) {
				write_sz = write(g_savefd, buf->GetData(), buf->GetUsedSize());
				printf("write %d(%s) bytes\n", write_sz, write_sz==buf->GetUsedSize()?"OK":"NG");
				g_hdr->data[g_hdr->index++] = write_sz;   //sample size
			} else
				printf("got buffer[%d], use size=%u\n", buf->getIndex(), buf->GetUsedSize());
			v4l2.putBuffer(buf);
		}
		v4l2.streaming(false);
	} else perror("STREAMON");

	if( g_savefd>=0 ) {
		unsigned char *tmp = (unsigned char *)g_hdr;
		tmp += 4;
		lseek(g_savefd, 0, SEEK_SET);
		write(g_savefd, tmp, g_hdr->size);
		close(g_savefd);
	}

	return 0;
}

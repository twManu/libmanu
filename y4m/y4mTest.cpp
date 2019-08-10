#include "y4mLib.h"
#include <unistd.h>
#include <string.h>

static char *g_file;
static int g_debugLevel;
static int g_saveFrameNr;
//char *g_file = (char *)"/home/manu/Downloads/ducks_take_off_444_720p50.y4m";
//char *g_file = (char *)"/home/manu/Downloads/in_to_tree_422_720p50.y4m"

static void usage() {
	printf("read y4m file\n");
	printf("Usage: y4mTest [-f inFile [-v level] [-s frameNr]\n");
	printf("      inFile - source file\n");
	printf("      level - debug level, default 0\n");
	printf("      frameNr - save specific 1-based frame\n");
	exit(-1);
}


static bool checkParam(int argc, char *argv[])
{
	int opt;

	while( (opt = getopt(argc, argv, "hf:v:s:")) != -1) {
		switch (opt) {
		case 'f':
			g_file = optarg;
			break;
		case 'v':
			g_debugLevel = atoi(optarg);
			break;
		case 's':
			g_saveFrameNr = atoi(optarg);
			break;
		default:
			usage();
		}
	}

	return true;
}


int main(int argc, char *argv[])
{
	cY4M *obj;
	char fname[64], *tmp, *buf=NULL;
	struct_y4m_param param;
	int frame_nr = 0;
	FILE *outFile;

	if( !checkParam(argc, argv) )
		return -1;
	obj = new cY4M(g_debugLevel);
	if( !obj ) {
		printf("fail to allocate object\n");
		return -1;
	}
	
	if( obj->init(g_file, param) ) {
		if( g_saveFrameNr ) {
			buf = (char *)malloc(param.frm_size);
			if( !buf ) {
				printf("fail to allocate save buffer\n");
				goto done;
			}
			do {
				frame_nr = obj->getFrame(buf);
				if( !frame_nr ) break;
				//this to save?
				if( frame_nr==g_saveFrameNr ) {
					tmp = strtok(g_file, ".");
					sprintf(fname, "%s-%d.yuv", tmp, frame_nr);
					printf("saving %s\n", fname);
					outFile = fopen(fname, "w");
					if( !outFile )
						printf("fail to create file\n");
					else {
						fwrite(buf, param.frm_size, 1, outFile);
						fclose(outFile);
					}
				}
			} while( 1 );
			free(buf);
		} else {
			do {
				frame_nr = obj->getFrame(NULL);
				if( !frame_nr ) break;
			} while( 1 );
		}
	}
done:
	delete obj;
	return 0;
}

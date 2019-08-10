#include "y4mLib.h"
#include <unistd.h>

static char *g_file;
static int g_debugLevel;
//char *g_file = (char *)"/home/manu/Downloads/ducks_take_off_444_720p50.y4m";
//char *g_file = (char *)"/home/manu/Downloads/in_to_tree_422_720p50.y4m"

static void usage() {
	printf("read y4m file\n");
	printf("Usage: y4mTest [-f inFile [-v level]\n");
	printf("      inFile - source file\n");
	printf("      level - debug level, default 0\n");
	exit(-1);
}


static bool checkParam(int argc, char *argv[])
{
	int opt;

	while( (opt = getopt(argc, argv, "hf:v:")) != -1) {
		switch (opt) {
		case 'f':
			g_file = optarg;
			break;
		case 'v':
			g_debugLevel = atoi(optarg);
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
	struct_y4m_param param;
	int frame_nr = 0;

	if( !checkParam(argc, argv) )
		return -1;
	obj = new cY4M(g_debugLevel);
	if( !obj ) {
		printf("fail to allocate object\n");
		return -1;
	}
	if( obj->init(g_file, param) ) {
		do {
			frame_nr = obj->getFrame(NULL);
			if( !frame_nr ) break;
			if( 0 /*manutest g_debugLevel */ )
				printf("frame %d got\n", frame_nr);
		} while( 1 );
	}
	delete obj;
	return 0;
}

#include "FileDump.h"
#include <stdio.h>
#include <stdlib.h>

const char *g_savefile = NULL;


void usage() {
	printf("Analyze header of a dump file\n");
	printf("Usage: FileDumpRead [-d SAVE_FILE]\n");
	printf("      SAVE_FILE - file name\n");
	exit(-1);
}


bool checkParam(int argc, char *argv[])
{
	int opt;

	while( (opt = getopt(argc, argv, "hd:")) != -1) {
		switch (opt) {
		case 'd':
			g_savefile = optarg;
			break;	
		default:
			usage();
		}
	}

	return true;
}


int main(int argc, char **argv)
{
	FileDump *dump = NULL;
	int hdrSize, maxDataSize, count;

	checkParam(argc, argv);
	if( g_savefile ) {
		dump = new FileDump(g_savefile, FileStream::BM_RDONLY);
		if( dump ) {
			if( dump->open() ) {
				dump->readHdr(hdrSize, maxDataSize, count);
				printf("%d, %d, %d\n", hdrSize, maxDataSize, count);
				dump->close();
			}
			delete dump;
			dump = NULL;
		}
	}

	return 0;
}

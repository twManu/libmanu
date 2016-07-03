#include "LinuxTCP.hpp"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define DEFAULT_SERVER_PORT	5000
static unsigned short g_ServerPort = DEFAULT_SERVER_PORT;
static unsigned int g_ServerIP = 0;         //any ip


void usage() {
	printf("Run as a client to get the IP:PORT back from the server\n");
	printf("Usage: client -i IP [-p PORT]\n");
	printf("       IP - interface to bind, any interface if not provided\n");
	printf("     PORT - port to listen to, default to %u\n", g_ServerPort);
	exit(-1);
}


bool checkParam(int argc, char *argv[])
{
	int opt;
	
	while( (opt = getopt(argc, argv, "hi:p:")) != -1) {
		switch (opt) {
		case 'i': {
			V4addr vaddr(optarg);
			vaddr.getAddr(&g_ServerIP, NULL);
			break;
		}
		case 'p':
			g_ServerPort = atoi(optarg);
			break;
		default:
			usage();
		}
	}
	
	if( 0==g_ServerIP || 0==g_ServerPort ) return false;
	return true;
}

#define MAX_SIZE	128
static char g_buffer[MAX_SIZE];

int main(int argc, char *argv[])
{
	if( !checkParam(argc, argv) ) {
		printf("Server IP not specified\n");
		usage();
		return -1;
	}
	LinuxTCP skt;
	V4addr serverAddr(g_ServerIP, g_ServerPort);
	int sz;
	
	if( skt.Connect(serverAddr) ) {
		skt.Read(g_buffer, 1);
		sz = g_buffer[0];                //first byte is length
		sz = skt.Read(g_buffer, sz);
		if( 0==sz ) {
			printf("Got no answer\n");
			return -1;
		} else if( sz<0 ) {
			sz *= -1;
		}
		
		g_buffer[sz] = 0;
		printf("%s\n", g_buffer);
	} else {
		string str;
		serverAddr.getIPPortString(str);
		printf("failure in connect to %s\n", str.c_str());
	}
	return 0;
}
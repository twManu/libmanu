#include "LinuxTCP.hpp"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define DEFAULT_SERVER_PORT	5000
static unsigned short g_ServerPort = DEFAULT_SERVER_PORT;
static unsigned int g_ServerIP = 0;         //any ip


void usage() {
	printf("Run as a server to response the IP:PORT back to the client\n");
	printf("Usage: server [-i IP] [-p PORT]\n");
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
	return true;
}


int main(int argc, char *argv[])
{
	checkParam(argc, argv);
	LinuxTCP skt(g_ServerIP, g_ServerPort);
	LinuxTCP *newSkt = NULL;
	V4addr newAddr;
	std::string peerIPPort, localIPPort;
	char sz;
	
	if( skt.Listen(128) ) {
		skt.getLocalAddr(newAddr);
		newAddr.getIPPortString(localIPPort);
		printf("Listen at %s\n", localIPPort.c_str());
		newSkt = dynamic_cast <LinuxTCP *> (skt.Accept());
		if( newSkt ) {
			newSkt->getPeerAddr(newAddr);
			newAddr.getIPPortString(peerIPPort);
			sz = peerIPPort.size();
			newSkt->Write(&sz, 1);    //write size and string w/o null end
			newSkt->Write((char *)peerIPPort.c_str(), sz);
			delete newSkt;
		} else printf("failure in accept\n");
	} else printf("failure in listen\n");
	return 0;
}
#include <stdlib.h>
#include <stdio.h>

#define D4_N            10000

int main(int argc, char *argv[])
{

	int i, endN=D4_N;

	if( argc>1 ) endN=atoi(argv[1]);
	for( i=1; i<endN; ++i ) {
		if( i%3 ) continue;              //mod 3 = 0
		if( 1!=(i&0x03) ) continue;      //mod 4 = 1
		if( 1!=(i%5) ) continue;         //mod 5 = 1
		if( 3!=(i%6) ) continue;         //mod 6 = 3
		if( i%7 ) continue;              //mod 7 = 0
		if( 1!=(i%8) ) continue;         //mod 8 = 1
		if( i%9 ) continue;              //mod 9 = 0
		printf("%d ", i);
	}


	return 0;
}



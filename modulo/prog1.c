#include <stdlib.h>
#include <stdio.h>

#define D4_N            10000

int main(int argc, char *argv[])
{

	int i, endN=D4_N;
	int ansCount=0;

	if( argc>1 ) endN=atoi(argv[1]);
	for( i=1; i<endN; ++i ) {
		if( 1!=(i&0x7) ) continue;       //mod 8 = 1, 4=1
		if( i%(7*9) ) continue;          //mod 9 = 0, 3=0, mod 7=0
		if( 1!=(i%5) ) continue;         //mod 5 = 1
		if( 3!=(i%6) ) continue;         //mod 6 = 3
		//printf("%d ", i);
		++ansCount;
	}
	if( ansCount ) printf("%d answers found\n", ansCount);
	else printf("Fails to find any answer\n");


	return 0;
}



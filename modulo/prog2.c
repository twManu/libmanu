#include <stdlib.h>
#include <stdio.h>

static struct {
	int modulo;
	int residule;
} g_data[] = {
	  {2, 1}
	, {3, 0}
	, {4, 1}
	, {5, 1}
	, {6, 3}
	, {7, 0}
	, {8, 1}
	, {9, 0}
};

#define SIZE_DATA	( (sizeof(g_data))/(sizeof(g_data[0])) )
#define D4_N            10000

int main(int argc, char *argv[])
{

	int i, endN=D4_N;
	int loop;


	if( argc>1 ) endN=atoi(argv[1]);

	for( i=1; i<endN; ++i ) {
		for( loop=0; loop<SIZE_DATA; ++loop )
			if( g_data[loop].residule != (i%g_data[loop].modulo) )
				break;
		if( SIZE_DATA==loop ) printf("%d ", i);
	}

	return 0;
}


#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#define D4_N            10000
#define SIZE_DATA	( (sizeof(g_data))/(sizeof(g_data[0])) )

static int g_maxValue = D4_N;                    //maximum to check with
static int g_verbose = 0;                        //not to show answer

static struct {
	int modulo;
	int residule;
} g_data[] = {
	  {8, 1}
	, {63, 0}
	, {5, 1}
	, {6, 3}
};


static void usage() {
	printf("Find out values smaller than maxValue with given criteria\n");
	printf("Usage: prog [-m MAX_VALUE][-v]\n");
	printf("   MAX_VALUE - default to %d\n", D4_N);
	printf("   v - verbose answers\n");
	exit(-1);
}


/*
 * Check if parameters valid
 * In  : argc and argv as main
 * Out : g_MaxValue
 *       g_verbose
 * Ret : 1 - successful
 *       0 - failure
 */
static int checkParam(int argc, char *argv[])
{
	int opt;

	while( (opt = getopt(argc, argv, "m:vh")) != -1) {
		switch (opt) {
		case 'm':
			g_maxValue = atoi(optarg);
			break;
		case 'v':
			g_verbose = 1;
			break;
		default:
			usage();
		}
	}

	if( g_maxValue<=0 ) {
		printf("Invalid maximal value %d provided\n", g_maxValue);
		return 0;
	}

	return 1;
}



int main(int argc, char *argv[])
{

	int i, ansCount=0;
	int loop;


	if( !checkParam(argc, argv) ) exit(-1);
	for( i=1; i<g_maxValue; ++i ) {
		for( loop=0; loop<SIZE_DATA; ++loop )
			if( g_data[loop].residule != (i%g_data[loop].modulo) )
				break;
		if( SIZE_DATA==loop ) {
			if( g_verbose ) printf("%d ", i);
			++ansCount;
		}
	}

        if( ansCount ) printf("%d answers found\n", ansCount);
        else printf("Fails to find any answer\n");

	return 0;
}


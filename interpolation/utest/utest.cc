#include "../interpolate.hpp"
#include <stdlib.h>
#include <stdio.h>

static short sndL[] = { 0, 1, 2, 3, 4, 5};
static short sndR[] = { 10, 11, 12, 13, 14, 15};
static short sndLR[] = {
	0, 10,
	1, 11,
	2, 12,
	3, 13,
	4, 14,
	5, 15
};

static short data[20];

int main()
{
	interpolate<short> intplt;
	
	intplt.resample(6, sndL, 5, data, interpolate<short>::CHANNEL_L);
	printf("Left: ");
	for( int i=0; i<5; ++i ) printf("%d ", data[i]);
	printf("\n");

	
	intplt.resample(6, sndR, 5, data, interpolate<short>::CHANNEL_R);
	printf("Right: ");
	for( int i=0; i<5; ++i ) printf("%d ", data[i]);
	printf("\n");
	
	intplt.resetLastData();
	intplt.resample(6, sndLR, 5, data);
	for( int i=0; i<10; ++i ) printf("%d ", data[i]);
	printf("\n");
	
	return 0;
}
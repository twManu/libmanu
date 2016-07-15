#include <stdio.h>
#include <stdlib.h>

#define ALL_1	((unsigned int)-1)

int main()
{
	unsigned int bm = ((unsigned int)(ALL_1 ^ (ALL_1>>1)));

	printf("0x%x\n", ALL_1);
	printf("0x%x\n", bm);
	return 0;
}

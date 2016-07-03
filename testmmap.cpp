#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char** argv)
{
	char *buf;

	printf("Before allocating 4K buffer, press ENTER to continue ... ");
	getchar();
	buf = (char *) malloc(4096);
	if( !buf ) {
		printf("Fails to allocate buffer\n");
		return 0;
	}
	printf("After allocating 4K buffer, press ENTER exit ... ");
	getchar();
	free(buf);
	return 0;
}


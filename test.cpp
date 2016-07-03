#include <stdio.h>
#include <stdlib.h>
#include "OSLinux.h"
#include "BufferList.h"


#define	BUF_COUNT         10
int main(int argc, char *argv[])
{
	BufferList *list;
	Buffer *buffer;
	LinuxLock *flock, *ulock;
	int i, *iptr;

	flock = new LinuxLock;
	if( !flock ) {
		printf("fail to allocate flock\n");
		return -1;
	}
	ulock = new LinuxLock;
	if( !ulock ) {
		printf("fail to allocate ulock\n");
		goto err_free_flock;
	}


	list = new BufferList();
	if( !list ) {
		printf("fail to allocate list\n");
		goto err_free_ulock;
	}
	if( !list->Init(BUF_COUNT, 1024, flock, ulock) ) {
		printf("fail to init list\n");
		goto err_free_list;
	}
	//producer
	for( i=0; i<BUF_COUNT; ++i ) {
		buffer = list->GetFree(0, NULL);
		if( buffer ) {
			iptr = (int *) buffer->GetData();
			*iptr = 10-i;
			list->PutUsed(buffer, 4);
		}
	}

	//consumer
	for( i=0; i<BUF_COUNT; ++i ) {
		buffer = list->GetUsed(0, NULL);
		if( buffer ) {
			iptr = (int *) buffer->GetData();
			printf("%d-st iteration got %d\n", i, *iptr);
			list->PutFree(buffer);
		}
	}

err_free_list:
	delete list;
err_free_ulock:
	delete ulock;
err_free_flock:
	delete flock;
	return 0;
}

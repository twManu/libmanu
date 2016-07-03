#ifndef	__LIBCOMMON_H__
#define	__LIBCOMMON_H__

#include "debug.h"

#define	TEST_THEN_DELETE(obj)               \
	do {                            \
		if( obj ) {             \
			delete obj;     \
			obj = NULL;     \
		}                       \
	} while( 0 )

typedef struct val_name {
	unsigned int         val;
const	char                 *name;
} val_name_t;

#define	DEF_VAL_NAME(val)	{(unsigned int)val, #val}
#define SIZE_ARRAY(arr)	((int)(sizeof(arr)/sizeof((arr)[0])))

/*! @details
  Get bit field from network byte 'data'.
  @param start - 0 - 7
  @param length - 1-8, and start+length<=8
  @return -1 means wrong param, otherwise value shifted and returned
 */
extern unsigned short getBit(unsigned char data, unsigned char start, unsigned char length);

#endif	//__LIBCOMMON_H__
#ifndef	__DEBUG_H__
#define __DEBUG_H__

/*
 * for common debug macro
 * feature specific debug macro is defined in specific headers
 */
#ifdef	DEF_DEBUG
	#ifdef IF_LINUX_TYPE_DEBUG
		#include <stdio.h>
		#define	ERROR(fmt, arg...)     do {\
			printf(fmt, ##arg);        \
			fflush(stdout);            \
		} while (0)
	#else	//IF_LINUX_TYPE_DEBUG
		#include <stdio.h>
		#define	ERROR(fmt, ...)        do {\
			printf(fmt, __VA_ARGS__);  \
			fflush(stdout);            \
		} while (0)
	#endif	//IF_LINUX_TYPE_DEBUG
#else	//DEF_DEBUG
	#ifdef IF_LINUX_TYPE_DEBUG
		#define	ERROR(arg...)	do {} while( 0 )
	#else	//IF_LINUX_TYPE_DEBUG
		#define	ERROR(...)	do {} while( 0 )
	#endif	//IF_LINUX_TYPE_DEBUG
#endif	//DEF_DEBUG

#endif	//__DEBUG_H__

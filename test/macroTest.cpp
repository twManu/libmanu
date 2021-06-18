#include <stdio.h>
#include <stdlib.h>

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A) STRINGIZE_NX(A)
#define T1 s
#define T2 1


#define checkFree(ptr, func)   do {\
	if( ptr ) {\
		STRINGIZE_NX(func(ptr));\
		ptr = NULL;\
	}\
} while( 0 )

int main(int argc, char *argv[])
{
	argc = argc;
	argv = argv;
#if 1
	const char *str1=STRINGIZE(PPCAT(T1, T2));
	//const char *str1="s1";
	
	const char *str2=STRINGIZE(PPCAT_NX(T1, T2));
	//const char str2="T1T2";
	
	const char *str3=STRINGIZE_NX(PPCAT_NX(T1, T2));
	//const char str3="PPCAT_NX(T1, T2)";

	#define T1T2 visit the zoo
	const char *str4=STRINGIZE(PPCAT_NX(T1, T2));
       	//const char *str4="visit the zoo";

#else
	//hundred elements of size 4
	char *buffer = (char *)calloc(4, 100);
	checkFree(buffer, "free");
#endif
	return 0;
}

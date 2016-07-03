#include <stdlib.h>
#include <stdio.h>


#define BYTE_0		(0xF0)
#define BYTE_1		(0xF1)
#define BYTE_2		(0xF2)
#define BYTE_3		(0x43)
#define LE_VALUE	(BYTE_0|(BYTE_1<<8)|(BYTE_2<<16)|(BYTE_3<<24))
#define BE_VALUE	(BYTE_3|(BYTE_2<<8)|(BYTE_1<<16)|(BYTE_0<<24))


void shift_test()
{
	char sc = BYTE_1;
	int si;
	unsigned int ui;

	si = sc>>1;
	printf("Assign right shifted char 0x%02x(%d) to int got 0x%08x(%d)\n", sc&0xff, sc&0xff, si, si);
	
	/**** shift -> assign ****/
	si = sc<<1;
	printf("Assign left shifted char 0x%02x(%d) to int got 0x%08x(%d)\n", sc&0xff, sc&0xff, si, si);

	ui = sc>>1;
	printf("Assign right shifted char 0x%02x(%d) to unsigned int got 0x%08x(%u)\n", sc&0xff, sc&0xff, ui, ui);
	
	ui = sc<<1;
	printf("Assign left shifted char 0x%02x(%d) to unsigned int got 0x%08x(%u)\n", sc&0xff, sc&0xff, ui, ui);

	sc = BYTE_3;
	si = sc>>1;
	printf("Assign right shifted char 0x%02x(%d) to int got 0x%08x(%d)\n", sc&0xff, sc&0xff, si, si);
	
	si = sc<<1;
	printf("Assign left shifted char 0x%02x(%d) to int got 0x%08x(%d)\n", sc&0xff, sc&0xff, si, si);

	ui = sc>>1;
	printf("Assign right shifted char 0x%02x(%d) to unsigned int got 0x%08x(%u)\n", sc&0xff, sc&0xff, ui, ui);
	
	ui = sc<<1;
	printf("Assign left shifted char 0x%02x(%d) to unsigned int got 0x%08x(%u)\n", sc&0xff, sc&0xff, ui, ui);
	printf("\n");
}


void endian_test()
{
	unsigned char a_uc[4] = {
		  BYTE_0
		, BYTE_1
		, BYTE_2
		, BYTE_3
	};
	unsigned int *p_ui = (unsigned int *) a_uc;


	switch( p_ui[0] ) {
	case BE_VALUE:
		printf("System is Big Endian, ");
		break;
	case LE_VALUE:
		printf("System is Little Endian, ");
		break;
	default:
		printf("System Endianess unknown, ");
		break;
	}
}


void assignment_test()
{
	unsigned int ui;
	int si;
	unsigned char uc=BYTE_1;
	char sc=BYTE_1;

	ui = uc;
	printf("Assign unsigned char 0x%02x(%u) to unsigned int got 0x%08x(%u)\n", uc&0xff, uc&0xff, ui, ui);

	ui = sc;
	printf("Assign char 0x%02x(%u) to unsigned int got 0x%08x(%u)\n", sc&0xff, sc&0xff, ui, ui);

	si = uc;
	printf("Assign unsigned char 0x%02x(%d) to int got 0x%08x(%d)\n", uc&0xff, uc&0xff, si, si);

	si = sc;
	printf("Assign char 0x%02x(%d) to int got 0x%08x(%d)\n", sc&0xff, sc&0xff, si, si);

	si = -2;
	ui = si;
	printf("Assign -2 to unsigned int got 0x%08x(%u)\n", ui, ui);
	printf("\n");
}


void operation_test()
{
	unsigned char uc0=BYTE_0;
	unsigned char uc1=BYTE_1;
	unsigned char uc2=BYTE_2;
	unsigned char uc3=BYTE_3;
	char c0=BYTE_0;
	char c1=BYTE_1;
	char c2=BYTE_2;
	char c3=BYTE_3;
	int si1;
	unsigned int ui1;

	si1 = uc0|uc1<<8|uc2<<16|uc3<<24;
	printf("Assign \"uc0|uc1<<8|uc2<<16|uc3<<24\" to an integer got 0x%08x\n", si1);

	si1 = uc0+(uc1<<8)+(uc2<<16)+(uc3<<24);
	printf("Assign \"uc0+\(uc1<<8)+\(uc2<<16)+\(uc3<<24)\" to an integer got 0x%08x\n", si1);

	si1 = uc0+uc1+uc2+(uc3>>1);
	printf("Assign \"uc0+uc1+uc2+\(uc3>>1)\" to an integer got 0x%08x\n", si1);

	si1 = uc0|(uc1*256)|(uc2*256*256)|(uc3*256*256*256);
	printf("Assign \"uc0|\(uc1*256)|\(uc2*256*256)|\(uc3*256*256*256)\" to an integer got 0x%08x\n", si1);

	si1 = uc0+(uc1*256)+(uc2*256*256)+(uc3*256*256*256);
	printf("Assign \"uc0+\(uc1*256)+\(uc2*256*256)+\(uc3*256*256*256)\" to an integer got 0x%08x\n", si1);

	/**** promotion -> OR'ed ****/
	si1 = c0|c1|c2|c3;
	printf("Assign \"c0|c1|c2|c3\" to an integer got 0x%08x\n", si1);

	/**** promotion -> shift -> OR'ed ****/
	si1 = c3|(c2<<8)|(c1<<16)|(c0<<24);
	printf("Assign \"c3|c2<<8|c1<<16|c0<<24\" to an integer got 0x%08x\n", si1);

	/**** promotion -> shift -> add ****/
	si1 = c0+(c1<<8)+(c2<<16)+(c3<<24);
	printf("Assign \"c0+\(c1<<8)+\(c2<<16)+\(c3<<24)\" to an integer got 0x%08x\n", si1);

	si1 = c0+c1+c2+(c3>>1);
	printf("Assign \"c0+c1+c2+\(c3>>1)\" to an integer got 0x%08x\n", si1);

	si1 = c0|(c1*256)|(c2*256*256)|(c3*256*256*256);
	printf("Assign \"c0|\(c1*256)|\(c2*256*256)|\(c3*256*256*256)\" to an integer got 0x%08x\n", si1);

	si1 = c0+(c1*256)+(c2*256*256)+(c3*256*256*256);
	printf("Assign \"c0+\(c1*256)+\(c2*256*256)+\(c3*256*256*256)\" to an integer got 0x%08x\n", si1);

	printf("\n\n");
	ui1 = uc0|uc1<<8|uc2<<16|uc3<<24;
	printf("Assign \"uc0|uc1<<8|uc2<<16|uc3<<24\" to an unisgned integer got 0x%08x\n", ui1);

	ui1 = uc0+(uc1<<8)+(uc2<<16)+(uc3<<24);
	printf("Assign \"uc0+\(uc1<<8)+\(uc2<<16)+\(uc3<<24)\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = uc0+uc1+uc2+(uc3>>1);
	printf("Assign \"uc0+uc1+uc2+\(uc3>>1)\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = uc0|(uc1*256)|(uc2*256*256)|(uc3*256*256*256);
	printf("Assign \"uc0|\(uc1*256)|\(uc2*256*256)|\(uc3*256*256*256)\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = uc0+(uc1*256)+(uc2*256*256)+(uc3*256*256*256);
	printf("Assign \"uc0+\(uc1*256)+\(uc2*256*256)+\(uc3*256*256*256)\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = c0|c1<<8|c2<<16|c3<<24;
	printf("Assign \"c0|c1<<8|c2<<16|c3<<24\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = c0+(c1<<8)+(c2<<16)+(c3<<24);
	printf("Assign \"c0+\(c1<<8)+\(c2<<16)+\(c3<<24)\" to an unsigned  integer got 0x%08x\n", ui1);

	ui1 = c0+c1+c2+(c3>>1);
	printf("Assign \"c0+c1+c2+\(c3>>1)\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = c0|(c1*256)|(c2*256*256)|(c3*256*256*256);
	printf("Assign \"c0|\(c1*256)|\(c2*256*256)|\(c3*256*256*256)\" to an unsigned integer got 0x%08x\n", ui1);

	ui1 = c0+(c1*256)+(c2*256*256)+(c3*256*256*256);
	printf("Assign \"c0+\(c1*256)+\(c2*256*256)+\(c3*256*256*256)\" to an unsigned integer got 0x%08x\n", ui1);
}


void print_info()
{
#if	defined(__GNUC__)
	printf("GCC v%d.%d\n", __GNUC__, __GNUC_MINOR__);
#else	//defined(__GNUC__)
	printf("Not built by a gcc compiler\n");
#endif	//defined(__GNUC__)
	endian_test();
	printf("LE value=0x%08x, BE value=0x%08x\n\n", LE_VALUE, BE_VALUE);
}


int main()
{
	print_info();
	assignment_test();
	shift_test();
	operation_test();
	return 0;
}

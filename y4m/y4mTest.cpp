#include "y4mLib.h"

char *g_file = (char *)"/home/manu/Downloads/ducks_take_off_444_720p50.y4m";
//char *g_file = (char *)"/home/manu/Downloads/in_to_tree_422_720p50.y4m"

int main(int argc, char *argv[])
{
	cY4M obj;
	struct_y4m_param param;
	int frame_nr = 0;

	if( obj.init(g_file, param) ) {
		do {
			frame_nr = obj.getFrame(NULL);
			if( !frame_nr ) break;
			printf("frame %d got\n", frame_nr);
		} while( 1 );
	}
	return 0;
}

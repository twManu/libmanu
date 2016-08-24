#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int main(void) {
    char c;
    static struct termios oldtio, newtio;
    tcgetattr(0, &oldtio);
    newtio = oldtio;
    newtio.c_lflag &= ~ICANON;
    newtio.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &newtio);

    printf("Give text:\n");
    fflush(stdout);
    while (1) {
        c = getchar();

        // is this an escape sequence?
        if (c == 27) {
            // "throw away" next two characters which specify escape sequence
            c = getchar();
            c = getchar();
		switch( c ) {
		case 'A':
			printf("Up\n");
			break;
		case 'B':
			printf("Down\n");
			break;
		case 'C':
			printf("Left\n");
			break;
		case 'D':
			printf("Right\n");
			break;
		}

            continue;
        }

        // if backspace
        if (c == 0x7f) {
            // go one char left
            printf("\b");
            // overwrite the char with whitespace
            printf(" ");
            // go back to "now removed char position"
            printf("\b");
            continue;
        }

        if (c == 'q') {
            break;
        }
        printf("%c", c);
    }
    printf("\n");
    tcsetattr(0, TCSANOW, &oldtio);

    return 0;
}


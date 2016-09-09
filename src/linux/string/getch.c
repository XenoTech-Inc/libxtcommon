// XT headers
#include <xt/string.h>

// System headers
#include <termios.h> // termios functions
#include <unistd.h> // STDIN_FILENO

// STD headers
#include <stdio.h> // getchar

int xtGetch(void)
{
	struct termios oldattr, newattr;
	int ch;
	tcgetattr(STDIN_FILENO, &oldattr);
	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
	return ch;
}

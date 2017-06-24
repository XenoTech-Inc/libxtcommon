// XT headers
#include <xt/os.h>
#include <xt/error.h>

#include <ctype.h>
#include <string.h>

int xtConsolePutString(const char *str, unsigned textWidth, bool hardSplit, unsigned alignment)
{
	int retval;
	unsigned cols;
	size_t size, rowsize, align;
	unsigned char *ptr, *next;

	retval = xtConsoleGetSize(&cols, NULL);

	if (retval)
		return retval;

	size = strlen(str);

	/* Check if specified alignment is correct */
	switch (alignment) {
	case XT_CONSOLE_ALIGN_LEFT:
	case XT_CONSOLE_ALIGN_CENTER:
	case XT_CONSOLE_ALIGN_RIGHT:
		break;
	default:
		return XT_EINVAL;
	}

	if (!textWidth)
		textWidth = cols;

	/* Check if the string is going to fit */
	if (size > textWidth)
		for (rowsize = 0, ptr = (unsigned char*)str; *ptr; ++ptr) {
			if (isspace(*ptr) || *ptr == '\n')
				rowsize = 0;
			else if (++rowsize >= textWidth && !hardSplit)
				return XT_EMSGSIZE;
		}

	for (ptr = (unsigned char*)str, next = ptr; *ptr; next = ptr) {
		for (rowsize = 0; *next; ++rowsize, ++next) {
			if (*next == '\n')
				break;

			if (rowsize >= textWidth) {
				if (!hardSplit)
					while (!isspace(*next))
						--next;
				break;
			}
		}

		switch (alignment) {
		case XT_CONSOLE_ALIGN_LEFT:
			align = 0;
			break;
		case XT_CONSOLE_ALIGN_CENTER:
			align = (cols - rowsize) / 2;
			break;
		case XT_CONSOLE_ALIGN_RIGHT:
			align = cols - rowsize;
			break;
		}

		while (align --> 0)
			putchar(' ');

		/* Print row */
		while (ptr < next)
			putchar(*ptr++);

		putchar('\n');

		while (isspace(*ptr))
			++ptr;
	}

	return 0;
}

// XT headers
#include <xt/string.h>

// STD headers
#include <stdarg.h>
#include <stdio.h>

int _xt_vsnprintf(char *str, size_t size, char *format, char *end, va_list args)
{
	(void)end;
	int ret = 0;
	// TODO interpret %zu and %zd
	ret = vsnprintf(str, size, format, args);
	// proper NULL termination...
	if (size)
		str[size - 1] = '\0';
	return ret;
}

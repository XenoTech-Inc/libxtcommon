// STD headers
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int _xtvsnprintf(char *str, size_t size, char *format, char *end, va_list args)
{
	(void)end;
	return vsnprintf(str, size, format, args);
}

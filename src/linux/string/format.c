// XT headers
#include <xt/string.h>

// STD headers
#include <stdarg.h>
#include <stdio.h>

int _xt_vsnprintf(char *str, size_t size, char *format, char *end, va_list args)
{
	(void)end;
	return vsnprintf(str, size, format, args);
}

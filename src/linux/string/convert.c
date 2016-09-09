// XT headers
#include <xt/string.h>

// STD headers
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int xtCharToDigit(char c)
{
	return ((c >= '0' && c <= '9') ? c - '0' : 10);
}

char *xtInt64ToStr(int64_t value, char *buf, size_t buflen)
{
	snprintf(buf, buflen, "%" PRId64, value);
	return buf;
}

char *xtUint64ToStr(uint64_t value, char *buf, size_t buflen)
{
	snprintf(buf, buflen, "%" PRIu64, value);
	return buf;
}

char *xtSizetToStr(size_t value, char *buf, size_t buflen)
{
	snprintf(buf, buflen, "%zu", value);
	return buf;
}

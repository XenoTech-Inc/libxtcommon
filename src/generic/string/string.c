// STD headers
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *xtFormatBytesSI(char *restrict buf, size_t buflen, uint64_t value, unsigned decimals, bool strictBinary, unsigned *restrict base)
{
	const char *siBaseStr = " KMGTPE";
	const char *si = siBaseStr;
	size_t d = value;
	unsigned npow = 0;
	unsigned siBase = strictBinary ? 1024 : 1000;
	while (d >= siBase) {
		d /= siBase;
		++si;
		++npow;
	}
	if (!decimals || !npow) {
		if (npow) {
			const char *format = strictBinary ? "%zu%ciB" : "%zu%cB";
			snprintf(buf, buflen, format, d, *si);
		} else
			snprintf(buf, buflen, "%zuB", d);
	} else {
		char sbuf[32];
		const char *format = strictBinary ? "%%.0%ulf%%ciB" : "%%.0%ulf%%cB";
		snprintf(sbuf, sizeof sbuf, format, decimals);
		double v = value;
		for (unsigned i = 0; i < npow; ++i)
			v /= siBase;
		snprintf(buf, buflen, sbuf, v, *si);
	}
	if (base)
		*base = (unsigned) (si - siBaseStr);
	return buf;
}

char *xtFormatCommasLLU(char *buf, size_t buflen, unsigned long long value, int sep)
{
	int n = 3; // Format every thousand
	char *p = buf;
	uint64_t i;
	int j = 0;
	i = value;
	do {
		++p;
		if (++j == n && i > 10) {
			j = 0;
			++p;
		}
		i /= 10;
	} while (i);
	// overflow check
	if (p + 1 > buf + buflen)
		return NULL;
	*p = '\0';
	j = 0;
	do {
		*--p = '0' + (value % 10);
		if (++j == n && value > 10) {
			j = 0;
			*--p = sep;
		}
		value /= 10;
	} while (value);
	return buf;
}

char *xtFormatCommasLL(char *buf, size_t buflen, long long value, int sep)
{
	if (!buflen)
		return NULL;
	if (value < 0) {
		buf[0] = '-';
		value = llabs(value);
		return xtFormatCommasLLU(buf + 1, buflen - 1, value, sep);
	} else
		return xtFormatCommasLLU(buf, buflen, value, sep);
}

char *xtFormatHex(char *restrict buf, size_t buflen, const void *restrict data, size_t datalen, int sep, bool uppercase)
{
	if (!buflen)
		return NULL;
	const char *hex = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	const char *ptr = data;
	for (size_t i = 0, j = 0; i < buflen && j < datalen; i += 3, ++j) {
		buf[i + 0] = hex[(ptr[j] >> 4) & 0xf];
		buf[i + 1] = hex[ ptr[j]       & 0xf];
		buf[i + 2] = sep;
	}
	size_t max = buflen - 1 > 3 * datalen ? 3 * datalen : buflen - 1;
	buf[3 * (max / 3) - 1] = '\0';
	return buf;
}

void xtRot13(void *buf, size_t buflen)
{
	unsigned char *xbuf = buf;
	for (size_t i = 0; i < buflen; ++i) {
		if (xbuf[i] >= 'a' && xbuf[i] <= 'z')
			xbuf[i] = (xbuf[i] - 'a' + 13) % 26 + 'a';
		else if (xbuf[i] >= 'A' && xbuf[i] <= 'Z')
			xbuf[i] = (xbuf[i] - 'A' + 13) % 26 + 'A';
	}
}

bool xtStringContainsLen(const char *restrict haystack, const char *restrict needle, size_t haystackLen)
{
	size_t needleLen = strlen(needle);
	for (size_t i = 0, j = 0; i < haystackLen && j < needleLen; ++i) {
		if (needle[j] == haystack[i])
			++j;
		else
			j = 0;
		if (j == needleLen)
			return true;
	}
	return false;
}

bool xtStringContains(const char *restrict haystack, const char *restrict needle)
{
	return xtStringContainsLen(haystack, needle, strlen(haystack));
}


bool xtStringEndsWith(const char *restrict haystack, const char *restrict needle)
{
	size_t needleLen = strlen(needle), haystackLen = strlen(haystack);
	if (needleLen > haystackLen)
		return false;
	return strncmp(haystack + haystackLen - needleLen, needle, needleLen) == 0;
}

char *xtStringReadLine(char *restrict str, size_t num, size_t *restrict bytesRead, FILE *restrict f)
{
	register int c = '\0';
	register char *cs = str;
	while (--num > 0 && (c = getc(f)) != EOF) {
		if (c == '\n') {
			// Non-Linux line endings
			if (cs[-1] == '\r')
				--cs;
			break;
		}
		*cs++ = c;
	}
	if (bytesRead)
		*bytesRead = cs - str;
	*cs = '\0';
	return (c == EOF && cs == str) ? NULL : str;
}

char *xtStringReverseLen(char *str, size_t len)
{
	char *begin = str, *end = str + len, tmp;
	for (--end; begin < end; ++begin, --end) {
		tmp = *begin;
		*begin = *end;
		*end = tmp;
	}
	return str;
}

char *xtStringReverse(char *str)
{
	return xtStringReverseLen(str, strlen(str));
}

void xtStringSplit(char *restrict str, const char *restrict delim, char **restrict tokens, unsigned *restrict num)
{
#define strtok_s strtok_r
	char *save_ptr, *token = strtok_r(str, delim, &save_ptr);
	unsigned i = 0;
	for (; i < *num && token; ++i) {
		tokens[i] = token;
		token = strtok_r(save_ptr, delim, &save_ptr);
	}
	*num = i;
#undef strtok_r
}

bool xtStringStartsWith(const char *restrict haystack, const char *restrict needle)
{
	return strncmp(haystack, needle, strlen(needle)) == 0;
}

char *xtStringToLower(char *str)
{
	for (size_t i = 0; str[i] != '\0'; ++i)
		str[i] = tolower(str[i]);
	return str;
}

char *xtStringToUpper(char *str)
{
	for (size_t i = 0; str[i] != '\0'; ++i)
		str[i] = toupper(str[i]);
	return str;
}

char *xtStringReplaceAll(char *str, const char org, const char replacer)
{
	size_t len = strlen(str);
	for (size_t i = 0; i < len; ++i) {
		if (str[i] == org)
			str[i] = replacer;
	}
	return str;
}

char *xtStringTrim(char *str)
{
	if (!*str)
		return str;
	size_t oldstrlen = strlen(str);
	// First look where the last REAL character is
	char *lastchar = &str[0]; // Just init this for safety
	for (size_t i = oldstrlen; i > 0; --i) {
		// Good enough, just assume the rest is real data
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\0') {
			lastchar = &str[i];
			break;
		}
	}
	// Now find the start of the first character!
	char *p = str;
	while ((*p == ' ' || *p == '\t') && *p != '\0') ++p;
	// Unfortunately we do have to make a seperate buffer...
	size_t newlen = lastchar - p + 2;
	char *buf = malloc(newlen);
	if (!buf)
		return str;
	// First copy it to the new buffer...
	memmove(buf, p, newlen);
	// Then copy it back to the old buffer
	memmove(str, buf, newlen);
	str[newlen - 1] = '\0';
	free(buf);
	return str;
}

char *xtStringTrimWords(char *str)
{
	char *p, *q;
	p = q = str;
	// skip leading spaces
	while (isspace(*p)) ++p;
	for (; *p; ++p) {
		*q++ = isspace(*p) ? ' ' : *p;
		if (!isspace(*p))
			continue;
		// skip remaining spaces
		if (!*++p) {
			q[-1] = '\0';
			break;
		}
		while (isspace(*p)) ++p;
		*q++ = isspace(*p) ? ' ' : *p;
	}
	*q = '\0';
	return str;
}

// STD headers
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *xtFormatCommasLLU(unsigned long long v, char *buf, int sep);
char *xtFormatCommasLL(long long v, char *buf, int sep)
{
	if (v < 0) {
		buf[0] = '-';
		v = llabs(v);
		return xtFormatCommasLLU(v, buf + 1, sep);
	} else
		return xtFormatCommasLLU(v, buf, sep);
}

char *xtFormatCommasLLU(unsigned long long v, char *buf, int sep)
{
	int n = 3; // Format every thousand
	char *p = buf;
	uint64_t i;
	int j = 0;
	i = v;
	do {
		++p;
		if (++j == n && i > 10) {
			j = 0;
			++p;
		}
		i /= 10;
	} while (i);
	*p = '\0';
	j = 0;
	do {
		*--p = '0' + (v % 10);
		if (++j == n && v > 10) {
			j = 0;
			*--p = sep;
		}
		v /= 10;
	} while (v);
	return buf;
}

void xtRot13(void *buf, size_t buflen)
{
	unsigned char *xbuf = (unsigned char*) buf;
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


bool xtStringEndsWith(const char *haystack, const char *needle)
{
	size_t needleLen = strlen(needle);
	return strncmp(haystack + (strlen(haystack) - needleLen), needle, needleLen) == 0;
}

char *xtStringReadLine(char *restrict str, size_t num, size_t *restrict bytesRead, FILE *restrict f)
{
#if 0
	if (!fgets(buf, num, f))
		return NULL;
	size_t len = strlen(buf);
	if (bytesRead)
		*bytesRead = len;
	// Remove the stupid \n
	if (len > 0)
		buf[len - 1] = '\0';
	return buf;
#else
	register int c = '\0';
	register char *cs = str;
	while (--num > 0 && (c = getc(f)) != EOF) {
		if (c == '\n') {
			// Non-Linux line endings
			if (*cs - 1 == '\r')
				--cs;
			break;
		}
		*cs++ = c;
	}
	if (bytesRead)
		*bytesRead = cs - str;
	*cs = '\0';
	return (c == EOF && cs == str) ? NULL : str;
#endif
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

bool xtStringStartsWith(const char *haystack, const char *needle)
{
	return memcmp(haystack, needle, strlen(needle)) == 0;
}

char *xtStringToLower(char *str)
{
	for (unsigned i = 0; str[i] != '\0'; ++i)
		str[i] = (char) tolower((int) str[i]);
	return str;
}

char *xtStringToUpper(char *str)
{
	for (unsigned i = 0; str[i] != '\0'; ++i)
		str[i] = (char) toupper((int) str[i]);
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
	int oldstrlen = strlen(str);
	if (oldstrlen == 0)
		return str;
	// First look where the last REAL character is
	char *lastchar = &str[0]; // Just init this for safety
	for (int i = oldstrlen; i > 0; --i) {
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
	int newlen = lastchar - p;
	char *buf = (char*) malloc(newlen);
	if (!buf)
		return str;
	// First copy it to the new buffer...
	memmove(buf, p, newlen);
	// Then copy it back to the old buffer
	memmove(str, buf, newlen);
	str[newlen] = '\0';
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

#define skipspace(p) while(isspace(*p)) ++p;
#define revskipspace(start,p) while(p>start&&isspace(*p)) --p;
char *xtStringGetWord(const char *str, int wordindex)
{
	char *ptr = (char*) str;
	if (!ptr || !*ptr || !wordindex) return NULL;
	int index = 0;
	bool current = false, newstate;
	if (wordindex < 0) {
		// check for null string
		// backwards
		while (*ptr) ++ptr;
		// we zijn een te ver
		--ptr;
		revskipspace(str, ptr);
		while (ptr > str) {
			newstate = isspace(*ptr);
			if (newstate != current) {
				// kijk of we een nieuw woord ingaan
				if (newstate && --index == wordindex) {
					// gevonden
					skipspace(ptr);
					return ptr;
				}
				current = newstate;
			}
			--ptr;
		}
		// randgeval voor de eerste
		if (ptr == str && --index == wordindex) {
			return ptr;
		}
	} else {
		skipspace(ptr);
		if (wordindex == 1)
			return ptr;
		while (*ptr) {
			newstate = isspace(*ptr);
			if (newstate != current) {
				// kijk of we een nieuw woord ingaan
				if (!newstate && ++index == wordindex - 1) {
					// gevonden
					return ptr;
				}
				current = newstate;
			}
			++ptr;
		}
	}
	// niet gevonden
	return NULL;
}

#define NUM 16
#define MASK (NUM-1)
void fprinthex(const void *buf, FILE *f, char sep, size_t len)
{
	// Do not specify the array size already, since Visual Studio is too stupid to compile it then
	const char *hexbase = "0123456789abcdef";
	char *ptr = (char*) buf;
	fputc(hexbase[(*ptr >> 4) & MASK], f);
	fputc(hexbase[*ptr & MASK], f);
	++ptr;
	while (--len) {
		fputc(sep, f);
		fputc(hexbase[(*ptr >> 4) & MASK], f);
		fputc(hexbase[(*ptr++) & MASK], f);
	}
}

void printhex(const void *buf, char sep, size_t len)
{
	// Do not specify the array size already, since Visual Studio is too stupid to compile it then
	const char *hexbase = "0123456789abcdef";
	char *ptr = (char*) buf;
	putchar(hexbase[(*ptr >> 4) & MASK]);
	putchar(hexbase[*ptr & MASK]);
	++ptr;
	while (--len) {
		putchar(sep);
		putchar(hexbase[(*ptr >> 4) & MASK]);
		putchar(hexbase[(*ptr++) & MASK]);
	}
}
#undef MASK
#undef NUM

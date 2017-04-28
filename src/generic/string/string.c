// XT headers
#include <xt/string.h>

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

char *xtFormatTime(char *buf, size_t buflen, unsigned timestamp_secs)
{
	if (!buflen)
		return NULL;
	time_t t = timestamp_secs;
	struct tm lt;
	if (!_xtGMTime(&t, &lt) || strftime(buf, buflen, "%Y-%m-%d %H:%M:%S", &lt) == 0)
		return NULL;
	return buf;
}

char *xtFormatTimePrecise(char *buf, size_t buflen, struct xtTimestamp *timestamp)
{
	if (!xtFormatTime(buf, buflen, timestamp->sec))
		return NULL;
	char buf2[16];
	unsigned long long nanos = timestamp->nsec;
	snprintf(buf2, sizeof buf2, " %03llu:%03llu:%03llu", (nanos / 1000000LU) % 1000, (nanos / 1000LU) % 1000, nanos % 1000);
	size_t n = strlen(buf);
	if (n + 1 <= buflen) {
		strncpy(buf + n, buf2, buflen - n - 1);
		buf[buflen - 1] = '\0';
	}
	return buf;
}

unsigned xtFormatTimeDuration(char *buf, size_t buflen, const char *format, struct xtTimestamp *start, struct xtTimestamp *end)
{
	struct xtTimestamp diff;
	xtTimestampDiff(&diff, start, end);
	return xtFormatTimestamp(buf, buflen, format, &diff);
}

unsigned xtFormatTimestamp(char *buf, size_t buflen, const char *format, const struct xtTimestamp *time)
{
	char abuf[32], *g;
	unsigned mag = 0;
	char *bufend = buf + buflen, *ptr = buf;
	unsigned n = 0, rn; int nn;
	unsigned weeks, days, hours, mins, secs;
	unsigned msec, usec, nsec;
	struct xtTimestamp t = *time;
	if (t.sec) {
		if (t.sec >= XT_DATE_WEEK)
			mag = 7;
		else if (t.sec >= XT_DATE_DAY)
			mag = 6;
		else if (t.sec >= XT_DATE_HOUR)
			mag = 5;
		else if (t.sec >= XT_DATE_MIN)
			mag = 4;
		else
			mag = 3;
	} else {
		if (t.nsec >= XT_DATE_MSEC)
			mag = 2;
		else if (t.nsec >= XT_DATE_USEC)
			mag = 1;
	}
	weeks = t.sec / XT_DATE_WEEK; t.sec %= XT_DATE_WEEK;
	days  = t.sec / XT_DATE_DAY ; t.sec %= XT_DATE_DAY ;
	hours = t.sec / XT_DATE_HOUR; t.sec %= XT_DATE_HOUR;
	mins  = t.sec / XT_DATE_MIN ; t.sec %= XT_DATE_MIN ;
	secs  = t.sec;
	msec = t.nsec / XT_DATE_MSEC; t.nsec %= XT_DATE_MSEC;
	usec = t.nsec / XT_DATE_USEC; t.nsec %= XT_DATE_USEC;
	nsec = t.nsec;
	for (const char *fptr = format; *fptr; ++fptr) {
		switch (*fptr) {
		case '%':
			if (fptr[1] == '%') {
				++fptr;
				goto put;
			}
			nn = 0;
			switch (fptr[1]) {
			case 'g': {
				// tricky case, because it calls itself
				const char *ftbl[] = {
					"%n",
					"%u %n",
					"%t %u %n",
					"%s %t %u %n",
					"%m %s %t %u %n",
					"%h %m %s %t %u %n",
					"%d %h %m %s %t %u %n",
					"%w %d %h %m %s %t %u %n"
				};
				rn = xtFormatTimestamp(ptr, (size_t)(bufend - ptr), ftbl[mag], time);
				ptr += rn;
				n += rn;
				++fptr;
			}
				break;
			case 'G':
				g = abuf;
				if (weeks) { strcpy(g, "%w "); g += 3; }
				if (days ) { strcpy(g, "%d "); g += 3; }
				if (hours) { strcpy(g, "%h "); g += 3; }
				if (mins ) { strcpy(g, "%m "); g += 3; }
				if (secs ) { strcpy(g, "%s "); g += 3; }
				if (msec ) { strcpy(g, "%t "); g += 3; }
				if (usec ) { strcpy(g, "%u "); g += 3; }
				if (nsec ) { strcpy(g, "%n "); g += 3; }
				*g = '\0';
				rn = xtFormatTimestamp(ptr, (size_t)(bufend - ptr), abuf, time);
				ptr += rn;
				n += rn;
				++fptr;
				break;
			case 'W':
				nn = snprintf(abuf, sizeof abuf, "%u", weeks);
				goto append;
			case 'w':
				nn = snprintf(abuf, sizeof abuf, "%u %s", weeks, weeks == 1 ? "week" : "weeks");
				goto append;
			case 'D':
				nn = snprintf(abuf, sizeof abuf, "%u", days);
				goto append;
			case 'd':
				nn = snprintf(abuf, sizeof abuf, "%u %s", days, days == 1 ? "day" : "days");
				goto append;
			case 'H':
				nn = snprintf(abuf, sizeof abuf, "%u", hours);
				goto append;
			case 'h':
				nn = snprintf(abuf, sizeof abuf, "%u %s", hours, hours == 1 ? "hour" : "hours");
				goto append;
			case 'M':
				nn = snprintf(abuf, sizeof abuf, "%u", mins);
				goto append;
			case 'm':
				nn = snprintf(abuf, sizeof abuf, "%u %s", mins, mins == 1 ? "minute" : "minutes");
				goto append;
			case 'S':
				nn = snprintf(abuf, sizeof abuf, "%u", secs);
				goto append;
			case 's':
				nn = snprintf(abuf, sizeof abuf, "%u %s", secs, secs == 1 ? "second" : "seconds");
				goto append;
			case 'T':
				nn = snprintf(abuf, sizeof abuf, "%u", msec);
				goto append;
			case 't':
				nn = snprintf(abuf, sizeof abuf, "%u %s", msec, msec == 1 ? "millisecond" : "milliseconds");
				goto append;
			case 'U':
				nn = snprintf(abuf, sizeof abuf, "%u", usec);
				goto append;
			case 'u':
				nn = snprintf(abuf, sizeof abuf, "%u %s", usec, usec == 1 ? "microsecond" : "microseconds");
				goto append;
			case 'N':
				nn = snprintf(abuf, sizeof abuf, "%u", nsec);
				goto append;
			case 'n':
				nn = snprintf(abuf, sizeof abuf, "%u %s", nsec, nsec == 1 ? "nanosecond" : "nanoseconds");
				goto append;
			}
			break;
append:
			if (nn < 0 || ptr + nn >= bufend)
				goto end;
			strcpy(ptr, abuf);
			ptr += nn;
			n += nn;
			++fptr;
			break;
		default:
put:
			if (ptr >= bufend)
				goto end;
			++n;
			*ptr++ = *fptr;
			break;
		}
	}
end:
	if (n)
		buf[n] = '\0';
	return n;
}

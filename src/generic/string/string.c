// XT headers
#include <xt/os_macros.h>
#include <xt/error.h>
#include <xt/string.h>

// STD headers
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XT_PRINTF_BUFSZ 4096

#ifdef XT_PRINTF_DEBUG
#define dbgf(f,...) printf(f, ## __VA_ARGS__)
#define dbgs(s) puts(s)
#else
#define dbgf(f,...) ((void)0)
#define dbgs(s) ((void)0)
#endif

extern int _xt_vsnprintf(char *str, size_t size, char *format, char *end, va_list args);

int xtCharToDigit(char c)
{
	return ((c >= '0' && c <= '9') ? c - '0' : 10);
}

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
			xtsnprintf(buf, buflen, format, d, *si);
		} else
			xtsnprintf(buf, buflen, "%zuB", d);
	} else {
		char sbuf[32];
		const char *format = strictBinary ? "%%.0%ulf%%ciB" : "%%.0%ulf%%cB";
		xtsnprintf(sbuf, sizeof sbuf, format, decimals);
		double v = value;
		for (unsigned i = 0; i < npow; ++i)
			v /= siBase;
		xtsnprintf(buf, buflen, sbuf, v, *si);
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

static const unsigned char _xt_rot13tbl[256] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x41,0x42,
	0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x5b,0x5c,0x5d,0x5e,0x5f,
	0x60,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x61,0x62,
	0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
};

void xtRot13(void *buf, size_t buflen)
{
	unsigned char *xbuf = buf;
	for (size_t i = 0; i < buflen; ++i)
		xbuf[i] = _xt_rot13tbl[xbuf[i]];
}

int xtprintf(const char *format, ...)
{
	int ret = 0;
	va_list args;
	va_start(args, format);
	ret = xtvfprintf(stdout, format, args);
	va_end(args);
	return ret;
}

int xtvprintf(const char *format, va_list args)
{
	return xtvfprintf(stdout, format, args);
}

int xtfprintf(FILE *stream, const char *format, ...)
{
	int ret = 0;
	va_list args;
	va_start(args, format);
	ret = xtvfprintf(stream, format, args);
	va_end(args);
	return ret;
}

int xtvfprintf(FILE *stream, const char *format, va_list args)
{
	char sbuf[XT_PRINTF_BUFSZ], *dbuf = NULL, *buf = sbuf;
	size_t l;
	int ret = 0;
	l = strlen(format);
	if (l > XT_PRINTF_BUFSZ - 1) {
		dbuf = malloc(l + 1);
		if (!dbuf)
			goto fail;
		buf = dbuf;
	} else
		l = XT_PRINTF_BUFSZ;
	ret = xtvsnprintf(buf, l + 1, format, args);
fail:
	if (dbuf)
		free(dbuf);
	if (ret)
		fputs(buf, stream);
	return ret;
}

int xtsnprintf(char *str, size_t size, const char *format, ...)
{
	int ret = 0;
	va_list args;
	va_start(args, format);
	ret = xtvsnprintf(str, size, format, args);
	va_end(args);
	return ret;
}

int xtvsnprintf(char *str, size_t size, const char *format, va_list args)
{
	const char *fptr, *rep;
	char sbuf[XT_PRINTF_BUFSZ], *dbuf = NULL, *buf = sbuf, *ptr, *end;
	size_t l;
	int ret = 0;
	l = strlen(format);
	if (l > XT_PRINTF_BUFSZ - 1) {
		dbuf = malloc(l + 1);
		if (!dbuf)
			goto fail;
		buf = dbuf;
		end = buf + l;
	} else
		l = XT_PRINTF_BUFSZ;
	end = buf + XT_PRINTF_BUFSZ;
	// safe guard termination in case snprintf fucks up
	// (*ahum* windoze *ahum*)
	if (size)
		str[0] = str[size - 1] = '\0';
	for (ptr = buf, fptr = format; *fptr; ++fptr) {
		switch (*fptr) {
		case '%': {
			char *sub;
			int ptype = -1, mod = -1, conv = -1;
			unsigned flags = 0, prec = 0, cfw = 0;
			int fw = 0;
			const char *fstr = "#0- +'", *aptr = fptr + 1;
#ifdef XT_PRINTF_DEBUG
			const char *arg = fptr;
#endif
			const char *lmod[] = {
				"hh", "h", "ll", "l", "L", "q", "j", "z", "t",
				// XXX consider s/I/N/
				// 9    10     11     12
				"I8", "I16", "I32", "I64",
			};
			const char *cspec[] = {
				"d", "i",
				"o", "u", "x", "X",
				"e", "E",
				"f", "F",
				"g", "G",
				"a", "A",
				"c", "s",
				"C",
				"S",
				"p",
				"n",
				"m",
				"%",
			};
#define LMODSZ (sizeof(lmod)/sizeof(lmod[0]))
#define CSPECSZ (sizeof(cspec)/sizeof(cspec[0]))
			const char *i8tbl[] = {
				PRId8, PRIi8, PRIo8, PRIu8, PRIx8, PRIX8
			};
			const char *i16tbl[] = {
				PRId16, PRIi16, PRIo16, PRIu16, PRIx16, PRIX16
			};
			const char *i32tbl[] = {
				PRId32, PRIi32, PRIo32, PRIu32, PRIx32, PRIX32
			};
			const char *i64tbl[] = {
				PRId64, PRIi64, PRIo64, PRIu64, PRIx64, PRIX64
			};
			while (*aptr && (sub = strchr(fstr, *aptr))) {
				flags |= 1 << (sub - fstr);
				++aptr;
			}
			if (!*aptr)
				goto stat;
			if (*aptr == '-' || isdigit(*aptr)) {
				dbgf("aptr=%s\n", aptr);
				cfw = 1;
				sscanf(aptr, "%d", &fw);
				if (*aptr == '-') ++aptr;
				while (*aptr && isdigit(*aptr))
					++aptr;
			}
			dbgf("aptr=%s\n", aptr);
			if (!*aptr)
				goto stat;
			if (*aptr == '.') {
				ptype = 0;
				if (!aptr[1])
					goto stat;
				if (*++aptr == '*') {
					ptype = 1;
					++aptr;
				}
				if (isdigit(*aptr)) {
					++ptype;
					dbgf("precision=%s\n", aptr - 1);
					// spec requires int, but does not make sense
					sscanf(aptr, "%u", &prec);
					while (*aptr && isdigit(*aptr))
						++aptr;
				}
			}
			if (!*aptr)
				goto stat;
			for (unsigned i = 0; i < LMODSZ; ++i)
				if (xtStringStartsWith(aptr, lmod[i])) {
					mod = i;
					aptr += strlen(lmod[i]);
					break;
				}
			if (!*aptr)
				goto stat;
			for (unsigned i = 0; i < CSPECSZ; ++i)
				if (xtStringStartsWith(aptr, cspec[i])) {
					conv = i;
					aptr += strlen(cspec[i]);
					break;
				}
			// interpret (u)int*_t variants
			switch (mod) {
			case 9:
				rep = i8tbl[conv];
				dbgs("int8_t");
				goto custom_put;
			case 10:
				rep = i16tbl[conv];
				dbgs("int16_t");
				goto custom_put;
			case 11:
				rep = i32tbl[conv];
				dbgs("int32_t");
				goto custom_put;
			case 12:
				rep = i64tbl[conv];
				dbgs("int64_t");
				goto custom_put;
			default:
				break;
custom_put:
				dbgf("arglen=%u\n", (unsigned)(aptr - arg));
				l = strlen(rep);
				dbgf("replen=%u (%%%s)\n", (unsigned)l, rep);
				// reconstruct flags
				char argf[16], *argfp = argf;
				unsigned argfn = 0;
				argf[0] = '\0';
				for (unsigned i = flags, j = 0; i; i >>= 1, ++j)
					if (i & 1) {
						*argfp++ = fstr[j];
						++argfn;
					}
				argf[argfn] = '\0';
				dbgf("argf=%s (%u): flags=%u\n", argf, argfn, flags);
				char argfw[32];
				unsigned argfwn = 0;
				argfw[0] = '\0';
				if (cfw) {
					// reconstruct field width
					sprintf(argfw, "%d", fw);
					argfwn = strlen(argfw);
				}
				dbgf("argfw=%s (%u)\n", argfw, argfwn);
				unsigned argsn = argfn + argfwn;
				if (ptr + l + argsn + 1 >= end)
					goto resize;
				argfw[argfwn] = '\0';
				// move format if replacement is longer OR shorter
				int diff = 0;
				unsigned arglen = aptr - fptr + argsn;
				if (l >= arglen) {
					diff = l - arglen + 1;
					size_t n = end - aptr - 1;
					dbgf("move: %d,%u\n", diff, (unsigned)n);
					memmove((void*)(aptr + diff), (const void*)aptr, n);
				}
				dbgf("diff: %d\n", diff);
				fptr = aptr + diff - 1;
				*ptr++ = '%';
				if (argfn) {
					memcpy(ptr, argf, argfn);
					ptr += argfn;
				}
				if (argfwn) {
					memcpy(ptr, argfw, argfwn);
					ptr += argfwn;
				}
				memcpy(ptr, rep, l);
				ptr += l;
				dbgf("fptr=%s\n", fptr);
				dbgf("format=%s\n", format);
				dbgf("flags=%u,fw=%d,prec=(%d,%u),mod=%d,conv=%d\n", flags, fw, ptype, prec, mod, conv);
				continue;
			}
stat:
			dbgf("flags=%u,fw=%d,prec=(%d,%u),mod=%d,conv=%d\n", flags, fw, ptype, prec, mod, conv);
		}
			goto put;
put:
		default:
			if (ptr >= end)
				goto resize;
			*ptr++ = *fptr;
			break;
		}
	}
	*ptr = '\0';
	dbgf("format=%s\n", buf);
	ret = _xt_vsnprintf(str, size, buf, end, args);
resize:
	// FIXME try resize for all goto statements to this label
fail:
	if (dbuf)
		free(dbuf);
	return ret;
}

char *xtstrncpy(char *restrict dest, const char *restrict src, size_t n)
{
	if (n) {
		strncpy(dest, src, n - 1);
		dest[n - 1] = '\0';
	}
	return dest;
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

#if XT_IS_WINDOWS
	#define strtok_r strtok_s
#endif

void xtStringSplit(char *restrict str, const char *restrict delim, char **restrict tokens, unsigned *restrict num)
{
	char *save_ptr, *token = strtok_r(str, delim, &save_ptr);
	unsigned i = 0;
	for (str = NULL; i < *num && token; ++i) {
		tokens[i] = token;
		token = strtok_r(str, delim, &save_ptr);
	}
	*num = i;
}

#if XT_IS_WINDOWS
	#undef strtok_r
#endif

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
	char *start, *end;
	for (start = str; ; ++start)
		if (!isspace(*start))
			break;
	for (end = start + strlen(start); end > start; --end)
		if (!isspace(end[-1]))
			break;
	memmove(str, start, end - start);
	end -= start - str;
	*end = '\0';
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
	xtsnprintf(buf2, sizeof buf2, " %03llu:%03llu:%03llu", (nanos / 1000000LU) % 1000, (nanos / 1000LU) % 1000, nanos % 1000);
	size_t n = strlen(buf);
	if (n + 1 <= buflen) {
		strncpy(buf + n, buf2, buflen - n - 1);
		buf[buflen - 1] = '\0';
	}
	return buf;
}

unsigned xtFormatTimeDuration(char *buf, size_t buflen, const char *format, const struct xtTimestamp *start, const struct xtTimestamp *end)
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
				nn = snprintf(abuf, sizeof abuf, "%02u", hours);
				goto append;
			case 'h':
				nn = snprintf(abuf, sizeof abuf, "%u %s", hours, hours == 1 ? "hour" : "hours");
				goto append;
			case 'M':
				nn = snprintf(abuf, sizeof abuf, "%02u", mins);
				goto append;
			case 'm':
				nn = snprintf(abuf, sizeof abuf, "%u %s", mins, mins == 1 ? "minute" : "minutes");
				goto append;
			case 'S':
				nn = snprintf(abuf, sizeof abuf, "%02u", secs);
				goto append;
			case 's':
				nn = snprintf(abuf, sizeof abuf, "%u %s", secs, secs == 1 ? "second" : "seconds");
				goto append;
			case 'T':
				nn = snprintf(abuf, sizeof abuf, "%03u", msec);
				goto append;
			case 't':
				nn = snprintf(abuf, sizeof abuf, "%u %s", msec, msec == 1 ? "millisecond" : "milliseconds");
				goto append;
			case 'U':
				nn = snprintf(abuf, sizeof abuf, "%03u", usec);
				goto append;
			case 'u':
				nn = snprintf(abuf, sizeof abuf, "%u %s", usec, usec == 1 ? "microsecond" : "microseconds");
				goto append;
			case 'N':
				nn = snprintf(abuf, sizeof abuf, "%03u", nsec);
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

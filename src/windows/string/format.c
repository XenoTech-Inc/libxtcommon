/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
// NOTE os_macros.h must be first in order for this to work
#include <xt/os_macros.h>
#include <xt/string.h>

// STD headers
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef XT_PRINTF_DEBUG
#define dbgf(f,...) printf(f, ## __VA_ARGS__)
#define dbgs(s) puts(s)
#else
#define dbgf(f,...) ((void)0)
#define dbgs(s) ((void)0)
#endif

int _xtvsnprintf(char *str, size_t size, char *format, char *end, va_list args)
{
	const char *rep;
	char *fptr, *buf, *ptr;
	size_t l;
	int ret = 0;
	buf = format;
	/*
	 * Interpret arguments where Microsoft does not adhere to the standard.
	 * These include: %z[diouxX], %ll[diouxX]
	 * We are not going to reorder the arguments because it is very ugly and
	 * error prone. This unfortunately means we have to use multiple
	 * hacks...
	 */
	const char *lltbl[] = {
		PRId64, PRIi64, PRIo64, PRIu64, PRIx64, PRIX64
	};
#if XT_IS_X64
	const char *ztbl[] = {
		PRId64, PRIi64, PRIo64, PRIu64, PRIx64, PRIX64
	};
#else
	const char *ztbl[] = {
		PRId32, PRIi32, PRIo32, PRIu32, PRIx32, PRIX32
	};
#endif
	for (ptr = buf, fptr = format; *fptr; ++fptr) {
		switch (*fptr) {
		case '%': {
			const char *fstr = "#0- +'";
			char *aptr = fptr + 1;
#ifdef XT_PRINTF_DEBUG
			const char *arg = fptr;
#endif
			const char *lmod[] = {
				"hh", "h", "ll", "l", "L", "q", "j", "z", "t",
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
#define LMODSZ (sizeof lmod/sizeof lmod[0])
#define CSPECSZ (sizeof cspec/sizeof cspec[0])
			char *sub;
			int ptype = -1, mod = -1, conv = -1;
			unsigned flags = 0, prec = 0, cfw = 0;
			int fw = 0;
			while (*aptr && (sub = strchr(fstr, *aptr))) {
				flags |= 1 << (sub - fstr);
				++aptr;
			}
			if (!*aptr)
				goto stat;
			if (*aptr == '-' || isdigit(*aptr)) {
				sscanf(aptr, "%d", &fw);
				if (*aptr == '-') ++aptr;
				while (*aptr && isdigit(*aptr))
					++aptr;
			}
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
			// interpret long long, unsigned long long, ssize_t and size_t
			switch (mod) {
			case 2:
				rep = lltbl[conv];
				dbgs("(unsigned) long long");
				goto custom_put;
			case 7:
				rep = ztbl[conv];
				dbgs("(s)size_t");
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
			if (ptr >= end) {
				dbgs("put");
				dbgf("offset: %d\n", (int)(end - ptr));
				dbgf("length: %d\n", (int)(end - buf));
					goto resize;
			}
			*ptr++ = *fptr;
			break;
		}
	}
	*ptr = '\0';
	dbgf("format=%s\n", buf);
	ret = vsnprintf(str, size, buf, args);
	goto end;
resize:
	// FIXME try resize for all goto statements to this label
	dbgs("resize");
end:
	// proper NULL termination...
	if (size)
		str[size - 1] = '\0';
	return ret;
}

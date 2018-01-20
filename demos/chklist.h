/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#ifndef CHKLIST_H
#define CHKLIST_H
#define cat(a,b) a ## b
#define cate(a,b) cat(a,b)
#define str(x) #x
#define stre(x) str(x)
#endif

static void cate(chklist, CHK_SUBTYPE)(CHK_TYPE *a, size_t n, int ascend, const char *type)
{
	char buf[256];
	if (ascend) {
		snprintf(buf, sizeof buf, "xtSort" stre(CHK_SUBTYPE) "() - %s ascending", type);
		for (size_t i = 1; i < n; ++i)
			if (a[i - 1] > a[i]) {
				FAIL(buf);
				return;
			}
		PASS(buf);
	} else {
		snprintf(buf, sizeof buf, "xtSort" stre(CHK_SUBTYPE) "() - %s descending", type);
		for (size_t i = 1; i < n; ++i)
			if (a[i - 1] < a[i]) {
				FAIL(buf);
				return;
			}
		PASS(buf);
	}
}

static void cate(arnd, CHK_SUBTYPE)(CHK_TYPE *a, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		a[i] = rand();
}

#undef CHK_SUBTYPE
#undef CHK_TYPE

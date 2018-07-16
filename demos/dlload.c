/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/dlload.h>

#if XT_IS_WINDOWS
	#include <windows.h>
#else
	#include <math.h>
	#include <gnu/lib-names.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static struct stats stats;

int main(void)
{
	stats_init(&stats, "dlload");
	puts("-- DLLOAD TEST");

	xtDLHandle handle = NULL;

#if XT_IS_WINDOWS
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	const char *functionName = "GetNativeSystemInfo";
	PGNSI funcptr;
	handle = xtDLOpen("kernel32.dll", XT_DL_LAZY);
#else
	typedef double (*cosine)(double);
	const char *functionName = "cos";
	cosine funcptr;
	handle = xtDLOpen(LIBM_SO, XT_DL_LAZY);
#endif

	if (handle)
		PASS("xtDLOpen()");
	else {
		FAIL("xtDLOpen()");
		char sbuf[256];
		puts(xtDLError(sbuf, sizeof sbuf));
		XFAIL("xtDLGetProcAddress()");
		XFAIL("Execute loaded function");
		goto cleanup;
	}

	*(void**)&funcptr = xtDLGetProcAddress(handle, functionName);

	if (funcptr)
		PASS("xtDLGetProcAddress()");
	else {
		FAIL("xtDLGetProcAddress()");
		XFAIL("Execute loaded function");
		goto cleanup;
	}

#if XT_IS_WINDOWS
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof SYSTEM_INFO);

	if (funcptr(&si))
		PASS("Execute loaded function");
	else {
		FAIL("Execute loaded function");
		goto cleanup;
	}
#else
	printf("cos(1234567) == %lf\n", cos(1234567));
	if (fabs(cos(1234567) - -0.931222) < 0.00001)
		PASS("Execute loaded function");
	else {
		FAIL("Execute loaded function");
		goto cleanup;
	}
#endif

cleanup:
	if (handle)
		xtDLClose(handle);

	stats_info(&stats);
	return stats_status(&stats);
}

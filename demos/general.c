#include <xt/error.h>
#include <xt/file.h>
#include <xt/os.h>
#include <xt/socket.h>
#include <xt/string.h>
#include <xt/thread.h>
#include <xt/time.h>
#include <xt/utils.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static struct stats stats;

static void fileTest(void)
{
	bool result;
	unsigned long long size;
	char sbuf[256], sbuf2[256];
	if (xtFileGetExecutablePath(sbuf, sizeof sbuf) == 0) {
		PASS("xtFileGetExecutablePath()");
	} else
		FAIL("xtFileGetExecutablePath()");

	if (xtFileExists(sbuf, &result) == 0) {
		PASS("xtFileExists()");
	} else
		FAIL("xtFileExists()");
	FILE *f = fopen(sbuf, "r");
	if (f) {
		PASS("fopen()");
		size = 0;
		if (xtFileGetSizeByHandle(f, &size) == 0) {
			PASS("xtFileGetSizeByHandle()");
		} else
			FAIL("xtFileGetSizeByHandle()");
		fclose(f);
	} else {
		FAIL("fopen()");
		XFAIL("xtFileGetSizeByHandle()");
	}

	if (xtFileGetSizeByName(sbuf, &size) == 0) {
		PASS("xtFileGetSizeByName()");
	} else
		FAIL("xtFileGetSizeByName()");

	if (xtFileGetRealPath(sbuf2, 256, sbuf) == 0) {
		PASS("xtFileGetRealPath()");
	} else
		FAIL("xtFileGetRealPath()");

	if (xtFileIsDir(sbuf, &result) == 0) {
		PASS("xtFileIsDir()");
	} else
		FAIL("xtFileIsDir()");

	if (xtFileGetHomeDir(sbuf2, sizeof sbuf2)) {
		PASS("xtFileGetHomeDir()");
	} else
		FAIL("xtFileGetHomeDir()");

	if (xtFileGetTempDir(sbuf2, sizeof sbuf2) == 0) {
		PASS("xtFileGetTempDir()");
	} else
		FAIL("xtFileGetTempDir()");

	if (xtFileGetCWD(sbuf2, sizeof sbuf2) == 0 && xtFileSetCWD(sbuf2) == 0
		&& xtFileGetCWD(sbuf2, sizeof sbuf2) == 0)
		PASS("xtFileGetCWD() & xtFileSetCWD()");
	else
		FAIL("xtFileGetCWD() & xtFileSetCWD()");

	if (xtFileCreateDir("test_dir") == 0) {
		PASS("xtFileCreateDir()");
		if (xtFileRemoveDir("test_dir") == 0)
			PASS("xtFileRemoveDir()");
		else
			FAIL("xtFileRemoveDir()");
	} else {
		FAIL("xtFileCreateDir()");
		XFAIL("xtFileRemoveDir()");
	}

	FILE *tmpFile;
	char tmpFilePath[1024];
	if (xtFileTempFile(tmpFilePath, sizeof tmpFilePath, &tmpFile) == 0) {
		PASS("xtFileTempFile()");
		fputs("Yo!", tmpFile);
		fflush(tmpFile);
		fclose(tmpFile);
		if (xtFileExecute(tmpFilePath) == 0)
			PASS("xtFileExecute()");
		else
			FAIL("xtFileExecute()");
		xtSleepMS(3000);
		char newPath[256];
		xtsnprintf(newPath, sizeof newPath, "%s/tmp_file.txt", sbuf2);

		if (xtFileCopy(tmpFilePath, newPath) == 0)
			PASS("xtFileCopy()");
		else
			FAIL("xtFileCopy()");

		if (xtFileRemove(tmpFilePath) == 0 && xtFileRemove(newPath) == 0)
			PASS("xtFileRemove()");
		else
			FAIL("xtFileRemove()");
	} else {
		FAIL("xtFileTempFile()");
		XFAIL("xtFileExecute()");
		XFAIL("xtFileCopy()");
		XFAIL("xtFileRemove()");
	}
}

static void osTest(void)
{
	char sbuf[256];
	xtConsoleSetTitle("New console title!");
	if (xtGetOSName(sbuf, 256)) {
		PASS("xtGetOSName()");
	} else
		FAIL("xtGetOSName()");

	struct xtRAMInfo ramInfo;
	if (xtRAMGetInfo(&ramInfo) == 0) {
		PASS("xtRAMGetInfo()");
	} else
		FAIL("xtRAMGetInfo()");

	unsigned width, height;
	if (xtConsoleGetSize(&width, &height) == 0) {
		PASS("xtConsoleGetSize()");
	} else
		FAIL("xtConsoleGetSize()");

	if (xtConsoleFillLine("#") == 0)
		PASS("xtConsoleFillLine()");
	else
		FAIL("xtConsoleFillLine()");

	struct xtCPUInfo info;
	if (xtCPUGetInfo(&info)) {
		PASS("xtCPUGetInfo()");
	} else
		FAIL("xtCPUGetInfo()");

}

static void tMainTask(struct xtThread *restrict t1, struct xtThread *restrict t2)
{
	(void)t1;
	(void)t2;
#if 0
	xtprintf("TMain: Does T1 equal T2?: %s\n", (xtThreadGetID(t1) == xtThreadGetID(t2) ? "Yes" : "No"));
	xtprintf("TMain: ID of T1: %zu\n", xtThreadGetID(t1));
	xtprintf("TMain: ID of T2: %zu\n", xtThreadGetID(t2));
	xtprintf("TMain: Is T1 alive?: %s\n", (xtThreadIsAlive(t1) ? "Yes" : "No"));
	xtprintf("TMain: Is T2 alive?: %s\n", (xtThreadIsAlive(t2) ? "Yes" : "No"));
#endif
}

static void *t1Task(struct xtThread *t, void *arg)
{
	(void)t;
	(void)arg;
#if 0
	int ret;
	xtMutex *m = arg;
	xtprintf("T1: My ID is %zu\n", xtThreadGetID(NULL));
	ret = xtMutexTryLock(m);
	if (ret == 0) {
		xtprintf("T1: I have obtained the lock to the mutex!\n");
		xtSleepMS(50); // Sleep intentionally so that the other thread should have
		// already tried to lock it, and fail
	} else
		xtprintf("T1: The other thread obtained the lock first\n");
	if (ret == 0)
		xtMutexUnlock(m);
#endif
	return (void*) 1;
}

static void *t2Task(struct xtThread *t, void *arg)
{
	(void)t;
	(void)arg;
#if 0
	int ret;
	xtMutex *m = arg;
	xtprintf("T2: My ID is %zu\n", xtThreadGetID(NULL));
	ret = xtMutexTryLock(m);
	if (ret == 0) {
		xtprintf("T2: I have obtained the lock to the mutex!\n");
		xtSleepMS(50); // Sleep intentionally so that the other thread should have
		// already tried to lock it, and fail
	} else
		xtprintf("T2: The other thread obtained the lock first\n");
	if (ret == 0)
		xtMutexUnlock(m);
#endif
	return (void*) 2;
}

static void threadTest(void)
{
	char sbuf[256];
	xtMutex m;
	struct xtThread t1, t2;
	if (xtThreadGetName(sbuf, sizeof sbuf))
		PASS("xtThreadGetName()");
	else
		FAIL("xtThreadGetName()");
	xtThreadSetName("thread_worker");
	if (xtMutexCreate(&m) == 0) {
		PASS("xtMutexCreate()");
		if (xtMutexLock(&m) == 0 && xtMutexUnlock(&m) == 0)
			PASS("xtMutexLock() & xtMutexUnlock()");
		else
			FAIL("xtMutexLock() & xtMutexUnlock()");
		if (xtThreadCreate(&t1, t1Task, &m, 0, 0) == 0 && xtThreadCreate(&t2, t2Task, &m, 0, 0) == 0) {
			PASS("xtThreadCreate()");
			tMainTask(&t1, &t2);
			void *ret1, *ret2;
			if (xtThreadJoin(&t1, &ret1) == 0 && xtThreadJoin(&t2, &ret2) == 0)
				PASS("xtThreadJoin()");
			else
				FAIL("xtThreadJoin()");
		} else {
			FAIL("xtThreadCreate()");
			XFAIL("xtThreadJoin()");
		}
		xtMutexDestroy(&m);
	} else {
		FAIL("xtMutexCreate()");
	}
}

static void timeTest(void)
{
	char sbuf[256];
	struct xtTimestamp ts, timeNow;
	if (xtClockGetRes(&ts, XT_CLOCK_MONOTONIC) == 0)
		PASS("xtClockGetRes(XT_CLOCK_MONOTONIC)");
	else
		FAIL("xtClockGetRes(XT_CLOCK_MONOTONIC)");
	if (xtClockGetRes(&ts, XT_CLOCK_MONOTONIC_COARSE) == 0)
		PASS("xtClockGetRes(XT_CLOCK_MONOTONIC_COARSE)");
	else
		FAIL("xtClockGetRes(XT_CLOCK_MONOTONIC_COARSE)");
	if (xtClockGetRes(&ts, XT_CLOCK_MONOTONIC_RAW) == 0)
		PASS("xtClockGetRes(XT_CLOCK_MONOTONIC_RAW)");
	else
		FAIL("xtClockGetRes(XT_CLOCK_MONOTONIC_RAW)");
	if (xtClockGetRes(&ts, XT_CLOCK_REALTIME) == 0)
		PASS("xtClockGetRes(XT_CLOCK_REALTIME)");
	else
		FAIL("xtClockGetRes(XT_CLOCK_REALTIME)");
	if (xtClockGetRes(&ts, XT_CLOCK_REALTIME_COARSE) == 0)
		PASS("xtClockGetRes(XT_CLOCK_REALTIME_COARSE)");
	else
		FAIL("xtClockGetRes(XT_CLOCK_REALTIME_COARSE)");
	if (xtClockGetRes(&ts, XT_CLOCK_REALTIME_NOW) == 0)
		PASS("xtClockGetRes(XT_CLOCK_REALTIME_NOW)");
	else
		FAIL("xtClockGetRes(XT_CLOCK_REALTIME_NOW)");

	if (xtClockGetTime(&ts, XT_CLOCK_MONOTONIC) == 0)
		PASS("xtClockGetTime(XT_CLOCK_MONOTONIC)");
	else
		FAIL("xtClockGetTime(XT_CLOCK_MONOTONIC)");
	if (xtClockGetTime(&ts, XT_CLOCK_MONOTONIC_COARSE) == 0)
		PASS("xtClockGetTime(XT_CLOCK_MONOTONIC_COARSE)");
	else
		FAIL("xtClockGetTime(XT_CLOCK_MONOTONIC_COARSE)");
	if (xtClockGetTime(&ts, XT_CLOCK_MONOTONIC_RAW) == 0)
		PASS("xtClockGetTime(XT_CLOCK_MONOTONIC_RAW)");
	else
		FAIL("xtClockGetTime(XT_CLOCK_MONOTONIC_RAW)");
	if (xtClockGetTime(&ts, XT_CLOCK_REALTIME) == 0)
		PASS("xtClockGetTime(XT_CLOCK_REALTIME)");
	else
		FAIL("xtClockGetTime(XT_CLOCK_REALTIME)");
	if (xtClockGetTime(&ts, XT_CLOCK_REALTIME_COARSE) == 0)
		PASS("xtClockGetTime(XT_CLOCK_REALTIME_COARSE)");
	else
		FAIL("xtClockGetTime(XT_CLOCK_REALTIME_COARSE)");
	if (xtClockGetTime(&ts, XT_CLOCK_REALTIME_NOW) == 0)
		PASS("xtClockGetTime(XT_CLOCK_REALTIME_NOW)");
	else
		FAIL("xtClockGetTime(XT_CLOCK_REALTIME_NOW)");

	if (xtFormatTimePrecise(sbuf, sizeof sbuf, &timeNow))
		PASS("xtFormatTimePrecise()");
	else
		FAIL("xtFormatTimePrecise()");
}

static void socketTest(void)
{
	struct xtSockaddr sa;
	xtSocket serverSock;

	if (xtSocketInit())
		PASS("xtSocketInit()");
	else
		FAIL("xtSocketInit()");

	if (xtSocketCreate(&serverSock, XT_SOCKET_PROTO_TCP) == 0)
		PASS("xtSocketCreate()");
	else
		FAIL("xtSocketCreate()");

	if (xtSocketBindToAny(serverSock, 0) == 0)
		PASS("xtSocketBindToAny()");
	else
		FAIL("xtSocketBindToAny()");

	if (xtSocketListen(serverSock, 64) == 0)
		PASS("xtSocketListen()");
	else
		FAIL("xtSocketListen()");

	if (xtSocketGetLocalSocketAddress(serverSock, &sa) == 0)
		PASS("xtSocketGetLocalSocketAddress()");
	else
		FAIL("xtSocketGetLocalSocketAddress()");

	if (xtSocketClose(&serverSock) == 0)
		PASS("xtSocketClose()");
	else
		FAIL("xtSocketClose()");
	xtSocketDestruct();
}

static void stringTest(void)
{
	char buf[256];
	const char *text = "The pope uses dope";
	if (xtFormatHex(buf, sizeof buf, text, strlen(text), ' ', false)) {
		PASS("xtFormatHex()");
	} else
		FAIL("xtFormatHex()");
}

static void *threadTestSleep(struct xtThread *t, void *arg)
{
	(void) arg;
	if (xtThreadSuspend(t) == 0)
		PASS("xtThreadSuspend()");
	else
		FAIL("xtThreadSuspend()");
	return NULL;
}

int main(void)
{
	stats_init(&stats, "general");
	xtConsoleFillLine("-");
	puts("-- CONFIGURE TEST");
	xtprintf(
		"configured with: %s\n"
		"built on %s\n"
		"%s (%u.%u.%u)\n",
		XT_BUILD_OPTIONS.configure,
		XT_BUILD_OPTIONS.date,
		XT_BUILD_OPTIONS.version,
		XT_BUILD_OPTIONS.versionMajor,
		XT_BUILD_OPTIONS.versionMinor,
		XT_BUILD_OPTIONS.versionPatch
	);
	xtConsoleFillLine("-");
	puts("-- FILE TEST");
	fileTest();
	xtConsoleFillLine("-");
	puts("-- OS TEST");
	osTest();
	xtConsoleFillLine("-");
	puts("-- THREAD TEST");
	threadTest();
	xtConsoleFillLine("-");
	puts("-- THREAD SLEEP TEST");
	struct xtThread t;
	xtThreadCreate(&t, threadTestSleep, NULL, 64, 0);
	xtSleepMS(500);
	xtThreadContinue(&t);
	xtThreadJoin(&t, NULL);
	xtConsoleFillLine("-");
	puts("-- TIME TEST");
	timeTest();
	xtConsoleFillLine("-");
	puts("-- SOCKET TEST");
	socketTest();
	xtConsoleFillLine("-");
	puts("-- STRING TEST");
	stringTest();
	stats_info(&stats);
	return stats_status(&stats);
}

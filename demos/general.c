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

static void fileTest(void)
{
	char sbuf[256], sbuf2[256];
	if (xtFileGetExecutablePath(sbuf, sizeof(sbuf)) != 0) {
		xtfprintf(stderr, "Failed to retrieve the path of myself\n");
		xtsnprintf(sbuf, sizeof(sbuf), "./general.o");
	}
	xtprintf("Executable path: %s\n", sbuf);

	bool result;
	int ret;
	unsigned long long size;
	result = false;
	ret = xtFileExists(sbuf, &result);
	xtprintf("File exists: %d - %s\n", ret, (result ? "Yes" : "No"));
	FILE *f = fopen(sbuf, "r");
	size = 0;
	ret = xtFileGetSizeByHandle(f, &size);
	xtprintf("File size by handle: %d - %llu\n", ret, size);
	if (f)
		fclose(f);
	size = 0;
	ret = xtFileGetSizeByName(sbuf, &size);
	xtprintf("File size by path: %d - %llu\n", ret, size);
	result = false;
	ret = xtFileGetRealPath(sbuf2, 256, sbuf);
	xtprintf("Absolute path of file: %d - %s\n", ret, sbuf2);
	xtprintf("Base name of file: %s\n", xtFileGetBaseName(sbuf2));
	xtprintf("File extension: %s\n", xtFileGetExtension(sbuf2));
	result = false;
	ret = xtFileIsDir(sbuf, &result);
	xtprintf("Is file a directory?: %d - %s\n", ret, (result ? "Yes" : "No"));

	xtprintf("Home dir: %s\n", xtFileGetHomeDir(sbuf2, sizeof(sbuf2)));

	ret = xtFileGetTempDir(sbuf2, sizeof(sbuf2));
	xtprintf("System temp dir: %d - %s\n", ret, sbuf2);

	ret = xtFileGetCWD(sbuf2, sizeof(sbuf2));
	xtprintf("CWD before: %d - %s\n", ret, sbuf2);
	ret = xtFileSetCWD(sbuf2);
	xtprintf("CWD change: %d\n", ret);
	ret = xtFileGetCWD(sbuf2, sizeof(sbuf2));
	xtprintf("CWD after : %d - %s\n", ret, sbuf2);

	xtprintf("Creation of dir: %d\n", xtFileCreateDir("test_dir"));
	xtprintf("Removal of dir: %d\n", xtFileRemoveDir("test_dir"));

	FILE *tmpFile;
	char tmpFilePath[1024];
	xtprintf("Creation of tmpfile: %d\n", xtFileTempFile(tmpFilePath, sizeof(tmpFilePath), &tmpFile));
	if (tmpFile) {
		xtprintf("Path of tmpfile: %s\n", tmpFilePath);
		fputs("HEY!", tmpFile);
		fflush(tmpFile);
		fclose(tmpFile);
		xtprintf("Executing temporary file\n");
		xtFileExecute(tmpFilePath);
		puts("Giving the program a couple of seconds to open it...");
		xtSleepMS(3000);
		char newPath[256];
		xtsnprintf(newPath, sizeof(newPath), "%s/tmp_file.txt", sbuf2);
		xtprintf("Copy tmp file: %d\n", xtFileCopy(tmpFilePath, newPath));
		xtprintf("Remove tmp file: %d\n", xtFileRemove(tmpFilePath));
		xtprintf("Remove tmp copy file: %d\n", xtFileRemove(newPath));
	}
}

static void osTest(void)
{
	char sbuf[256];
	xtConsoleSetTitle("New console title!");
	xtprintf("OS name: %s\n", xtGetOSName(sbuf, 256));
	xtprintf("Uptime in seconds: %u\n", xtGetUptime());
	xtprintf("RAM total: %llu bytes\nRAM available: %llu bytes\n", xtRAMGetAmountTotal(), xtRAMGetAmountFree());
	unsigned width, height;
	int ret = xtConsoleGetSize(&width, &height);
	xtprintf("Is a console available?: %s\n", (xtConsoleIsAvailable() ? "Yes" : "No"));
	xtprintf("Console size retval=%d, size: w=%u, h=%u\n", ret, width, height);
	xtConsoleFillLine("#");
	puts("## CPU info");
	struct xtTimestamp end, start;
	xtClockGetTime(&start, XT_CLOCK_MONOTONIC);
	struct xtCPUInfo info;
	bool retval = xtCPUGetInfo(&info);
	xtClockGetTime(&end, XT_CLOCK_MONOTONIC);
	xtprintf("All CPU info retrieved?: %s\n", (retval ? "Yes" : "No"));
	xtprintf(
		"CPU info retrieval time taken : %llu usecs\n",
		xtTimestampToUS(&end) - xtTimestampToUS(&start)
	);
	xtCPUDump(&info, stdout);
}

static void tMainTask(struct xtThread *restrict t1, struct xtThread *restrict t2)
{
	xtprintf("TMain: Does T1 equal T2?: %s\n", (xtThreadGetID(t1) == xtThreadGetID(t2) ? "Yes" : "No"));
	xtprintf("TMain: ID of T1: %zu\n", xtThreadGetID(t1));
	xtprintf("TMain: ID of T2: %zu\n", xtThreadGetID(t2));
	xtprintf("TMain: Is T1 alive?: %s\n", (xtThreadIsAlive(t1) ? "Yes" : "No"));
	xtprintf("TMain: Is T2 alive?: %s\n", (xtThreadIsAlive(t2) ? "Yes" : "No"));
}

static void *t1Task(struct xtThread *t, void *arg)
{
	(void) t;
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
	return (void*) 1;
}

static void *t2Task(struct xtThread *t, void *arg)
{
	(void) t;
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
	return (void*) 2;
}

static void threadTest(void)
{
	char sbuf[256];
	int ret;
	xtMutex m;
	struct xtThread t1, t2;
	if ((ret = xtMutexCreate(&m)) != 0)
		goto err;
	if ((ret = xtThreadCreate(&t1, t1Task, &m, 0)) != 0)
		goto err;
	if ((ret = xtThreadCreate(&t2, t2Task, &m, 0)) != 0)
		goto err;
	tMainTask(&t1, &t2);
	printf("T1 ret: %p\n", xtThreadJoin(&t1));
	printf("T2 ret: %p\n", xtThreadJoin(&t2));
	xtMutexDestroy(&m);
	return;
err:
	xtDie(xtStrError(sbuf, sizeof(sbuf), ret));
}

static void timeTest(void)
{
	char sbuf[256];
	struct xtTimestamp ts, timeNow, timeLater;
	int ret;

#define TEST_CLOCK(clockId, clockName)\
	ret = xtClockGetRes(&ts, clockId);\
	xtprintf("%s res: %d - %uNS\n", clockName, ret, ts.nsec);\
	ret = xtClockGetTime(&ts, clockId);\
	xtprintf("%s: %d - %s\n", clockName, ret, xtFormatTimePrecise(sbuf, 255, &ts));

	TEST_CLOCK(XT_CLOCK_MONOTONIC, "Monotonic");
	TEST_CLOCK(XT_CLOCK_MONOTONIC_COARSE, "Monotonic coarse");
	TEST_CLOCK(XT_CLOCK_MONOTONIC_RAW, "Monotonic raw");
	TEST_CLOCK(XT_CLOCK_REALTIME, "Realtime");
	TEST_CLOCK(XT_CLOCK_REALTIME_COARSE, "Realtime coarse");
	TEST_CLOCK(XT_CLOCK_REALTIME_NOW, "Realtime now");

	xtClockGetTime(&timeNow, XT_CLOCK_REALTIME);
	xtprintf("Time now in msecs non-gmt corrected raw: %llu\n", xtTimestampToMS(&timeNow));
	xtprintf("Time now in msecs non-gmt corrected: %s\n", xtFormatTimePrecise(sbuf, 255, &timeNow));
	xtClockGetTime(&timeNow, XT_CLOCK_REALTIME_NOW);
	xtprintf("Time now in msecs gmt (with any dst) corrected: %s\n", xtFormatTimePrecise(sbuf, 255, &timeNow));

	const unsigned sleepTimeMS = 100;
	xtClockGetTime(&timeNow, XT_CLOCK_MONOTONIC);
	xtprintf("Time mono in usecs: %llu\n", xtTimestampToUS(&timeNow));
	xtSleepMS(sleepTimeMS);
	xtClockGetTime(&timeLater, XT_CLOCK_MONOTONIC);
	xtprintf(
		"Time mono diff %u msecs later (in usecs): %llu\n", sleepTimeMS,
		xtTimestampToUS(&timeLater) - xtTimestampToUS(&timeNow)
	);
}

static void *socketTestT2(struct xtThread *t, void *arg)
{
	(void) t;
	(void) arg;
	char sbuf[256];
	int ret;
	struct xtSockaddr sa;
	xtSocket s;
	ret = xtSocketCreate(&s, XT_SOCKET_PROTO_TCP);
	xtprintf("Socket creation: %s\n", xtGetErrorStr(ret));

	xtSockaddrFromString(&sa, "127.0.0.1:25659", 0);
	const int maxTries = 10;
	for (int i = 0; i < maxTries; ++i) {
		xtprintf("Connecting %d/%d\n", i + 1, maxTries);
		ret = xtSocketConnect(s, &sa);
		xtprintf("Socket connect to %s: %s\n", xtSockaddrToString(&sa, sbuf, sizeof(sbuf)), xtGetErrorStr(ret));
		if (ret == 0)
			break;
	}
	xtSocketClose(&s);
	return NULL;
}

static void socketTest(void)
{
	if (!xtSocketInit()) {
		puts("Failure to initialize the socket system");
		return;
	}
	char sbuf[256];
	int ret;
	struct xtSockaddr sa;
	xtSocket serverSock;
	ret = xtSocketCreate(&serverSock, XT_SOCKET_PROTO_TCP);


	struct xtThread t;
	xtThreadCreate(&t, socketTestT2, NULL, 64);

	ret = xtSocketBindToAny(serverSock, 25659);
	xtSocketGetLocalSocketAddress(serverSock, &sa);
	xtprintf("Socket bind to %s: %s\n", xtSockaddrToString(&sa, sbuf, sizeof(sbuf)), xtGetErrorStr(ret));

	ret = xtSocketListen(serverSock, 10);
	xtprintf("Socket listen: %s\n", xtGetErrorStr(ret));

	struct xtSockaddr peerAddr;
	xtSocket peerSocket;

	const int maxTries = 10;
	for (int i = 0; i < maxTries; ++i) {
		xtprintf("Accepting %d/%d\n", i + 1, maxTries);
		ret = xtSocketTCPAccept(serverSock, &peerSocket, &peerAddr);
		xtSleepMS(0);
		xtprintf("Socket accept: %s\n", xtGetErrorStr(ret));
		if (ret == 0) {
			xtprintf("Socket accept: %s has connected!\n", xtSockaddrToString(&peerAddr, sbuf, sizeof(sbuf)));
			xtSocketClose(&peerSocket);
			break;
		}
	}
	xtSocketClose(&serverSock);

	xtThreadJoin(&t);
	xtSocketDestruct();
}

static void stringTest(void)
{
	puts("The following 2 numbers are supposed to be the same and equal 18446744073709551615");
	xtprintf("Max LLU value raw llu: %llu\n", ULLONG_MAX);
	xtprintf("Max LLU value uint64 to string : %I64u\n", (uint64_t)(ULLONG_MAX));
	char buf[256];
	const char *text = "The pope uses dope";
	xtprintf("Hexdump of \"%s\": ", text);
	if (!xtFormatHex(buf, sizeof buf, text, strlen(text), ' ', false)) {
		fputs("oops\n", stderr);
		return;
	}
	puts(buf);
}

static void *threadTestSleep(struct xtThread *t, void *arg)
{
	(void) arg;
	puts("Going to suspend soon!");
	xtThreadSuspend(t);
	puts("Awoken!");
	return NULL;
}

int main(void)
{
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
	xtThreadCreate(&t, threadTestSleep, NULL, 64);
	xtSleepMS(500);
	xtThreadContinue(&t);
	xtThreadJoin(&t);
	xtConsoleFillLine("-");
	puts("-- TIME TEST");
	timeTest();
	xtConsoleFillLine("-");
	puts("-- SOCKET TEST");
	socketTest();
	xtConsoleFillLine("-");
	puts("-- STRING TEST");
	stringTest();
	return EXIT_SUCCESS;
}

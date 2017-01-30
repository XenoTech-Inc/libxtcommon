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
		fprintf(stderr, "Failed to retrieve the path of myself\n");
		snprintf(sbuf, sizeof(sbuf), "test_file");
	}
	printf("Executable path: %s\n", sbuf);
	
	bool result;
	int ret;
	unsigned long long size;
	result = false;
	ret = xtFileExists(sbuf, &result);
	printf("File exists: %d - %s\n", ret, (result ? "Yes" : "No"));
	FILE *f = fopen(sbuf, "r");
	size = 0;
	ret = xtFileGetSizeByHandle(f, &size);
	printf("File size by handle: %d - %llu\n", ret, size);
	if (f)
		fclose(f);
	size = 0;
	ret = xtFileGetSizeByName(sbuf, &size);
	printf("File size by path: %d - %llu\n", ret, size);
	result = false;
	ret = xtFileGetRealPath(sbuf, sbuf2, 256);
	printf("Absolute path of file: %d - %s\n", ret, sbuf2);
	printf("Base name of file: %s\n", xtFileGetBaseName(sbuf2));
	printf("File extension: %s\n", xtFileGetExtension(sbuf2));
	result = false;
	ret = xtFileIsDir(sbuf, &result);
	printf("Is file a directory?: %d - %s\n", ret, (result ? "Yes" : "No"));
	
	printf("Home dir: %s\n", xtFileGetHomeDir(sbuf2, sizeof(sbuf2)));
	
	ret = xtFileGetTempDir(sbuf2, sizeof(sbuf2));
	printf("System temp dir: %d - %s\n", ret, sbuf2);
	
	ret = xtFileGetCWD(sbuf2, sizeof(sbuf2));
	printf("CWD before: %d - %s\n", ret, sbuf2);
	ret = xtFileSetCWD(sbuf2);
	printf("CWD change: %d\n", ret);
	ret = xtFileGetCWD(sbuf2, sizeof(sbuf2));
	printf("CWD after : %d - %s\n", ret, sbuf2);
	
	printf("Creation of dir: %d\n", xtFileCreateDir("test_dir"));
	printf("Removal of dir: %d\n", xtFileRemoveDir("test_dir"));
	
	FILE *tmpFile;
	char tmpFilePath[1024];
	printf("Creation of tmpfile: %d\n", xtFileTempFile(&tmpFile, tmpFilePath, sizeof(tmpFilePath)));
	if (tmpFile) {
		printf("Path of tmpfile: %s\n", tmpFilePath);
		fputs("HEY!", tmpFile);
		fflush(tmpFile);
		fclose(tmpFile);
		printf("Executing temporary file\n");
		xtFileExecute(tmpFilePath);
		puts("Giving the program a couple of seconds to open it...");
		xtSleepMS(3000);
		char newPath[256];
		snprintf(newPath, sizeof(newPath), "%s/tmp_file.txt", sbuf2);
		printf("Copy tmp file: %d\n", xtFileCopy(tmpFilePath, newPath));
		printf("Remove tmp file: %d\n", xtFileRemove(tmpFilePath));
		printf("Remove tmp copy file: %d\n", xtFileRemove(newPath));
	}
}

static void osTest(void)
{
	char sbuf[256];
	xtConsoleSetTitle("New console title!");
	printf("OS name: %s\n", xtGetOSName(sbuf, 256));
	printf("Uptime in seconds: %u\n", xtGetUptime());
	printf("RAM total: %llu bytes\nRAM available: %llu bytes\n", xtRAMGetAmountTotal(), xtRAMGetAmountFree());
	unsigned width, height;
	int ret = xtConsoleGetSize(&width, &height);
	printf("Is a console available?: %s\n", (xtConsoleIsAvailable() ? "Yes" : "No"));
	printf("Console size retval=%d, size: w=%u, h=%u\n", ret, width, height);
	xtConsoleFillLine("#");
	puts("## CPU info");
	unsigned long long end, start = xtClockGetMonotimeUS();
	struct xtCPUInfo info;
	bool retval = xtCPUGetInfo(&info);
	end = xtClockGetMonotimeUS();
	printf("All CPU info retrieved?: %s\n", (retval ? "Yes" : "No"));
	printf("CPU info retrieval time taken : %llu usecs\n", end - start);
	xtCPUDump(&info, stdout);
}

static void tMainTask(struct xtThread *restrict t1, struct xtThread *restrict t2)
{
	char nbuf[32];
	printf("TMain: Does T1 equal T2?: %s\n", (xtThreadGetID(t1) == xtThreadGetID(t2) ? "Yes" : "No"));
	printf("TMain: ID of T1: %s\n", xtSizetToStr(xtThreadGetID(t1), nbuf, 32));
	printf("TMain: ID of T2: %s\n", xtSizetToStr(xtThreadGetID(t2), nbuf, 32));
	printf("TMain: Is T1 alive?: %s\n", (xtThreadIsAlive(t1) ? "Yes" : "No"));
	printf("TMain: Is T2 alive?: %s\n", (xtThreadIsAlive(t2) ? "Yes" : "No"));
}

static void *t1Task(struct xtThread *t, void *arg)
{
	(void) t;
	int ret;
	char nbuf[32];
	xtMutex *m = arg;
	printf("T1: My ID is %s\n", xtSizetToStr(xtThreadGetID(NULL), nbuf, 32));
	ret = xtMutexTryLock(m);
	if (ret == 0) {
		printf("T1: I have obtained the lock to the mutex!\n");
		xtSleepMS(50); // Sleep intentionally so that the other thread should have 
		// already tried to lock it, and fail
	} else
		printf("T1: The other thread obtained the lock first\n");
	if (ret == 0)
		xtMutexUnlock(m);
	return NULL;
}

static void *t2Task(struct xtThread *t, void *arg)
{
	(void) t;
	int ret;
	char nbuf[32];
	xtMutex *m = arg;
	printf("T2: My ID is %s\n", xtSizetToStr(xtThreadGetID(NULL), nbuf, 32));
	ret = xtMutexTryLock(m);
	if (ret == 0) {
		printf("T2: I have obtained the lock to the mutex!\n");
		xtSleepMS(50); // Sleep intentionally so that the other thread should have 
		// already tried to lock it, and fail
	} else
		printf("T2: The other thread obtained the lock first\n");
	if (ret == 0)
		xtMutexUnlock(m);
	return NULL;
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
	xtThreadJoin(&t1);
	xtThreadJoin(&t2);
	xtMutexDestroy(&m);
	return;
err:
	xtDie(xtStrError(ret, sbuf, sizeof(sbuf)));
}

static void timeTest(void)
{
	char sbuf[256];
	unsigned long long timeNow = xtClockGetRealtimeUS() / 1000;
	printf("Time now in msecs: %llu\n", timeNow);
	printf("Time now in msecs non-gmt corrected: %s\n", xtFormatTime(timeNow / 1000, sbuf, 255));
	timeNow = xtClockGetCurrentTimeUS() / 1000; 
	printf("Time now in msecs gmt (with any dst) corrected: %s\n", xtFormatTime(timeNow / 1000, sbuf, 255));
	
	const unsigned sleepTimeMS = 100;
	timeNow = xtClockGetMonotimeUS();
	printf("Time mono in usecs: %llu\n", timeNow);
	xtSleepMS(sleepTimeMS);
	unsigned long long timeLater = xtClockGetMonotimeUS();
	printf("Time mono diff %u msecs later (in usecs): %llu\n", sleepTimeMS, (timeLater - timeNow));
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
	printf("Socket creation: %s\n", xtGetErrorStr(ret));
	
	xtSockaddrFromString(&sa, "127.0.0.1:25659", 0);
	const int maxTries = 10;
	for (int i = 0; i < maxTries; ++i) {
		printf("Connecting %d/%d\n", i + 1, maxTries);
		ret = xtSocketConnect(s, &sa);
		printf("Socket connect to %s: %s\n", xtSockaddrToString(&sa, sbuf, sizeof(sbuf)), xtGetErrorStr(ret));
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
	printf("Socket bind to %s: %s\n", xtSockaddrToString(&sa, sbuf, sizeof(sbuf)), xtGetErrorStr(ret));
	
	ret = xtSocketListen(serverSock, 10);
	printf("Socket listen: %s\n", xtGetErrorStr(ret));
	
	struct xtSockaddr peerAddr;
	xtSocket peerSocket;
	
	const int maxTries = 10;
	for (int i = 0; i < maxTries; ++i) {
		printf("Accepting %d/%d\n", i + 1, maxTries);
		ret = xtSocketTCPAccept(serverSock, &peerSocket, &peerAddr);
		xtSleepMS(0);
		printf("Socket accept: %s\n", xtGetErrorStr(ret));
		if (ret == 0) {
			printf("Socket accept: %s has connected!\n", xtSockaddrToString(&peerAddr, sbuf, sizeof(sbuf)));
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
	char nbuf[32];
	puts("The following 2 numbers are supposed to be the same and equal 18446744073709551615");
	printf("Max LLU value raw llu: %llu\n", ULLONG_MAX);
	printf("Max LLU value uint64 to string : %s\n", xtUint64ToStr(ULLONG_MAX, nbuf, 32));
	char buf[256];
	const char *text = "The pope uses dope";
	printf("Hexdump of \"%s\": ", text);
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
	printf(
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

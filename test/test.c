#include <xt/thread.h>
#include <xt/time.h>
#include <xt/utils.h>
#include <xt/string.h>
#include <xt/os.h>
#include <xt/error.h>
#include <xt/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

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
	puts("#####################################\n## CPU info");
	unsigned long long end, start = xtClockGetMonotimeUS();
	xtCPU info;
	bool retval = xtCPUGetInfo(&info);
	end = xtClockGetMonotimeUS();
	printf("All CPU info retrieved?: %s\n", (retval ? "Yes" : "No"));
	printf("CPU info retrieval time taken : %llu usecs\n", end - start);
	xtCPUDump(&info, stdout);
}

static void tMainTask(xtThread *restrict t1, xtThread *restrict t2)
{
	char nbuf[32];
	printf("TMain: Does T1 equal T2?: %s\n", (xtThreadGetID(t1) == xtThreadGetID(t2) ? "Yes" : "No"));
	printf("TMain: ID of T1: %s\n", xtSizetToStr(xtThreadGetID(t1), nbuf, 32));
	printf("TMain: ID of T2: %s\n", xtSizetToStr(xtThreadGetID(t2), nbuf, 32));
	printf("TMain: Is T1 alive?: %s\n", (xtThreadIsAlive(t1) ? "Yes" : "No"));
	printf("TMain: Is T2 alive?: %s\n", (xtThreadIsAlive(t2) ? "Yes" : "No"));
}

static void *t1Task(xtThread *t, void *arg)
{
	(void) t;
	char nbuf[32];
	xtSleepMS(50);
	xtMutex *m = arg;
	bool lockedMutex = xtMutexTryLock(m);
	printf("T1: My ID is %s\n", xtSizetToStr(xtThreadGetID(NULL), nbuf, 32));
	if (lockedMutex) {
		xtMutexUnlock(m);
		printf("T1: I obtained the lock to the mutex\n");
	} else {
		printf("T1: Unable to obtain the lock to the mutex\n");
	}
	return NULL;
}

static void *t2Task(xtThread *t, void *arg)
{
	(void) t;
	char nbuf[32];
	xtSleepMS(50);
	xtMutex *m = arg;
	bool lockedMutex = xtMutexTryLock(m);
	printf("T2: My ID is %s\n", xtSizetToStr(xtThreadGetID(NULL), nbuf, 32));
	if (lockedMutex) {
		xtMutexUnlock(m);
		printf("T2: I obtained the lock to the mutex\n");
	} else {
		printf("T2: Unable to obtain the lock to the mutex\n");
	}
	return NULL;
}

static void threadTest(void)
{
	char sbuf[256];
	int ret;
	xtMutex m;
	xtThread t1, t2;
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
	printf("Time now in msecs non-gmt corrected: %s\n", xtClockFormatTime(timeNow / 1000, sbuf, 255));
	timeNow = xtClockGetCurrentTimeUS() / 1000; 
	printf("Time now in msecs gmt (with any dst) corrected: %s\n", xtClockFormatTime(timeNow / 1000, sbuf, 255));
	
	const unsigned sleepTimeMS = 100;
	timeNow = xtClockGetMonotimeUS();
	printf("Time mono in usecs: %llu\n", timeNow);
	xtSleepMS(sleepTimeMS);
	unsigned long long timeLater = xtClockGetMonotimeUS();
	printf("Time mono diff %u msecs later (in usecs): %llu\n", sleepTimeMS, (timeLater - timeNow));
}

static void *socketTestT2(xtThread *t, void *arg)
{
	(void) t;
	(void) arg;
	char sbuf[256];
	int ret;
	xtSockaddr sa;
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
	xtSockaddr sa;
	xtSocket serverSock;
	ret = xtSocketCreate(&serverSock, XT_SOCKET_PROTO_TCP);
	

	xtThread t;
	xtThreadCreate(&t, socketTestT2, NULL, 64);
	
	ret = xtSocketBindToAny(serverSock, 25659);
	xtSocketGetLocalSocketAddress(serverSock, &sa);
	printf("Socket bind to %s: %s\n", xtSockaddrToString(&sa, sbuf, sizeof(sbuf)), xtGetErrorStr(ret));
	
	ret = xtSocketListen(serverSock, 10);
	printf("Socket listen: %s\n", xtGetErrorStr(ret));
	
	xtSockaddr peerAddr;
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
}

static void *threadTestSleep(xtThread *t, void *arg)
{
	(void)arg;
	puts("Going to suspend soon!");
	xtThreadSuspend(t);
	puts("Awoken!");
	return NULL;
}

int main(void)
{
	puts("--------------------------------------------------------------------------------\n-- CONFIGURE TEST");
	printf(
		"configured with: %s\n"
		"built on %s\n"
		"%s (%u.%u)\n",
		XT_BUILD_OPTIONS.configure,
		XT_BUILD_OPTIONS.date,
		XT_BUILD_OPTIONS.version,
		XT_BUILD_OPTIONS.versionMajor,
		XT_BUILD_OPTIONS.versionMinor
	);
	puts("--------------------------------------------------------------------------------\n-- OS TEST");
	osTest();
	puts("--------------------------------------------------------------------------------\n-- THREAD TEST");
	threadTest();
	puts("--------------------------------------------------------------------------------\n-- THREAD SLEEP TEST");
		xtThread t;
		xtThreadCreate(&t, threadTestSleep, NULL, 64);
		xtSleepMS(500);
		xtThreadContinue(&t);
		xtThreadJoin(&t);
	puts("--------------------------------------------------------------------------------\n-- TIME TEST");
	timeTest();
	puts("--------------------------------------------------------------------------------\n-- SOCKET TEST");
	socketTest();
	puts("--------------------------------------------------------------------------------\n-- STRING TEST");
	stringTest();
	return EXIT_SUCCESS;
}

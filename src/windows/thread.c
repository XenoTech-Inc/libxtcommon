/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/thread.h>
#include <_xt/error.h>
#include <xt/error.h>
#include <xt/dlload.h>

// System headers
#include <ntstatus.h> // NTSTATUS and STATUS_XXX
#include <process.h> // beginthreadex

extern NTSTATUS WINAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *crit);
extern NTSTATUS WINAPI RtlDeleteCriticalSection(RTL_CRITICAL_SECTION *crit);
extern NTSTATUS WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
extern BOOL WINAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
extern NTSTATUS WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *crit);

static volatile HANDLE _xtLockMutex = NULL;

static int _xtLockMutexLock(void)
{
	if (!_xtLockMutex) {
		HANDLE p = CreateMutex(NULL, FALSE, NULL);
		while (NULL != InterlockedCompareExchangePointer((PVOID*)&_xtLockMutex, (PVOID)p, NULL))
			;
	}
	return WaitForSingleObject(_xtLockMutex, INFINITE) == WAIT_FAILED;
}

static int _xtLockMutexUnlock(void)
{
	return ReleaseMutex(_xtLockMutex);
}

int xtMutexCreate(xtMutex *m)
{
	RtlInitializeCriticalSection(m);
	return 0;
}

void xtMutexDestroy(xtMutex *m)
{
	// Windows seems to have builtin destruction guards so that means
	// that we don't have to do anything here
	RtlDeleteCriticalSection(m);
}

static bool _xtMutexIsInit(const xtMutex *m)
{
	xtMutex empty = XT_MUTEX_INIT;
	return memcmp(m, &empty, sizeof empty) != 0;
}

int xtMutexLock(xtMutex *m)
{
	_xtLockMutexLock();
	/* Create if lock is statically initialized */
	if (!_xtMutexIsInit(m)) {
		int retval = xtMutexCreate(m);
		if (retval)
			return retval;
	}
	RtlEnterCriticalSection(m);
	_xtLockMutexUnlock();
	return 0;
}

int xtMutexTryLock(xtMutex *m)
{
	int retval;
	_xtLockMutexLock();
	/* Create if lock is statically initialized */
	if (!_xtMutexIsInit(m)) {
		int retval = xtMutexCreate(m);
		if (retval)
			return retval;
	}
	retval = RtlTryEnterCriticalSection(m) == TRUE ? 0 : XT_EBUSY;
	_xtLockMutexUnlock();
	return retval;
}

int xtMutexUnlock(xtMutex *m)
{
	RtlLeaveCriticalSection(m);
	return 0;
}

int xtThreadContinue(struct xtThread *t)
{
	if (xtThreadGetID(t) == xtThreadGetID(NULL))
		return XT_EINVAL; // Do not allow the same caller
	xtMutexLock(&t->suspendMutex);
	int suspendCount = --t->suspendCount;
	xtMutexUnlock(&t->suspendMutex);
	if (suspendCount <= 0)
		ResumeThread(t->nativeThread);
	return 0;
}

static unsigned __stdcall thread_start(void *arg)
{
	struct xtThread *t = arg;
	// Execute the function
	t->funcRet = t->func(t, t->arg);
	SetEvent(t->exitEvent); // Signal that the thread has ended
	return 0;
}

int xtThreadCreate(struct xtThread *t, void *(*func)(struct xtThread *t, void *arg), void *arg, unsigned stackSizeKB, int guardSizeKB)
{
	(void)guardSizeKB;
	t->func = func;
	t->arg = arg;
	t->exitEvent = NULL;
	t->nativeThread = NULL;
	if (xtMutexCreate(&t->suspendMutex) != 0)
		goto error;
	t->exitEvent = CreateEvent(NULL, true, false, NULL); // Set the object to non-signaled
	if (t->exitEvent == NULL)
		goto error;
	t->suspendCount = 0;
	// Specifying zero as stack size to _beginthreadex makes it use the main threads stack size
	t->nativeThread = (HANDLE)_beginthreadex(NULL, stackSizeKB * 1024, thread_start, t, 0, &t->tid);
	if (t->nativeThread == 0)
		goto error;
	return 0;
error:
	CloseHandle(t->nativeThread);
	CloseHandle(t->exitEvent);
	xtMutexDestroy(&t->suspendMutex);
	return _xtTranslateSysError(errno); // Yes, this time it's errno on Windows instead of GetLastError()
}

size_t xtThreadGetID(const struct xtThread *t)
{
	if (t)
		return t->tid;
	else
		return GetCurrentThreadId();
}

typedef HRESULT (WINAPI *_xtGetThreadDescription)(HANDLE hThread, PWSTR *description);
typedef HRESULT (WINAPI *_xtSetThreadDescription)(HANDLE hThread, PCWSTR description);

char *xtThreadGetName(char *buf, size_t buflen)
{
	HRESULT hr;
	PWSTR data = NULL;
	char *ptr = NULL;
	_xtGetThreadDescription func = (_xtGetThreadDescription)GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")), "GetThreadDescription"
	);
	if (!func)
		goto fail;
	/* Grab name */
	hr = func(GetCurrentThread(), &data);
	if (!SUCCEEDED(hr))
		goto fail;
	/* Convert name */
	if (!WideCharToMultiByte(CP_OEMCP, 0, data, -1, buf, buflen, 0, 0))
		goto fail;
	ptr = buf;
fail:
	if (data)
		LocalFree(data);
	return ptr;
}

int xtThreadGetSuspendCount(struct xtThread *t)
{
	xtMutexLock(&t->suspendMutex);
	int suspendCount = t->suspendCount;
	xtMutexUnlock(&t->suspendMutex);
	return suspendCount;
}

bool xtThreadIsAlive(const struct xtThread *t)
{
	// Check if the event is signaled. As long as it ain't, the thread is still alive
	return WaitForSingleObject(t->exitEvent, 0) != WAIT_OBJECT_0;
}

int xtThreadJoin(struct xtThread *t, void **ret)
{
	// Block until the thread has terminated
	if (WaitForSingleObject(t->exitEvent, INFINITE) == WAIT_FAILED)
		return _xtTranslateSysError(GetLastError());
	// Perform cleanup
	CloseHandle(t->nativeThread);
	CloseHandle(t->exitEvent);
	xtMutexDestroy(&t->suspendMutex);
	if (ret != NULL) // Optional
		*ret = t->funcRet;
	return 0;
}

void xtThreadSetName(const char *name)
{
	size_t len = strlen(name);
	WCHAR *lname = NULL;
	_xtSetThreadDescription func = (_xtSetThreadDescription)GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")), "SetThreadDescription"
	);
	if (!func)
		goto fail;
	/* Convert name */
	if (!(lname = malloc((len + 1) * sizeof *lname)))
		goto fail;
	if (!MultiByteToWideChar(CP_OEMCP, 0, name, -1, lname, len + 1))
		goto fail;
	/* Apply name */
	func(GetCurrentThread(), lname);
fail:
	if (lname)
		free(lname);
}

int xtThreadSuspend(struct xtThread *t)
{
	if (xtThreadGetID(t) != xtThreadGetID(NULL))
		return XT_EINVAL; // Only allow the same caller
	xtMutexLock(&t->suspendMutex);
	int suspendCount = ++t->suspendCount;
	xtMutexUnlock(&t->suspendMutex);
	if (suspendCount > 0)
		SuspendThread(t->nativeThread);
	return 0;
}

void xtThreadYield(void)
{
	SwitchToThread();
}

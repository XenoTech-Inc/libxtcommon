// XT headers
#include <xt/thread.h>
#include <_xt/error.h>
#include <xt/error.h>

// System headers
#include <ntstatus.h> // NTSTATUS and STATUS_XXX
#include <process.h> // beginthreadex

extern NTSTATUS WINAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *crit);
extern NTSTATUS WINAPI RtlDeleteCriticalSection(RTL_CRITICAL_SECTION *crit);
extern NTSTATUS WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
extern BOOL WINAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
extern NTSTATUS WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *crit);

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

int xtMutexLock(xtMutex *m)
{
	RtlEnterCriticalSection(m);
	return 0;
}

int xtMutexTryLock(xtMutex *m)
{
	return RtlTryEnterCriticalSection(m) == TRUE ? 0 : XT_EBUSY;
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

char *xtThreadGetName(char *buf, size_t buflen)
{
	(void)buf;
	(void)buflen;
	return NULL;
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
	(void)name;
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

// XT headers
#include <xt/thread.h>
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

static unsigned __stdcall _xtThreadStart(void *arg)
{
	struct xtThread *t = arg;
	// Execute the function
	t->func(t, t->arg);
	// The thread has forfilled it's purpose now, let it die in peace
	SetEvent(t->exitEvent); // Signal that the thread has ended
	// Let the join function do the cleanup. That way thread status can still be requested
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

int xtThreadCreate(struct xtThread *t, void *(*func) (struct xtThread *t, void *arg), void *arg, unsigned stackSizeKB)
{
	// Turn it into KB's
	stackSizeKB *= 1024;
	t->func = func;
	t->arg = arg;
	t->exitEvent = NULL;
	t->nativeThread = NULL;
	t->exitEvent = CreateEvent(NULL, true, false, NULL); // Set the object to non-signaled
	if (t->exitEvent == NULL)
		goto error;
	t->suspendCount = 0;
	// Specifying zero as stack size to _beginthreadex makes it use the OS default already
	t->nativeThread = (HANDLE) _beginthreadex(NULL, stackSizeKB, _xtThreadStart, t, 0, &t->tid);
	if (t->nativeThread == 0)
		goto error;
	return 0;
error:
	CloseHandle(t->exitEvent);
	CloseHandle(t->nativeThread);
	return _xtTranslateSysError(errno); // Yes, this time it's errno on Windows instead of GetLastError()
}

size_t xtThreadGetID(const struct xtThread *t)
{
	if (t)
		return t->tid;
	else
		return GetCurrentThreadId();
}

inline int xtThreadGetSuspendCount(const struct xtThread *t)
{
	return t->suspendCount;
}

bool xtThreadIsAlive(const struct xtThread *t)
{
	// Check if the event is signaled. As long as it ain't, the thread is still alive
	return WaitForSingleObject(t->exitEvent, 0) != WAIT_OBJECT_0;
}

bool xtThreadJoin(struct xtThread *t)
{
	// No need to check if the thread is alive. This check should always block until the thread has terminated, 
	// and otherwise return immediately
	return WaitForSingleObject(t->exitEvent, INFINITE) == WAIT_OBJECT_0;
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

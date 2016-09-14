// XT headers
#include <xt/thread.h>
#include <xt/error.h>

// System headers
#include <ntstatus.h> // NTSTATUS and STATUS_XXX
#include <process.h> // beginthreadex

int xtMutexCreate(xtMutex *m)
{
	extern NTSTATUS WINAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlInitializeCriticalSection(m) == STATUS_SUCCESS ? 0 : _xtTranslateSysError(GetLastError());
}

int xtMutexDestroy(xtMutex *m)
{
	extern NTSTATUS WINAPI RtlDeleteCriticalSection(RTL_CRITICAL_SECTION *crit);
	RtlDeleteCriticalSection(m);
	return 0;
}

int xtMutexLock(xtMutex *m)
{
	extern NTSTATUS WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlEnterCriticalSection(m) == STATUS_SUCCESS ? 0 : _xtTranslateSysError(GetLastError());
}

int xtMutexTryLock(xtMutex *m)
{
	extern BOOL WINAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlTryEnterCriticalSection(m) == TRUE ? 0 : _xtTranslateSysError(GetLastError());
}

int xtMutexUnlock(xtMutex *m)
{
	extern NTSTATUS WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlLeaveCriticalSection(m) == STATUS_SUCCESS ? 0 : _xtTranslateSysError(GetLastError());
}

static unsigned __stdcall _xtThreadStart(void *arg)
{
	xtThread *t = arg;
	// Execute the function
	t->func(t, t->arg);
	// The thread has forfilled it's purpose now, let it die in peace
	SetEvent(t->exitEvent); // Signal that the thread has ended
	// Let the join function do the cleanup. That way thread status can still be requested
	return 0;
}

int xtThreadContinue(xtThread *t)
{
	return ResumeThread(t->nativeThread);
}

int xtThreadCreate(xtThread *t, void *(*func) (xtThread *t, void *arg), void *arg, unsigned stackSizeKB)
{
	// Turn it into KB's
	stackSizeKB *= 1024;
	// If it's zero, it will be the default size
	// If it's larger than zero but small than the minimum stack size, make it the minimum size
	if (stackSizeKB < 65535)
		stackSizeKB = 65535;
	t->func = func;
	t->arg = arg;
	t->exitEvent = NULL;
	t->nativeThread = NULL;
	t->exitEvent = CreateEvent(NULL, true, false, NULL); // Set the object to non-signaled
	if (t->exitEvent == NULL)
		goto error;
	t->nativeThread = (HANDLE) _beginthreadex(NULL, stackSizeKB, _xtThreadStart, t, 0, &t->tid);
	if (t->nativeThread == 0)
		goto error;
	return 0;
error:
	CloseHandle(t->exitEvent);
	CloseHandle(t->nativeThread);
	return XT_ENOMEM;
}

size_t xtThreadGetID(const xtThread *t)
{
	if (t)
		return t->tid;
	else
		return GetCurrentThreadId();
}

bool xtThreadIsAlive(const xtThread *t)
{
	// Check if the event is signaled. As long as it ain't, the thread is still alive
	return WaitForSingleObject(t->exitEvent, 0) != WAIT_OBJECT_0;
}

bool xtThreadJoin(xtThread *t)
{
	// No need to check if the thread is alive. This check should always block until the thread has terminated, 
	// and otherwise return immidiately
	return WaitForSingleObject(t->exitEvent, INFINITE) == WAIT_OBJECT_0;
}

int xtThreadSuspend(xtThread *t)
{
	if (xtThreadGetID(t) != xtThreadGetID(NULL))
		return 0;
	return SuspendThread(t->nativeThread);
}

void xtThreadYield(void)
{
	SwitchToThread();
}

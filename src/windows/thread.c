// XT headers
#include <xt/thread.h>
#include <xt/error.h>

// System headers
#include <ntstatus.h> // NTSTATUS and STATUS_XXX
#include <process.h> // beginthreadex

int xtMutexCreate(xtMutex *m)
{
	// Always succeeds on Windows, since there is no real good way to check it
	//InitializeCriticalSection(m);
	extern NTSTATUS WINAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlInitializeCriticalSection(m) == STATUS_SUCCESS ? 0 : XT_ENOMEM;
}

void xtMutexDestroy(xtMutex *m)
{
	//DeleteCriticalSection(m);
	extern NTSTATUS WINAPI RtlDeleteCriticalSection(RTL_CRITICAL_SECTION *crit);
	RtlDeleteCriticalSection(m);
}

bool xtMutexLock(xtMutex *m)
{
	//EnterCriticalSection(m);
	extern NTSTATUS WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlEnterCriticalSection(m) == STATUS_SUCCESS;
}

bool xtMutexTryLock(xtMutex *m)
{
	//return TryEnterCriticalSection(m) == TRUE;
	extern BOOL WINAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlTryEnterCriticalSection(m) == TRUE;
}

bool xtMutexUnlock(xtMutex *m)
{
	//LeaveCriticalSection(m);
	extern NTSTATUS WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *crit);
	return RtlLeaveCriticalSection(m) == STATUS_SUCCESS;
}

static unsigned __stdcall _xtThreadStart(void *arg)
{
	xtThread *t = arg;
	// Execute the function
	t->func(t->arg);
	// The thread has forfilled it's purpose now, let it die in peace
	SetEvent(t->exitEvent); // Signal that the thread has ended
	// Let the join function do the cleanup. That way thread status can still be requested
	return 0;
}

int xtThreadCreate(xtThread *t, void *(*func) (void *arg), void *arg, unsigned stackSizeKB)
{
	// Turn it into KB's
	stackSizeKB *= 1024;
	// If it's zero, it will be the default size
	// If it's larger than zero but small than the minimum stack size, make it the minimum size
	if (stackSizeKB == 0)
		stackSizeKB = 1024 * 1024 * 1;
	else if (stackSizeKB < 65535)
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

void xtThreadYield(void)
{
	SwitchToThread();
}

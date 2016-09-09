// XT headers
#include <xt/thread.h>
#include <xt/error.h>

// System headers
#include <errno.h> // for ESRCH
#include <signal.h> // pthread_kill
#include <sched.h> // All sched things
#include <sys/syscall.h> // syscall
#include <unistd.h> // usleep

// STD headers
#include <stdlib.h> // malloc, free
#include <time.h> // struct timespec and friends

int xtMutexCreate(xtMutex *m)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	return pthread_mutex_init(m, &attr) == 0 ? 0 : XT_ENOMEM;
}

void xtMutexDestroy(xtMutex *m)
{
	pthread_mutex_destroy(m);
}

bool xtMutexLock(xtMutex *m)
{
	return pthread_mutex_lock(m) == 0;
}

bool xtMutexTryLock(xtMutex *m)
{
	return pthread_mutex_trylock(m) == 0;
}

bool xtMutexUnlock(xtMutex *m)
{
	return pthread_mutex_unlock(m) == 0;
}

static void *_xtThreadStart(void *arg)
{
	xtThread *t = arg;
	// Execute the function
	t->func(t->arg);
	// The thread has forfilled it's purpose now, let it die in peace
	// Let the join function do the cleanup. That way thread status can still be requested
	return NULL;
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
	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0 || pthread_attr_setstacksize(&attr, stackSizeKB) != 0 || 
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0 ||
		pthread_attr_setguardsize(&attr, 0) != 0) // Disable the stack guard so we won't waste resources
		goto error;
	if (pthread_create(&t->nativeThread, &attr, _xtThreadStart, t) != 0)
		goto error;
	pthread_attr_destroy(&attr); // Can do this right now, since it is copied over to the new thread by the implementation
	return 0;
error:
	return XT_ENOMEM;
}

size_t xtThreadGetID(const xtThread *t)
{
	if (t)
		return t->nativeThread;
	else
		return pthread_self();
}

bool xtThreadIsAlive(const xtThread *t)
{
	// If ESRCH is returned it means that no such thread was found, thus the thread is dead in our words
	return pthread_kill(t->nativeThread, 0) != ESRCH;
}

bool xtThreadJoin(xtThread *t)
{
	// No need to check if the thread is alive. This check should always block until the thread has terminated, 
	// and otherwise return immidiately
	return pthread_join(t->nativeThread, NULL) == 0;
}

void xtThreadYield(void)
{
	sched_yield();
}

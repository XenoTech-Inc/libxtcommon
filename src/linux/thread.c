// XT headers
#include <xt/thread.h>
#include <xt/error.h>

// System headers
#include <errno.h> // for ESRCH and the errno variable
#include <signal.h> // pthread_kill
#include <sched.h> // All sched things
#include <sys/syscall.h> // syscall
#include <unistd.h> // usleep

// STD headers
#include <limits.h> // PTHREAD_STACK_MIN
#include <stdlib.h> // malloc, free
#include <time.h> // struct timespec and friends

int xtMutexCreate(xtMutex *m)
{
	int ret;
	pthread_mutexattr_t attr;
	ret = pthread_mutexattr_init(&attr);
	if (ret != 0)
		return _xtTranslateSysError(ret);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	ret = pthread_mutex_init(m, &attr);
	pthread_mutexattr_destroy(&attr);
	return ret == 0 ? 0 : _xtTranslateSysError(ret);
}

int xtMutexDestroy(xtMutex *m)
{
	int ret = pthread_mutex_destroy(m);
	return ret == 0 ? 0 : _xtTranslateSysError(ret);
}

int xtMutexLock(xtMutex *m)
{
	int ret = pthread_mutex_lock(m);
	return ret == 0 ? 0 : _xtTranslateSysError(ret);
}

int xtMutexTryLock(xtMutex *m)
{
	int ret = pthread_mutex_trylock(m);
	return ret == 0 ? 0 : _xtTranslateSysError(ret);
}

int xtMutexUnlock(xtMutex *m)
{
	int ret = pthread_mutex_unlock(m);
	return ret == 0 ? 0 : _xtTranslateSysError(ret);
}

static void *_xtThreadStart(void *arg)
{
	xtThread *t = arg;
	// Execute the function
	t->func(t, t->arg);
	// The thread has forfilled it's purpose now, let it die in peace
	// Let the join function do the cleanup. That way thread status can still be requested
	return NULL;
}

int xtThreadContinue(xtThread *t)
{
	pthread_mutex_lock(&t->suspendMutex);
	--t->suspendCount;
	pthread_cond_signal(&t->suspendCond);
	pthread_mutex_unlock(&t->suspendMutex);
	return t->suspendCount;
}

int xtThreadCreate(xtThread *t, void *(*func) (xtThread *t, void *arg), void *arg, unsigned stackSizeKB)
{
	// Turn it into KB's
	stackSizeKB *= 1024;
	// If it's zero, it will be the default size
	// If it's larger than zero but small than the minimum stack size, make it the minimum size
	if (stackSizeKB == 0)
		stackSizeKB = PTHREAD_STACK_MIN;
	t->func = func;
	t->arg = arg;
	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0)
		goto error;
	// Can fail if someone specifies a ridiculous size
	if (pthread_attr_setstacksize(&attr, stackSizeKB) != 0) {
		pthread_attr_destroy(&attr);
		goto error;
	}
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	// Disable the stack guard so we won't waste resources
	pthread_attr_setguardsize(&attr, 0);
	if (pthread_cond_init(&t->suspendCond, NULL) != 0) {
		pthread_attr_destroy(&attr);
		goto error;
	}
	if (pthread_mutex_init(&t->suspendMutex, NULL) != 0) {
		pthread_cond_destroy(&t->suspendCond);
		pthread_attr_destroy(&attr);
		goto error;
	}
	t->suspendCount = 0;
	if (pthread_create(&t->nativeThread, &attr, _xtThreadStart, t) != 0) {
		pthread_mutex_destroy(&t->suspendMutex);
		pthread_cond_destroy(&t->suspendCond);
		pthread_attr_destroy(&attr);
		goto error;
	}
	pthread_attr_destroy(&attr); // Can do this right now, since it is copied over to the new thread by the implementation
	return 0;
error:
	return _xtTranslateSysError(errno);
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
	pthread_mutex_destroy(&t->suspendMutex);
	pthread_cond_destroy(&t->suspendCond);
	// No need to check if the thread is alive. This check should always block until the thread has terminated, 
	// and otherwise return immediately
	return pthread_join(t->nativeThread, NULL) == 0;
}

int xtThreadSuspend(xtThread *t)
{
	if (xtThreadGetID(t) != xtThreadGetID(NULL))
		return 0;
	pthread_mutex_lock(&t->suspendMutex);
	++t->suspendCount;
	while (t->suspendCount > 0) {
		pthread_cond_wait(&t->suspendCond, &t->suspendMutex);
	}
	pthread_mutex_unlock(&t->suspendMutex);
	return t->suspendCount;
}

void xtThreadYield(void)
{
	sched_yield();
}

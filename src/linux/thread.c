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
	if (ret != 0) // Should be impossible to happen but still check it
		return _xtTranslateSysError(ret);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	ret = pthread_mutex_init(m, &attr);
	pthread_mutexattr_destroy(&attr);
	return ret == 0 ? 0 : _xtTranslateSysError(ret);
}

void xtMutexDestroy(xtMutex *m)
{
	// Linux seems to have built-in destruction guards so that means
	// that we don't have to do anything here
	pthread_mutex_destroy(m);
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
	struct xtThread *t = arg;
	// Execute the function
	return t->func(t, t->arg);
}

int xtThreadContinue(struct xtThread *t)
{
	if (xtThreadGetID(t) == xtThreadGetID(NULL))
		return XT_EINVAL; // Do not allow the same caller
	pthread_mutex_lock(&t->suspendMutex);
	int suspendCount = --t->suspendCount;
	pthread_mutex_unlock(&t->suspendMutex);
	if (suspendCount <= 0)
		pthread_cond_signal(&t->suspendCond);
	return 0;
}

int xtThreadCreate(struct xtThread *t, void *(*func) (struct xtThread *t, void *arg), void *arg, unsigned stackSizeKB, int guardSizeKB)
{
	t->func = func;
	t->arg = arg;
	if (pthread_attr_init(&t->attr) != 0)
		goto error;
	// Set the custom stack size if desired
	if (stackSizeKB != 0) {
		if (pthread_attr_setstacksize(&t->attr, stackSizeKB * 1024) != 0)
			goto error;
	}
	if (guardSizeKB == -1 || guardSizeKB > 0) {
		if (pthread_attr_setguardsize(&t->attr, guardSizeKB == -1 ? 0 : guardSizeKB * 1024) != 0)
			goto error;
	} else {
		pthread_attr_destroy(&t->attr);
		return XT_EINVAL;
	}
	// Even although the default should be joinable, set it manually to be ultra-safe.
	pthread_attr_setdetachstate(&t->attr, PTHREAD_CREATE_JOINABLE);
	if (pthread_cond_init(&t->suspendCond, NULL) != 0) {
		goto error;
	}
	if (pthread_mutex_init(&t->suspendMutex, NULL) != 0) {
		pthread_cond_destroy(&t->suspendCond);
		goto error;
	}
	t->suspendCount = 0;
	if (pthread_create(&t->nativeThread, &t->attr, _xtThreadStart, t) != 0) {
		pthread_mutex_destroy(&t->suspendMutex);
		pthread_cond_destroy(&t->suspendCond);
		goto error;
	}
	return 0;
error:
	pthread_attr_destroy(&t->attr);
	return _xtTranslateSysError(errno);
}

size_t xtThreadGetID(const struct xtThread *t)
{
	if (t)
		return t->nativeThread;
	else
		return pthread_self();
}

int xtThreadGetSuspendCount(struct xtThread *t)
{
	pthread_mutex_lock(&t->suspendMutex);
	int suspendCount = t->suspendCount;
	pthread_mutex_unlock(&t->suspendMutex);
	return suspendCount;
}

bool xtThreadIsAlive(const struct xtThread *t)
{
	// If ESRCH is returned it means that no such thread was found, thus the thread is dead in our words
	return pthread_kill(t->nativeThread, 0) != ESRCH;
}

void *xtThreadJoin(struct xtThread *t)
{
	// Block until the thread has terminated
	void* ret;
	pthread_join(t->nativeThread, &ret);
	// Perform cleanup
	pthread_mutex_destroy(&t->suspendMutex);
	pthread_cond_destroy(&t->suspendCond);
	pthread_attr_destroy(&t->attr);
	return ret;
}

int xtThreadSuspend(struct xtThread *t)
{
	if (xtThreadGetID(t) != xtThreadGetID(NULL))
		return XT_EINVAL; // Only allow the same caller
	pthread_mutex_lock(&t->suspendMutex);
	++t->suspendCount;
	while (t->suspendCount > 0)
		pthread_cond_wait(&t->suspendCond, &t->suspendMutex);
	pthread_mutex_unlock(&t->suspendMutex);
	return 0;
}

void xtThreadYield(void)
{
	sched_yield();
}

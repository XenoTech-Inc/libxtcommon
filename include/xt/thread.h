/**
 * @brief Cross platform thread functionality.
 *
 * Lightweight threads with all the functionality that you need!
 * @file thread.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_THREAD_H
#define _XT_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/os_macros.h>

// System headers
#if XT_IS_LINUX
	#include <pthread.h>
#elif XT_IS_WINDOWS
	#include <windows.h>
#endif

// STD headers
#include <stdbool.h>

/**
 * @brief Cross platform mutex.
 *
 * On Linux the native type is pthread_mutex_t, on Windows the RTL_CRITICAL_SECTION type is used, which
 * is the fastest mutex type that we can get on Windows. All mutexes are of the recursive type.
 * An extra performance boost is gained because the mutexes are created on the stack.
 * You should threat this mutex as if it were opaque.
 */
#if XT_IS_LINUX
	typedef pthread_mutex_t xtMutex;
#elif XT_IS_WINDOWS
	typedef RTL_CRITICAL_SECTION xtMutex;
#endif
/**
 * Creates a new mutex. Attempting to initialize an already initialized mutex results in undefined behavior.
 * @return Zero if the mutex has been created, otherwise an error code.
 */
int xtMutexCreate(xtMutex *m);
/**
 * Destroys the mutex, all memory associated with the mutex is released.
 * Destroying a mutex which is still locked or is unintialized results in undefined behavior.
 */
void xtMutexDestroy(xtMutex *m);
/**
 * Attempts to lock the mutex. If the mutex is already locked by another thread,
 * the thread waits for the mutex to become available. The thread that has locked a mutex becomes its
 * current owner and remains the owner until the same thread has unlocked it.
 * When the mutex has the attribute of recursive, the use of the lock may be different.
 * When this kind of mutex is locked multiple times by the same thread, then a count is incremented and
 * no waiting thread is posted. The owning thread must call xtMutexUnlock() the same number of times to decrement the count to zero.
 * @return Zero if the mutex has been locked, otherwise an error code.
 */
int xtMutexLock(xtMutex *m);
/**
 * Tries to lock the mutex. Returns instantly whether or not it failed. Use this to retrieve to the lock to a mutex without blocking.
 * For recursive mutexes, this function will effectively add to the count of the number of times xtMutexUnlock() must
 * be called by the thread to release the mutex. (That is, it has the same behavior as xtMutexLock().)
 * @return Zero if the mutex has been locked, otherwise an error code.
 */
int xtMutexTryLock(xtMutex *m);
/**
 * Unlocks the mutex so it is ready for use by other threads.
 * @return Zero if the mutex has been unlocked, otherwise an error code.
 */
int xtMutexUnlock(xtMutex *m);
/**
 * @brief Cross platform thread.
 *
 * You should threat this struct as if it were opaque.
 */
struct xtThread {
	/** The target function for the thread to execute. */
	void *(*func)(struct xtThread *t, void *arg);
	/** The argument that is passed to the target function. */
	void *arg;
	int suspendCount;
#if XT_IS_LINUX
	pthread_t nativeThread;
	pthread_cond_t suspendCond;
	pthread_mutex_t suspendMutex;
	pthread_attr_t attr;
#elif XT_IS_WINDOWS
	HANDLE exitEvent, nativeThread;
	xtMutex suspendMutex;
	unsigned tid;
	void *funcRet;
#endif
};
/**
 * This function decreases the thread's suspend count by one.
 * If the suspend count is zero or lower, the thread shall be woken up when asleep.
 * If the resulting value is zero, then the execution of the subject thread is resumed.
 * @param t - The thread of which to decrease the suspend count. This must NOT be the caller thread.
 * @return Zero on success, otherwise an error code.
 */
int xtThreadContinue(struct xtThread *t);
/**
 * Creates a new thread. If the thread has been created successfully, it will start execution immediately.
 * @param func - A function pointer to the function which the thread shall execute.
 * @param arg - An optional argument which the function takes.
 * @param stackSizeKB - The stack size in KB for the thread. Specify zero to use the OS default stack size. If this value
 * is too low for your new thread, the thread may suddenly terminate when starting.
 * @param guardSizeKB - The guard size in KB for the thread. Specify zero to use the OS default guard size, -1 to disable the guard.
 * @return Zero if the thread has been created, otherwise an error code.
 * @remarks You need to call xtThreadJoin() to clean up the new thread properly, otherwise system resources will leak.
 */
int xtThreadCreate(struct xtThread *t, void *(*func)(struct xtThread *t, void *arg), void *arg, unsigned stackSizeKB, int guardSizeKB);
/**
 * Returns the unique identifier of the specified thread. Pass a NULL to get the ID of the caller thread.
 */
size_t xtThreadGetID(const struct xtThread *t);
/**
 * Retrieves the name of the caller thread.
 * @return A pointer to \a buf on success, NULL on failure.
 */
char *xtThreadGetName(char *buf, size_t buflen);
/**
 * Returns the current suspend count for the specified thread.
 */
int xtThreadGetSuspendCount(struct xtThread *t);
/**
 * Returns if the specified thread is still executing their task and has not terminated yet.
 */
bool xtThreadIsAlive(const struct xtThread *t);
/**
 * Joins the specified thread. This operation blocks until the specified thread has terminated.
 * All of the thread's resources will be cleaned up. A second call to this function for an
 * already joined thread will result in undefined behavior.
 * @param ret - An optional pointer which shall receive the thread's exit value
 * on success.
 * @return - Zero on success, otherwise an error code.
 */
int xtThreadJoin(struct xtThread *t, void **ret);
/**
 * Sets the name of the caller thread. The thread's name is the name that also
 * shows up in debuggers.
 * @param name - The new name for the caller thread. It may be a maximum of 16
 * characters (including the null-terminator) long. Any characters beyond that
 * will silently be truncated.
 */
void xtThreadSetName(const char *name);
/**
 * Increases the suspend count by one. If the suspend count is higher than zero, the thread will be suspended
 * on a call to this function. The thread will not be suspended when the suspend count is zero or lower.
 * @param t - The thread to put to sleep. This must ALWAYS be the caller thread.
 * @return - Zero on success, otherwise an error code.
 */
int xtThreadSuspend(struct xtThread *t);
/**
 * Causes the calling thread to relinquish the CPU.
 * The thread is moved to the end of the queue for its static priority and a new thread gets to run.
 */
void xtThreadYield(void);

#ifdef __cplusplus
}
#endif

#endif

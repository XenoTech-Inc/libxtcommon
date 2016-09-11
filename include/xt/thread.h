/**
 * @brief Cross platform thread functionality.
 * 
 * Lightweight threads with all the functionality that you need!
 * @file thread.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_THREAD_H
#define _XT_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/_base.h>
#include <xt/os_macros.h>

// System headers
#if defined(XT_IS_LINUX)
	#include <pthread.h>
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
#if defined(XT_IS_LINUX)
	typedef pthread_mutex_t xtMutex;
#elif defined(XT_IS_WINDOWS)
	typedef RTL_CRITICAL_SECTION xtMutex;
#endif
/**
 * @brief Cross platform thread.
 * 
 * You should threat this struct as if it were opaque.
 */
typedef struct xtThread {
	/** The target function for the thread to execute. */
	void *(*func) (struct xtThread *t, void *arg);
	/** The argument that is passed to the target function. */
	void *arg;
	#if defined(XT_IS_LINUX)
		pthread_t nativeThread;
		pthread_cond_t suspendCond;
		int suspendCount;
		pthread_mutex_t suspendMutex;
	#elif defined(XT_IS_WINDOWS)
		HANDLE exitEvent, nativeThread;
		unsigned tid;
	#endif
} xtThread;
/**
 * Creates a new mutex. Attempting to initialize an already initialized mutex results in undefined behavior.
 * @return Zero if the mutex has been created, otherwise an error code.
 */
int xtMutexCreate(xtMutex *m);
/**
 * Destroys the mutex, all memory associated with the mutex is released. 
 * Destroying a mutex which is still locked or is unintialized results in undefined behavior.
 * @return Zero if the mutex has been destroyed, otherwise an error code.
 */
int xtMutexDestroy(xtMutex *m);
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
typedef struct xtThread {
	/** The target function for the thread to execute. */
	void *(*func) (struct xtThread *t, void *arg);
	/** The argument that is passed to the target function. */
	void *arg;
#if defined(XT_IS_LINUX)
	pthread_t nativeThread;
	pthread_cond_t suspendCond;
	int suspendCount;
	pthread_mutex_t suspendMutex;
#elif defined(XT_IS_WINDOWS)
	HANDLE exitEvent, nativeThread;
	unsigned tid;
#endif
} xtThread;
/**
 * If the suspend count is zero, the thread is not currently suspended. 
 * Otherwise, the subject thread's suspend count is decremented. 
 * If the resulting value is zero, then the execution of the subject thread is resumed.
 * @return - The previous suspend count for the thread.
 */
int xtThreadContinue(xtThread *t);
/**
 * If the suspend count is zero, the thread is not currently suspended. 
 * Otherwise, the subject thread's suspend count is decremented. 
 * If the resulting value is zero, then the execution of the subject thread is resumed.
 * @return - The previous suspend count for the thread.
 */
int xtThreadContinue(xtThread *t);
/**
 * Creates a new thread. If the thread has been created successfully, it will start execution immidiately.\n
 * Errors:\n
 * XT_ENOMEM - The limit of threads has been reached or if the system is lacking the resources to create a new thread.
 * @param func - A function pointer to the function which the thread shall execute.
 * @param arg - An optional argument which the function takes.
 * @param stackSizeKB - The stack size in KB for the thread. Specify zero to get the lowest size possible. If this value 
 * is too low for your new thread, the thread may suddenly terminate when starting.
 * @return 0 for success. Otherwise an error code.
 * @remarks You need to call the join function before exiting. Otherwise system resources will leak.
 */
int xtThreadCreate(xtThread *t, void *(*func) (xtThread *t, void *arg), void *arg, unsigned stackSizeKB);
/**
 * Returns the unique identifier of the specified thread. Pass a null to get the ID of the caller thread.
 */
size_t xtThreadGetID(const xtThread *t);
/**
 * Returns if the specified thread is still executing their task and has not terminated yet. 
 */
bool xtThreadIsAlive(const xtThread *t);
/**
 * Joins the specified thread. This operation blocks until the specified thread has terminated. 
 * All of the thread's resources will then be cleaned up. A call to this function should ONLY be made once.
 * Once this function has executed, the specified thread becomes invalid and should NOT be used again for any purpose.
 * @returns If the join operation was successful.
 * @remarks This function call MUST be made to a thread! If this is not done, resources will leak.
 */
bool xtThreadJoin(xtThread *t);
/**
 * Each thread has a suspend count, and if the suspend count is greater than zero, the thread is suspended; otherwise, the thread is not suspended 
 * and is eligible for execution. Calling xtThreadSuspend() causes the target thread's suspend count to be incremented.
 * @param t - The thread to put to sleep. This must ALWAYS be the caller thread.
 * @return - The previous suspend count for the thread.
 */
int xtThreadSuspend(xtThread *t);
/**
 * Causes the calling thread to relinquish the CPU. 
 * The thread is moved to the end of the queue for its static priority and a new thread gets to run.
 */
void xtThreadYield(void);

#ifdef __cplusplus
}
#endif

#endif

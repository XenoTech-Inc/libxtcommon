/**
 * @brief Used to manage running processes on the system.
 *
 * @file proc.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_PROC_H
#define _XT_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>

/**
 * This structure contains the status of memory usage of a process.
 * All values are in bytes.
 */
struct xtProcMemoryInfo {
	/** Peak resident set size ("high water mark"). */
	unsigned long long hwm;
	/**
	 * Resident set size (RSS). This the portion of memory that resides in the
	 * physical memory.
	 */
	unsigned long long rss;
	/** The amount of bytes that are swapped out. */
	unsigned long long swap;
	/** Peak virtual memory size. */
	unsigned long long vmPeak;
};
/**
 * All supported signals that can be sent to processes.
 */
enum xtProcSignal {
	/** Hangup detected on controlling terminal or death of the controlling
	 * process.
	 */
	XT_SIGHUP,
	/** Interrupt from keyboard. */
	XT_SIGINT,
	/** Kill signal. */
	XT_SIGKILL,
	/** Termination signal. */
	XT_SIGTERM,
	/** User-defined signal 1. */
	XT_SIGUSR1,
	/** User-defined signal 2. */
	XT_SIGUSR2,
	/** Continue if stopped */
	XT_SIGCONT,
	/** Stop process. */
	XT_SIGSTOP
};
/**
 * Returns the PID for the current process. This function will always succeed.
 */
unsigned xtProcGetCurrentPID(void);
/**
 * Retrieves the memory information for \a pid.
 * @return Zero if the information has been retrieved, otherwise an error code.
 */
int xtProcGetMemoryInfo(struct xtProcMemoryInfo *info, unsigned pid);

int xtProcGetName(char *buf, size_t buflen, unsigned pid);
/**
 * Fetches the PID's of all running processes. The order of the PID's is
 * random.
 * @param pids - The buffer which will receive all PID's.
 * @param pidCount - The amount of PID's that will fit into \a pids. On
 * success, it shall be updated to contain the amount of PID's that have
 * been retrieved.
 * @return Zero if all running PID's have been retrieved, otherwise an
 * error code.
 */
int xtProcGetPids(unsigned *restrict pids, unsigned *restrict pidCount);
/**
 * Returns the amount of processes that are active on the system. Zero is
 * returned on error.
 */
unsigned xtProcGetProcessCount(void);

bool xtProcIsAlive(unsigned pid);
/**
 * Kills \a pid with the specified signal. Do note that on Windows many
 * signals are not supported. Instead these signals perform a standard kill.
 */
int xtProcKill(unsigned pid, enum xtProcSignal signal);
/**
 * Blocks for a maximum of \a timeout milliseconds until the specified process
 * has terminated. If the process cannot be found, this function returns
 * immidiately. If a timeout occurs, the process would still be alive.
 * @param timeout - The maximum timeout in milliseconds. Zero means block
 * indefinitely.
 * @return Zero if the process existed and is no longer running, otherwise an
 * error code.
 */
int xtProcWait(unsigned pid, unsigned timeout);

#ifdef __cplusplus
}
#endif

#endif

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

// XT headers
#include <xt/_base.h>
#include <xt/os_macros.h>

// STD headers
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Used to calculate the CPU utilization of a process.
 *
 * You should threat this struct as if it were opaque.
 */
struct xtProcCPUTime {
#if XT_IS_LINUX
	long cstime, cutime;
	unsigned long stime, utime;
	unsigned long cpuTime;
#elif XT_IS_WINDOWS
	unsigned long long timestamp, stime, utime;
#endif
};
/**
 * @brief Contains memory usage information of a process.
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
 * Calculates the system wide CPU utilization given the timestamps provided by
 * \a start and \a end.
 * @return The global CPU utilization in percentage. This means that if you are
 * e.g. on an 8 core system and 100% is returned, that all cores are busy.
 */
float xtProcCPUTimeCalculate(const struct xtProcCPUTime *start,
	const struct xtProcCPUTime *end);
/**
 * Retrieves the CPU timestamps for \a pid. This function needs to be called a
 * second time after a small interval (500 msecs at minimum is recommended) in
 * order to be able to calculate the global CPU usage afterwards.
 * @return Zero if the timestamp has been retrieved, otherwise an error code.
 */
int xtProcGetCPUTime(unsigned pid, struct xtProcCPUTime *cpuTime);
/**
 * Returns the PID for the current process. This function will always succeed.
 */
unsigned xtProcGetCurrentPID(void);
/**
 * Retrieves the memory usage information for \a pid.
 * @return Zero if the information has been retrieved, otherwise an error code.
 */
int xtProcGetMemoryInfo(unsigned pid, struct xtProcMemoryInfo *info);

int xtProcGetName(unsigned pid, char *buf, size_t buflen);
/**
 * Fetches the PID's of all running processes. The order of the PID's is
 * undefined.
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
 * Kills \a pid with the specified signal.
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

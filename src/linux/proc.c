// XT headers
#include <xt/proc.h>
#include <_xt/error.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

// STD headers
#include <stdlib.h>
#include <string.h>

float xtProcCPUTimeCalculate(const struct xtProcCPUTime *start,
	const struct xtProcCPUTime *end)
{
	unsigned long diff = end->cpuTime - start->cpuTime;
	float scpu = (end->stime + end->cstime) - (start->stime + start->cstime);
	float ucpu = (end->utime + end->cutime) - (start->utime + start->cutime);
	return scpu / diff * 100 + ucpu / diff * 100;
}

int xtProcGetCPUTime(unsigned pid, struct xtProcCPUTime *cpuTime)
{
	int ret = 1;
	unsigned long cpuTimeRaw[10] = {0};
	FILE *pidStat = NULL, *procStat = NULL;
	char pidStatPath[64];
	cpuTime->cstime = 0;
	cpuTime->cutime = 0;
	cpuTime->stime = 0;
	cpuTime->utime = 0;
	cpuTime->cpuTime = 0;
	snprintf(pidStatPath, sizeof pidStatPath, "/proc/%u/stat", pid);
	if (!(pidStat = fopen(pidStatPath, "r")) ||
		!(procStat = fopen("/proc/stat", "r")))
		goto error;
	if (fscanf(pidStat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u"
		"%lu %lu %ld %ld %*d %*d %*d %*d %*u %*u %*d",
		&cpuTime->utime, &cpuTime->stime,
		&cpuTime->cutime, &cpuTime->cstime) == EOF)
		goto error;
	if (fscanf(procStat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		&cpuTimeRaw[0], &cpuTimeRaw[1], &cpuTimeRaw[2], &cpuTimeRaw[3],
		&cpuTimeRaw[4], &cpuTimeRaw[5], &cpuTimeRaw[6], &cpuTimeRaw[7],
		&cpuTimeRaw[8], &cpuTimeRaw[9]) == EOF)
		goto error;
	for (int i = 0; i < 10; ++i)
		cpuTime->cpuTime += cpuTimeRaw[i];
	ret = 0;
error:
	if (procStat)
		fclose(procStat);
	if (pidStat)
		fclose(pidStat);
	return ret == 0 ? 0 : _xtTranslateSysError(errno);
}

unsigned xtProcGetCurrentPID(void)
{
	return getpid();
}

int xtProcGetMemoryInfo(unsigned pid, struct xtProcMemoryInfo *info)
{
	char path[64];
	snprintf(path, sizeof path, "/proc/%u/status", pid);
	FILE *f = fopen(path, "r");
	if (!f)
		return _xtTranslateSysError(errno);
	memset(info, 0, sizeof *info); // Be safe incase we can't read everything
	char sbuf[256];
	char *tokens[10];
	unsigned numTokens;
	while (xtStringReadLine(sbuf, sizeof sbuf, NULL, f)) {
		numTokens = sizeof tokens;
		if (xtStringStartsWith(sbuf, "VmHWM")) {
			xtStringSplit(sbuf, " ", tokens, &numTokens);
			info->hwm = strtoull(tokens[1], NULL, 10) * 1024;
		} else if (xtStringStartsWith(sbuf, "VmRSS")) {
			xtStringSplit(sbuf, " ", tokens, &numTokens);
			info->rss = strtoull(tokens[1], NULL, 10) * 1024;
		} else if (xtStringStartsWith(sbuf, "VmSwap")) {
			xtStringSplit(sbuf, " ", tokens, &numTokens);
			info->swap = strtoull(tokens[1], NULL, 10) * 1024;
		} else if (xtStringStartsWith(sbuf, "VmPeak")) {
			xtStringSplit(sbuf, " ", tokens, &numTokens);
			info->vmPeak = strtoull(tokens[1], NULL, 10) * 1024;
		}
	}
	fclose(f);
	return 0;
}

int xtProcGetName(unsigned pid, char *buf, size_t buflen)
{
	char path[64];
	snprintf(path, sizeof path, "/proc/%u/cmdline", pid);
	int fd = open(path, O_RDONLY);
	if (fd == -1)
		return _xtTranslateSysError(errno);
	int ret = read(fd, buf, buflen);
	if (ret == -1)
		return _xtTranslateSysError(errno);
	else if (ret == 0) {
		// No bytes read, it's very likely that we don't
		// have access to read. Process owned by root probably.
		return XT_EPERM;
	}
	// Place a null terminator
	buf[(unsigned) ret >= buflen ? buflen - 1 : (unsigned) ret] = '\0';
	close(fd);
	return 0;
}

int xtProcGetPids(unsigned *restrict pids, unsigned *restrict pidCount)
{
	DIR *d;
	struct dirent *dir;
	if (!(d = opendir("/proc")))
		return _xtTranslateSysError(errno);
	unsigned currentPidCount = 0;
	unsigned maxPidCount = *pidCount;
	for (; currentPidCount < maxPidCount && (dir = readdir(d));) {
		// If the map starts with a number, it's always a legit process
		unsigned long pid = strtoul(dir->d_name, NULL, 10);
		if (pid != 0) { // The init process will not be logged, but we have no real choice
			pids[currentPidCount] = pid;
			++currentPidCount;
		}
	}
	closedir(d);
	*pidCount = currentPidCount;
	return 0;
}

unsigned xtProcGetProcessCount(void)
{
	DIR *d;
	struct dirent *dir;
	unsigned count = 0;
	if (!(d = opendir("/proc")))
		return 0;
	while ((dir = readdir(d))) {
		// If the map starts with a number, it's always a legit process
		if (strtoul(dir->d_name, NULL, 10) != 0) // The init process will not be logged, but we have no real choice
			++count;
	}
	closedir(d);
	return count;
}

bool xtProcIsAlive(unsigned pid)
{
	return kill(pid, 0) == 0 ? true : errno != ESRCH;
}

static int proc_signal_to_sys(enum xtProcSignal signal)
{
	switch (signal) {
	case XT_SIGHUP:  return SIGHUP;
	case XT_SIGINT:  return SIGINT;
	case XT_SIGKILL: return SIGKILL;
	case XT_SIGTERM: return SIGTERM;
	case XT_SIGUSR1: return SIGUSR1;
	case XT_SIGUSR2: return SIGUSR2;
	case XT_SIGCONT: return SIGCONT;
	case XT_SIGSTOP: return SIGSTOP;
	default:		 return 0;
	}
}

int xtProcKill(unsigned pid, enum xtProcSignal signal)
{
	int ret = kill(pid, proc_signal_to_sys(signal));
	return ret == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtProcWait(unsigned pid, unsigned timeout)
{
	// TODO This code is kinda stupid. We should be using waitpid() but
	// the description for it is kinda strange, so instead just use this.
	const unsigned sleepTime = 50;
	if (!xtProcIsAlive(pid))
		return XT_ENOENT;
	while (xtProcIsAlive(pid)) {
		if (timeout != 0) {
			if (timeout <= sleepTime)
				return XT_ETIMEDOUT;
			timeout -= sleepTime;
		}
		usleep(sleepTime * 1000);
	}
	return 0;
}

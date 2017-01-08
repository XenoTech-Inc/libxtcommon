// XT headers
#include <xt/proc.h>
#include <xt/error.h>

// System headers
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// STD headers
#include <stdlib.h>

static int _xtProcSignalToSys(enum xtProcSignal signal)
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

unsigned xtProcGetCurrentPID(void)
{
	return getpid();
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
		if (strtoul(dir->d_name, NULL, 10) != 0)
			++count;
	}
	closedir(d);
	return count;
}

bool xtProcIsAlive(unsigned pid)
{
	return kill(pid, 0) == 0 ? true : errno != ESRCH;
}

int xtProcKill(unsigned pid, enum xtProcSignal signal)
{
	int ret = kill(pid, _xtProcSignalToSys(signal));
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
		if (timeout <= sleepTime)
			return XT_ETIMEDOUT;
		timeout -= sleepTime;
		usleep(sleepTime * 1000);
	}
	return 0;
}
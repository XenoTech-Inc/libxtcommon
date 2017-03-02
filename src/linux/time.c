// XT headers
#include <xt/time.h>
#include <xt/error.h>

// System headers
#include <sys/sysinfo.h> // for the sysinfo function and struct
#include <sys/time.h> // for some time thingies, like clock_gettime
#include <unistd.h> // usleep

// STD headers
#include <stdio.h>
#include <time.h>

int xtCalendarGetGMTOffset(int *offset)
{
	time_t t = time(NULL);
	struct tm lt;
	if (!localtime_r(&t, &lt))
		return XT_EOVERFLOW;
	*offset = lt.tm_gmtoff / 60;
	return 0;
}

int xtCalendarIsDST(bool *isDST)
{
	time_t t = time(NULL);
	struct tm lt;
	if (!gmtime_r(&t, &lt))
		return XT_EOVERFLOW;
	// If greater than zero DST is in effect
	*isDST = lt.tm_isdst > 0;
	return 0;
}

unsigned long long xtClockGetCurrentTimeUS(void)
{
	unsigned long long now = xtClockGetRealtimeUS();
	if (now == 0)
		return 0;
	int gmtOffset;
	int retval = xtCalendarGetGMTOffset(&gmtOffset);
	if (retval != 0)
		return 0;
	bool isDST;
	if (xtCalendarIsDST(&isDST) != 0)
		return 0;
	if (isDST)
		now += 3600000000000LLU; // Advance by 1 hour if we're in DST
	now += gmtOffset / 60 * 3600000000LLU;
	return now;
}

unsigned long long xtClockGetMonotimeUS(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		return (ts.tv_sec * 1000000LLU) + (ts.tv_nsec / 1000LLU);
	}
	// It should be impossible to get here, since almost all kernels should have CLOCK_MONOTONIC
	return 0;
}

unsigned long long xtClockGetRealtimeUS(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
		return (ts.tv_sec * 1000000LLU) + (ts.tv_nsec / 1000LLU);
	}
	// It should be impossible to get here, since all kernels should have CLOCK_REALTIME
	return 0;
}

struct tm *_xt_gmtime(time_t *t, struct tm *tm)
{
	return gmtime_r(t, tm);
}

unsigned xtGetUptime(void)
{
	struct sysinfo info;
	sysinfo(&info); // Cannot fail
	return info.uptime;
}

void xtSleepMS(unsigned msecs)
{
	usleep(msecs * 1000);
}

// XT headers
#include <xt/time.h>
#include <_xt/error.h>
#include <xt/error.h>

// System headers
#include <errno.h>
#include <sys/sysinfo.h> // for the sysinfo function and struct
#include <sys/time.h> // for some time thingies, like clock_gettime
#include <unistd.h> // usleep

// STD headers
#include <stdio.h>
#include <time.h>

struct tm *_xtGMTime(time_t *t, struct tm *tm)
{
	return gmtime_r(t, tm);
}

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

int xtClockGetResolution(struct xtTimestamp *timestamp, enum xtClock clock)
{
	clockid_t clockId;
	switch (clock) {
	case XT_CLOCK_MONOTONIC:        clockId = CLOCK_MONOTONIC; break;
	case XT_CLOCK_MONOTONIC_COARSE: clockId = CLOCK_MONOTONIC_COARSE; break;
	case XT_CLOCK_MONOTONIC_RAW:    clockId = CLOCK_MONOTONIC_RAW; break;
	case XT_CLOCK_REALTIME:         clockId = CLOCK_REALTIME; break;
	case XT_CLOCK_REALTIME_COARSE:  clockId = CLOCK_REALTIME_COARSE; break;
	case XT_CLOCK_REALTIME_NOW:     clockId = CLOCK_REALTIME; break;
	default:                        return XT_EINVAL;
	}
	struct timespec ts;
	if (clock_getres(clockId, &ts) == -1)
		return _xtTranslateSysError(errno);
	timestamp->sec = ts.tv_sec;
	timestamp->nsec = ts.tv_nsec;
	return 0;
}

int xtClockGetTime(struct xtTimestamp *timestamp, enum xtClock clock)
{
	int _xtClockGetTimeNow(struct xtTimestamp *timestamp);
	clockid_t clockId;
	switch (clock) {
	case XT_CLOCK_MONOTONIC:        clockId = CLOCK_MONOTONIC; break;
	case XT_CLOCK_MONOTONIC_COARSE: clockId = CLOCK_MONOTONIC_COARSE; break;
	case XT_CLOCK_MONOTONIC_RAW:    clockId = CLOCK_MONOTONIC_RAW; break;
	case XT_CLOCK_REALTIME:         clockId = CLOCK_REALTIME; break;
	case XT_CLOCK_REALTIME_COARSE:  clockId = CLOCK_REALTIME_COARSE; break;
	case XT_CLOCK_REALTIME_NOW:     return _xtClockGetTimeNow(timestamp);
	default:                        return XT_EINVAL;
	}
	struct timespec ts;
	if (clock_gettime(clockId, &ts) == -1)
		return _xtTranslateSysError(errno);
	timestamp->sec = ts.tv_sec;
	timestamp->nsec = ts.tv_nsec;
	return 0;
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

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

void xtTimestampMono(struct xtTimestamp *timestamp)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		timestamp->sec = ts.tv_sec;
		timestamp->nsec = ts.tv_nsec;
		return;
	}
	timestamp->sec = 0;
	timestamp->nsec = 0;
}

void xtTimestampReal(struct xtTimestamp *timestamp)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
		timestamp->sec = ts.tv_sec;
		timestamp->nsec = ts.tv_nsec;
		return;
	}
	timestamp->sec = 0;
	timestamp->nsec = 0;
}

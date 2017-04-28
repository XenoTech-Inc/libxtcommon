// XT headers
#include <xt/time.h>

unsigned long long xtTimestampToMS(const struct xtTimestamp *timestamp)
{
	return timestamp->sec * 1000LLU + timestamp->nsec / 1000000LLU;
}

unsigned long long xtTimestampToUS(const struct xtTimestamp *timestamp)
{
	return timestamp->sec * 1000000LLU + timestamp->nsec / 1000LLU;
}
/**
 * This function is used by xtClockGetTime() to retrieve the realtime
 * DST and GMT corrected.
 */
int _xtClockGetTimeNow(struct xtTimestamp *timestamp)
{
	int gmtOffset, ret;
	bool isDST;
	struct xtTimestamp now;
	// Satisfy -O3 warnings
	now.sec = 0;
	now.nsec = 0;
	if ((ret = xtClockGetTime(&now, XT_CLOCK_REALTIME)) != 0)
		return ret;
	if ((ret = xtCalendarGetGMTOffset(&gmtOffset)) != 0)
		return ret;
	if ((ret = xtCalendarIsDST(&isDST) != 0))
		return ret;
	if (isDST)
		now.sec += 3600LLU; // Advance by 1 hour if we're in DST
	now.sec += gmtOffset * 60;
	timestamp->sec = now.sec;
	timestamp->nsec = now.nsec;
	return 0;
}

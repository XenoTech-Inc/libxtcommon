/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/time.h>

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

void xtTimestampDiff(struct xtTimestamp *dest, const struct xtTimestamp *start, const struct xtTimestamp *end)
{
	if (start->nsec <= end->nsec) {
		dest->sec  = end->sec  - start->sec;
		dest->nsec = end->nsec - start->nsec;
	} else {
		dest->sec  = end->sec - start->sec - 1;
		dest->nsec = 1000000000LLU - (start->nsec - end->nsec);
	}
}

void xtTimestampToDate(struct xtDate *date, const struct xtTimestamp *timestamp)
{
	struct xtTimestamp time = *timestamp;
	date->week = time.sec / XT_DATE_WEEK; time.sec %= XT_DATE_WEEK;
	date->day  = time.sec / XT_DATE_DAY ; time.sec %= XT_DATE_DAY ;
	date->hour = time.sec / XT_DATE_HOUR; time.sec %= XT_DATE_HOUR;
	date->min  = time.sec / XT_DATE_MIN ; time.sec %= XT_DATE_MIN ;
	date->sec  = time.sec;
	date->msec = time.nsec / XT_DATE_MSEC; time.nsec %= XT_DATE_MSEC;
	date->usec = time.nsec / XT_DATE_USEC; time.nsec %= XT_DATE_USEC;
	date->nsec = time.nsec;
}

void xtDateToTimestamp(struct xtTimestamp *timestamp, const struct xtDate *date)
{
	timestamp->nsec = date->nsec
		+ XT_DATE_USEC * date->usec
		+ XT_DATE_MSEC * date->msec;
	timestamp->sec = date->sec
		+ XT_DATE_MIN  * date->min
		+ XT_DATE_HOUR * date->hour
		+ XT_DATE_DAY  * date->day
		+ XT_DATE_WEEK * date->week;
}

unsigned long long xtTimestampToMS(const struct xtTimestamp *timestamp)
{
	return timestamp->sec * 1000LLU + timestamp->nsec / 1000000LLU;
}

unsigned long long xtTimestampToUS(const struct xtTimestamp *timestamp)
{
	return timestamp->sec * 1000000LLU + timestamp->nsec / 1000LLU;
}

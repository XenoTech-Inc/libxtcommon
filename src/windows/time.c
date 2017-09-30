// XT headers
#include <xt/time.h>
#include <xt/error.h>
#include <_xt/time.h>

// System headers
#include <windows.h>

// STD headers
#include <stdio.h>
#include <time.h>

unsigned long long _xtFileTimeToLLU(const FILETIME *ft)
{
	return (((ULONGLONG) ft->dwHighDateTime) << 32) | ft->dwLowDateTime;
}

struct tm *_xtGMTime(const time_t *t, struct tm *tm)
{
	struct tm *lt = gmtime(t);
	if (lt) *tm = *lt;
	return tm;
}

static SYSTEMTIME get_system_time_diff(const SYSTEMTIME *st1, const SYSTEMTIME *st2)
{
	SYSTEMTIME stDiff;
	FILETIME v_ftime;
	ULARGE_INTEGER v_ui;
	__int64 v_right, v_left, v_res;
	SystemTimeToFileTime(st1, &v_ftime);
	v_ui.LowPart = v_ftime.dwLowDateTime;
	v_ui.HighPart = v_ftime.dwHighDateTime;
	v_right = v_ui.QuadPart;

	SystemTimeToFileTime(st2, &v_ftime);
	v_ui.LowPart = v_ftime.dwLowDateTime;
	v_ui.HighPart = v_ftime.dwHighDateTime;
	v_left = v_ui.QuadPart;

	v_res = v_right - v_left;

	v_ui.QuadPart = v_res;
	v_ftime.dwLowDateTime = v_ui.LowPart;
	v_ftime.dwHighDateTime = v_ui.HighPart;
	FileTimeToSystemTime(&v_ftime, &stDiff);
	return stDiff;
}

int xtCalendarGetGMTOffset(int *offset)
{
	SYSTEMTIME gmtSystemTime, utcSystemTime;
	// It seems that these functions cannot fail
	// Get the GMT time
	GetLocalTime(&gmtSystemTime);
	// Get the UTC time
	GetSystemTime(&utcSystemTime);
	SYSTEMTIME diff;
	if (gmtSystemTime.wHour > utcSystemTime.wHour || gmtSystemTime.wDay > utcSystemTime.wDay) {
		diff = get_system_time_diff(&gmtSystemTime, &utcSystemTime);
	} else {
		diff = get_system_time_diff(&utcSystemTime, &gmtSystemTime);
	}
	*offset = diff.wHour * 60 + diff.wMinute;
	return 0;
}

int xtCalendarIsDST(bool *isDST)
{
	time_t t = time(NULL);
	// gmtime uses thread local storage by default on Windows, so this is safe
	struct tm *lt = gmtime(&t);
	if (!lt) {
		// Windows should return EINVAL, but for the sense of logic, we make it an overflow
		return XT_EOVERFLOW;
	}
	// If greater than zero DST is in effect
	*isDST = lt->tm_isdst > 0;
	return 0;
}

int xtClockGetRes(struct xtTimestamp *timestamp, enum xtClock clock)
{
	switch (clock) {
	case XT_CLOCK_MONOTONIC:
	case XT_CLOCK_MONOTONIC_COARSE:
	case XT_CLOCK_MONOTONIC_RAW:    timestamp->sec = 0; timestamp->nsec = 1000; break;
	case XT_CLOCK_REALTIME:
	case XT_CLOCK_REALTIME_COARSE:
	case XT_CLOCK_REALTIME_NOW:     timestamp->sec = 0; timestamp->nsec = 100; break;
	default:                        return XT_EINVAL;
	}
	return 0;
}

int xtClockGetTime(struct xtTimestamp *timestamp, enum xtClock clock)
{
	switch (clock) {
	case XT_CLOCK_MONOTONIC:
	case XT_CLOCK_MONOTONIC_COARSE:
	case XT_CLOCK_MONOTONIC_RAW: {
		LARGE_INTEGER frequency, counter;
		// These cannot fail if running on Windows XP or higher
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&counter);
		timestamp->sec = counter.QuadPart / frequency.QuadPart;
		timestamp->nsec = (counter.QuadPart * 1000000000LLU / frequency.QuadPart) % 1000000000LLU;
		return 0;
	}
	case XT_CLOCK_REALTIME:
	case XT_CLOCK_REALTIME_COARSE: {
		FILETIME fileTime;
		// Instead of first getting a SYSTEMTIME. This function cannot fail
		GetSystemTimeAsFileTime(&fileTime);
		// Filetime in 100 nanosecond resolution
		ULONGLONG fileTimeNano100 = (((ULONGLONG) fileTime.dwHighDateTime) << 32) + fileTime.dwLowDateTime;
		// To milliseconds and unix windows epoch offset removed
		timestamp->sec  = (fileTimeNano100 / 10 - 11644473600000000LLU) / 1000000LLU;
		timestamp->nsec = fileTimeNano100 * 100LLU % 1000000000LLU;
		return 0;
	}
	case XT_CLOCK_REALTIME_NOW: {
		return _xtClockGetTimeNow(timestamp);
	}
	default: return XT_EINVAL;
	}
}

unsigned xtGetUptime(void)
{
	LARGE_INTEGER frequency, counter;
	// These cannot fail if running on Windows XP or higher
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * 1000000 / frequency.QuadPart / 1000000; // Convert to secs
}

void xtSleepMS(unsigned msecs)
{
	Sleep(msecs);
}

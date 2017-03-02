// XT headers
#include <xt/time.h>
#include <xt/error.h>

// System headers
#include <windows.h>

// STD headers
#include <stdio.h>
#include <time.h>

static SYSTEMTIME _xtGetSystemTimeDiff(const SYSTEMTIME *st1, const SYSTEMTIME *st2)
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
		diff = _xtGetSystemTimeDiff(&gmtSystemTime, &utcSystemTime);
	} else {
		diff = _xtGetSystemTimeDiff(&utcSystemTime, &gmtSystemTime);
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
	// Very unprecise... like 10-16 milliseconds off
	// Multiply by 1000 just to convert to microseconds (US)
	//return GetTickCount64() * 1000;
	// -----------------------------------------------
	// New code which is much more accurate!!
	LARGE_INTEGER frequency, temp;
	// These cannot fail if running on Windows XP or higher
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&temp);
	return temp.QuadPart * 1000000 / frequency.QuadPart;
}

unsigned long long xtClockGetRealtimeUS(void)
{
	FILETIME fileTime;
	// Instead of first getting a SYSTEMTIME. This function cannot fail
	GetSystemTimeAsFileTime(&fileTime);
	// Filetime in 100 nanosecond resolution
	ULONGLONG fileTimeNano100 = (((ULONGLONG) fileTime.dwHighDateTime) << 32) + fileTime.dwLowDateTime;
	// To milliseconds and unix windows epoch offset removed
	return fileTimeNano100 / 10 - 11644473600000LLU * 1000;
}

struct tm *_xt_gmtime(time_t *t, struct tm *tm)
{
	struct tm *lt = gmtime(t);
	if (lt) *tm = *lt;
	return lt;
}

unsigned xtGetUptime(void)
{
	LARGE_INTEGER frequency, temp;
	// These cannot fail if running on Windows XP or higher
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&temp);
	return temp.QuadPart * 1000000 / frequency.QuadPart / 1000000; // Convert to secs
}

void xtSleepMS(unsigned msecs)
{
	Sleep(msecs);
}

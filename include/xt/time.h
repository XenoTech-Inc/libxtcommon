/**
 * @brief Provides very accurate timestamps and other time related functions.
 *
 * The accuracy of some functions may be different on your system
 * depending on what the kernel implementation offers.
 * On systems with 32 bit timers, some functions will no
 * longer work correctly after the year 2037.
 * @file time.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_TIME_H
#define _XT_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

enum xtClock {
	/**
	 * Clock that represents monotonic time since some
	 * unspecified starting point.
	 */
	XT_CLOCK_MONOTONIC,
	/** A faster but less precise version of XT_CLOCK_MONOTONIC. */
	XT_CLOCK_MONOTONIC_COARSE,
	/**
	 * Similar to XT_CLOCK_MONOTONIC, but provides access to a raw
	 * hardware-based time that is not subject to NTP adjustments
	 */
	XT_CLOCK_MONOTONIC_RAW,
	/** Wall clock time. */
	XT_CLOCK_REALTIME,
	/** A faster but less precise version of XT_CLOCK_REALTIME. */
	XT_CLOCK_REALTIME_COARSE,
	/** Wall clock time but GMT and DST corrected. */
	XT_CLOCK_REALTIME_NOW
};
/**
 * @brief High accuracy timestamp container.
 *
 * When a function works with this struct, you should assume that it has
 * nanosecond accuracy, unless documented otherwise by the function.
 */
struct xtTimestamp {
	unsigned long long sec;
	unsigned nsec;
};

#define XT_DATE_WEEK 604800U
#define XT_DATE_DAY   86400U
#define XT_DATE_HOUR   3600U
#define XT_DATE_MIN      60U
#define XT_DATE_MSEC 1000000LLU
#define XT_DATE_USEC    1000LLU

struct xtDate {
	unsigned week;
	unsigned day;
	unsigned hour;
	unsigned min;
	unsigned sec;
	unsigned msec;
	unsigned usec;
	unsigned nsec;
};
/**
 * Tells you the GMT offset in minutes. e.g If you get 60, this means
 * you are GMT +1. (60 mins in 1 hour). Do note that this function grabs
 * the OS timezone information. If you have changed the time "by hand",
 * the wrong information will be returned. e.g If you set the timezone to
 * Amsterdam (GMT +1) and then advance the time by 10 minutes by hand,
 * you will still get 60 minutes ahead of UTC instead of 70. This should
 * be a very rare occurrence nonetheless. Daylight Saving Time is ignored.
 * It will not be taken into account by this function.
 * @param offset - Receives the GMT offset in minutes.
 * This can be a negative value!
 * @return 0 for success, otherwise an error code.
 */
int xtCalendarGetGMTOffset(int *offset);
/**
 * Tells you if DST is currently active for your timezone.
 * @param isDST - Receives whether DST is active or not.
 * @return 0 for success, otherwise an error code.
 */
int xtCalendarIsDST(bool *isDST);
/**
 * Finds the resolution (precision) of the specified clock \a clock.
 * The resolution of the available clocks depends on the implementation.
 * @remarks This function is rather meaningless on Windows. It shall
 * always report the maximum precision that each clock offers on Windows
 * even if your hardware would not support it. This is because Windows
 * just does not offer a way to retrieve the clock accuracy.
 */
int xtClockGetRes(struct xtTimestamp *timestamp, enum xtClock clock);
/**
 * Retrieves the time for the specified clock \a clock.
 * @return 0 for success, otherwise an error code.
 */
int xtClockGetTime(struct xtTimestamp *timestamp, enum xtClock clock);
/**
 * Returns the uptime of the device in seconds.
 */
unsigned xtGetUptime(void);
/**
 * Suspends the calling thread for approximately the specified amount
 * of \a msecs milliseconds. The accuracy heavily depends on the OS
 * thread scheduler. The smaller the amount, the more it can differ
 * from the specified amount of time. On e.g. Windows, this can differ
 * up to 17 milliseconds.
 */
void xtSleepMS(unsigned msecs);
/**
 * Compute the difference between \a start and \a end.
 * @param dest - Destination timestamp.
 * @param start - Start timestamp.
 * @param end - End timestamp.
 */
void xtTimestampDiff(struct xtTimestamp *dest, const struct xtTimestamp *start, const struct xtTimestamp *end);
void xtTimestampToDate(struct xtDate *date, const struct xtTimestamp *timestamp);
void xtDateToTimestamp(struct xtTimestamp *timestamp, const struct xtDate *date);
/**
 * Converts \a timestamp to milliseconds.
 * @param timestamp - The timestamp in seconds and nanoseconds.
 */
unsigned long long xtTimestampToMS(struct xtTimestamp *timestamp);
/**
 * Converts \a timestamp to nanoseconds.
 * @param timestamp - The timestamp in seconds and nanoseconds.
 */
unsigned long long xtTimestampToUS(struct xtTimestamp *timestamp);
/**
 * Cross platform call to gmtime(). Internal use only.
 */
struct tm *_xtGMTime(time_t *t, struct tm *tm);

#ifdef __cplusplus
}
#endif

#endif

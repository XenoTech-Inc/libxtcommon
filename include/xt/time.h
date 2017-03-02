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
 * The time points of this clock cannot decrease as physical time moves forward.
 * This clock is not related to wall clock time (for example, it can be time
 * since last reboot), and is most suitable for measuring intervals.
 * Accuracy: Linux: 1NS, Windows: 100NS.
 * @param timestamp - Receives the monotonic time. On failure, the time will
 * be zero.
 */
void xtTimestampMono(struct xtTimestamp *timestamp);
/**
 * Tells you the true time that your device is reporting right now since UNIX
 * time. This value is DST and GMT adjusted.
 * Accuracy: Linux: 1NS, Windows: 100NS.
 * @param timestamp - The time that your device is reporting, GMT and DST
 * corrected. On failure, the time will be zero.
 */
void xtTimestampNow(struct xtTimestamp *timestamp);
/**
 * Tells you the realtime AKA the UNIX timestamp.
 * That is, the time elapsed since January 1, 1970 at 00:00:00 UTC.
 * Accuracy: Linux: 1NS, Windows: 100NS.
 * @param timestamp - Receives the realtime. On failure, the time will
 * be zero.
 */
void xtTimestampReal(struct xtTimestamp *timestamp);
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

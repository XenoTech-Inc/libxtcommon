/**
 * @brief Provides very accurate timestamps and other time related functions.
 *
 * All functions in here should provide true microsecond precision, they
 * are also thread safe. On 32 bit systems (systems with 32 bit timers),
 * some functions will no longer work correctly after the year 2037.
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

/**
 * Tells you the GMT offset in minutes. e.g If you get 60, this means
 * you are GMT +1. (60 mins in 1 hour). Do note that on Linux & Windows
 * it grabs the timezone information. If you have changed the time
 * "by hand", the wrong information will be returned. e.g If you set the
 * timezone to Amsterdam (GMT +1), and then advance the time by 10
 * minutes by hand, you will still get 60 minutes ahead of UTC instead
 * of 70. This should be a very rare occurrence nonetheless.\n
 * Daylight Saving Time is ignored. It will not be taken into account
 * by this function.\n
 * Errors:\n
 * XT_EOVERFLOW - The result cannot be represented.
 * @param offset - Receives the GMT offset in minutes.
 * This can be a negative value!
 * @return 0 for success. Otherwise an error code.
 */
int xtCalendarGetGMTOffset(int *offset);
/**
 * Tells you if DST is currently active for your timezone.\n
 * Errors: XT_EOVERFLOW if the result cannot be represented.
 * @param isDST - Receives whether DST is active or not.
 * @return 0 for success. Otherwise an error code.
 */
int xtCalendarIsDST(bool *isDST);
/**
 * Returns the true time that your device is reporting right now in
 * microseconds since UNIX time. This value is DST and GMT adjusted.
 * @return The time that your device is reporting, GMT and DST corrected.
 * Zero is returned on failure.
 */
unsigned long long xtClockGetCurrentTimeUS(void);
/**
 * The time points of this clock cannot decrease as physical time moves forward.
 * This clock is not related to wall clock time (for example, it can be time
 * since last reboot), and is most suitable for measuring intervals.
 * @return The monotonic time in microseconds. Zero is returned on failure.
 */
unsigned long long xtClockGetMonotimeUS(void);
/**
 * Tells you the current time in microseconds, AKA the UNIX timestamp.
 * That is, the time elapsed since January 1, 1970 at 00:00:00 UTC.
 * @return The realtime in microseconds. Zero is returned on failure.
 */
unsigned long long xtClockGetRealtimeUS(void);
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

#ifdef __cplusplus
}
#endif

#endif

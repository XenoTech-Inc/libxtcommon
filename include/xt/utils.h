/**
 * @brief Various functions which can come in handy.
 * @file utils.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_UTILS_H
#define _XT_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#define XT_VERSION_MAJOR 0
#define XT_VERSION_MINOR 6

// STD headers
#include <stdbool.h>

struct xtBuildOptions {
	/** Contains the CFLAGS options. */
	const char *configure;
	const char *date;
	const char *version;
	unsigned versionMajor;
	unsigned versionMinor;
};
/**
 * Contains information regarding how the library was compiled.
 */
extern const struct xtBuildOptions XT_BUILD_OPTIONS;
/**
 * Prints the specified message to stderr and exits the program with EXIT_FAILURE.
 */
void xtDie(const char *msg);
/**
 * Prints the specified message to stderr and exits the program with EXIT_FAILURE ONLY if the condition is false.
 */
void xtDieOn(bool cond, const char *msg);

#ifdef __cplusplus
}
#endif

#endif

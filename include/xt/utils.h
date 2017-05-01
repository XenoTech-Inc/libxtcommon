/**
 * @brief Various functions which can come in handy.
 * @file utils.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_UTILS_H
#define _XT_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#define XT_VERSION_MAJOR 0
#define XT_VERSION_MINOR 8
#define XT_VERSION_PATCH 0

// STD headers
#include <stdbool.h>
#include <stdint.h>

struct xtBuildOptions {
	/** Contains the CFLAGS options. */
	const char *configure;
	const char *date;
	const char *version;
	unsigned versionMajor;
	unsigned versionMinor;
	unsigned versionPatch;
};
/**
 * Contains information regarding how the library was compiled.
 */
extern const struct xtBuildOptions XT_BUILD_OPTIONS;
/**
 * Prints the specified message to stderr and exits the program with EXIT_FAILURE.
 * stderr is always flushed first.
 */
void xtDie(const char *msg);
/**
 * Prints the specified message to stderr and exits the program with EXIT_FAILURE ONLY if the condition is false.
 * stderr is always flushed first.
 */
void xtDieOn(bool cond, const char *msg);
/**
 * Returns a pseudo-random number in the range of 0 to UINT64_MAX.
 */
uint64_t xtRandLLU(void);
/**
 * Use this function to initialize the pseudo-random number generator functions.
 */
void xtSeedRand(unsigned seed);


#ifdef __cplusplus
}
#endif

#endif

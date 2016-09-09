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

// STD headers
#include <stdbool.h>

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

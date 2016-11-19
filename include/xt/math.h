/**
 * @brief Contains handy math functions.
 *
 * None of these functions actually require the C standard header math.h
 * @file math.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_MATH_H
#define _XT_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stdint.h>

#define XT_MATH_CELCIUS_TO_KELVIN(x) (x + 273.15)
#define XT_MATH_CELCIUS_TO_FAHRENHEIT(x) (x * 1.8 + 32)
#define XT_MATH_KELVIN_TO_CELCIUS(x) (x - 273.15)
#define XT_MATH_KELVIN_TO_FAHRENHEIT(x) ((x - 273.15) * 1.8 + 32)
#define XT_MATH_FAHRENHEIT_TO_CELCIUS(x) ((x - 32) / 1.8)
#define XT_MATH_FAHRENHEIT_TO_KELVIN(x) ((x + 459.67) / 1.8)
#define XT_MATH_KPH_TO_MPH(x) (x / 1.60934400061)
#define XT_MATH_MPH_TO_KPH(x) (x * 1.60934400061)
/**
 * Returns the number that is the highest of the two parameters.
 * @remarks This macro has side effects.
 */
#define XT_MATH_MAX(a, b) ((a) > (b) ? (a) : (b))
/**
 * Returns the number that is the lowest of the two parameters.
 * @remarks This macro has side effects.
 */
#define XT_MATH_MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * Used for splitting memory into chunks, this calculates how many chunks are necessary, and how large the last chunk will be.
 * @param len - The total length of the data that is to be split
 * @param chunkSize - How large each chunk should be
 * @param chunks - This pointer will receive how many chunks were created
 * @param lastChunkSize - This pointer will receive what the size of the last chunk is
 */
void xtMathMemSplit(unsigned long long len, unsigned chunkSize, unsigned *chunks, unsigned *lastChunkSize);
/**
 * Returns if a number is a power of 2.
 */
bool xtMathIsPow2(uint64_t v);
/**
 * Returns the nearest power of 2 for the specified number.
 * If the number is already a power of 2, then the same number will be returned.
 * @remarks If zero is specified, one is returned.
 */
uint64_t xtMathNextPow2(uint64_t v);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @brief Functions to manipulate strings with ease.
 * @file string.h
 * @author Folkert van Verseveld
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_STRING_H
#define _XT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/time.h>

// STD headers
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int xtBase64Decode(void *buf, size_t buflen, const void *data);
int xtBase64Encode(void *buf, size_t buflen, const void *data, size_t datalen);

int xtCharToDigit(char c);
/**
 * Format number in SI metric system and return power of prefix.
 * @param buf - Will receive the formatted number. Bounds checking is performed.
 * @param buflen - Maximum buffer length.
 * @param value - The number to format in SI metric system.
 * @param decimals - Number of fractional decimals (0 disables fractions).
 * @param strictBinary - Use 1024 (e.g. KiB, MiB) rather than 1000 (e.g. KB, MB).
 * @param base - An optional pointer which receives the base of the formatted bytes.
 * E.g.: 0 is the number in bytes, 1 in kilobytes, 2 in megabytes etc.
 * @returns A pointer to \a buf.
 */
char *xtFormatBytesSI(char *restrict buf, size_t buflen, uint64_t value, unsigned decimals, bool strictBinary, unsigned *restrict base);
/**
 * Formats every thousand in the number with the specified seperator.
 * \a buf will receive the final string. No bounds checking is performed.
 */
char *xtFormatCommasLL(char *buf, size_t buflen, long long value, int sep);
/**
 * Formats every thousand in the number with the specified seperator.
 * \a buf will receive the final string. No bounds checking is performed.
 */
char *xtFormatCommasLLU(char *buf, size_t buflen, unsigned long long value, int sep);
/**
 * Format block of data as a hexadecimal string seperating each byte using \a sep.
 * @param buf - Will receive the formatted buffer. Bounds checking is performed.
 * @param buflen - Maximum buffer length.
 * @param data - Block of data.
 * @param datalen - Length of data block.
 * @param sep - Separator character for each byte in \a data.
 * @param uppercase - Whether to print each byte in uppercase or lowercase.
 * @returns A pointer to \a buf.
 */
char *xtFormatHex(char *restrict buf, size_t buflen, const void *restrict data, size_t datalen, int sep, bool uppercase);
/**
 * Fills the buffer with the current time in the following format:
 * YYYY-mm-dd HH:MM:SS. The clock uses the 24 hour format.
 * This function MAY fail if the timestamp exceeds
 * the OS timer limits.
 * @param timestamp_secs - Your timestamp in seconds.
 * @returns A pointer to the buffer. Null is returned on failure.
 */
char *xtFormatTime(char *buf, size_t buflen, unsigned timestamp_secs);
/**
 * Fills the buffer with the current time in the following format:
 * YYYY-mm-dd HH:MM:SS mmm:uuu:nnn. The clock uses the 24 hour format.
 * This function MAY fail if the timestamp exceeds
 * the OS timer limits.
 * @param timestamp - Your timestamp.
 * @returns A pointer to the buffer. Null is returned on failure.
 */
char *xtFormatTimePrecise(char *buf, size_t buflen, struct xtTimestamp *timestamp);
/**
 * Fills the buffer with the elapsed time in the specified format.
 * If \a end is a timestamp before \a start, the result is unspecified.
 * The following arguments are supported:
 * * %W - Weeks in numbers
 * * %w - Number of weeks
 * * %D - Days in numbers (0-6)
 * * %d - number of days (0-6 days)
 * * %H - Hours in 2 decimal numbers (00-23)
 * * %h - Number of hours
 * * %M - Minutes in 2 decimal numbers (00-59)
 * * %m - Number of minutes
 * * %S - Seconds in 2 decimal numbers (00-59)
 * * %s - Number of seconds
 * * %T - Milliseconds in 3 decimal numbers (000-999)
 * * %t - Number of milliseconds
 * * %U - Microseconds in 3 decimal numbers (000-999)
 * * %u - Number of microseconds
 * * %N - Nanoseconds in 3 decimal numbers (000-999)
 * * %n - Number of nanoseconds
 * These special arguments are also supported:
 * * %g - Full format starting with most significant unit
 * * %G - Full format starting with most significant unit skipping units that are zero.
 * @param format - The format string.
 * @param start - Start timestamp.
 * @param end - End timestamp.
 * @returns Number of written characters to buffer. Zero is returned on failure.
 */
unsigned xtFormatTimeDuration(char *buf, size_t buflen, const char *format, const struct xtTimestamp *start, const struct xtTimestamp *end);
unsigned xtFormatTimestamp(char *buf, size_t buflen, const char *format, const struct xtTimestamp *time);
/**
 * Reads a single character from the console without the need to press the ENTER key.
 * @remarks This function is kind of expensive since it needs to change the terminal settings on each call on Linux.
 * On Windows, do NOT call this function in Cygwin! The program will then block permanently unless CTRL+C is pressed.
 */
int xtGetch(void);
/**
 * Performs the ROT13 swapping on the specified data up to \a buflen bytes.
 */
void xtRot13(void *buf, size_t buflen);
/**
 * Formatted output conversion to stdout. It is similar to printf but, with
 * extensions for (u)int(8|16|32|64)_t: You can specify %I(8|16|32|64)[diouxX]
 * as argument if you want to use these.
 * It also fixes NULL termination problems on Windows because their broken libc
 * implementation does not guarantee NULL termination if it does not fit.
 * @returns Number of written characters. Zero or negative on failure.
 */
int xtprintf(const char *format, ...);
int xtvprintf(const char *format, va_list args);
/**
 * Formatted output conversion to file. It is similar to fprintf but, with
 * extensions for (u)int(8|16|32|64)_t: You can specify %I(8|16|32|64)[diouxX]
 * as argument if you want to use these.
 * It also fixes NULL termination problems on Windows because their broken libc
 * implementation does not guarantee NULL termination if it does not fit.
 * @returns Number of written characters. Zero or negative on failure.
 */
int xtfprintf(FILE *stream, const char *format, ...);
int xtvfprintf(FILE *stream, const char *format, va_list args);
/**
 * Formatted output conversion to buffer. It is similar to snprintf but, with
 * extensions for (u)int(8|16|32|64)_t: You can specify %I(8|16|32|64)[diouxX]
 * as argument if you want to use these.
 * It also fixes NULL termination problems on Windows because their broken libc
 * implementation does not guarantee NULL termination if it does not fit.
 * @returns Number of written characters. Zero or negative on failure.
 */
int xtsnprintf(char *str, size_t size, const char *format, ...);
int xtvsnprintf(char *str, size_t size, const char *format, va_list args);
/**
 * Copy a string of maximum \a n characters and guarantee NULL-termination. This
 * behaves the same as strncpy except it always guarantees NULL-termination.
 * @param dest - Pointer to copy string to.
 * @param src - Pointer to copy string from.
 * @param n - Maximum number of characters including null byte.
 * @returns Pointer to copy string to.
 */
char *xtstrncpy(char *restrict dest, const char *restrict src, size_t n);
/**
 * Checks if the haystack contains the needle somewhere in it.
 * @param needle - A pointer to a null terminated string that serves as needle.
 */
bool xtStringContainsLen(const char *restrict haystack, const char *restrict needle, size_t haystackLen);
bool xtStringContains(const char *restrict haystack, const char *restrict needle);
/**
 * Checks if the haystack ends with the needle.
 */
bool xtStringEndsWith(const char *restrict haystack, const char *restrict needle);
/**
 * Reads a line from the given file pointer into the buffer until a newline character is found.
 * @param str - Pointer to an array of chars where the string read is copied to.
 * @param num - Maximum number of characters to be copied into str (including the terminating null-character).
 * @param bytesRead - A pointer which will receive the length of the string, excluding the null terminator. This pointer is optional.
 * @param f - File stream to read the data from.
 * @returns \a str on success, null pointer on failure.
 */
char *xtStringReadLine(char *restrict str, size_t num, size_t *restrict bytesRead, FILE *restrict f);
/**
 * Reverses the string in-place up to \a len characters.
 * @returns A pointer to \a str.
 */
char *xtStringReverseLen(char *str, size_t len);
char *xtStringReverse(char *str);
/**
 * A more friendly version of strtok_r.
 * @param str - Pointer to a null-terminated string which is editable.
 * @param delim - The delimeter to use. It may be more than one character long.
 * @param tokens - Will receive pointers to all found tokens. All of these will be null-terminated.
 * @param num - A pointer to the maximum amount of words to look up. This pointer will receive how many words were found.
 */
void xtStringSplit(char *restrict str, const char *restrict delim, char **restrict tokens, unsigned *restrict num);
/**
 * Checks if the haystack starts with the needle.
 */
bool xtStringStartsWith(const char *restrict haystack, const char *restrict needle);
/**
 * Converts a string to lowercase.
 * @returns The modified string.
 */
char *xtStringToLower(char *str);
/**
 * Converts a string to uppercase.
 * @returns The modified string.
 */
char *xtStringToUpper(char *str);
/**
 * Replaces all occurences of the specified character in the string.
 * @returns The modified string.
 */
char *xtStringReplaceAll(char *str, const char org, const char replacer);
/**
 * Removes all leading and trailing whitespace from the string.
 * @remarks Beware that if the whole string is whitespace, it will be completely erased.
 * @returns The modified string.
 */
char *xtStringTrim(char *str);
/**
 * Removes all leading and trailing whitespace between words and leaves one space there.\n
 * This means that between each word there is at maximum one space after calling this function.\n
 * e.g (Pretend that '_' is a space) _wassup___world__!!! becomes wassup_world_!!.\n
 * Also just like the regular trim function, any leading and trailing whitespace is removed.
 * @remarks Beware that if the whole string is whitespace, it will be completely erased.
 * @returns The modified string.
 */
char *xtStringTrimWords(char *str);

#ifdef __cplusplus
}
#endif

#endif

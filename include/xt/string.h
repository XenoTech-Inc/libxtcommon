/**
 * @brief Functions to manipulate strings with ease.
 * @file string.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_STRING_H
#define _XT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int xtCharToDigit(char c);
/**
 * Converts an int64_t to a string suitable for printing.
 */
char *xtInt64ToStr(int64_t value, char *buf, size_t buflen);
/**
 * Formats every thousand in the number with the specified seperator. 
 * \a buf will receive the final string. No bounds checking is performed.
 */
char *xtFormatCommasLL(long long v, char *buf, int sep);
/**
 * Formats every thousand in the number with the specified seperator. 
 * \a buf will receive the final string. No bounds checking is performed.
 */
char *xtFormatCommasLLU(unsigned long long v, char *buf, int sep);
/**
 * Converts an uint64_t to a string suitable for printing.
 */
char *xtUint64ToStr(uint64_t value, char *buf, size_t buflen);
char *xtSizetToStr(size_t value, char *buf, size_t buflen);
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
 * Checks if the haystack contains the needle somewhere in it. 
 * @param needle - A pointer to a null terminated string that serves as needle.
 */
bool xtStringContainsLen(const char *restrict haystack, const char *restrict needle, size_t haystackLen);
bool xtStringContains(const char *restrict haystack, const char *restrict needle);
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
 * Checks if the haystack ends with the needle.
 */
bool xtStringEndsWith(const char *haystack, const char *needle);
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
void xtStringSplit(char *str, const char *delim, char **tokens, unsigned *num);
/**
 * Checks if the haystack starts with the needle.
 */
bool xtStringStartsWith(const char *haystack, const char *needle);
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

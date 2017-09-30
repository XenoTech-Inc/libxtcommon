/**
 * @brief Used to load .so and .dll files. on the fly.
 *
 * @file dlload.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_DLLOAD_H
#define _XT_DLLOAD_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stddef.h>

/**
 * All possible flags that you can pass to xtDLOpen().
 */
enum xtDLFlag {
	/** The symbol will be loaded when you call that particular symbol. */
	XT_DL_LAZY,
	/** Loads all unresolved symbols immediately. */
	XT_DL_NOW
};
/**
 * A pointer to a generic function. The sole purpose of this typedef is to
 * prevent UB when casting function pointers in xtDLSym().
 */
typedef void (*xtGenericFuncPtr)(void*);
/**
 * Attempts to load the dynamic library. If NULL is returned,
 * you can call xtDLError() to get extended information about the error.
 * This function ALWAYS clears any previous errors!
 * @param flag - Specifies the behavior for symbol loading. On Windows this parameter is ignored.
 * @return A handle to the loaded library. Null is returned on error.
 * @remarks You should call xtDLClose() on any handle that you don't need any longer.
 * Failure to do so will result in leakage of system resources.
 */
void *xtDLOpen(const char *filename, enum xtDLFlag flag);
/**
 * Closes a handle and releases any associated system resources with it.
 */
void xtDLClose(void *handle);
/**
 * Prints a human-readable string to the specified buffer should any error with xtDLOpen() or
 * xtDLSym() have occurred, then the pointer to that buffer is returned. If no error has occurred,
 * NULL is returned. Calling this function will result in the removal of any previous error.
 */
char *xtDLError(char *buf, size_t buflen);
/**
 * Returns the address of the function in the dynamic library. You should cast
 * the returned value to your target function.
 * If the handle is invalid, or the symbol is not found, NULL is returned.
 * You can call xtDLError() to get extended information about the error.
 * You should call xtDLError() before calling this function to clear any previous error.
 */
xtGenericFuncPtr xtDLSym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif

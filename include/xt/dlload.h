/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @brief Used to load .so and .dll files on the fly.
 *
 * @file dlload.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright LGPL v3.0.
 */

#ifndef _XT_DLLOAD_H
#define _XT_DLLOAD_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/_base.h>

// STD headers
#include <stddef.h>

/**
 * A handle to a loaded dynamic library.
 */
typedef void *xtDLHandle;
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
 * Closes a handle and releases any associated system resources with it.
 */
void xtDLClose(xtDLHandle handle);
/**
 * Prints a human-readable string to the specified buffer should any error with
 * xtDLOpen() or xtDLGetProcAddress() occur, in which case a pointer to \a buf
 * is returned. If no error has occurred, NULL is returned.
 * Calling this function will clear any previous error automatically.
 */
char *xtDLError(char *buf, size_t buflen);
/**
 * Returns the address of the function in the dynamic library. You should cast
 * the returned value to your target function or variable.
 * If the symbol is not found, NULL is returned.
 */
void *xtDLGetProcAddress(xtDLHandle handle, const char *symbol);
/**
 * Attempts to load the dynamic library. If NULL is returned, you can call
 * xtDLError() to get extended information about the error.
 * This function ALWAYS clears any previous errors!
 * @param flag - Specifies the behavior for symbol loading.
 * @return A handle to the loaded library. NULL is returned on error.
 * @remarks You should call xtDLClose() on any handle that was opened through
 * this function when you no longer need it. Failing to do so will result in
 * leaking system resources.
 */
xtDLHandle xtDLOpen(const char *path, enum xtDLFlag flag);

#ifdef __cplusplus
}
#endif

#endif

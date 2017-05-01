/**
 * @brief Contains some macros to detect what OS we're compiling on.
 *
 * @file os_macros.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_OS_MACROS_H
#define _XT_OS_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__gnu_linux__)
	#define XT_IS_LINUX 1
	#include <limits.h>
	#if UINTPTR_MAX == 0x0FFFFFFFFFFFFFFFFLLU
		#define XT_IS_X64 1
	#elif UINTPTR_MAX == 0x0FFFFFFFF
		#define XT_IS_X86 1
	#endif
#elif defined(_WIN64) || defined(_WIN32)
	#define XT_IS_WINDOWS 1
	#if defined(_WIN64)
		#define XT_IS_X64 1
	#else
		#define XT_IS_X86 1
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif

/**
 * @brief Contains some macros to detect what OS we're compiling on.
 * 
 * @file os_macros.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_OS_MACROS_H
#define _XT_OS_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__gnu_linux__)
	#define XT_IS_LINUX 1
#elif defined(_WIN32) && !defined(_WIN64)
	#define XT_IS_WINDOWS 1
	#define XT_IS_WINDOWS_X86 1
#elif defined(_WIN32) && defined(_WIN64)
	#define XT_IS_WINDOWS 1 
	#define XT_IS_WINDOWS_X64 1
#endif

#ifdef __cplusplus
}
#endif

#endif

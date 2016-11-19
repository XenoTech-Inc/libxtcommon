/**
 * @brief Exists only to solve include problems.
 *
 * This header includes several headers on different platforms to fix include order problems.
 * It should only be included by headers which include system headers in themselfs.
 * @file _include_common.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT__BASE_H
#define _XT__BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__gnu_linux__)

#elif defined(_WIN32) && !defined(_WIN64)
	#include <ws2tcpip.h> // Necessary for a hell lot of stuff regarding sockets
	#include <windows.h> // Necessary for everything else on this world
#elif defined(_WIN32) && defined(_WIN64)
	#include <ws2tcpip.h> // Necessary for a hell lot of stuff regarding sockets
	#include <windows.h> // Necessary for everything else on this world
#endif

#ifdef __cplusplus
}
#endif

#endif

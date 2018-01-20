/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @file time.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright LGPL v3.0.
 */

#ifndef __XT_TIME_H
#define __XT_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xt/os_macros.h>
#include <xt/time.h>

#include <time.h>

/**
 * This function is used by xtClockGetTime() to retrieve the realtime
 * DST and GMT corrected.
 */
int _xtClockGetTimeNow(struct xtTimestamp *timestamp);
/**
 * Cross platform call to gmtime().
 */
struct tm *_xtGMTime(const time_t *t, struct tm *tm);

#ifdef XT_IS_WINDOWS

#include <windows.h>

unsigned long long _xtFileTimeToLLU(const FILETIME *ft);

#endif

#ifdef __cplusplus
}
#endif

#endif

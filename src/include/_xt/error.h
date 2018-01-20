/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @brief Used for error handling.
 *
 * @file error.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright LGPL v3.0.
 */

#ifndef __XT_ERROR_H
#define __XT_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Translates a system specific error to an XT error code.
 */
int _xtTranslateSysError(int syserrnum);

#ifdef __cplusplus
}
#endif

#endif

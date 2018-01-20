/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#ifndef ___XT_STRING_H
#define ___XT_STRING_H

#include <stdarg.h>
#include <stddef.h>

/* Internal use only */
int _xtvsnprintf(char *str, size_t size, char *format, char *end, va_list args);

#endif

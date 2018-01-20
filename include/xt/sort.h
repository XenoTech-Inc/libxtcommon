/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @brief Contains various sorting algorithms.
 * @file sort.h
 * @author Folkert van Verseveld
 * @date 2017
 * @copyright LGPL v3.0
 */
#ifndef _XT_SORT_H
#define _XT_SORT_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/_base.h>

// STD headers
#include <stddef.h>
#include <stdbool.h>

enum xtSortType {
	XT_SORT_BUBBLE,
	XT_SORT_HEAP  ,
	XT_SORT_INSERT,
	XT_SORT_QUICK ,
	XT_SORT_SELECT,
	XT_SORT_RADIX ,
};

int xtSortU(unsigned *list, size_t count, enum xtSortType type, bool ascend);
int xtSortD(int *list, size_t count, enum xtSortType type, bool ascend);
int xtSortP(void *list, size_t count, enum xtSortType type, int (*cmp)(const void*, const void*), bool ascend, size_t elemSize);
int xtSortStr(const char **list, size_t count, enum xtSortType type, bool ascend);

#ifdef __cplusplus
}
#endif

#endif

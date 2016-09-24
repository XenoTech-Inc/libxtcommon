/**
 * @brief Contains various sorting algorithms.
 * @file sort.h
 * @author Folkert van Verseveld
 * @date 2016
 * @copyright XT-License
 */
#ifndef _XT_SORT_H
#define _XT_SORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

typedef enum xtSortType {
	XT_SORT_BUBBLE,
	XT_SORT_BUCKET,
	XT_SORT_HEAP,
	XT_SORT_HYBRID,
	XT_SORT_INSERT,
	XT_SORT_MERGE,
	XT_SORT_QUICK,
	XT_SORT_RADIX,
	XT_SORT_SELECT,
} xtSortType;

int xtSortU(unsigned *list, size_t count, xtSortType type, bool ascend);
int xtSortD(int *list, size_t count, xtSortType type, bool ascend);
int xtSortP(void *list, size_t count, xtSortType type, int (*cmp)(void*, void*), bool ascend, size_t elemSize);

#ifdef __cplusplus
}
#endif

#endif

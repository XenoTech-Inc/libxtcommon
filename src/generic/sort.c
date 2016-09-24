#include <xt/error.h>
#include <xt/sort.h>
#include <stdlib.h>
#include <string.h>

static inline void _xtSwapD(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

static inline void _xtSwapU(unsigned *a, unsigned *b)
{
	unsigned tmp = *a;
	*a = *b;
	*b = tmp;
}

static inline void _xtSwapP(void *a, void *b, void *tmp, size_t size)
{
	memcpy(tmp, a  , size);
	memcpy(a  , b  , size);
	memcpy(b  , tmp, size);
}

static void _xtInsertionSortD(int *a, size_t n, bool ascend)
{
	size_t i, j;
	if (ascend)
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] > a[j]; --j)
				_xtSwapD(&a[j], &a[j - 1]);
	else
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] < a[j]; --j)
				_xtSwapD(&a[j], &a[j - 1]);
}

static void _xtInsertionSortU(unsigned *a, size_t n, bool ascend)
{
	size_t i, j;
	if (ascend)
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] > a[j]; --j)
				_xtSwapU(&a[j], &a[j - 1]);
	else
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] < a[j]; --j)
				_xtSwapU(&a[j], &a[j - 1]);
}

static int _xtInsertionSortP(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
{
	size_t i, j;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return XT_ENOMEM;
	if (ascend)
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && cmp(&a[(j - 1) * elemsize], &a[j * elemsize]) > 0; --j)
				_xtSwapP(&a[j * elemsize], &a[(j - 1) * elemsize], tmp, elemsize);
	else
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && cmp(&a[(j - 1) * elemsize], &a[j * elemsize]) < 0; --j)
				_xtSwapP(&a[j * elemsize], &a[(j - 1) * elemsize], tmp, elemsize);
	free(tmp);
	return 0;
}

static void _xtSelectionSortD(int *a, size_t n, bool ascend)
{
	size_t i, j, min;
	if (ascend)
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] < a[min])
					min = i;
			if (min != j)
				_xtSwapD(&a[min], &a[j]);
		}
	else
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] > a[min])
					min = i;
			if (min != j)
				_xtSwapD(&a[min], &a[j]);
		}
}

static void _xtSelectionSortU(unsigned *a, size_t n, bool ascend)
{
	size_t i, j, min;
	if (ascend)
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] < a[min])
					min = i;
			if (min != j)
				_xtSwapU(&a[min], &a[j]);
		}
	else
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] > a[min])
					min = i;
			if (min != j)
				_xtSwapU(&a[min], &a[j]);
		}
}

static int _xtSelectionSortP(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
{
	size_t i, j, min;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return XT_ENOMEM;
	if (ascend)
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (cmp(&a[i * elemsize], &a[min * elemsize]) < 0)
					min = i;
			if (min != j)
				_xtSwapP(&a[min * elemsize], &a[j * elemsize], tmp, elemsize);
		}
	else
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (cmp(&a[i * elemsize], &a[min * elemsize]) > 0)
					min = i;
			if (min != j)
				_xtSwapP(&a[min * elemsize], &a[j * elemsize], tmp, elemsize);
		}
	free(tmp);
	return 0;
}

/*
Some sorting algorithms require additional routines.
The ones ending in A are for ascending order and D for descending.
*/
static void _xtHeapPercolateD_A(int *a, size_t low, size_t high)
{
	int p;
	size_t q;
	for (p = a[low]; 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && a[q] < a[q + 1])
			++q;
		if (p < a[q])
			a[low] = a[q];
		else
			break;
	}
	a[low] = p;
}

static void _xtHeapPercolateD_D(int *a, size_t low, size_t high)
{
	int p;
	size_t q;
	for (p = a[low]; 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && a[q] > a[q + 1])
			++q;
		if (p > a[q])
			a[low] = a[q];
		else
			break;
	}
	a[low] = p;
}

static void _xtHeapSortD(int *a, size_t n, bool ascend) {
	ssize_t i;
	if (ascend) {
		for (i = n / 2; i >= 0; --i)
			_xtHeapPercolateD_A(a, i, n);
		for (i = n - 1; i > 0; --i) {
			_xtSwapD(a, a + i);
			_xtHeapPercolateD_A(a, 0, i);
		}
	} else {
		for (i = n / 2; i >= 0; --i)
			_xtHeapPercolateD_D(a, i, n);
		for (i = n - 1; i > 0; --i) {
			_xtSwapD(a, a + i);
			_xtHeapPercolateD_D(a, 0, i);
		}
	}
}

static void _xtHeapPercolateU_A(unsigned *a, size_t low, size_t high)
{
	unsigned p;
	size_t q;
	for (p = a[low]; 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && a[q] < a[q + 1])
			++q;
		if (p < a[q])
			a[low] = a[q];
		else
			break;
	}
	a[low] = p;
}

static void _xtHeapPercolateU_D(unsigned *a, size_t low, size_t high)
{
	unsigned p;
	size_t q;
	for (p = a[low]; 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && a[q] > a[q + 1])
			++q;
		if (p > a[q])
			a[low] = a[q];
		else
			break;
	}
	a[low] = p;
}

static void _xtHeapSortU(unsigned *a, size_t n, bool ascend) {
	ssize_t i;
	if (ascend) {
		for (i = n / 2; i >= 0; --i)
			_xtHeapPercolateU_A(a, i, n);
		for (i = n - 1; i > 0; --i) {
			_xtSwapU(a, a + i);
			_xtHeapPercolateU_A(a, 0, i);
		}
	} else {
		for (i = n / 2; i >= 0; --i)
			_xtHeapPercolateU_D(a, i, n);
		for (i = n - 1; i > 0; --i) {
			_xtSwapU(a, a + i);
			_xtHeapPercolateU_D(a, 0, i);
		}
	}
}

static void _xtHeapPercolateP_A(void *list, void *tmp, size_t elemsize, int (*cmp)(void*, void*), size_t low, size_t high)
{
	void *p = tmp;
	char *a = list;
	size_t q;
	for (memcpy(tmp, &a[low * elemsize], elemsize); 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && cmp(&a[q * elemsize], &a[(q + 1) * elemsize]) < 0)
			++q;
		if (cmp(p, &a[q * elemsize]) < 0)
			memcpy(&a[low * elemsize], &a[q * elemsize], elemsize);
		else
			break;
	}
	memcpy(&a[low * elemsize], p, elemsize);
}

static void _xtHeapPercolateP_D(void *list, void *tmp, size_t elemsize, int (*cmp)(void*, void*), size_t low, size_t high)
{
	void *p = tmp;
	char *a = list;
	size_t q;
	for (memcpy(tmp, &a[low * elemsize], elemsize); 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && cmp(&a[q * elemsize], &a[(q + 1) * elemsize]) > 0)
			++q;
		if (cmp(p, &a[q * elemsize]) > 0)
			memcpy(&a[low * elemsize], &a[q * elemsize], elemsize);
		else
			break;
	}
	memcpy(&a[low * elemsize], p, elemsize);
}

static int _xtHeapSortP(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
{
	ssize_t i;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return XT_ENOMEM;
	if (ascend) {
		for (i = n / 2; i >= 0; --i)
			_xtHeapPercolateP_A(list, tmp, elemsize, cmp, i, n);
		for (i = n - 1; i > 0; --i) {
			_xtSwapP(a, &a[i * elemsize], tmp, elemsize);
			_xtHeapPercolateP_A(list, tmp, elemsize, cmp, 0, i);
		}
	} else {
		for (i = n / 2; i >= 0; --i)
			_xtHeapPercolateP_D(list, tmp, elemsize, cmp, i, n);
		for (i = n - 1; i > 0; --i) {
			_xtSwapP(a, &a[i * elemsize], tmp, elemsize);
			_xtHeapPercolateP_D(list, tmp, elemsize, cmp, 0, i);
		}
	}
	free(tmp);
	return 0;
}

int xtSortU(unsigned *list, size_t count, xtSortType type, bool ascend)
{
	switch (type) {
	case XT_SORT_HEAP:
		_xtHeapSortU(list, count, ascend);
		break;
	case XT_SORT_SELECT:
		_xtSelectionSortU(list, count, ascend);
		break;
	case XT_SORT_INSERT:
	default:
		_xtInsertionSortU(list, count, ascend);
		break;
	}
	return 0;
}

int xtSortD(int *list, size_t count, xtSortType type, bool ascend)
{
	switch (type) {
	case XT_SORT_HEAP:
		_xtHeapSortD(list, count, ascend);
		break;
	case XT_SORT_SELECT:
		_xtSelectionSortD(list, count, ascend);
		break;
	case XT_SORT_INSERT:
	default:
		_xtInsertionSortD(list, count, ascend);
		break;
	}
	return 0;
}

int xtSortP(void *list, size_t count, xtSortType type, int (*cmp)(void*,void*), bool ascend, size_t elemSize)
{
	if (!cmp)
		return XT_EINVAL;
	switch (type) {
	case XT_SORT_HEAP  : return _xtHeapSortP     (list, elemSize, count, cmp, ascend);
	case XT_SORT_SELECT: return _xtSelectionSortP(list, elemSize, count, cmp, ascend);
	case XT_SORT_INSERT:
	default            : return _xtInsertionSortP(list, elemSize, count, cmp, ascend);
	}
}

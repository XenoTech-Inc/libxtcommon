#include <xt/error.h>
#include <xt/sort.h>
#include <stdlib.h>
#include <string.h>

/* swap element routines are inlined so they
have to be defined first before we can use it */
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

static void _xtBubbleSortU(unsigned *a, size_t n, bool ascend)
{
	size_t i, x;
	if (ascend)
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] > a[i])
					_xtSwapU(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
	else
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] < a[i])
					_xtSwapU(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
}

static void _xtBubbleSortD(int *a, size_t n, bool ascend)
{
	size_t i, x;
	if (ascend)
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] > a[i])
					_xtSwapD(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
	else
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] < a[i])
					_xtSwapD(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
}

static int _xtBubbleSortP(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
{
	size_t i, x;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return XT_ENOMEM;
	if (ascend)
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (cmp(&a[(i - 1) * elemsize], &a[i * elemsize]) > 0)
					_xtSwapP(&a[(i - 1) * elemsize], &a[(x = i) * elemsize], tmp, elemsize);
			--n;
		} while (x);
	else
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (cmp(&a[(i - 1) * elemsize], &a[i * elemsize]) < 0)
					_xtSwapP(&a[(i - 1) * elemsize], &a[(x = i) * elemsize], tmp, elemsize);
			--n;
		} while (x);
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

static void _xtHeapSortU(unsigned *a, size_t n, bool ascend)
{
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

static void _xtQuickSortD_A(int *a, ssize_t low, ssize_t high)
{
	ssize_t i, j;
	int pivot;
	i = low; j = high;
	pivot = a[low + (high - low) / 2];
	while (i <= j) {
		while (a[i] < pivot)
			++i;
		while (a[j] > pivot)
			--j;
		if (i <= j)
			_xtSwapD(&a[i++], &a[j--]);
	}
	if (low < j)
		_xtQuickSortD_A(a, low, j);
	if (i < high)
		_xtQuickSortD_A(a, i, high);
}

static void _xtQuickSortD_D(int *a, ssize_t low, ssize_t high)
{
	ssize_t i, j;
	int pivot;
	i = low; j = high;
	pivot = a[low + (high - low) / 2];
	while (i <= j) {
		while (a[i] > pivot)
			++i;
		while (a[j] < pivot)
			--j;
		if (i <= j)
			_xtSwapD(&a[i++], &a[j--]);
	}
	if (low < j)
		_xtQuickSortD_D(a, low, j);
	if (i < high)
		_xtQuickSortD_D(a, i, high);
}

static void _xtQuickSortU_A(unsigned *a, ssize_t low, ssize_t high)
{
	ssize_t i, j;
	unsigned pivot;
	i = low; j = high;
	pivot = a[low + (high - low) / 2];
	while (i <= j) {
		while (a[i] < pivot)
			++i;
		while (a[j] > pivot)
			--j;
		if (i <= j)
			_xtSwapU(&a[i++], &a[j--]);
	}
	if (low < j)
		_xtQuickSortU_A(a, low, j);
	if (i < high)
		_xtQuickSortU_A(a, i, high);
}

static void _xtQuickSortU_D(unsigned *a, ssize_t low, ssize_t high)
{
	ssize_t i, j;
	unsigned pivot;
	i = low; j = high;
	pivot = a[low + (high - low) / 2];
	while (i <= j) {
		while (a[i] > pivot)
			++i;
		while (a[j] < pivot)
			--j;
		if (i <= j)
			_xtSwapU(&a[i++], &a[j--]);
	}
	if (low < j)
		_xtQuickSortU_D(a, low, j);
	if (i < high)
		_xtQuickSortU_D(a, i, high);
}

static int _xtQuickSortP_A(void *list, size_t elemsize, ssize_t low, ssize_t high, int (*cmp)(void*, void*))
{
	int ret = 1;
	ssize_t i, j;
	char *a = list;
	void *pivot = malloc(elemsize);
	void *tmp = malloc(elemsize);
	if (!pivot || !tmp)
		return XT_ENOMEM;
	i = low; j = high;
	memcpy(pivot, &a[low + (high - low) / 2], elemsize);
	while (i <= j) {
		while (cmp(&a[i * elemsize], pivot) < 0)
			++i;
		while (cmp(&a[j * elemsize], pivot) > 0)
			--j;
		if (i <= j)
			_xtSwapP(&a[i++ * elemsize], &a[j-- * elemsize], tmp, elemsize);
	}
	if (low < j && (ret = _xtQuickSortP_A(list, elemsize, low, j, cmp)) != 0)
		goto fail;
	if (i < high && (ret = _xtQuickSortP_A(list, elemsize, i, high, cmp)) != 0)
		goto fail;
	ret = 0;
fail:
	free(tmp);
	free(pivot);
	return ret;
}

static int _xtQuickSortP_D(void *list, size_t elemsize, ssize_t low, ssize_t high, int (*cmp)(void*, void*))
{
	int ret = 1;
	ssize_t i, j;
	char *a = list;
	void *pivot = malloc(elemsize);
	void *tmp = malloc(elemsize);
	if (!pivot || !tmp)
		return XT_ENOMEM;
	i = low; j = high;
	memcpy(pivot, &a[low + (high - low) / 2], elemsize);
	while (i <= j) {
		while (cmp(&a[i * elemsize], pivot) > 0)
			++i;
		while (cmp(&a[j * elemsize], pivot) < 0)
			--j;
		if (i <= j)
			_xtSwapP(&a[i++ * elemsize], &a[j-- * elemsize], tmp, elemsize);
	}
	if (low < j && (ret = _xtQuickSortP_A(list, elemsize, low, j, cmp)) != 0)
		goto fail;
	if (i < high && (ret = _xtQuickSortP_A(list, elemsize, i, high, cmp)) != 0)
		goto fail;
	ret = 0;
fail:
	free(tmp);
	free(pivot);
	return ret;
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

#define K 10

static int _xtRadixSortD(int *a, size_t n, bool ascend)
{
	int m, exp, bb[K], *b;
	size_t i, j;
	b = malloc(n * sizeof(int));
	if (!b)
		return XT_ENOMEM;
	for (m = a[0], exp = i = 1; i < n; ++i)
		if (a[i] > m)
			m = a[i];
	while (m / exp > 0) {
		memset(bb, 0, K * sizeof(int));
		for (i = 0; i < n; ++i)
			++bb[(a[i] / exp) % K];
		for (i = 1; i < K; ++i)
			bb[i] += bb[i - 1];
		for (i = n; i > 0;) {
			--i;
			b[--bb[(a[i] / exp) % K]] = a[i];
		}
		memcpy(a, b, n * sizeof(int));
		exp *= K;
	}
	if (!ascend) {
	        for (i = 0, j = n - 1; i < n; ++i, --j)
			b[i] = a[j];
		memcpy(a, b, n * sizeof(int));
	}
	free(b);
	return 0;
}

static int _xtRadixSortU(unsigned *a, size_t n, bool ascend)
{
	unsigned m, exp, bb[K], *b;
	size_t i, j;
	b = malloc(n * sizeof(unsigned));
	if (!b)
		return XT_ENOMEM;
	for (m = a[0], exp = i = 1; i < n; ++i)
		if (a[i] > m)
			m = a[i];
	while (m / exp > 0) {
		memset(bb, 0, K * sizeof(unsigned));
		for (i = 0; i < n; ++i)
			++bb[(a[i] / exp) % K];
		for (i = 1; i < K; ++i)
			bb[i] += bb[i - 1];
		for (i = n; i > 0;) {
			--i;
			b[--bb[(a[i] / exp) % K]] = a[i];
		}
		memcpy(a, b, n * sizeof(unsigned));
		exp *= K;
	}
	if (!ascend) {
	        for (i = 0, j = n - 1; i < n; ++i, --j)
			b[i] = a[j];
		memcpy(a, b, n * sizeof(unsigned));
	}
	free(b);
	return 0;
}

int xtSortU(unsigned *list, size_t count, xtSortType type, bool ascend)
{
	switch (type) {
	case XT_SORT_BUBBLE:
		_xtBubbleSortU(list, count, ascend);
		break;
	case XT_SORT_HEAP:
		_xtHeapSortU(list, count, ascend);
		break;
	case XT_SORT_INSERT:
		_xtInsertionSortU(list, count, ascend);
		break;
	case XT_SORT_QUICK:
		if (ascend)
			_xtQuickSortU_A(list, 0, count - 1);
		else
			_xtQuickSortU_D(list, 0, count - 1);
		break;
	case XT_SORT_SELECT:
		_xtSelectionSortU(list, count, ascend);
		break;
	case XT_SORT_RADIX:
		return _xtRadixSortU(list, count, ascend);
	default:
		return XT_EINVAL;
	}
	return 0;
}

int xtSortD(int *list, size_t count, xtSortType type, bool ascend)
{
	switch (type) {
	case XT_SORT_BUBBLE:
		_xtBubbleSortD   (list, count, ascend);
		break;
	case XT_SORT_HEAP:
		_xtHeapSortD     (list, count, ascend);
		break;
	case XT_SORT_INSERT:
		_xtInsertionSortD(list, count, ascend);
		break;
	case XT_SORT_QUICK:
		if (ascend)
			_xtQuickSortD_A(list, 0, count - 1);
		else
			_xtQuickSortD_D(list, 0, count - 1);
		break;
	case XT_SORT_SELECT:
		_xtSelectionSortD(list, count, ascend);
		break;
	case XT_SORT_RADIX:
		return _xtRadixSortD(list, count, ascend);
	default:
		return XT_EINVAL;
	}
	return 0;
}

int xtSortP(void *list, size_t count, xtSortType type, int (*cmp)(void*,void*), bool ascend, size_t elemSize)
{
	if (!cmp)
		return XT_EINVAL;
	switch (type) {
	case XT_SORT_BUBBLE: return _xtBubbleSortP   (list, elemSize, count, cmp, ascend);
	case XT_SORT_HEAP  : return _xtHeapSortP     (list, elemSize, count, cmp, ascend);
	case XT_SORT_INSERT: return _xtInsertionSortP(list, elemSize, count, cmp, ascend);
	case XT_SORT_QUICK :
		if (ascend)
			return _xtQuickSortP_A(list, elemSize, 0, count - 1, cmp);
		else
			return _xtQuickSortP_D(list, elemSize, 0, count - 1, cmp);
	case XT_SORT_SELECT: return _xtSelectionSortP(list, elemSize, count, cmp, ascend);
	default:
		return XT_EINVAL;
	}
}

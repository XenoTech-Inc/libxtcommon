// XT headers
#include <xt/error.h>
#include <xt/os_macros.h>
#include <xt/sort.h>

// STD headers
#include <stdlib.h>
#include <string.h>

#ifndef _SSIZE_T_DEFINED
#if XT_IS_WINDOWS
	#if XT_IS_X64
		typedef long long ssize_t;
	#elif XT_IS_X86
		typedef long ssize_t;
	#endif
#endif
#endif

/* swap element routines are inlined so they
have to be defined first before we can use it */
static inline void swap_d(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}
static inline void swap_u(unsigned *a, unsigned *b)
{
	unsigned tmp = *a;
	*a = *b;
	*b = tmp;
}

static inline void swap_p(void *a, void *b, void *tmp, size_t size)
{
	memcpy(tmp, a  , size);
	memcpy(a  , b  , size);
	memcpy(b  , tmp, size);
}

static void bubble_sort_u(unsigned *a, size_t n, bool ascend)
{
	size_t i, x;
	if (ascend)
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] > a[i])
					swap_u(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
	else
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] < a[i])
					swap_u(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
}

static void bubble_sort_d(int *a, size_t n, bool ascend)
{
	size_t i, x;
	if (ascend)
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] > a[i])
					swap_d(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
	else
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (a[i - 1] < a[i])
					swap_d(&a[i - 1], &a[x = i]);
			--n;
		} while (x);
}

static int bubble_sort_p(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
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
					swap_p(&a[(i - 1) * elemsize], &a[(x = i) * elemsize], tmp, elemsize);
			--n;
		} while (x);
	else
		do {
			x = 0;
			for (i = 1; i < n; ++i)
				if (cmp(&a[(i - 1) * elemsize], &a[i * elemsize]) < 0)
					swap_p(&a[(i - 1) * elemsize], &a[(x = i) * elemsize], tmp, elemsize);
			--n;
		} while (x);
	free(tmp);
	return 0;
}

/*
Some sorting algorithms require additional routines.
The ones ending in A are for ascending order and D for descending.
*/
static void heap_percolate_d_a(int *a, size_t low, size_t high)
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

static void heap_percolate_d_d(int *a, size_t low, size_t high)
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

static void heap_sort_d(int *a, size_t n, bool ascend) {
	ssize_t i;
	if (ascend) {
		for (i = n / 2; i >= 0; --i)
			heap_percolate_d_a(a, i, n);
		for (i = n - 1; i > 0; --i) {
			swap_d(a, a + i);
			heap_percolate_d_a(a, 0, i);
		}
	} else {
		for (i = n / 2; i >= 0; --i)
			heap_percolate_d_d(a, i, n);
		for (i = n - 1; i > 0; --i) {
			swap_d(a, a + i);
			heap_percolate_d_d(a, 0, i);
		}
	}
}

static void heap_percolate_u_a(unsigned *a, size_t low, size_t high)
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

static void heap_percolate_u_d(unsigned *a, size_t low, size_t high)
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

static void heap_sort_u(unsigned *a, size_t n, bool ascend)
{
	ssize_t i;
	if (ascend) {
		for (i = n / 2; i >= 0; --i)
			heap_percolate_u_a(a, i, n);
		for (i = n - 1; i > 0; --i) {
			swap_u(a, a + i);
			heap_percolate_u_a(a, 0, i);
		}
	} else {
		for (i = n / 2; i >= 0; --i)
			heap_percolate_u_d(a, i, n);
		for (i = n - 1; i > 0; --i) {
			swap_u(a, a + i);
			heap_percolate_u_d(a, 0, i);
		}
	}
}

static void heap_percolate_p_a(void *list, void *tmp, size_t elemsize, int (*cmp)(void*, void*), size_t low, size_t high)
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

static void heap_percolate_p_d(void *list, void *tmp, size_t elemsize, int (*cmp)(void*, void*), size_t low, size_t high)
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

static int heap_sort_p(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
{
	ssize_t i;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return XT_ENOMEM;
	if (ascend) {
		for (i = n / 2; i >= 0; --i)
			heap_percolate_p_a(list, tmp, elemsize, cmp, i, n);
		for (i = n - 1; i > 0; --i) {
			swap_p(a, &a[i * elemsize], tmp, elemsize);
			heap_percolate_p_a(list, tmp, elemsize, cmp, 0, i);
		}
	} else {
		for (i = n / 2; i >= 0; --i)
			heap_percolate_p_d(list, tmp, elemsize, cmp, i, n);
		for (i = n - 1; i > 0; --i) {
			swap_p(a, &a[i * elemsize], tmp, elemsize);
			heap_percolate_p_d(list, tmp, elemsize, cmp, 0, i);
		}
	}
	free(tmp);
	return 0;
}

static void insertion_sort_d(int *a, size_t n, bool ascend)
{
	size_t i, j;
	if (ascend)
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] > a[j]; --j)
				swap_d(&a[j], &a[j - 1]);
	else
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] < a[j]; --j)
				swap_d(&a[j], &a[j - 1]);
}

static void insertion_sort_u(unsigned *a, size_t n, bool ascend)
{
	size_t i, j;
	if (ascend)
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] > a[j]; --j)
				swap_u(&a[j], &a[j - 1]);
	else
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && a[j - 1] < a[j]; --j)
				swap_u(&a[j], &a[j - 1]);
}

static int insertion_sort_p(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
{
	size_t i, j;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return XT_ENOMEM;
	if (ascend)
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && cmp(&a[(j - 1) * elemsize], &a[j * elemsize]) > 0; --j)
				swap_p(&a[j * elemsize], &a[(j - 1) * elemsize], tmp, elemsize);
	else
		for (i = 1; i < n; ++i)
			for (j = i; j > 0 && cmp(&a[(j - 1) * elemsize], &a[j * elemsize]) < 0; --j)
				swap_p(&a[j * elemsize], &a[(j - 1) * elemsize], tmp, elemsize);
	free(tmp);
	return 0;
}

static void radix_quick_sort_d_a(int *a, ssize_t low, ssize_t high)
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
			swap_d(&a[i++], &a[j--]);
	}
	if (low < j)
		radix_quick_sort_d_a(a, low, j);
	if (i < high)
		radix_quick_sort_d_a(a, i, high);
}

static void radix_quick_sort_d_d(int *a, ssize_t low, ssize_t high)
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
			swap_d(&a[i++], &a[j--]);
	}
	if (low < j)
		radix_quick_sort_d_d(a, low, j);
	if (i < high)
		radix_quick_sort_d_d(a, i, high);
}

static void radix_quick_sort_u_a(unsigned *a, ssize_t low, ssize_t high)
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
			swap_u(&a[i++], &a[j--]);
	}
	if (low < j)
		radix_quick_sort_u_a(a, low, j);
	if (i < high)
		radix_quick_sort_u_a(a, i, high);
}

static void radix_quick_sort_u_d(unsigned *a, ssize_t low, ssize_t high)
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
			swap_u(&a[i++], &a[j--]);
	}
	if (low < j)
		radix_quick_sort_u_d(a, low, j);
	if (i < high)
		radix_quick_sort_u_d(a, i, high);
}

static int radix_quick_sort_p_a(void *list, size_t elemsize, ssize_t low, ssize_t high, int (*cmp)(void*, void*))
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
			swap_p(&a[i++ * elemsize], &a[j-- * elemsize], tmp, elemsize);
	}
	if (low < j && (ret = radix_quick_sort_p_a(list, elemsize, low, j, cmp)))
		goto fail;
	if (i < high && (ret = radix_quick_sort_p_a(list, elemsize, i, high, cmp)))
		goto fail;
	ret = 0;
fail:
	free(tmp);
	free(pivot);
	return ret;
}

static int radix_quick_sort_p_d(void *list, size_t elemsize, ssize_t low, ssize_t high, int (*cmp)(void*, void*))
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
			swap_p(&a[i++ * elemsize], &a[j-- * elemsize], tmp, elemsize);
	}
	if (low < j && (ret = radix_quick_sort_p_d(list, elemsize, low, j, cmp)))
		goto fail;
	if (i < high && (ret = radix_quick_sort_p_d(list, elemsize, i, high, cmp)))
		goto fail;
	ret = 0;
fail:
	free(tmp);
	free(pivot);
	return ret;
}

static void selection_sort_d(int *a, size_t n, bool ascend)
{
	size_t i, j, min;
	if (ascend)
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] < a[min])
					min = i;
			if (min != j)
				swap_d(&a[min], &a[j]);
		}
	else
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] > a[min])
					min = i;
			if (min != j)
				swap_d(&a[min], &a[j]);
		}
}

static void selection_sort_u(unsigned *a, size_t n, bool ascend)
{
	size_t i, j, min;
	if (ascend)
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] < a[min])
					min = i;
			if (min != j)
				swap_u(&a[min], &a[j]);
		}
	else
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (a[i] > a[min])
					min = i;
			if (min != j)
				swap_u(&a[min], &a[j]);
		}
}

static int selection_sort_p(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*), bool ascend)
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
				swap_p(&a[min * elemsize], &a[j * elemsize], tmp, elemsize);
		}
	else
		for (j = 0; j < n - 1; ++j) {
			min = j;
			for (i = j + 1; i < n; ++i)
				if (cmp(&a[i * elemsize], &a[min * elemsize]) > 0)
					min = i;
			if (min != j)
				swap_p(&a[min * elemsize], &a[j * elemsize], tmp, elemsize);
		}
	free(tmp);
	return 0;
}

#define K 10

static int radix_sort_d(int *a, size_t n, bool ascend)
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

static int radix_sort_u(unsigned *a, size_t n, bool ascend)
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

int xtSortU(unsigned *list, size_t count, enum xtSortType type, bool ascend)
{
	switch (type) {
	case XT_SORT_BUBBLE:
		bubble_sort_u(list, count, ascend);
		break;
	case XT_SORT_HEAP:
		heap_sort_u(list, count, ascend);
		break;
	case XT_SORT_INSERT:
		insertion_sort_u(list, count, ascend);
		break;
	case XT_SORT_QUICK:
		if (ascend)
			radix_quick_sort_u_a(list, 0, count - 1);
		else
			radix_quick_sort_u_d(list, 0, count - 1);
		break;
	case XT_SORT_SELECT:
		selection_sort_u(list, count, ascend);
		break;
	case XT_SORT_RADIX:
		return radix_sort_u(list, count, ascend);
	default:
		return XT_EINVAL;
	}
	return 0;
}

int xtSortD(int *list, size_t count, enum xtSortType type, bool ascend)
{
	switch (type) {
	case XT_SORT_BUBBLE:
		bubble_sort_d   (list, count, ascend);
		break;
	case XT_SORT_HEAP:
		heap_sort_d     (list, count, ascend);
		break;
	case XT_SORT_INSERT:
		insertion_sort_d(list, count, ascend);
		break;
	case XT_SORT_QUICK:
		if (ascend)
			radix_quick_sort_d_a(list, 0, count - 1);
		else
			radix_quick_sort_d_d(list, 0, count - 1);
		break;
	case XT_SORT_SELECT:
		selection_sort_d(list, count, ascend);
		break;
	case XT_SORT_RADIX:
		return radix_sort_d(list, count, ascend);
	default:
		return XT_EINVAL;
	}
	return 0;
}

int xtSortP(void *list, size_t count, enum xtSortType type, int (*cmp)(void*,void*), bool ascend, size_t elemSize)
{
	if (!cmp)
		return XT_EINVAL;
	switch (type) {
	case XT_SORT_BUBBLE: return bubble_sort_p   (list, elemSize, count, cmp, ascend);
	case XT_SORT_HEAP  : return heap_sort_p     (list, elemSize, count, cmp, ascend);
	case XT_SORT_INSERT: return insertion_sort_p(list, elemSize, count, cmp, ascend);
	case XT_SORT_QUICK :
		if (ascend)
			return radix_quick_sort_p_a(list, elemSize, 0, count - 1, cmp);
		else
			return radix_quick_sort_p_d(list, elemSize, 0, count - 1, cmp);
	case XT_SORT_SELECT: return selection_sort_p(list, elemSize, count, cmp, ascend);
	default:
		return XT_EINVAL;
	}
}

static int sort_str_cmp(void *a, void *b)
{
	return strcmp(*(char**)a, *(char**)b);
}

int xtSortStr(char **list, size_t count, enum xtSortType type, bool ascend)
{
	return xtSortP(list, count, type, sort_str_cmp, ascend, sizeof(char*));
}

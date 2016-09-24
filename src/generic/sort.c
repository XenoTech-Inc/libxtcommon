#include <stdlib.h>
#include <string.h>
#include <xt/sort.h>

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

static void _xtISortD(int *a, size_t n)
{
	size_t i, j;
	for (i = 1; i < n; ++i)
		for (j = i; j > 0 && a[j - 1] > a[j]; --j)
			_xtSwapD(&a[j], &a[j - 1]);
}

static void _xtISortU(unsigned *a, size_t n)
{
	size_t i, j;
	for (i = 1; i < n; ++i)
		for (j = i; j > 0 && a[j - 1] > a[j]; --j)
			_xtSwapU(&a[j], &a[j - 1]);
}

static int _xtISortP(void *list, size_t elemsize, size_t n, int (*cmp)(void*, void*))
{
	size_t i, j;
	char *a = list;
	void *tmp = malloc(elemsize);
	if (!tmp)
		return 1;
	for (i = 1; i < n; ++i)
		for (j = i; j > 0 && cmp(&a[(j - 1) * elemsize], &a[j * elemsize]) > 0; --j)
			_xtSwapP(&a[j * elemsize], &a[(j - 1) * elemsize], tmp, elemsize);
	free(tmp);
	return 0;
}

int xtSortU(unsigned *list, size_t count, xtSortType type, bool ascend)
{
	_xtISortU(list, count);
	(void)type;
	(void)ascend;
	return 0;
}

int xtSortD(int *list, size_t count, xtSortType type, bool ascend)
{
	_xtISortD(list, count);
	(void)type;
	(void)ascend;
	return 0;
}

int xtSortP(void *list, size_t count, xtSortType type, int (*cmp)(void*,void*), bool ascend, size_t elemSize)
{
	(void)type;
	(void)ascend;
	if (!cmp)
		return 1;
	return _xtISortP(list, elemsize, count, cmp);
}

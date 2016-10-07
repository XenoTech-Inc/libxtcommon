#include <xt/sort.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void chklistu(unsigned *a, size_t n, int ascend)
{
	if (ascend)
		for (size_t i = 1; i < n; ++i)
			assert(a[i - 1] <= a[i]);
	else
		for (size_t i = 1; i < n; ++i)
			assert(a[i - 1] >= a[i]);
}

static void chklistd(int *a, size_t n, int ascend)
{
	if (ascend)
		for (size_t i = 1; i < n; ++i)
			assert(a[i - 1] <= a[i]);
	else
		for (size_t i = 1; i < n; ++i)
			assert(a[i - 1] >= a[i]);
}

#define ASZ 4096

static void arndu(unsigned *a, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		a[i] = rand();
}

static void arndd(int *a, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		a[i] = rand();
}

xtSortType types[] = {
	XT_SORT_BUBBLE,
	XT_SORT_HEAP  ,
	XT_SORT_INSERT,
	XT_SORT_QUICK ,
	XT_SORT_SELECT,
	XT_SORT_RADIX ,
};

const char *names[] = {
	"bubble", "heap", "insert",
	"quick", "select", "radix"
};

#define NTYPE (sizeof(types)/sizeof types[0])

static void sortu(void)
{
	unsigned a[ASZ];
	printf("Sort ascending %u unsigned ints", ASZ);
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndu(a, ASZ);
		assert(!xtSortU(a, ASZ, types[i], 1));
		chklistu(a, ASZ, 1);
		putchar('.');
	}
	putchar('\n');
	printf("Sort descending %u unsigned ints", ASZ);
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndu(a, ASZ);
		assert(!xtSortU(a, ASZ, types[i], 0));
		chklistu(a, ASZ, 0);
		putchar('.');
	}
	putchar('\n');
}

static void sortd(void)
{
	int a[ASZ];
	printf("Sort ascending %u unsigned ints", ASZ);
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndd(a, ASZ);
		assert(!xtSortD(a, ASZ, types[i], 1));
		chklistd(a, ASZ, 1);
		putchar('.');
	}
	putchar('\n');
	printf("Sort descending %u unsigned ints", ASZ);
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndd(a, ASZ);
		assert(!xtSortD(a, ASZ, types[i], 0));
		chklistd(a, ASZ, 0);
		putchar('.');
	}
	putchar('\n');
}

int main(void)
{
	srand(time(NULL));
	fputs("SORT TEST\nAlgorithms:", stdout);
	for (unsigned i = 0; i < NTYPE; ++i)
		printf(" %s", names[i]);
	putchar('\n');
	sortu();
	sortd();
	puts("done");
	return 0;
}

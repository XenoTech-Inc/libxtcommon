#include <xt/sort.h>
#include <xt/time.h>
#include <xt/os.h>
#include <xt/string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

static struct stats stats;

#define CHK_SUBTYPE U
#define CHK_TYPE unsigned

#include "chklist.h"

#define CHK_SUBTYPE D
#define CHK_TYPE int

#include "chklist.h"

#define ASZ 4096

enum xtSortType types[] = {
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
	char buf[256];
	unsigned a[ASZ];
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndU(a, ASZ);
		if (xtSortU(a, ASZ, types[i], 1)) {
			snprintf(buf, sizeof buf, "xtSortU() - %s ascending", names[i]);
			FAIL(buf);
			continue;
		}
		chklistU(a, ASZ, 1, names[i]);
	}
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndU(a, ASZ);
		if (xtSortU(a, ASZ, types[i], 0)) {
			snprintf(buf, sizeof buf, "xtSortU() - %s descending", names[i]);
			FAIL(buf);
			continue;
		}
		chklistU(a, ASZ, 0, names[i]);
	}
}

static void sortd(void)
{
	char buf[256];
	int a[ASZ];
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndD(a, ASZ);
		if (xtSortD(a, ASZ, types[i], 1)) {
			snprintf(buf, sizeof buf, "xtSortD() - %s ascending", names[i]);
			FAIL(buf);
			continue;
		}
		chklistD(a, ASZ, 1, names[i]);
	}
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndD(a, ASZ);
		if (xtSortD(a, ASZ, types[i], 0)) {
			snprintf(buf, sizeof buf, "xtSortD() - %s descending", names[i]);
			FAIL(buf);
			continue;
		}
		chklistD(a, ASZ, 0, names[i]);
	}
}

static void large(void)
{
	size_t n = 1 << 14LLU;
	unsigned *a = malloc(n * sizeof(unsigned));
	if (!a) abort();
	struct xtTimestamp then, now;
	char buf[256];
	xtprintf("Sort ascending %zu unsigned ints\n", n);
	for (unsigned i = 0; i < NTYPE; ++i) {
		arndU(a, n);
		xtfprintf(stdout, "%s: ", names[i]);
		fflush(stdout);
		xtClockGetTime(&then, XT_CLOCK_MONOTONIC);
		xtSortU(a, n, types[i], 1);
		xtClockGetTime(&now, XT_CLOCK_MONOTONIC);
		chklistU(a, n, 1, names[i]);
		xtFormatTimeDuration(buf, sizeof buf, "Elapsed time: %G", &then, &now);
		puts(buf);
	}
	free(a);
}

int main(void)
{
	stats_init(&stats, "sort");
	srand(time(NULL));
	puts("-- SORT TEST");
	fputs("Algorithms:", stdout);
	for (unsigned i = 0; i < NTYPE; ++i)
		xtprintf(" %s", names[i]);
	putchar('\n');
	sortu();
	sortd();
	large();
	puts("done");
	stats_info(&stats);
	return stats_status(&stats);
}

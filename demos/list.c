// os_macros.h must be first in order to make it work
#include <xt/os_macros.h>
#include <xt/file.h>
#include <xt/list.h>
#include <xt/os.h>
#include <xt/string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

static struct stats stats;

#if XT_IS_LINUX
	#define PTR_CAST long
#elif XT_IS_WINDOWS
	#define PTR_CAST long long
#endif

static struct xtListHD listHD;
static struct xtListD  listD;
static struct xtListU  listU;
static struct xtListLU listLU;
static struct xtListZU listZU;
static struct xtListP  listP;

// Internal arrays which we're using to check later on if the xtLists
// do not corrupt themselves
static size_t cntHD = 0, cntD = 0, cntU = 0, cntLU = 0, cntZU = 0, cntP = 0;
static short *arrayHD;
static int *arrayD;
static unsigned *arrayU;
static unsigned long *arrayLU;
static size_t *arrayZU;
static void **arrayP;

#define INIT_HD  1
#define INIT_D   2
#define INIT_U   4
#define INIT_LU  8
#define INIT_ZU 16
#define INIT_P  32

static unsigned init = 0;

void add(unsigned times)
{
	unsigned i;
	char buf[256];

	printf("Adding %u random elements of data to all lists\n", times);

	if (init & INIT_HD) {
		for (i = 0; i < times; ++i, ++cntHD) {
			short valHD = rand();
			if (xtListHDAdd(&listHD, valHD)) {
				xtsnprintf(buf, sizeof buf, "xtListHDAdd(%hd)", valHD);
				FAIL(buf);
				break;
			}
			arrayHD[i] = valHD;
		}
		if (i == times)
			PASS("xtListHDAdd");
	} else
		SKIP("xtListHDAdd");

	if (init & INIT_D) {
		for (i = 0; i < times; ++i, ++cntD) {
			int valD = rand();
			if (xtListDAdd(&listD, valD)) {
				xtsnprintf(buf, sizeof buf, "xtListDAdd(%d)", valD);
				FAIL(buf);
				break;
			}
			arrayD[i] = valD;
		}
		if (i == times)
			PASS("xtListDAdd");
	} else
		SKIP("xtListDAdd");

	if (init & INIT_U) {
		for (i = 0; i < times; ++i, ++cntU) {
			unsigned valU = rand();
			if (xtListUAdd(&listU, valU)) {
				xtsnprintf(buf, sizeof buf, "xtListUAdd(%u)", valU);
				FAIL(buf);
				break;
			}
			arrayU[i] = valU;
		}
		if (i == times)
			PASS("xtListUAdd");
	} else
		SKIP("xtListUAdd");

	if (init & INIT_LU) {
		for (i = 0; i < times; ++i, ++cntLU) {
			unsigned long valLU = rand();
			if (xtListLUAdd(&listLU, valLU)) {
				xtsnprintf(buf, sizeof buf, "xtListLUdd(%lu)", valLU);
				FAIL(buf);
				break;
			}
			arrayLU[i] = valLU;
		}
		if (i == times)
			PASS("xtListLUAdd");
	} else
		SKIP("xtListLUAdd");

	if (init & INIT_ZU) {
		for (i = 0; i < times; ++i, ++cntZU) {
			size_t valZU = rand();
			if (xtListZUAdd(&listZU, valZU)) {
				xtsnprintf(buf, sizeof buf, "xtListZUAdd(%zu)", valZU);
				FAIL(buf);
				break;
			}
			arrayZU[i] = valZU;
		}
		if (i == times)
			PASS("xtListZUAdd");
	} else
		SKIP("xtListZUAdd");

	if (init & INIT_P) {
		for (i = 0; i < times; ++i, ++cntP) {
			// Suppress compiler warning for casting scalar
			// type of different size to void pointer.
			PTR_CAST v = rand();
			PTR_CAST *vp = &v;
			void *valP = vp;
			if (xtListPAdd(&listP, valP)) {
				xtsnprintf(buf, sizeof buf, "xtListPAdd(%p)", valP);
				FAIL(buf);
				break;
			}
			arrayP[i] = valP;
		}
		if (i == times)
			PASS("xtListPAdd");
	} else
		SKIP("xtListPAdd");
}

void compareLists(void)
{
	size_t i;
	char buf[256];

	puts("Comparing lists with internal arrays to see if no corruption has occurred");

	if (init & INIT_HD) {
		if (cntHD != xtListHDGetCount(&listHD)) {
			xtsnprintf(buf, sizeof buf, "xtListHDGetCount: expected %zu, but got %zu", cntHD, xtListHDGetCount(&listHD));
			FAIL(buf);
		} else
			PASS("xtListHDGetCount");
	} else
		SKIP("xtListHDGetCount");

	if (init & INIT_D) {
		if (cntD != xtListDGetCount(&listD)) {
			xtsnprintf(buf, sizeof buf, "xtListDGetCount: expected %zu, but got %zu", cntD, xtListDGetCount(&listD));
			FAIL(buf);
		} else
			PASS("xtListDGetCount");
	} else
		SKIP("xtListDGetCount");

	if (init & INIT_U) {
		if (cntU != xtListUGetCount(&listU)) {
			xtsnprintf(buf, sizeof buf, "xtListUGetCount: expected %zu, but got %zu", cntU, xtListUGetCount(&listU));
			FAIL(buf);
		} else
			PASS("xtListUGetCount");
	} else
		SKIP("xtListUGetCount");

	if (init & INIT_LU) {
		if (cntLU != xtListLUGetCount(&listLU)) {
			xtsnprintf(buf, sizeof buf, "xtListLUGetCount: expected %zu, but got %zu", cntLU, xtListLUGetCount(&listLU));
			FAIL(buf);
		} else
			PASS("xtListLUGetCount");
	} else
		SKIP("xtListLUGetCount");

	if (init & INIT_ZU) {
		if (cntZU != xtListZUGetCount(&listZU)) {
			xtsnprintf(buf, sizeof buf, "xtListZUGetCount: expected %zu, but got %zu", cntZU, xtListZUGetCount(&listZU));
			FAIL(buf);
		} else
			PASS("xtListZUGetCount");
	} else
		SKIP("xtListZUGetCount");

	if (init & INIT_P) {
		if (cntP != xtListPGetCount(&listP)) {
			xtsnprintf(buf, sizeof buf, "xtListPGetCount: expected %zu, but got %zu", cntP, xtListPGetCount(&listP));
			FAIL(buf);
		} else
			PASS("xtListPGetCount");
	} else
		SKIP("xtListPGetCount");

	short valHD;
	int valD;
	unsigned valU;
	unsigned long valLU;
	size_t valZU;
	void *valP;

	if (init & INIT_HD) {
		for (i = 0; i < cntHD; ++i)
			if (xtListHDGet(&listHD, i, &valHD) != 0 || arrayHD[i] != valHD) {
				xtsnprintf(buf, sizeof buf, "xtListHDGet(p, %zu, p): expected %hd, but got %hd", arrayHD[i], valHD);
				FAIL(buf);
				break;
			}
		if (i == cntHD)
			PASS("xtListHDGet");
	} else
		SKIP("xtListHDGet");

	if (init & INIT_D) {
		for (i = 0; i < cntD; ++i)
			if (xtListDGet(&listD, i, &valD) != 0 || arrayD[i] != valD) {
				xtsnprintf(buf, sizeof buf, "xtListDGet(p, %zu, p): expected %d, but got %d", arrayD[i], valD);
				FAIL(buf);
				break;
			}
		if (i == cntD)
			PASS("xtListDGet");
	} else
		SKIP("xtListDGet");

	if (init & INIT_HD) {
		for (i = 0; i < cntU; ++i)
			if (xtListUGet(&listU, i, &valU) != 0 || arrayU[i] != valU) {
				xtsnprintf(buf, sizeof buf, "xtListUGet(p, %zu, p): expected %u, but got %u", arrayU[i], valU);
				FAIL(buf);
				break;
			}
		if (i == cntU)
			PASS("xtListUGet");
	} else
		SKIP("xtListUGet");

	if (init & INIT_LU) {
		for (i = 0; i < cntLU; ++i)
			if (xtListLUGet(&listLU, i, &valLU) != 0 || arrayLU[i] != valLU) {
				xtsnprintf(buf, sizeof buf, "xtListLUGet(p, %zu, p): expected %lu, but got %lu", arrayLU[i], valLU);
				FAIL(buf);
				break;
			}
		if (i == cntLU)
			PASS("xtListLUGet");
	} else
		SKIP("xtListLUGet");

	if (init & INIT_ZU) {
		for (i = 0; i < cntZU; ++i)
			if (xtListZUGet(&listZU, i, &valZU) != 0 || arrayZU[i] != valZU) {
				xtsnprintf(buf, sizeof buf, "xtListZUGet(p, %zu, p): expected %zu, but got %zu", arrayZU[i], valZU);
				FAIL(buf);
				break;
			}
		if (i == cntZU)
			PASS("xtListZUGet");
	} else
		SKIP("xtListZUGet");

	if (init & INIT_P) {
		for (i = 0; i < cntP; ++i)
			if (xtListPGet(&listP, i, &valP) != 0 || arrayP[i] != valP) {
				xtsnprintf(buf, sizeof buf, "xtListPGet(p, %zu, p): expected %p, but got %p", arrayP[i], valP);
				FAIL(buf);
				break;
			}
		if (i == cntP)
			PASS("xtListPGet");
	} else
		SKIP("xtListPGet");
}

void create(size_t initialCapacity, size_t maxCapacity)
{
	puts("Initializing all lists");
	if (xtListHDCreate(&listHD, initialCapacity))
		FAIL("xtListHDCreate");
	else
		init |= INIT_HD;
	if (xtListDCreate(&listD, initialCapacity))
		FAIL("xtListDCreate");
	else
		init |= INIT_D;
	if (xtListUCreate(&listU, initialCapacity))
		FAIL("xtListUCreate");
	else
		init |= INIT_U;
	if (xtListLUCreate(&listLU, initialCapacity))
		FAIL("xtListLUCreate");
	else
		init |= INIT_LU;
	if (xtListZUCreate(&listZU, initialCapacity))
		FAIL("xtListZUCreate");
	else
		init |= INIT_ZU;
	if (xtListPCreate(&listP, initialCapacity))
		FAIL("xtListPCreate");
	else
		init |= INIT_P;

	arrayHD = malloc(maxCapacity * sizeof *arrayHD);
	arrayD = malloc(maxCapacity * sizeof *arrayD);
	arrayU = malloc(maxCapacity * sizeof *arrayU);
	arrayLU = malloc(maxCapacity * sizeof *arrayLU);
	arrayZU = malloc(maxCapacity * sizeof *arrayZU);
	arrayP = malloc(maxCapacity * sizeof *arrayP);

	if (!arrayHD || !arrayD || !arrayU || !arrayLU || !arrayZU || !arrayP) {
		fputs("Out of memory\n", stderr);
		abort();
	}
}

void destroy(void)
{
	puts("Clearing and destroying all lists");

	if (init & INIT_HD) {
		xtListHDClear(&listHD);
		xtListHDDestroy(&listHD);
	}

	if (init & INIT_D) {
		xtListDClear(&listD);
		xtListDDestroy(&listD);
	}

	if (init & INIT_U) {
		xtListUClear(&listU);
		xtListUDestroy(&listU);
	}

	if (init & INIT_LU) {
		xtListLUClear(&listLU);
		xtListLUDestroy(&listLU);
	}

	if (init & INIT_ZU) {
		xtListZUClear(&listZU);
		xtListZUDestroy(&listZU);
	}

	if (init & INIT_P) {
		xtListPClear(&listP);
		xtListPDestroy(&listP);
	}

	free(arrayHD);
	free(arrayD);
	free(arrayU);
	free(arrayLU);
	free(arrayZU);
	free(arrayP);
}

void ensureCapacity(size_t newCapacity)
{
	int ret;
	char buf[256];

	puts("Growth test - All lists will be enlarged");

	if (init & INIT_HD) {
		ret = xtListHDEnsureCapacity(&listHD, newCapacity);

		if (ret || xtListHDGetCapacity(&listHD) != newCapacity) {
			xtsnprintf(buf, sizeof buf,
				"xtListHDEnsureCapacity: expected %zu, but got %zu",
				newCapacity, xtListHDGetCapacity(&listHD)
			);
			FAIL(buf);
		}

		PASS("xtListHDEnsureCapacity");
	} else
		SKIP("xtListHDEnsureCapacity");

	if (init & INIT_D) {
		ret = xtListDEnsureCapacity(&listD, newCapacity);

		if (ret || xtListDGetCapacity(&listD) != newCapacity) {
			xtsnprintf(buf, sizeof buf,
				"xtListDEnsureCapacity: expected %zu, but got %zu",
				newCapacity, xtListDGetCapacity(&listD)
			);
			FAIL(buf);
		}

		PASS("xtListDEnsureCapacity");
	} else
		SKIP("xtListDEnsureCapacity");

	if (init & INIT_U) {
		ret = xtListUEnsureCapacity(&listU, newCapacity);

		if (ret || xtListUGetCapacity(&listU) != newCapacity) {
			xtsnprintf(buf, sizeof buf,
				"xtListUEnsureCapacity: expected %zu, but got %zu",
				newCapacity, xtListUGetCapacity(&listU)
			);
			FAIL(buf);
		}

		PASS("xtListUEnsureCapacity");
	} else
		SKIP("xtListUEnsureCapacity");

	if (init & INIT_LU) {
		ret = xtListLUEnsureCapacity(&listLU, newCapacity);

		if (ret || xtListLUGetCapacity(&listLU) != newCapacity) {
			xtsnprintf(buf, sizeof buf,
				"xtListLUEnsureCapacity: expected %zu, but got %zu",
				newCapacity, xtListLUGetCapacity(&listLU)
			);
			FAIL(buf);
		}

		PASS("xtListLUEnsureCapacity");
	} else
		SKIP("xtListLUEnsureCapacity");

	if (init & INIT_ZU) {
		ret = xtListZUEnsureCapacity(&listZU, newCapacity);

		if (ret || xtListZUGetCapacity(&listZU) != newCapacity) {
			xtsnprintf(buf, sizeof buf,
				"xtListZUEnsureCapacity: expected %zu, but got %zu",
				newCapacity, xtListZUGetCapacity(&listZU)
			);
			FAIL(buf);
		}

		PASS("xtListZUEnsureCapacity");
	} else
		SKIP("xtListZUEnsureCapacity");

	if (init & INIT_P) {
		ret = xtListPEnsureCapacity(&listP, newCapacity);

		if (ret || xtListPGetCapacity(&listP) != newCapacity) {
			xtsnprintf(buf, sizeof buf,
				"xtListPEnsureCapacity: expected %zu, but got %zu",
				newCapacity, xtListPGetCapacity(&listP)
			);
			FAIL(buf);
		}

		PASS("xtListPEnsureCapacity");
	} else
		SKIP("xtListPEnsureCapacity");
}

void removeAt(void)
{
	int index = 2;

	puts("Removing elements");

	if (init & INIT_HD) {
		if (xtListHDRemoveAt(&listHD, index))
			FAIL("xtListHDRemoveAt");
		else {
			PASS("xtListHDRemoveAt");
			memmove(&arrayHD[index], &arrayHD[index + 1], (--cntHD - index) * sizeof *arrayHD);
		}
	} else
		SKIP("xtListHDRemoveAt");

	if (init & INIT_D) {
		if (xtListDRemoveAt(&listD, index))
			FAIL("xtListDRemoveAt");
		else {
			PASS("xtListDRemoveAt");
			memmove(&arrayD[index], &arrayD[index + 1], (--cntD - index) * sizeof *arrayD);
		}
	} else
		SKIP("xtListDRemoveAt");

	if (init & INIT_U) {
		if (xtListURemoveAt(&listU, index))
			FAIL("xtListURemoveAt");
		else {
			PASS("xtListURemoveAt");
			memmove(&arrayU[index], &arrayU[index + 1], (--cntU - index) * sizeof *arrayU);
		}
	} else
		SKIP("xtListURemoveAt");

	if (init & INIT_LU) {
		if (xtListLURemoveAt(&listLU, index))
			FAIL("xtListLURemoveAt");
		else {
			PASS("xtListLURemoveAt");
			memmove(&arrayLU[index], &arrayLU[index + 1], (--cntLU - index) * sizeof *arrayLU);
		}
	} else
		SKIP("xtListLURemoveAt");

	if (init & INIT_ZU) {
		if (xtListZURemoveAt(&listZU, index))
			FAIL("xtListZURemoveAt");
		else {
			PASS("xtListZURemoveAt");
			memmove(&arrayZU[index], &arrayZU[index + 1], (--cntZU - index) * sizeof *arrayZU);
		}
	} else
		SKIP("xtListZURemoveAt");

	if (init & INIT_P) {
		if (xtListPRemoveAt(&listP, index))
			FAIL("xtListPRemoveAt");
		else {
			PASS("xtListPRemoveAt");
			memmove(&arrayP[index], &arrayP[index + 1], (--cntP - index) * sizeof *arrayP);
		}
	} else
		SKIP("xtListPRemoveAt");
}

int main(void)
{
	stats_init(&stats, "list");
	srand(time(NULL));
	size_t maxCapacity = rand() % 1024 * 256 + 65535;
	puts("-- LIST TEST");

	create(maxCapacity / 3, maxCapacity);
	ensureCapacity(maxCapacity / 2);
	add(maxCapacity);
	removeAt();
	compareLists();
	destroy();

	stats_info(&stats);
	return stats_status(&stats);
}

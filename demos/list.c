#include <xt/file.h>
#include <xt/list.h>
#include <xt/os.h>
#include <xt/os_macros.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
// do not corrupt themselfs
static size_t cntHD = 0, cntD = 0, cntU = 0, cntLU = 0, cntZU = 0, cntP = 0;
static short *arrayHD;
static int *arrayD;
static unsigned *arrayU;
static unsigned long *arrayLU;
static size_t *arrayZU;
static void **arrayP;

void add(unsigned times)
{
	xtConsoleFillLine("-");
	printf("Adding %u random elements of data to all lists\n", times);
	for (unsigned i = 0; i < times; ++i, ++cntHD) {
		short valHD = rand();
		assert(xtListHDAdd(&listHD, valHD) == 0);
		arrayHD[i] = valHD;
	}
	for (unsigned i = 0; i < times; ++i, ++cntD) {
		int valD = rand();
		assert(xtListDAdd(&listD, valD) == 0);
		arrayD[i] = valD;
	}
	for (unsigned i = 0; i < times; ++i, ++cntU) {
		unsigned valU = rand();
		assert(xtListUAdd(&listU, valU) == 0);
		arrayU[i] = valU;
	}
	for (unsigned i = 0; i < times; ++i, ++cntLU) {
		unsigned long valLU = rand();
		assert(xtListLUAdd(&listLU, valLU) == 0);
		arrayLU[i] = valLU;
	}
	for (unsigned i = 0; i < times; ++i, ++cntZU) {
		size_t valZU = rand();
		assert(xtListZUAdd(&listZU, valZU) == 0);
		arrayZU[i] = valZU;
	}
	for (unsigned i = 0; i < times; ++i, ++cntP) {
		void *valP = (void*) (PTR_CAST) rand();
		assert(xtListPAdd(&listP, valP) == 0);
		arrayP[i] = valP;
	}
	
	puts("Test passed");
}

void compareLists(void)
{
	xtConsoleFillLine("-");
	puts("Comparing lists with internal arrays to see if no corruption has occurred");
	assert(cntHD == xtListHDGetCount(&listHD));
	assert(cntD == xtListDGetCount(&listD));
	assert(cntU == xtListUGetCount(&listU));
	assert(cntLU == xtListLUGetCount(&listLU));
	assert(cntZU == xtListZUGetCount(&listZU));
	assert(cntP == xtListPGetCount(&listP));
	
	short valHD;
	int valD;
	unsigned valU;
	unsigned long valLU;
	size_t valZU;
	void *valP;
	for (size_t i = 0; i < cntHD; ++i)
		assert(xtListHDGet(&listHD, i, &valHD) == 0 && arrayHD[i] == valHD);
	for (size_t i = 0; i < cntD; ++i)
		assert(xtListDGet(&listD, i, &valD) == 0 && arrayD[i] == valD);
	for (size_t i = 0; i < cntU; ++i)
		assert(xtListUGet(&listU, i, &valU) == 0 && arrayU[i] == valU);
	for (size_t i = 0; i < cntLU; ++i)
		assert(xtListLUGet(&listLU, i, &valLU) == 0 && arrayLU[i] == valLU);
	for (size_t i = 0; i < cntZU; ++i)
		assert(xtListZUGet(&listZU, i, &valZU) == 0 && arrayZU[i] == valZU);
	for (size_t i = 0; i < cntP; ++i)
		assert(xtListPGet(&listP, i, &valP) == 0 && arrayP[i] == valP);
		
	puts("Everything is alright!");
}

void create(size_t initialCapacity, size_t maxCapacity)
{
	puts("Initializing all lists");
	assert(xtListHDCreate(&listHD, initialCapacity) == 0);
	assert(xtListDCreate(&listD, initialCapacity) == 0);
	assert(xtListUCreate(&listU, initialCapacity) == 0);
	assert(xtListLUCreate(&listLU, initialCapacity) == 0);
	assert(xtListZUCreate(&listZU, initialCapacity) == 0);
	assert(xtListPCreate(&listP, initialCapacity) == 0);
	
	assert(arrayHD = malloc(maxCapacity * sizeof(*arrayHD)));
	assert(arrayD = malloc(maxCapacity * sizeof(*arrayD)));
	assert(arrayU = malloc(maxCapacity * sizeof(*arrayU)));
	assert(arrayLU = malloc(maxCapacity * sizeof(*arrayLU)));
	assert(arrayZU = malloc(maxCapacity * sizeof(*arrayZU)));
	assert(arrayP = malloc(maxCapacity * sizeof(*arrayP)));
	
	puts("Initialization succeeded");
}

void destroy(void)
{
	xtConsoleFillLine("-");
	puts("Clearing and destroying all lists");
	xtListHDClear(&listHD);
	xtListHDDestroy(&listHD);
	
	xtListDClear(&listD);
	xtListDDestroy(&listD);
	
	xtListUClear(&listU);
	xtListUDestroy(&listU);
	
	xtListLUClear(&listLU);
	xtListLUDestroy(&listLU);
	
	xtListZUClear(&listZU);
	xtListZUDestroy(&listZU);
	
	xtListPClear(&listP);
	xtListPDestroy(&listP);
	
	free(arrayHD);
	free(arrayD);
	free(arrayU);
	free(arrayLU);
	free(arrayZU);
	free(arrayP);
	
	puts("All lists have been destroyed");
}

void ensureCapacity(size_t newCapacity)
{
	int ret;
	xtConsoleFillLine("-");
	puts("Growth test - All lists will be enlarged");
	
	ret = xtListHDEnsureCapacity(&listHD, newCapacity);
	assert(ret == 0 && xtListHDGetCapacity(&listHD) >= newCapacity);
	
	ret = xtListDEnsureCapacity(&listD, newCapacity);
	assert(ret == 0 && xtListDGetCapacity(&listD) >= newCapacity);
	
	ret = xtListUEnsureCapacity(&listU, newCapacity);
	assert(ret == 0 && xtListUGetCapacity(&listU) >= newCapacity);
	
	ret = xtListLUEnsureCapacity(&listLU, newCapacity);
	assert(ret == 0 && xtListLUGetCapacity(&listLU) >= newCapacity);
	
	ret = xtListZUEnsureCapacity(&listZU, newCapacity);
	assert(ret == 0 && xtListZUGetCapacity(&listZU) >= newCapacity);
	
	ret = xtListPEnsureCapacity(&listP, newCapacity);
	assert(ret == 0 && xtListPGetCapacity(&listP) >= newCapacity);
	
	puts("Growth test passed");
}

void removeAt(void)
{
	xtConsoleFillLine("-");
	puts("Removing elements");
	int index = 2;
	assert(xtListHDRemoveAt(&listHD, index) == 0);
	assert(xtListDRemoveAt(&listD, index) == 0);
	assert(xtListURemoveAt(&listU, index) == 0);
	assert(xtListLURemoveAt(&listLU, index) == 0);
	assert(xtListZURemoveAt(&listZU, index) == 0);
	assert(xtListPRemoveAt(&listP, index) == 0);
	
	memmove(&arrayHD[index], &arrayHD[index + 1], (cntHD - index - 1) * sizeof(*arrayHD));
	memmove(&arrayD[index], &arrayD[index + 1], (cntD - index - 1) * sizeof(*arrayD));
	memmove(&arrayU[index], &arrayU[index + 1], (cntU - index - 1) * sizeof(*arrayU));
	memmove(&arrayLU[index], &arrayLU[index + 1], (cntLU - index - 1) * sizeof(*arrayLU));
	memmove(&arrayZU[index], &arrayZU[index + 1], (cntZU - index - 1) * sizeof(*arrayZU));
	memmove(&arrayP[index], &arrayP[index + 1], (cntP - index - 1) * sizeof(*arrayP));
	
	--cntHD;
	--cntD;
	--cntU;
	--cntLU;
	--cntZU;
	--cntP;
	
	puts("Removing elements completed");
}

int main(void)
{
	srand(time(NULL));
	size_t maxCapacity = rand() % 1024 * 256 + 65535;
	xtConsoleFillLine("-");
	puts("-- LIST TEST");
	
	create(maxCapacity / 3, maxCapacity);
	ensureCapacity(maxCapacity / 2);
	add(maxCapacity);
	removeAt();
	compareLists();
	destroy();
	
	xtConsoleFillLine("-");
	puts("All tests have been completed!");
	return EXIT_SUCCESS;
}

#include <xt/error.h>
#include <xt/hashmap.h>
#include <xt/os.h>
#include <xt/time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

size_t _keyHash(const void *key)
{
	return *((size_t*) key);
}

bool _keyCompare(const void *key1, const void *key2)
{
	return *((size_t*) key1) == *((size_t*) key2);
}

#define HASHMAP_SIZE 256
#define TEST_SIZE (2*HASHMAP_SIZE)

int main(void)
{
	int ret = 0;
	struct xtHashmap map;
	xtConsoleFillLine("-");
	puts("-- HASHMAP TEST");
	ret = xtHashmapCreate(&map, HASHMAP_SIZE, _keyHash, _keyCompare);
	if (ret) {
		fputs("Create failed\n", stderr);
		return 1;
	}
	xtHashmapSetFlags(&map, XT_HASHMAP_FREE_KEY);
	size_t j = 0;
	void *key, *val;
	printf("Adding %d items\n", TEST_SIZE);
	for (size_t i = 0; i < TEST_SIZE; ++i) {
		size_t *v = malloc(sizeof(size_t));
		if (!v) {
			fputs("Out of memory\n", stderr);
			goto fail;
		}
		*v = i;
		int ret;
		if ((ret = xtHashmapAdd(&map, v, NULL))  != 0) {
			fprintf(stderr, "Failed to add `%zu': %s\n", i, xtGetErrorStr(ret));
			goto fail;
		}
		if (i > HASHMAP_SIZE) {
			if (i == HASHMAP_SIZE + 1)
				puts("Iterating hashmap while adding new data");
			if (!xtHashmapForeach(&map, &key, &val)) {
				fprintf(stderr, "Broken iterator state: (%p,%p)\n", key, val);
				goto fail;
			}
			++j;
			if (*(size_t*)key != j) {
				fprintf(stderr, "Value: %zu, (expected: %zu)\n", *(size_t*)key, j);
				goto fail;
			}
		}
		if ((i + 1) != xtHashmapGetCount(&map)) {
			fprintf(stderr, "Wrong size: %zu (expected: %zu)\n", xtHashmapGetCount(&map), i + 1);
			goto fail;
		}
	}
	puts("Done adding items");
	printf("Count: %zu\n", xtHashmapGetCount(&map));
	xtHashmapForeachEnd(&map);
	xtConsoleFillLine("-");
	puts("-- LOOP TEST");
	for (j = 0; xtHashmapForeach(&map, &key, &val); ++j)
		if (j != *(size_t*)key) {
			fprintf(stderr, "Value: %zu, (expected: %zu)\n", *(size_t*)key, j);
			goto fail;
		}
	if (j != TEST_SIZE) {
		fprintf(stderr, "Wrong size: %zu (expected: %d)\n", j, TEST_SIZE);
		goto fail;
	}
	puts("Looping is OK");
	ret = 0;
	xtConsoleFillLine("-");
	puts("All tests have been completed!");
fail:
	xtHashmapDestroy(&map);
	return ret;
}

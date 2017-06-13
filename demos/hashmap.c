#include <xt/error.h>
#include <xt/hashmap.h>
#include <xt/os.h>
#include <xt/string.h>
#include <xt/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static struct stats stats;

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
	char buf[256];
	stats_init(&stats, "string");
	puts("-- HASHMAP TEST");
	ret = xtHashmapCreate(&map, HASHMAP_SIZE, _keyHash, _keyCompare);
	if (ret) {
		FAIL("xtHashmapCreate()");
		goto end;
	}
	PASS("xtHashmapCreate()");
	xtHashmapSetFlags(&map, XT_HASHMAP_FREE_KEY);
	size_t j = 0;
	void *key, *val;
	xtprintf("Adding %d items\n", TEST_SIZE);
	for (size_t i = 0; i < TEST_SIZE; ++i) {
		size_t *v = malloc(sizeof(size_t));
		if (!v) {
			fputs("Out of memory\n", stderr);
			goto fail;
		}
		*v = i;
		int ret;
		if ((ret = xtHashmapAdd(&map, v, NULL))  != 0) {
			xtsnprintf(buf, sizeof buf, "xtHashmapForeach() - Failed to add `%zu': %s\n", i, xtGetErrorStr(ret));
			FAIL(buf);
			goto fail;
		}
		if (i > HASHMAP_SIZE) {
			if (i == HASHMAP_SIZE + 1)
				puts("Iterating hashmap while adding new data");
			if (!xtHashmapForeach(&map, &key, &val)) {
				xtsnprintf(buf, sizeof buf, "xtHashmapForeach() - Broken iterator state: (%p,%p)\n", key, val);
				FAIL(buf);
				goto fail;
			}
			++j;
			if (*(size_t*)key != j) {
				xtsnprintf(buf, sizeof buf, "xtHashmapForeach() - Value: %zu, (expected: %zu)\n", *(size_t*)key, j);
				FAIL(buf);
				goto fail;
			}
		}
		if ((i + 1) != xtHashmapGetCount(&map)) {
			xtsnprintf(buf, sizeof buf, "xtHashmapGetCount() - Wrong size: %zu (expected: %zu)\n", xtHashmapGetCount(&map), i + 1);
			FAIL(buf);
			goto fail;
		}
	}
	PASS("xtHashmapGetCount()");
	puts("Done adding items");
	xtprintf("Count: %zu\n", xtHashmapGetCount(&map));
	xtHashmapForeachEnd(&map);
	for (j = 0; xtHashmapForeach(&map, &key, &val); ++j)
		if (j != *(size_t*)key) {
			xtsnprintf(buf, sizeof buf, "xtHashmapForeach() - Value: %zu, (expected: %zu)\n", *(size_t*)key, j);
			FAIL(buf);
			goto fail;
		}
	if (j != TEST_SIZE) {
		xtsnprintf(buf, sizeof buf, "xtHashmapForeach() - Wrong size: %zu (expected: %d)\n", j, TEST_SIZE);
		FAIL(buf);
		goto fail;
	}
	PASS("xtHashmapForeach()");
	ret = 0;
fail:
	xtHashmapDestroy(&map);
end:
	stats_info(&stats);
	return stats_status(&stats);
}

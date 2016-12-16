// XT headers
#include <xt/hashmap.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>
#include <string.h>

static void xtHashmapDeleteBucket(struct xtHashBucket *bucket)
{
	free(bucket->value);
	free(bucket->key);
	free(bucket);
}

bool xtHashmapCreate(struct xtHashmap *map, size_t size, size_t keySize, bool (*keyCompare) (void*, void*), size_t (*keyHash) (void*))
{
	map->buckets = (struct xtHashBucket**) malloc(size * sizeof(struct xtHashBucket*));
	if (!map->buckets)
		return false;
	map->size = size;
	map->count = 0;
	for (size_t i = 0; i < size; ++i)
		map->buckets[i] = NULL;
	map->keyCompare = keyCompare;
	map->keyHash = keyHash;
	map->keySize = keySize;
	map->it.entry = NULL;
	map->it.nr = 0;
	return true;
}

void xtHashmapDestroy(struct xtHashmap *map)
{
	if (!map)
		return;
	for (size_t i = 0; i < map->size; ++i) {
		if (map->buckets[i]) {
			for (struct xtHashBucket *next, *b = map->buckets[i]; b; b = next) {
				next = b->next;
				xtHashmapDeleteBucket(b);
			}
		}
	}
	free(map->buckets);
}

struct xtHashBucket *xtHashmapGet(struct xtHashmap *map, void *key)
{
	size_t hash;
	hash = map->keyHash(key);
	struct xtHashBucket *b = map->buckets[hash % map->size];
	if (!b)
		return NULL;
	while (b) {
		if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key))
			return b;
		b = b->next;
	}
	return NULL;
}

size_t xtHashmapGetCount(const struct xtHashmap *map)
{
	return map->count;
}

size_t xtHashmapGetSize(const struct xtHashmap *map)
{
	return map->size;
}

void *xtHashmapGetValue(struct xtHashmap *map, void *key)
{
	struct xtHashBucket *b = xtHashmapGet(map, key);
	return b ? b->value : NULL;
}

static bool xtHashmapIteratorNext(struct xtHashmap *map, void **key, void **value)
{
	// if more entries left
	if (map->it.entry) {
		if (key)
			*key = map->it.entry->key;
		if (value)
			*value = map->it.entry->value;
		map->it.entry = map->it.entry->next;
		return true;
	}
	// look for next bucket
	for (++map->it.nr; map->it.nr < map->size; ++map->it.nr) {
		if (map->buckets[map->it.nr]) {
			struct xtHashBucket *b = map->buckets[map->it.nr];
			if (key)
				*key = b->key;
			if (value)
				*value = b->value;
			map->it.entry = b->next;
			return true;
		}
	}
	return false;
}

static bool xtHashmapIteratorStart(struct xtHashmap *map, void **key, void **value)
{
	for (size_t i = 0; i < map->size; ++i) {
		if (map->buckets[i]) {
			struct xtHashBucket *b;
			b = map->buckets[i];
			map->it.nr = i;
			if (key)
				*key = b->key;
			if (value)
				*value = b->value;
			map->it.entry = b->next;
			return true;
		}
	}
	return false;
}

bool xtHashmapForeach(struct xtHashmap *map, void **key, void **value)
{
	if (!map->it.entry && map->it.nr >= map->size) {
		map->it.nr = 0;
		if (xtHashmapIteratorStart(map, key, value))
			return true;
	} else if (xtHashmapIteratorNext(map, key, value))
		return true;
	map->it.entry = NULL;
	map->it.nr = map->size;
	return false;
}

void xtHashmapForeachEnd(struct xtHashmap *map)
{
	map->it.entry = NULL;
	map->it.nr = map->size;
}

bool xtHashmapPut(struct xtHashmap *map, void *key, void *value)
{
	struct xtHashBucket *entry;
	entry = (struct xtHashBucket*) malloc(sizeof(struct xtHashBucket));
	if (!entry)
		return false;
	entry->key = malloc(map->keySize);
	if (!entry->key)
		return false;
	memcpy(entry->key, key, map->keySize);
	entry->value = value;
	size_t hash = map->keyHash(key);
	struct xtHashBucket *b = map->buckets[hash % map->size];
	entry->next = NULL;
	if (!b)
		map->buckets[hash % map->size] = entry;
	else {
		while (b) {
			if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key))
				// oops, already exists
				return false;
			if (b->next)
				b = b->next;
			else
				break;
		}
		b->next = entry;
	}
	++map->count;
	return true;
}

bool xtHashmapRemove(struct xtHashmap *map, void *key)
{
	size_t hash;
	hash = map->keyHash(key);
	struct xtHashBucket *b = map->buckets[hash % map->size];
	if (!b)
		return false;
	struct xtHashBucket *prev = b;
	do {
		if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key)) {
			if (prev != b)
				prev->next = b->next;
			else
				map->buckets[hash % map->size] = b->next;
			xtHashmapDeleteBucket(b);
			--map->count;
			return true;
		}
		prev = b;
		b = b->next;
	} while (b);
	return false;
}

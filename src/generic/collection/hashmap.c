// XT headers
#include <xt/hashmap.h>
#include <xt/error.h>

// STD headers
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xtHashmapAdd(struct xtHashmap *map, void *key, void *value)
{
	if (map->count >= map->capacity * map->grow_limit) {
		// Do not grow if growth factor is less than 100%
		if (map->grow < 1.0001f)
			return XT_ENOBUFS;
		int ret = xtHashmapSetCapacity(map, map->capacity * map->grow);
		if (ret)
			return ret;
	}
	size_t hash;
	struct xtHashBucket *bucket, *entry;
	entry = malloc(sizeof *entry);
	if (!entry)
		return XT_ENOMEM;
	entry->key = key;
	entry->value = value;
	entry->next = NULL;
	hash = map->keyHash(key);
	bucket = map->buckets[hash % map->capacity];
	if (!bucket)
		map->buckets[hash % map->capacity] = entry;
	else {
		while (bucket) {
			if (hash == map->keyHash(bucket->key) && map->keyCompare(bucket->key, key))
				return XT_EEXIST;
			if (bucket->next)
				bucket = bucket->next;
			else
				break;
		}
		bucket->next = entry;
	}
	++map->count;
	return 0;
}

int xtHashmapCreate(
	struct xtHashmap *map, size_t capacity,
	size_t (*keyHash)(const void*),
	bool (*keyCompare)(const void*, const void*)
)
{
	map->buckets = malloc(capacity * sizeof *map->buckets);
	if (!map->buckets)
		return XT_ENOMEM;
	if (capacity == 0)
		capacity = XT_HASHMAP_CAPACITY_DEFAULT;
	map->capacity = capacity;
	map->count = 0;
	for (size_t i = 0; i < capacity; ++i)
		map->buckets[i] = NULL;
	map->keyHash = keyHash;
	map->keyCompare = keyCompare;
	map->it.entry = NULL;
	map->it.nr = 0;
	map->grow_limit = XT_HASHMAP_GROWTH_LIMIT_DEFAULT;
	map->grow = XT_HASHMAP_GROWTH_FACTOR_DEFAULT;
	map->flags = 0;
	return 0;
}

static void hashmap_delete_bucket(struct xtHashmap *map, struct xtHashBucket *bucket)
{
	unsigned flags = map->flags;
	if (flags & XT_HASHMAP_FREE_VALUE)
		free(bucket->value);
	if (flags & XT_HASHMAP_FREE_KEY)
		free(bucket->key);
	free(bucket);
}

void xtHashmapDestroy(struct xtHashmap *map)
{
	size_t capacity = map->capacity;
	if (!map->buckets)
		return;
	for (size_t i = 0; i < capacity; ++i)
		if (map->buckets[i])
			for (struct xtHashBucket *next, *b = map->buckets[i]; b; b = next) {
				next = b->next;
				hashmap_delete_bucket(map, b);
			}
	free(map->buckets);
	map->buckets = NULL;
}

int xtHashmapGet(const struct xtHashmap *map, const void *key, struct xtHashBucket **bucket)
{
	size_t hash;
	struct xtHashBucket *b;
	hash = map->keyHash(key);
	b = map->buckets[hash % map->capacity];
	if (!b)
		return XT_ENOENT;
	while (b) {
		const void *bkey = b->key;
		if (hash == map->keyHash(bkey) && map->keyCompare(bkey, key)) {
			*bucket = b;
			return 0;
		}
		b = b->next;
	}
	return XT_ENOENT;
}

size_t xtHashmapGetCapacity(const struct xtHashmap *map)
{
	return map->capacity;
}

size_t xtHashmapGetCount(const struct xtHashmap *map)
{
	return map->count;
}

unsigned xtHashmapGetFlags(const struct xtHashmap *map)
{
	return map->flags;
}

float xtHashmapGetGrowthFactor(const struct xtHashmap *map)
{
	return map->grow;
}

float xtHashmapGetGrowthLimit(const struct xtHashmap *map)
{
	return map->grow_limit;
}

int xtHashmapGetValue(const struct xtHashmap *map, const void *key, void **value)
{
	struct xtHashBucket *b;
	int ret = xtHashmapGet(map, key, &b);
	if (ret != 0)
		return ret;
	*value = b->value;
	return 0;
}

static bool hashmap_iterator_next(struct xtHashmap *map, struct xtHashmapIterator *it, void **key, void **value)
{
	/*
	 * Iterate through all entries in one bucket
	 * before looking for the next one.
	 */
	if (it->entry) {
		if (key)
			*key = it->entry->key;
		if (value)
			*value = it->entry->value;
		it->entry = it->entry->next;
		return true;
	}
	for (++it->nr; it->nr < map->capacity; ++it->nr)
		if (map->buckets[it->nr]) {
			struct xtHashBucket *b = map->buckets[it->nr];
			if (key)
				*key = b->key;
			if (value)
				*value = b->value;
			it->entry = b->next;
			return true;
		}
	return false;
}

static bool xtHashmapIteratorNext(struct xtHashmap *map, void **key, void **value)
{
	return hashmap_iterator_next(map, &map->it, key, value);
}

static bool hashmap_iterator_start(struct xtHashmap *map, struct xtHashmapIterator *it, void **key, void **value)
{
	for (size_t i = 0; i < map->capacity; ++i)
		if (map->buckets[i]) {
			struct xtHashBucket *b;
			b = map->buckets[i];
			it->nr = i;
			if (key)
				*key = b->key;
			if (value)
				*value = b->value;
			it->entry = b->next;
			return true;
		}
	return false;
}

bool xtHashmapForeach(struct xtHashmap *map, void **key, void **value)
{
	if (!map->it.entry && map->it.nr >= map->capacity) {
		map->it.nr = 0;
		if (hashmap_iterator_start(map, &map->it, key, value))
			return true;
	} else if (xtHashmapIteratorNext(map, key, value))
		return true;
	map->it.entry = NULL;
	map->it.nr = map->capacity;
	return false;
}

void xtHashmapForeachEnd(struct xtHashmap *map)
{
	map->it.entry = NULL;
	map->it.nr = map->capacity;
}

int xtHashmapRemove(struct xtHashmap *map, void *key)
{
	size_t hash;
	hash = map->keyHash(key);
	struct xtHashBucket *prev, *b = map->buckets[hash % map->capacity];
	if (!b)
		return XT_ENOENT;
	prev = b;
	do {
		if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key)) {
			if (prev != b)
				prev->next = b->next;
			else
				map->buckets[hash % map->capacity] = b->next;
			hashmap_delete_bucket(map, b);
			--map->count;
			return 0;
		}
		prev = b;
		b = b->next;
	} while (b);
	return XT_ENOENT;
}

/* Create internal copy and inherit all settings from old hashmap */
static int hashmap_copy(struct xtHashmap *new, const struct xtHashmap *orig, size_t size)
{
	int ret = xtHashmapCreate(new, size, orig->keyHash, orig->keyCompare);
	if (ret)
		return ret;
	xtHashmapSetGrowthFactor(new, xtHashmapGetGrowthFactor(orig));
	xtHashmapSetGrowthLimit(new, xtHashmapGetGrowthLimit(orig));
	xtHashmapSetFlags(new, xtHashmapGetFlags(orig));
	return 0;
}

int xtHashmapSetCapacity(struct xtHashmap *map, size_t capacity)
{
	if (capacity < 1)
		return XT_EINVAL;
	// Always grow by at least one element
	if (capacity < map->capacity)
		capacity = map->capacity + 1;
	// Save old iterator state
	struct xtHashBucket *obuck = map->it.nr >= map->capacity ? NULL : map->buckets[map->it.nr];
	// NOTE obuck->key may be NULL even if obuck is not NULL
	void *okey = obuck ? obuck->key : NULL;
	// Create a completely new hashmap and deep copy all data
	struct xtHashmap new;
	int ret = hashmap_copy(&new, map, capacity);
	if (ret)
		return ret;
	void *nkey, *nval;
	struct xtHashmapIterator it;
	if (hashmap_iterator_start(map, &it, &nkey, &nval))
		do {
			if ((ret = xtHashmapAdd(&new, nkey, nval)) != 0) {
				xtHashmapDestroy(&new);
				return ret;
			}
		} while (hashmap_iterator_next(map, &it, &nkey, &nval));
	else
		abort();
	// Clean up old hashmap and assign new one
	// Make sure the old hashmap does not free its items
	xtHashmapSetFlags(map, xtHashmapGetFlags(map) & ~XT_HASHMAP_FREE_ITEM);
	xtHashmapDestroy(map);
	// Restore iterator state
	new.it.entry = NULL;
	new.it.nr = capacity;
	if (obuck) {
		while (xtHashmapForeach(&new, &nkey, NULL))
			if (okey == nkey)
				goto assign;
		// New hashmap got corrupted
		abort();
	}
assign:
	*map = new;
	return 0;
}

void xtHashmapSetFlags(struct xtHashmap *map, unsigned flags)
{
	map->flags = flags;
}

void xtHashmapSetGrowthFactor(struct xtHashmap *map, float growthFactor)
{
	map->grow = growthFactor;
}

void xtHashmapSetGrowthLimit(struct xtHashmap *map, float growthLimit)
{
	map->grow_limit = growthLimit;
}

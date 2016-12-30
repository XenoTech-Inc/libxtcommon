// XT headers
#include <xt/hashmap.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>
#include <string.h>

static void _xtHashmapDeleteBucket(struct xtHashBucket *bucket)
{
	//free(bucket->value);
	//free(bucket->key);
	free(bucket);
}

int xtHashmapAdd(struct xtHashmap *map, void *key, void *value)
{
	struct xtHashBucket *entry;
	entry = (struct xtHashBucket*) malloc(sizeof(struct xtHashBucket));
	if (!entry)
		return XT_ENOMEM;
	entry->key = key;
	entry->value = value;
	size_t hash = map->keyHash(key);
	struct xtHashBucket *b = map->buckets[hash % map->capacity];
	entry->next = NULL;
	if (!b)
		map->buckets[hash % map->capacity] = entry;
	else {
		while (b) {
			if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key))
				// oops, already exists
				return XT_EEXIST;
			if (b->next)
				b = b->next;
			else
				break;
		}
		b->next = entry;
	}
	++map->count;
	return 0;
}

int xtHashmapCreate(
	struct xtHashmap *map, size_t capacity,
	size_t (*keyHash) (const void*), bool (*keyCompare) (const void*, const void*)
)
{
	map->buckets = malloc(capacity * sizeof(struct xtHashBucket*));
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
	return 0;
}

void xtHashmapDestroy(struct xtHashmap *map)
{
	size_t capacity = map->capacity;
	for (size_t i = 0; i < capacity; ++i) {
		if (map->buckets[i]) {
			for (struct xtHashBucket *next, *b = map->buckets[i]; b; b = next) {
				next = b->next;
				_xtHashmapDeleteBucket(b);
			}
		}
	}
	free(map->buckets);
}

int xtHashmapGet(const struct xtHashmap *map, const void *key, struct xtHashBucket **bucket)
{
	size_t hash;
	hash = map->keyHash(key);
	struct xtHashBucket *b = map->buckets[hash % map->capacity];
	if (!b)
		return XT_ENOENT;
	while (b) {
		if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key)) {
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

int xtHashmapGetValue(const struct xtHashmap *map, const void *key, void **value)
{
	struct xtHashBucket *b;
	int ret = xtHashmapGet(map, key, &b);
	if (ret != 0)
		return ret;
	*value = b->value;
	return 0;
}

static bool xtHashmapIteratorNext(struct xtHashmap *map, void **key, void **value)
{
	// If more entries left
	if (map->it.entry) {
		if (key)
			*key = map->it.entry->key;
		if (value)
			*value = map->it.entry->value;
		map->it.entry = map->it.entry->next;
		return true;
	}
	// Look for next bucket
	for (++map->it.nr; map->it.nr < map->capacity; ++map->it.nr) {
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

static bool _xtHashmapIteratorStart(struct xtHashmap *map, void **key, void **value)
{
	for (size_t i = 0; i < map->capacity; ++i) {
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
	if (!map->it.entry && map->it.nr >= map->capacity) {
		map->it.nr = 0;
		if (_xtHashmapIteratorStart(map, key, value))
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
	struct xtHashBucket *b = map->buckets[hash % map->capacity];
	if (!b)
		return XT_ENOENT;
	struct xtHashBucket *prev = b;
	do {
		if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key)) {
			if (prev != b)
				prev->next = b->next;
			else
				map->buckets[hash % map->capacity] = b->next;
			_xtHashmapDeleteBucket(b);
			--map->count;
			return 0;
		}
		prev = b;
		b = b->next;
	} while (b);
	return XT_ENOENT;
}

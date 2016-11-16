// XT headers
#include <xt/collection.h>
#include <xt/error.h>
#include <xt/math.h>

// STD headers
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void xtHashmapDeleteBucket(struct xtHashBucket *bucket)
{
	free(bucket->value);
	free(bucket->key);
	free(bucket);
}

bool xtHashmapCreate(xtHashmap *map, size_t size, size_t keySize, bool (*keyCompare) (void*, void*), size_t (*keyHash) (void*))
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

void xtHashmapDestroy(xtHashmap *map)
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

struct xtHashBucket *xtHashmapGet(xtHashmap *map, void *key)
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

size_t xtHashmapGetCount(const xtHashmap *map)
{
	return map->count;
}

size_t xtHashmapGetSize(const xtHashmap *map)
{
	return map->size;
}

void *xtHashmapGetValue(xtHashmap *map, void *key)
{
	struct xtHashBucket *b = xtHashmapGet(map, key);
	return b ? b->value : NULL;
}

static bool xtHashmapIteratorNext(xtHashmap *map, void **key, void **value)
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

static bool xtHashmapIteratorStart(xtHashmap *map, void **key, void **value)
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

bool xtHashmapForeach(xtHashmap *map, void **key, void **value)
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

void xtHashmapForeachEnd(xtHashmap *map)
{
	map->it.entry = NULL;
	map->it.nr = map->size;
}

bool xtHashmapPut(xtHashmap *map, void *key, void *value)
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

bool xtHashmapRemove(xtHashmap *map, void *key)
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

int xtListAdd(xtList *list, void *data)
{
	if (list->count == list->capacity) {
		if (!list->canGrow)
			return XT_ENOBUFS;
		// Try to grow
		int ret = xtListSetCapacity(list, xtListGetCapacity(list));
		if (ret != 0)
			return ret;
		// Success!
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListAddAt(xtList *list, void *data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL; // Can only replace elements
	else if (list->count == list->capacity)
		return XT_ENOBUFS;
	if (list->destroyElementFunc)
		list->destroyElementFunc(list, list->data[index]);
	list->data[index] = data;
	// Do not increase count! An element has been replaced
	return 0;
}

void xtListClear(xtList *list)
{
	// Remove elements at the end, this is faster
	while (list->count > 0)
		xtListRemoveAt(list, list->count - 1);
}

int xtListCreate(xtList *list, size_t capacity)
{
	if (capacity == 0)
		capacity = 1024;
	void **data = (void**) malloc(capacity * sizeof(capacity));
	if (!data)
		return XT_ENOMEM;
	for (size_t i = 0; i < capacity; ++i)
		data[i] = NULL;
	list->data = data;
	list->capacity = capacity;
	list->count = 0;
	list->canGrow = true;
	list->destroyElementFunc = NULL;
	list->destroyListFunc = NULL;
	return 0;
}

void xtListDestroy(xtList *list)
{
	xtListClear(list);
	if (list->destroyListFunc)
		list->destroyListFunc(list);
	free(list->data);
}

void xtListEnableGrowth(xtList *list, bool flag)
{
	list->canGrow = flag;
}

int xtListEnsureCapacity(xtList *list, size_t minCapacity)
{
	if (list->capacity >= minCapacity)
		return 0;
	return xtListSetCapacity(list, xtListGetCapacity(list) - minCapacity);
}

int xtListGet(const xtList *list, size_t index, void **data)
{
	if (index >= list->count)
		return XT_EINVAL;
	*data = list->data[index];
	return 0;
}

size_t xtListGetCapacity(const xtList *list)
{
	return list->capacity;
}

size_t xtListGetCount(const xtList *list)
{
	return list->count;
}

int xtListRemove(xtList *list, void *data)
{
	for (size_t i = 0; i < list->count; ++i) {
		if (list->data[i] == data) {
			xtListRemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListRemoveAt(xtList *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	if (list->destroyElementFunc)
		list->destroyElementFunc(list, list->data[index]);
	// If this is the last element, then there is no need to shift.
	// This trick greatly improves performance when destroying the list from the end!
	if (index == list->count - 1)
		list->data[index] = NULL;
	else
		memmove(&list->data[index], &list->data[index + 1], (list->capacity - index - 1) * sizeof(void*));
	--list->count;
	return 0;
}

int xtListSetCapacity(xtList *list, size_t n)
{
	void **temp;
	if (!(temp = (void**) realloc(list->data, ((list->capacity) + n) * sizeof(list->data))))
		return XT_ENOMEM;
	list->data = temp;
	// Clear all new memory
	for (size_t i = 1; i < n; ++i)
		list->data[list->capacity + i] = NULL;
	list->capacity += n;
	return 0;
}

void xtListSetElementDestroyFunc(xtList *list, void (*destroyElementFunc) (xtList *list, void *data))
{
	list->destroyElementFunc = destroyElementFunc;
}

void xtListSetListDestroyFunc(xtList *list, void (*destroyListFunc) (xtList *list))
{
	list->destroyListFunc = destroyListFunc;
}

// XT headers
#include <xt/collection.h>
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
			struct xtHashBucket *b = map->buckets[i];
			while (b) {
				struct xtHashBucket *next;
				next = b->next;
				xtHashmapDeleteBucket(b);
				b = next;
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
	if (!b) {
		map->buckets[hash % map->size] = entry;
	} else {
		while (b) {
			if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key)) {
				// oops, already exists
				return false;
			}
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
	if (!b) {
		return false;
	}
	struct xtHashBucket *prev = b;
	do {
		if (hash == map->keyHash(b->key) && map->keyCompare(b->key, key)) {
			if (prev != b) {
				prev->next = b->next;
			} else {
				map->buckets[hash % map->size] = b->next;
			}
			xtHashmapDeleteBucket(b);
			--map->count;
			return true;
		}
		prev = b;
		b = b->next;
	} while (b);
	return false;
}
/**
 * Grows the list by the specified amount. 
 * If this operation succeeds, all new elements are set to null.
 */
static bool _xtListGrow(xtList *list, size_t n)
{
	void **temp;
	if (!(temp = (void**) realloc(list->data, ((list->capacity) + n) * sizeof(list->data))))
		return false;
	list->data = temp;
	for (size_t i = 1; i < n; ++i) {
		list->data[list->capacity + i] = NULL;
	}
	list->lastFreeIndex = list->capacity;
	list->capacity += n;
	return true;
}

static bool _xtListFindFreeSpot(xtList *list)
{
	if (list->count == list->capacity)
		return false;
	// First try to look from the last free spot, it is possible that the list is empty so don't do +1 on the index
	for (size_t i = list->lastFreeIndex; i < list->capacity; i++) {
		if (list->data[i])
			continue;
		list->lastFreeIndex = i;
		return true;
	}
	// Nothing found? then try to search from the beginning
	for (size_t i = 0; i < list->capacity; i++) {
		if (list->data[i])
			continue;
		list->lastFreeIndex = i;
		return true;
	}
	return false;
}
/*
 * Shifts all elements past the index to the index. No index checking or anything is performed.
 */
static void _xtListShift(xtList *list, size_t index)
{
	memmove(&list->data[index], &list->data[index + 1], (list->capacity - index - 1) * sizeof(void*));
}

bool xtListAdd(xtList *list, void *v)
{
	if (list->count == list->capacity) {
		if (!list->canGrow)
			return false; // Can't grow? Return false
		// If it's a number of two already, add one
		if (!_xtListGrow(list, xtMathNextPow2((xtMathIsPow2(list->capacity) ? list->capacity + 1 : list->capacity))))
			return false; // Unable to grow
	}
	++list->count;
	list->data[list->lastFreeIndex] = v;
	_xtListFindFreeSpot(list);
	return true;
}

bool xtListAddAt(xtList *list, void *v, size_t index)
{
	if (index > list->capacity)
		return false;
	list->data[index] = v;
	return true;
}

void xtListClear(xtList *list)
{
	// Clear the whole list. It might be fragmented, we cannot know.
	for (size_t i = 0; i < xtListGetCapacity(list); ++i)
		list->data[i] = NULL;
	// Set this manually
	list->count = 0;
	list->lastFreeIndex = 0;
}

bool xtListCreate(xtList *list, size_t capacity)
{
	if (capacity < XT_LIST_CAPACITY_MIN)
		capacity = XT_LIST_CAPACITY_MIN;
	list->data = (void**) malloc(capacity * sizeof(list->data));
	if (!list->data)
		return false;
	list->canGrow = false;
	list->capacity = capacity;
	list->count = 0;
	list->lastFreeIndex = 0;
	list->shiftMemory = true;
	for (size_t i = 0; i < capacity; ++i)
		list->data[i] = NULL;
	return true;
}

void xtListDestroy(xtList *list)
{
	if (!list)
		return;
	free(list->data);
}

void xtListEnableGrowth(xtList *list, bool flag)
{
	list->canGrow = flag;
}

void xtListEnableShifting(xtList *list, bool flag)
{
	list->shiftMemory = flag;
}

bool xtListEnsureCapacity(xtList *list, size_t minCapacity)
{
	if (minCapacity >= xtListGetCapacity(list))
		return true; // The list already has enough space, no need to grow
	return _xtListGrow(list, xtListGetCapacity(list) - minCapacity);
}

void *xtListGet(const xtList *list, size_t index)
{
	if (index > list->capacity)
		return NULL;
	return list->data[index];
}

size_t xtListGetCapacity(const xtList *list)
{
	return list->capacity;
}

size_t xtListGetCount(const xtList *list)
{
	return list->count;
}

void *xtListRemove(xtList *list, void *element)
{
	for (size_t i = 0; i < xtListGetCapacity(list); ++i) {
		if (xtListGet(list, i) == element)
			return xtListRemoveAt(list, i);
	}
	return NULL;
}

void *xtListRemoveAt(xtList *list, size_t index)
{
	if (index > list->capacity)
		return NULL;
	// Check if there is something in there
	if (!list->data[index])
		return false;
	void *element = list->data[index];
	list->data[index] = NULL;
	--list->count;
	list->lastFreeIndex = index;
	if (list->shiftMemory) {
		// Now shift!
		_xtListShift(list, index);
		// If we shift, find a new free spot
		_xtListFindFreeSpot(list);
	}
	return element;
}

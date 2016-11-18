// XT headers
#include <xt/collection.h>
#include <xt/error.h>
#include <xt/math.h>

// STD headers
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define XT_LIST_CAPACITY_DEFAULT 1024

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
		int ret = xtListGrow(list, xtListGetCapacity(list));
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
		capacity = XT_LIST_CAPACITY_DEFAULT;
	void **data = malloc(capacity * sizeof(void*));
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
	return xtListGrow(list, minCapacity - xtListGetCapacity(list));
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

int xtListGrow(xtList *list, size_t n)
{
	void **temp;
	if (!(temp = realloc(list->data, (list->capacity + n) * sizeof(list->data))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity += n;
	return 0;
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
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(void*));
	--list->count;
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

/* STACK */

#define func_init(type) void type ## Init(struct type *t) { t->data = NULL; }
#define func_set_grow(type) void type ## SetGrow(struct type *t, int grow) { t->grow = grow; }
#define func_get_grow(type) int type ## GetGrow(struct type *t) { return t->grow; }

func_init(xtStackHD)
func_init(xtStackD)
func_init(xtStackU)
func_init(xtStackLU)
func_init(xtStackZU)

func_set_grow(xtStackHD)
func_set_grow(xtStackD)
func_set_grow(xtStackU)
func_set_grow(xtStackLU)
func_set_grow(xtStackZU)

func_get_grow(xtStackHD)
func_get_grow(xtStackD)
func_get_grow(xtStackU)
func_get_grow(xtStackLU)
func_get_grow(xtStackZU)

static inline int xt_stack_create(void **data, size_t elemsize, size_t n)
{
	if (!n)
		n = XT_STACK_CAPACITY_DEFAULT;
	if (!(*data = malloc(n * elemsize)))
		return XT_ENOMEM;
	return 0;
}

#define stack_init(this, data, cap) \
	this->data = data;\
	this->count = 0;\
	this->capacity = cap;\
	this->grow = -2;

int xtStackHDCreate(struct xtStackHD *this, size_t capacity)
{
	void *data;
	int ret = xt_stack_create(&data, sizeof(short), capacity);
	if (ret)
		return ret;
	stack_init(this, data, capacity);
	return 0;
}

int xtStackDCreate(struct xtStackD *this, size_t capacity)
{
	void *data;
	int ret = xt_stack_create(&data, sizeof(int), capacity);
	if (ret)
		return ret;
	stack_init(this, data, capacity)
	return 0;
}

int xtStackUCreate(struct xtStackU *this, size_t capacity)
{
	void *data;
	int ret = xt_stack_create(&data, sizeof(unsigned), capacity);
	if (ret)
		return ret;
	stack_init(this, data, capacity)
	return 0;
}

int xtStackLUCreate(struct xtStackLU *this, size_t capacity)
{
	void *data;
	int ret = xt_stack_create(&data, sizeof(unsigned long), capacity);
	if (ret)
		return ret;
	stack_init(this, data, capacity)
	return 0;
}

int xtStackZUCreate(struct xtStackZU *this, size_t capacity)
{
	void *data;
	int ret = xt_stack_create(&data, sizeof(size_t), capacity);
	if (ret)
		return ret;
	stack_init(this, data, capacity)
	return 0;
}

#define func_free(type) void type ## Destroy(struct type *t) { if (t->data) { free(t->data); t->data = NULL; } }

func_free(xtStackD )
func_free(xtStackHD)
func_free(xtStackU )
func_free(xtStackLU)
func_free(xtStackZU)

int xtStackHDPush(struct xtStackHD *this, short value)
{
	if (this->count != this->capacity)
		goto push;
	if (!this->grow)
		return XT_ENOBUFS;
	size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / -(unsigned)this->grow;
	// grow at least by one
	if (!grow) ++grow;
	short *tmp = realloc(this->data, this->capacity + grow);
	if (!tmp)
		return XT_ENOMEM;
	this->data = tmp;
	this->capacity += grow;
push:
	this->data[this->count++] = value;
	return 0;
}

int xtStackDPush(struct xtStackD *this, int value)
{
	if (this->count != this->capacity)
		goto push;
	if (!this->grow)
		return XT_ENOBUFS;
	size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / -(unsigned)this->grow;
	// grow at least by one
	if (!grow) ++grow;
	int *tmp = realloc(this->data, this->capacity + grow);
	if (!tmp)
		return XT_ENOMEM;
	this->data = tmp;
	this->capacity += grow;
push:
	this->data[this->count++] = value;
	return 0;
}

int xtStackUPush(struct xtStackU *this, unsigned value)
{
	if (this->count != this->capacity)
		goto push;
	if (!this->grow)
		return XT_ENOBUFS;
	size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / -(unsigned)this->grow;
	// grow at least by one
	if (!grow) ++grow;
	unsigned *tmp = realloc(this->data, this->capacity + grow);
	if (!tmp)
		return XT_ENOMEM;
	this->data = tmp;
	this->capacity += grow;
push:
	this->data[this->count++] = value;
	return 0;
}

int xtStackLUPush(struct xtStackLU *this, unsigned long value)
{
	if (this->count != this->capacity)
		goto push;
	if (!this->grow)
		return XT_ENOBUFS;
	size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / -(unsigned)this->grow;
	// grow at least by one
	if (!grow) ++grow;
	unsigned long *tmp = realloc(this->data, this->capacity + grow);
	if (!tmp)
		return XT_ENOMEM;
	this->data = tmp;
	this->capacity += grow;
push:
	this->data[this->count++] = value;
	return 0;
}

int xtStackZUPush(struct xtStackZU *this, size_t value)
{
	if (this->count != this->capacity)
		goto push;
	if (!this->grow)
		return XT_ENOBUFS;
	size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / -(unsigned)this->grow;
	// grow at least by one
	if (!grow) ++grow;
	size_t *tmp = realloc(this->data, this->capacity + grow);
	if (!tmp)
		return XT_ENOMEM;
	this->data = tmp;
	this->capacity += grow;
push:
	this->data[this->count++] = value;
	return 0;
}

bool xtStackHDPeek(struct xtStackHD *this, short *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->count - 1];
	return true;
}

bool xtStackDPeek(struct xtStackD *this, int *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->count - 1];
	return true;
}

bool xtStackUPeek(struct xtStackU *this, unsigned *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->count - 1];
	return true;
}

bool xtStackLUPeek(struct xtStackLU *this, unsigned long *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->count - 1];
	return true;
}

bool xtStackZUPeek(struct xtStackZU *this, size_t *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->count - 1];
	return true;
}

bool xtStackHDPop(struct xtStackHD *this, short *top)
{
	if (!this->count)
		return false;
	*top = this->data[--this->count];
	return true;
}

bool xtStackDPop(struct xtStackD *this, int *top)
{
	if (!this->count)
		return false;
	*top = this->data[--this->count];
	return true;
}

bool xtStackUPop(struct xtStackU *this, unsigned *top)
{
	if (!this->count)
		return false;
	*top = this->data[--this->count];
	return true;
}

bool xtStackLUPop(struct xtStackLU *this, unsigned long *top)
{
	if (!this->count)
		return false;
	*top = this->data[--this->count];
	return true;
}

bool xtStackZUPop(struct xtStackZU *this, size_t *top)
{
	if (!this->count)
		return false;
	*top = this->data[--this->count];
	return true;
}

#define func_drop(type) size_t type ## Drop(struct type *t) { return t->count ? --t->count : 0; }
#define func_get_size(type) size_t type ## GetSize(struct type *t) { return t->count; }
#define func_get_cap(type) size_t type ## GetCapacity(struct type *t) { return t->capacity; }

func_drop(xtStackHD)
func_drop(xtStackD )
func_drop(xtStackU )
func_drop(xtStackLU)
func_drop(xtStackZU)

func_get_size(xtStackHD)
func_get_size(xtStackD)
func_get_size(xtStackU )
func_get_size(xtStackLU)
func_get_size(xtStackZU)

func_get_cap(xtStackHD)
func_get_cap(xtStackD )
func_get_cap(xtStackU )
func_get_cap(xtStackLU)
func_get_cap(xtStackZU)

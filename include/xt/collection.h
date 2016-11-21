/**
 * @brief Contains various utilites like a Hashmap and List.
 * @file collection.h
 * @author Folkert van Verseveld and Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_COLLECTION_H
#define _XT_COLLECTION_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>

#define XT_LIST_CAPACITY_DEFAULT 1024
#define XT_STACK_CAPACITY_DEFAULT 256

/* HASHMAP */

/**
 * @brief An entry in the hashmap.
 *
 * This object contains the next bucket that comes in line, key, and value.
 */
struct xtHashBucket {
	struct xtHashBucket *next;
	void *key;
	void *value;
};
/**
 * @brief The iterator used by the hashmap.
 *
 * This iterator contains a pointer to the current bucket (entry), and the index of it.
 */
struct xtHashmapIterator {
	struct xtHashBucket *entry;
	size_t nr;
};
/**
 * @brief A very easy to use hashmap.
 *
 * This hashmap ONLY works with objects that ARE dynamically allocated. The keys however do not need to be dynamically allocated.
 * This hashmap is NOT thread safe. The hashmap manages almost all memory for you. The key and value are both destroyed if any data is removed.
 */
typedef struct xtHashmap {
	struct xtHashBucket **buckets;
	size_t count, size;
	bool (*keyCompare)(void *key1, void *key2);
	size_t (*keyHash) (void *key);
	size_t keySize;
	struct xtHashmapIterator it;
} xtHashmap;
/**
 * Creates a new hashmap.
 * If this function fails, you must NOT use the hashmap for any purpose. This is unsafe.
 * @param map - A pointer to the hashmap.
 * @param size - How many buckets the hashmap should have.
 * @param keySize - The size of your custom keys. sizeof([Your key type])
 * @param keyCompare - A function pointer to the function which will compare two keys, and return if they are the same.
 * @param keyHash - A function pointer to the function which will create a hash of your custom key type.
 */
bool xtHashmapCreate(xtHashmap *map, size_t size, size_t keySize, bool (*keyCompare) (void*, void*), size_t (*keyHash) (void*));
/**
 * Destroys the hashmap, all memory associated with the hashmap is released.
 * Once this function has executed, the specified hashmap becomes invalid and should NOT be used again for any purpose. This will result in undefined behaviour.
 * @param map - The hashmap to destroy. A null pointer is allowed.
 */
void xtHashmapDestroy(xtHashmap *map);
/**
 * Retrieves the bucket with the same key as is specified from the hashmap.
 * @param map - A pointer to the hashmap.
 * @param key - The key to look for.
 * @return A pointer to the bucket.
 */
struct xtHashBucket *xtHashmapGet(xtHashmap *map, void *key);
/**
 * Returns the amount of elements that currently reside in the hashmap.
 */
size_t xtHashmapGetCount(const xtHashmap *map);
/*
 * Returns the amount of elements that this hashmap is able to hold.
 */
size_t xtHashmapGetSize(const xtHashmap *map);
/**
 * Retrieves the value that the bucket with the given key holds.
 * @param map - A pointer to the hashmap.
 * @param key - The key to look for.
 * @return A pointer to the value that the found bucket holds.
 */
void *xtHashmapGetValue(xtHashmap *map, void *key);
/**
 * Retrieves the value and key that the bucket that comes next in line holds.
 * @param map - A pointer to the hashmap.
 * @param key - A pointer which will be changed to point to any found key.
 * @param value - A pointer which will be changed to point to any found value.
 * @return A boolean indicating if a next bucket was found. If not, the loop has ended and the iterator gets reset.
 */
bool xtHashmapForeach(xtHashmap *map, void **key, void **value);
/**
 * Resets the iterator, use this incase you want to prematurely terminate an xtHashmapForeach().
 */
void xtHashmapForeachEnd(xtHashmap *map);
/**
 * Puts an element in the hashmap.
 * @param map - A pointer to the hashmap.
 * @param key - A pointer to the key, which will be copied over.
 * @param value - A pointer to the value to put into the hashmap.
 * @return A boolean indicating if the bucket has been put into into the hashmap.
 */
bool xtHashmapPut(xtHashmap *map, void *key, void *value);
/**
 * Removes any found element from the hashmap.
 * This only works for objects allocated which are allocated by malloc or alike. C++ classes allocated with new should not be used.
 * The value and key are both freed.
 * @param map - A pointer to the hashmap
 * @param key - A pointer to the key, which will be used to lookup a value, and remove it.
 * @return A boolean indicating if anything was found and is removed.
 */
bool xtHashmapRemove(xtHashmap *map, void *key);

/* LIST */

/**
 * @brief A very easy to use (array) list.
 *
 * An (array) list holds elements all in a contiguous block of memory. Allowing for fast insertion and lookup, but
 * removal is pretty slow if you need to shift memory.
 * This means that if the array grows larger, removing elements at the beginning will get slower. Removing elements at the end is always fast.\n\n
 *
 * Bounds checking is performed on all operations.
 */
typedef struct xtList {
	bool canGrow;
	size_t count, capacity;
	void **data;
	void (*destroyElementFunc) (struct xtList *list, void *data);
	void (*destroyListFunc) (struct xtList *list);
} xtList;
/**
 * Attempts to add some data to the list.
 * The list will grow automatically if necessary and configured to allow this.
 * @return Zero if the data has been added, otherwise an error code.
 */
int xtListAdd(xtList *list, void *data);
/**
 * Overwrites the data at the specified index. You can ONLY replace elements with this function.
 * It is not possible to append elements to the end of the list with this function.
 * If an element is about to be replaced, it's destructor shall be called prior to replacement.
 * @return Zero if the data has been replaced, otherwise an error code.
 **/
int xtListAddAt(xtList *list, void *data, size_t index);
/**
 * Clears the list of all data. All element destructors shall be called.
 */
void xtListClear(xtList *list);
/**
 * Creates a new list. By default, automatic growth is enabled.
 * @param capacity - The initial capacity for the list. Specify zero to use the default value.
 * @return Zero if the list has been created successfully, otherwise an error code.
 */
int xtListCreate(xtList *list, size_t capacity);
/**
 * Destroys the list. All element destructors shall be called, including the list destructor.
 */
void xtListDestroy(xtList *list);
/**
 * Enables or disables automatic growth of the list.
 */
void xtListEnableGrowth(xtList *list, bool flag);
/**
 * Increases the capacity of the list to \a minCapacity if the list currently has a smaller capacity.
 * @return Zero if the capacity is already sufficiently large enough or if the growth has succeeded, otherwise an error code.
 */
int xtListEnsureCapacity(xtList *list, size_t minCapacity);
/**
 * Retrieves the element at the specified index from the list.
 * @param data - This pointer will receive the data from the list.
 * @return Zero if the data has been fetched successfully, otherwise an error code.
 */
int xtListGet(const xtList *list, size_t index, void **data);
/**
 * Returns the current capacity for the list.
 */
size_t xtListGetCapacity(const xtList *list);
/**
 * Returns the current amount of elements in the list.
 */
size_t xtListGetCount(const xtList *list);
/**
 * Increases the current capacity by \a n elements.
 * @return Zero if the capacity growth has succeeded, otherwise an error code.
 */
int xtListGrow(xtList *list, size_t n);
/**
 * Attempts to locate the element in the list, and then remove it.
 * @return Zero if the data has been found and removed, otherwise an error code.
 */
int xtListRemove(xtList *list, void *data);
/**
 * Removes the element at the specified position from the list.
 * @return Zero if the the data has been removed, otherwise an error code.
 */
int xtListRemoveAt(xtList *list, size_t index);
/**
 * Sets the function that is to be called when an element is removed or replaced.
 * Specify a null pointer to remove the function.
 */
void xtListSetElementDestroyFunc(xtList *list, void (*destroyElementFunc) (xtList *list, void *data));
/**
 * Sets the function that is to be called when the list is being destroyed.
 * Specify a null pointer to remove the function.
 */
void xtListSetListDestroyFunc(xtList *list, void (*destroyListFunc) (xtList *list));

/* STACK */

/*
 * NOTE: all stack operations are only documented for xtStackD,
 * since all other types (xtStackU, etc.) are exactly the same.
 */

struct xtStackHD {
	short *data;
	size_t count, capacity;
	int grow;
};

struct xtStackD {
	int *data;
	size_t count, capacity;
	int grow;
};

struct xtStackU {
	unsigned *data;
	size_t count, capacity;
	int grow;
};

struct xtStackLU {
	unsigned long *data;
	size_t count, capacity;
	int grow;
};

struct xtStackZU {
	size_t *data;
	size_t count, capacity;
	int grow;
};

void xtStackHDInit(struct xtStackHD *stack);
void xtStackDInit (struct xtStackD  *stack);
void xtStackUInit (struct xtStackU  *stack);
void xtStackLUInit(struct xtStackLU *stack);
void xtStackZUInit(struct xtStackZU *stack);

void xtStackHDDestroy(struct xtStackHD *stack);
void xtStackDDestroy (struct xtStackD  *stack);
void xtStackUDestroy (struct xtStackU  *stack);
void xtStackLUDestroy(struct xtStackLU *stack);
void xtStackZUDestroy(struct xtStackZU *stack);
/**
 * Creates a new stack. By default, automatic growth is enabled.
 * @param capacity Initial capacity (specify zero to use default value).
 * @return Zero if the stack has been created successfully, otherwise an error code.
 */
int xtStackDCreate (struct xtStackD  *stack, size_t capacity);
int xtStackHDCreate(struct xtStackHD *stack, size_t capacity);
int xtStackUCreate (struct xtStackU  *stack, size_t capacity);
int xtStackLUCreate(struct xtStackLU *stack, size_t capacity);
int xtStackZUCreate(struct xtStackZU *stack, size_t capacity);
/**
 * Changes grow policy. Positive values indicate fixed growth.
 * Negative values is relative growth. Zero disables growth.
 * E.g. grow(-2) is 100% / 2 = 50%, grow(-5) is 100% / 5 = 20%
 */
void xtStackDSetGrow (struct xtStackD  *stack, int grow);
void xtStackHDSetGrow(struct xtStackHD *stack, int grow);
void xtStackUSetGrow (struct xtStackU  *stack, int grow);
void xtStackLUSetGrow(struct xtStackLU *stack, int grow);
void xtStackZUSetGrow(struct xtStackZU *stack, int grow);
/**
 * Get grow policy. Positive values indicate fixed growth.
 * Negative values is relative growth. Zero disables growth.
 * E.g. grow(-2) is 100% / 2 = 50%, grow(-5) is 100% / 5 = 20%
 */
int xtStackDGetGrow (struct xtStackD  *stack);
int xtStackHDGetGrow(struct xtStackHD *stack);
int xtStackUGetGrow (struct xtStackU  *stack);
int xtStackLUGetGrow(struct xtStackLU *stack);
int xtStackZUGetGrow(struct xtStackZU *stack);
/**
 * Attempts to add \a value to stack.
 * The list will grow if configured to do so.
 * @return Zero if \a value has been added, otherwise an error code.
 */
int xtStackDPush (struct xtStackD  *stack, int value);
int xtStackHDPush(struct xtStackHD *stack, short value);
int xtStackUPush (struct xtStackU  *stack, unsigned value);
int xtStackLUPush(struct xtStackLU *stack, unsigned long value);
int xtStackZUPush(struct xtStackZU *stack, size_t value);
/**
 * Fetches last pushed item. Fails if stack is empty.
 */
bool xtStackDPeek (struct xtStackD  *stack, int *top);
bool xtStackHDPeek(struct xtStackHD *stack, short *top);
bool xtStackUPeek (struct xtStackU  *stack, unsigned *top);
bool xtStackLUPeek(struct xtStackLU *stack, unsigned long *top);
bool xtStackZUPeek(struct xtStackZU *stack, size_t *top);
/**
 * Free elements or do nothing if NULL has been specified.
 */
void xtStackDDestroy (struct xtStackD  *stack);
void xtStackHDDestroy(struct xtStackHD *stack);
void xtStackUDestroy (struct xtStackU  *stack);
void xtStackLUDestroy(struct xtStackLU *stack);
void xtStackZUDestroy(struct xtStackZU *stack);
/**
 * Stores and removes last pushed item. Fails if stack is empty.
 */
bool xtStackDPop (struct xtStackD  *stack, int *top);
bool xtStackHDPop(struct xtStackHD *stack, short *top);
bool xtStackUPop (struct xtStackU  *stack, unsigned *top);
bool xtStackLUPop(struct xtStackLU *stack, unsigned long *top);
bool xtStackZUPop(struct xtStackZU *stack, size_t *top);
/**
 * Removes last pushed item and return new count.
 * Fails if stack is empty (i.e. returns zero).
 */
size_t xtStackDDrop (struct xtStackD  *stack);
size_t xtStackHDDrop(struct xtStackHD *stack);
size_t xtStackUDrop (struct xtStackU  *stack);
size_t xtStackLUDrop(struct xtStackLU *stack);
size_t xtStackZUDrop(struct xtStackZU *stack);

size_t xtStackHDGetSize(struct xtStackHD *stack);
size_t xtStackDGetSize (struct xtStackD  *stack);
size_t xtStackUGetSize (struct xtStackU  *stack);
size_t xtStackLUGetSize(struct xtStackLU *stack);
size_t xtStackZUGetSize(struct xtStackZU *stack);

size_t xtStackHDGetCapacity(struct xtStackHD *stack);
size_t xtStackDGetCapacity (struct xtStackD  *stack);
size_t xtStackUGetCapacity (struct xtStackU  *stack);
size_t xtStackLUGetCapacity(struct xtStackLU *stack);
size_t xtStackZUGetCapacity(struct xtStackZU *stack);

#ifdef __cplusplus
}
#endif

#endif

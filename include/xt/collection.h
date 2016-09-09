/**
 * @brief Contains various utilites like a Hashmap and List.
 * @file collection.h
 * @author Tom Everaarts
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
 * This hashmap is NOT thread safe. The hashmap manages almost all memory for you. The key and value are destroyed if an element is removed.
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
 * Resets the iterator, use this incase you want to prematurely terminate an xtHashmapForeach()
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
/**
 * @brief A very easy to use (array) list.
 * 
 * An (array) list holds data all in a contiguous block of memory. Allowing for fast insertion and lookup, but removal is pretty slow if you need to shift memory. 
 * This means that if the array grows larger, removing data at the beginning will get slower. Removing data at the end is always fast.\n\n
 * 
 * Bounds checking is performed on all operations. 
 * This list does NOT perform any memory management for the objects that you put into it! You have to free them by hand if they consist of dynamic memory.
 */
typedef struct xtList {
	bool canGrow, shiftMemory;
	size_t count, capacity, lastFreeIndex;
	void **data;
} xtList;
/**
 * The minimum capacity that any list will have.
 */
#define XT_LIST_CAPACITY_MIN 64
/**
 * Attempts to add an object to the list. 
 * Depending on the settings, the list could grow if the element would not fit. 
 * A boolean is returned indicating if the element was added to the list.
 */
bool xtListAdd(xtList *list, void *v);
/**
 * Overwrites the element at the specified index.
 **/
bool xtListAddAt(xtList *list, void *v, size_t index);
/**
 * Clears the list of all elements. Regardless of how many elements are inside or if the list is fragmented.
 */
void xtListClear(xtList *list);
/**
 * Creates a new list. 
 * If this function fails, you must NOT use the list for any purpose.
 */
bool xtListCreate(xtList *list, size_t capacity);
/**
 * Destroys the list. 
 * Once this function has executed, the specified list becomes invalid and should NOT be used again for any purpose.
 * @param list - The list to destroy. A null pointer is allowed.
 */
void xtListDestroy(xtList *list);
/**
 * Enables or disables growing for the list if it is running out of space.
 */
void xtListEnableGrowth(xtList *list, bool flag);
/**
 * Enables or disables memory shifting.\n
 * False: When an element is removed, no elements are shifted. The list becomes fragmented and holes can occur.\n
 * True: When an element is removed, all elements beyond it are shifted. This is to make sure no holes occur in the list. 
 * This is slower, but this option is generally what you want.
 */
void xtListEnableShifting(xtList *list, bool flag);
/**
 * Increases the capacity of the list to \a minCapacity if the list has a smaller capacity, to ensure that the list has atleast \a minCapacity as capacity. 
 * Returns true is the capacity is already large enough, or if the growth has succeeded.
 */
bool xtListEnsureCapacity(xtList *list, size_t minCapacity);
/**
 * A pointer to the element at the specified index is returned, null if there is no element.
 */
void *xtListGet(const xtList *list, size_t index);
/**
 * Returns the current capacity for the list.
 */
size_t xtListGetCapacity(const xtList *list);
/**
 * Returns the amount of elements in the list.
 */
size_t xtListGetCount(const xtList *list);
/**
 * Attempts to locate the element in the list, and then remove it.
 * A pointer to the element is returned, null if there is no element.
 */
void *xtListRemove(xtList *list, void *element);
/**
 * Removes the element at the specified position from the list.
 * A pointer to the element is returned, null if there is no element.
 */
void *xtListRemoveAt(xtList *list, size_t index);

#ifdef __cplusplus
}
#endif

#endif

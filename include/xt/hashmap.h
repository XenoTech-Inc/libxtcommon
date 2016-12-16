/**
 * @brief Hashmap that hashes items to bucket.
 *
 * Average search, insert and remove is constant: O(1).
 * Worst case for each operation is O(n).
 * Initially implemented by Folkert van Verseveld.
 * Improved and extended by Tom Everaarts.
 *
 * @file hashmap.h
 * @author Folkert van Verseveld and Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_HASHMAP_H
#define _XT_HASHMAP_H

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
 * This hashmap is NOT thread safe. The hashmap manages almost all memory for you. The key and value are both destroyed if any data is removed.
 */
struct xtHashmap {
	struct xtHashBucket **buckets;
	size_t count, size;
	bool (*keyCompare)(void *key1, void *key2);
	size_t (*keyHash) (void *key);
	size_t keySize;
	struct xtHashmapIterator it;
};
/**
 * Creates a new hashmap.
 * If this function fails, you must NOT use the hashmap for any purpose. This is unsafe.
 * @param map - A pointer to the hashmap.
 * @param size - How many buckets the hashmap should have.
 * @param keySize - The size of your custom keys. sizeof([Your key type])
 * @param keyCompare - A function pointer to the function which will compare two keys, and return if they are the same.
 * @param keyHash - A function pointer to the function which will create a hash of your custom key type.
 */
bool xtHashmapCreate(struct xtHashmap *map, size_t size, size_t keySize, bool (*keyCompare) (void*, void*), size_t (*keyHash) (void*));
/**
 * Destroys the hashmap, all memory associated with the hashmap is released.
 * Once this function has executed, the specified hashmap becomes invalid and should NOT be used again for any purpose. This will result in undefined behaviour.
 * @param map - The hashmap to destroy. A null pointer is allowed.
 */
void xtHashmapDestroy(struct xtHashmap *map);
/**
 * Retrieves the bucket with the same key as is specified from the hashmap.
 * @param map - A pointer to the hashmap.
 * @param key - The key to look for.
 * @return A pointer to the bucket.
 */
struct xtHashBucket *xtHashmapGet(struct xtHashmap *map, void *key);
/**
 * Returns the amount of elements that currently reside in the hashmap.
 */
size_t xtHashmapGetCount(const struct xtHashmap *map);
/*
 * Returns the amount of elements that this hashmap is able to hold.
 */
size_t xtHashmapGetSize(const struct xtHashmap *map);
/**
 * Retrieves the value that the bucket with the given key holds.
 * @param map - A pointer to the hashmap.
 * @param key - The key to look for.
 * @return A pointer to the value that the found bucket holds.
 */
void *xtHashmapGetValue(struct xtHashmap *map, void *key);
/**
 * Retrieves the value and key that the bucket that comes next in line holds.
 * @param map - A pointer to the hashmap.
 * @param key - A pointer which will be changed to point to any found key.
 * @param value - A pointer which will be changed to point to any found value.
 * @return A boolean indicating if a next bucket was found. If not, the loop has ended and the iterator gets reset.
 */
bool xtHashmapForeach(struct xtHashmap *map, void **key, void **value);
/**
 * Resets the iterator, use this incase you want to prematurely terminate an xtHashmapForeach().
 */
void xtHashmapForeachEnd(struct xtHashmap *map);
/**
 * Puts an element in the hashmap.
 * @param map - A pointer to the hashmap.
 * @param key - A pointer to the key, which will be copied over.
 * @param value - A pointer to the value to put into the hashmap.
 * @return A boolean indicating if the bucket has been put into into the hashmap.
 */
bool xtHashmapPut(struct xtHashmap *map, void *key, void *value);
/**
 * Removes any found element from the hashmap.
 * This only works for objects allocated which are allocated by malloc or alike. C++ classes allocated with new should not be used.
 * The value and key are both freed.
 * @param map - A pointer to the hashmap
 * @param key - A pointer to the key, which will be used to lookup a value, and remove it.
 * @return A boolean indicating if anything was found and is removed.
 */
bool xtHashmapRemove(struct xtHashmap *map, void *key);

#ifdef __cplusplus
}
#endif

#endif

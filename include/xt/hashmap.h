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

#define XT_HASHMAP_CAPACITY_DEFAULT 1024

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
 */
struct xtHashmap {
	struct xtHashBucket **buckets;
	size_t capacity, count;
	size_t (*keyHash) (const void *key);
	bool (*keyCompare)(const void *key1, const void *key2);
	struct xtHashmapIterator it;
};
/**
 * Adds an element to the hashmap.
 * @return Zero if the element has been added to the hashmap, otherwise an error code.
 * @remarks The hashmap does NOT make a copy of the key and value! It is best practice
 * to allocate both of these dynamically and free them manually after removing them from
 * the hashmap.
 */
int xtHashmapAdd(struct xtHashmap *map, void *key, void *value);
/**
 * @param capacity - The initial capacity for the hashmap. Specify zero
 * to use the default value.
 * @param keyHash - A function pointer to the function which will create a hash of your custom key type.
 * @param keyCompare - A function pointer to the function which will compare two keys and return whether they are the same.
 * @return Zero if the hashmap has been created, otherwise an error code.
 */
int xtHashmapCreate(
	struct xtHashmap *map, size_t capacity,
	size_t (*keyHash) (const void*), bool (*keyCompare) (const void*, const void*)
);

void xtHashmapDestroy(struct xtHashmap *map);
/**
 * Retrieves the bucket that is associated with \a key.
 * @param bucket - This pointer will receive a pointer to the bucket in the hashmap.
 * @return Zero if the associated bucket has been found, otherwise an error code.
 */
int xtHashmapGet(const struct xtHashmap *map, const void *key, struct xtHashBucket **bucket);

size_t xtHashmapGetCapacity(const struct xtHashmap *map);

size_t xtHashmapGetCount(const struct xtHashmap *map);
/**
 * Retrieves the value that is associated with \a key.
 * @param value - This pointer will receive a pointer to the value in the hashmap.
 * @return Zero if the associated bucket has been found, otherwise an error code.
 */
int xtHashmapGetValue(const struct xtHashmap *map, const void *key, void **value);
/**
 * Retrieves the value and key that the bucket that comes next in line holds.
 * @param key - A pointer which will be changed to point to any found key.
 * @param value - A pointer which will be changed to point to any found value.
 * @return A boolean indicating if a next bucket was found. If not, the loop has ended and the iterator gets reset.
 */
bool xtHashmapForeach(struct xtHashmap *map, void **key, void **value);
/**
 * Resets the iterator. Use this when you want to prematurely terminate an xtHashmapForeach().
 */
void xtHashmapForeachEnd(struct xtHashmap *map);
/**
 * Removes the element associated with \a key from the hashmap.
 * @return Zero if the element was found and removed, otherwise an error code.
 * @remarks Do note that if the key / value was dynamically allocated, you have to
 * free them by hand!
 */
int xtHashmapRemove(struct xtHashmap *map, void *key);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @brief Resizable (array) list.
 *
 * Indexing is constant: O(1).
 * Worst case for each operation is O(n).
 *
 * @file list.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_LIST_H
#define _XT_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>

#define XT_LIST_CAPACITY_DEFAULT 1024

/**
 * @brief A very easy to use (array) list.
 *
 * An (array) list holds elements all in a contiguous block of memory. Allowing for fast insertion and lookup, but
 * removal is pretty slow if you need to shift memory.
 * This means that if the array grows larger, removing elements at the beginning will get slower. Removing elements at the end is always fast.\n\n
 *
 * Bounds checking is performed on all operations.
 */
struct xtList {
	bool canGrow;
	size_t count, capacity;
	void **data;
	void (*destroyElementFunc) (struct xtList *list, void *data);
	void (*destroyListFunc) (struct xtList *list);
};
/**
 * Attempts to add some data to the list.
 * The list will grow automatically if necessary and configured to allow this.
 * @return Zero if the data has been added, otherwise an error code.
 */
int xtListAdd(struct xtList *list, void *data);
/**
 * Overwrites the data at the specified index. You can ONLY replace elements with this function.
 * It is not possible to append elements to the end of the list with this function.
 * If an element is about to be replaced, it's destructor shall be called prior to replacement.
 * @return Zero if the data has been replaced, otherwise an error code.
 **/
int xtListAddAt(struct xtList *list, void *data, size_t index);
/**
 * Clears the list of all data. All element destructors shall be called.
 */
void xtListClear(struct xtList *list);
/**
 * Creates a new list. By default, automatic growth is enabled.
 * @param capacity - The initial capacity for the list. Specify zero to use the default value.
 * @return Zero if the list has been created successfully, otherwise an error code.
 */
int xtListCreate(struct xtList *list, size_t capacity);
/**
 * Destroys the list. All element destructors shall be called, including the list destructor.
 */
void xtListDestroy(struct xtList *list);
/**
 * Enables or disables automatic growth of the list.
 */
void xtListEnableGrowth(struct xtList *list, bool flag);
/**
 * Increases the capacity of the list to \a minCapacity if the list currently has a smaller capacity.
 * @return Zero if the capacity is already sufficiently large enough or if the growth has succeeded, otherwise an error code.
 */
int xtListEnsureCapacity(struct xtList *list, size_t minCapacity);
/**
 * Retrieves the element at the specified index from the list.
 * @param data - This pointer will receive the data from the list.
 * @return Zero if the data has been fetched successfully, otherwise an error code.
 */
int xtListGet(const struct xtList *list, size_t index, void **data);
/**
 * Returns the current capacity for the list.
 */
size_t xtListGetCapacity(const struct xtList *list);
/**
 * Returns the current amount of elements in the list.
 */
size_t xtListGetCount(const struct xtList *list);
/**
 * Increases the current capacity by \a n elements.
 * @return Zero if the capacity growth has succeeded, otherwise an error code.
 */
int xtListGrow(struct xtList *list, size_t n);
/**
 * Attempts to locate the element in the list, and then remove it.
 * @return Zero if the data has been found and removed, otherwise an error code.
 */
int xtListRemove(struct xtList *list, void *data);
/**
 * Removes the element at the specified position from the list.
 * @return Zero if the the data has been removed, otherwise an error code.
 */
int xtListRemoveAt(struct xtList *list, size_t index);
/**
 * Sets the function that is to be called when an element is removed or replaced.
 * Specify a null pointer to remove the function.
 */
void xtListSetElementDestroyFunc(struct xtList *list, void (*destroyElementFunc) (struct xtList *list, void *data));
/**
 * Sets the function that is to be called when the list is being destroyed.
 * Specify a null pointer to remove the function.
 */
void xtListSetListDestroyFunc(struct xtList *list, void (*destroyListFunc) (struct xtList *list));

#ifdef __cplusplus
}
#endif

#endif

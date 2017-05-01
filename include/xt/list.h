/**
 * @brief Resizable (array) list.
 *
 * Indexing is constant: O(1).
 * Worst case for each operation is O(n).
 *
 * @file list.h
 * @author Tom Everaarts
 * @date 2017
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

// All flags that are supported by the list.
/** Do nothing if an item has been removed */
#define XT_LIST_FREE_NOTHING 0x00
/** Free an item if it has been removed */
#define XT_LIST_FREE_ITEM    0x01

/**
 * @brief A very easy to use (array) list.
 *
 * A list holds elements all in a contiguous block of memory. Allowing
 * for fast insertion and lookup, but removal is pretty slow if you need
 * to shift memory. This means that if the array grows larger, removing
 * elements at the beginning will get slower. Removing elements at the
 * end is always fast. The default growth factor is 100%
 * (i.e. it doubles in size).\n\n
 *
 * Bounds checking is performed on all operations.
 */
struct xtListHD {
	short *data;
	size_t count, capacity;
	int grow;
	unsigned flags;
};

struct xtListD {
	int *data;
	size_t count, capacity;
	int grow;
	unsigned flags;
};

struct xtListU {
	unsigned *data;
	size_t count, capacity;
	int grow;
	unsigned flags;
};

struct xtListLU {
	unsigned long *data;
	size_t count, capacity;
	int grow;
	unsigned flags;
};

struct xtListZU {
	size_t *data;
	size_t count, capacity;
	int grow;
	unsigned flags;
};

struct xtListP {
	void **data;
	size_t count, capacity;
	int grow;
	unsigned flags;
};
/**
 * Attempts to add some data to the list.
 * The list will grow automatically if necessary and if it is configured
 * to do so.
 * @return Zero if the data has been added, otherwise an error code.
 */
int xtListHDAdd(struct xtListHD *list, short data);
int xtListDAdd (struct xtListD  *list, int data);
int xtListUAdd (struct xtListU  *list, unsigned data);
int xtListLUAdd(struct xtListLU *list, unsigned long data);
int xtListZUAdd(struct xtListZU *list, size_t data);
int xtListPAdd (struct xtListP  *list, void *data);
/**
 * Overwrites the data at the specified index. You can ONLY replace
 * elements with this function. It is not possible to append elements
 * to the end of the list with this function.
 * @return Zero if the data has been replaced, otherwise an error code.
 */
int xtListHDAddAt(struct xtListHD *list, short data, size_t index);
int xtListDAddAt (struct xtListD  *list, int data, size_t index);
int xtListUAddAt (struct xtListU  *list, unsigned data, size_t index);
int xtListLUAddAt(struct xtListLU *list, unsigned long data, size_t index);
int xtListZUAddAt(struct xtListZU *list, size_t data, size_t index);
int xtListPAddAt (struct xtListP  *list, void *data, size_t index);

void xtListHDClear(struct xtListHD *list);
void xtListDClear (struct xtListD  *list);
void xtListUClear (struct xtListU  *list);
void xtListLUClear(struct xtListLU *list);
void xtListZUClear(struct xtListZU *list);
void xtListPClear (struct xtListP  *list);
/**
 * Creates a new list. By default, automatic growth is enabled.
 * @param capacity - The initial capacity for the list. Specify zero
 * to use the default value.
 * @return Zero if the list has been created successfully, otherwise
 * an error code.
 */
int xtListHDCreate(struct xtListHD *list, size_t capacity);
int xtListDCreate (struct xtListD  *list, size_t capacity);
int xtListUCreate (struct xtListU  *list, size_t capacity);
int xtListLUCreate(struct xtListLU *list, size_t capacity);
int xtListZUCreate(struct xtListZU *list, size_t capacity);
int xtListPCreate (struct xtListP  *list, size_t capacity);

void xtListHDDestroy(struct xtListHD *list);
void xtListDDestroy (struct xtListD  *list);
void xtListUDestroy (struct xtListU  *list);
void xtListLUDestroy(struct xtListLU *list);
void xtListZUDestroy(struct xtListZU *list);
void xtListPDestroy (struct xtListP  *list);
/**
 * Increases the capacity of the list to \a minCapacity if the list
 * currently has a smaller capacity.
 * @return Zero if the capacity is already sufficiently large enough
 * or if the growth has succeeded, otherwise an error code.
 */
int xtListHDEnsureCapacity(struct xtListHD *list, size_t minCapacity);
int xtListDEnsureCapacity (struct xtListD  *list, size_t minCapacity);
int xtListUEnsureCapacity (struct xtListU  *list, size_t minCapacity);
int xtListLUEnsureCapacity(struct xtListLU *list, size_t minCapacity);
int xtListZUEnsureCapacity(struct xtListZU *list, size_t minCapacity);
int xtListPEnsureCapacity (struct xtListP  *list, size_t minCapacity);
/**
 * Retrieves the element at the specified index from the list.
 * @param data - This pointer will receive the data from the list.
 * @return Zero if the data has been fetched successfully, otherwise
 * an error code.
 */
int xtListHDGet(const struct xtListHD *list, size_t index, short *data);
int xtListDGet (const struct xtListD  *list, size_t index, int *data);
int xtListUGet (const struct xtListU  *list, size_t index, unsigned *data);
int xtListLUGet(const struct xtListLU *list, size_t index, unsigned long *data);
int xtListZUGet(const struct xtListZU *list, size_t index, size_t *data);
int xtListPGet (const struct xtListP  *list, size_t index, void **data);

size_t xtListHDGetCapacity(const struct xtListHD *list);
size_t xtListDGetCapacity (const struct xtListD  *list);
size_t xtListUGetCapacity (const struct xtListU  *list);
size_t xtListLUGetCapacity(const struct xtListLU *list);
size_t xtListZUGetCapacity(const struct xtListZU *list);
size_t xtListPGetCapacity (const struct xtListP  *list);

size_t xtListHDGetCount(const struct xtListHD *list);
size_t xtListDGetCount (const struct xtListD  *list);
size_t xtListUGetCount (const struct xtListU  *list);
size_t xtListLUGetCount(const struct xtListLU *list);
size_t xtListZUGetCount(const struct xtListZU *list);
size_t xtListPGetCount (const struct xtListP  *list);

unsigned xtListHDGetFlags(const struct xtListHD *list);
unsigned xtListDGetFlags (const struct xtListD  *list);
unsigned xtListUGetFlags (const struct xtListU  *list);
unsigned xtListLUGetFlags(const struct xtListLU *list);
unsigned xtListZUGetFlags(const struct xtListZU *list);
unsigned xtListPGetFlags (const struct xtListP  *list);

int xtListHDGetGrowthFactor(struct xtListHD *list);
int xtListDGetGrowthFactor (struct xtListD  *list);
int xtListUGetGrowthFactor (struct xtListU  *list);
int xtListLUGetGrowthFactor(struct xtListLU *list);
int xtListZUGetGrowthFactor(struct xtListZU *list);
int xtListPGetGrowthFactor (struct xtListP  *list);

int xtListHDRemove(struct xtListHD *list, short data);
int xtListDRemove (struct xtListD  *list, int data);
int xtListURemove (struct xtListU  *list, unsigned data);
int xtListLURemove(struct xtListLU *list, unsigned long data);
int xtListZURemove(struct xtListZU *list, size_t data);
int xtListPRemove (struct xtListP  *list, void *data);

int xtListHDRemoveAt(struct xtListHD *list, size_t index);
int xtListDRemoveAt (struct xtListD  *list, size_t index);
int xtListURemoveAt (struct xtListU  *list, size_t index);
int xtListLURemoveAt(struct xtListLU *list, size_t index);
int xtListZURemoveAt(struct xtListZU *list, size_t index);
int xtListPRemoveAt (struct xtListP  *list, size_t index);
/**
 * Sets the absolute capacity for the list. This function
 * can shrink the list too. This will result in the immidiate loss of
 * all trailing elements which are not going to fit!
 */
int xtListHDSetCapacity(struct xtListHD *list, size_t capacity);
int xtListDSetCapacity (struct xtListD  *list, size_t capacity);
int xtListUSetCapacity (struct xtListU  *list, size_t capacity);
int xtListLUSetCapacity(struct xtListLU *list, size_t capacity);
int xtListZUSetCapacity(struct xtListZU *list, size_t capacity);
int xtListPSetCapacity (struct xtListP  *list, size_t capacity);

void xtListHDSetFlags(struct xtListHD *list, unsigned flags);
void xtListDSetFlags (struct xtListD *list , unsigned flags);
void xtListUSetFlags (struct xtListU *list , unsigned flags);
void xtListLUSetFlags(struct xtListLU *list, unsigned flags);
void xtListZUSetFlags(struct xtListZU *list, unsigned flags);
void xtListPSetFlags (struct xtListP *list , unsigned flags);

void xtListHDSetGrowthFactor(struct xtListHD *list, int grow);
void xtListDSetGrowthFactor (struct xtListD  *list, int grow);
void xtListUSetGrowthFactor (struct xtListU  *list, int grow);
void xtListLUSetGrowthFactor(struct xtListLU *list, int grow);
void xtListZUSetGrowthFactor(struct xtListZU *list, int grow);
void xtListPSetGrowthFactor (struct xtListP  *list, int grow);

#ifdef __cplusplus
}
#endif

#endif

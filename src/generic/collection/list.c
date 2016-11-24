// XT headers
#include <xt/collection.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>
#include <string.h>

int xtListAdd(struct xtList *list, void *data)
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

int xtListAddAt(struct xtList *list, void *data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL; // Can only replace elements
	if (list->destroyElementFunc)
		list->destroyElementFunc(list, list->data[index]);
	list->data[index] = data;
	// Do not increase count! An element has been replaced
	return 0;
}

void xtListClear(struct xtList *list)
{
	// Remove elements at the end, this is faster
	while (list->count > 0)
		xtListRemoveAt(list, list->count - 1);
}

int xtListCreate(struct xtList *list, size_t capacity)
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

void xtListDestroy(struct xtList *list)
{
	xtListClear(list);
	if (list->destroyListFunc)
		list->destroyListFunc(list);
	free(list->data);
}

void xtListEnableGrowth(struct xtList *list, bool flag)
{
	list->canGrow = flag;
}

int xtListEnsureCapacity(struct xtList *list, size_t minCapacity)
{
	if (list->capacity >= minCapacity)
		return 0;
	return xtListGrow(list, minCapacity - xtListGetCapacity(list));
}

int xtListGet(const struct xtList *list, size_t index, void **data)
{
	if (index >= list->count)
		return XT_EINVAL;
	*data = list->data[index];
	return 0;
}

size_t xtListGetCapacity(const struct xtList *list)
{
	return list->capacity;
}

size_t xtListGetCount(const struct xtList *list)
{
	return list->count;
}

int xtListGrow(struct xtList *list, size_t n)
{
	void **temp;
	if (!(temp = realloc(list->data, (list->capacity + n) * sizeof(list->data))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity += n;
	return 0;
}

int xtListRemove(struct xtList *list, void *data)
{
	for (size_t i = 0; i < list->count; ++i) {
		if (list->data[i] == data) {
			xtListRemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListRemoveAt(struct xtList *list, size_t index)
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

void xtListSetElementDestroyFunc(struct xtList *list, void (*destroyElementFunc) (struct xtList *list, void *data))
{
	list->destroyElementFunc = destroyElementFunc;
}

void xtListSetListDestroyFunc(struct xtList *list, void (*destroyListFunc) (struct xtList *list))
{
	list->destroyListFunc = destroyListFunc;
}

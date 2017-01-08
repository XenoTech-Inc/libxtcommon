// XT headers
#include <xt/list.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>
#include <string.h>

static inline int _xtListCreate(void **data, size_t elemsize, size_t n)
{
	if (n == 0)
		n = XT_LIST_CAPACITY_DEFAULT;
	if (!(*data = malloc(n * elemsize)))
		return XT_ENOMEM;
	return 0;
}

int xtListHDAdd(struct xtListHD *list, short data)
{
	if (list->count == list->capacity) {
		if (list->grow == 0)
			return XT_ENOBUFS;
		size_t grow = list->grow > 0 ? (unsigned) list->grow : (list->capacity / -(unsigned) list->grow) + xtListHDGetCapacity(list);
		int ret = xtListHDSetCapacity(list, grow);
		if (ret != 0)
			return ret;
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListDAdd(struct xtListD *list, int data)
{
	if (list->count == list->capacity) {
		if (list->grow == 0)
			return XT_ENOBUFS;
		size_t grow = list->grow > 0 ? (unsigned) list->grow : (list->capacity / -(unsigned) list->grow) + xtListDGetCapacity(list);
		int ret = xtListDSetCapacity(list, grow);
		if (ret != 0)
			return ret;
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListUAdd(struct xtListU *list, unsigned data)
{
	if (list->count == list->capacity) {
		if (list->grow == 0)
			return XT_ENOBUFS;
		size_t grow = list->grow > 0 ? (unsigned) list->grow : (list->capacity / -(unsigned) list->grow) + xtListUGetCapacity(list);
		int ret = xtListUSetCapacity(list, grow);
		if (ret != 0)
			return ret;
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListLUAdd(struct xtListLU *list, unsigned long data)
{
	if (list->count == list->capacity) {
		if (list->grow == 0)
			return XT_ENOBUFS;
		size_t grow = list->grow > 0 ? (unsigned) list->grow : (list->capacity / -(unsigned) list->grow) + xtListLUGetCapacity(list);
		int ret = xtListLUSetCapacity(list, grow);
		if (ret != 0)
			return ret;
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListZUAdd(struct xtListZU *list, size_t data)
{
	if (list->count == list->capacity) {
		if (list->grow == 0)
			return XT_ENOBUFS;
		size_t grow = list->grow > 0 ? (unsigned) list->grow : (list->capacity / -(unsigned) list->grow) + xtListZUGetCapacity(list);
		int ret = xtListZUSetCapacity(list, grow);
		if (ret != 0)
			return ret;
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListPAdd(struct xtListP *list, void *data)
{
	if (list->count == list->capacity) {
		if (list->grow == 0)
			return XT_ENOBUFS;
		size_t grow = list->grow > 0 ? (unsigned) list->grow : (list->capacity / -(unsigned) list->grow) + xtListPGetCapacity(list);
		int ret = xtListPSetCapacity(list, grow);
		if (ret != 0)
			return ret;
	}
	list->data[list->count] = data;
	++list->count;
	return 0;
}

int xtListHDAddAt(struct xtListHD *list, short data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	list->data[index] = data;
	return 0;
}

int xtListDAddAt(struct xtListD *list, int data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	list->data[index] = data;
	return 0;
}

int xtListUAddAt(struct xtListU *list, unsigned data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	list->data[index] = data;
	return 0;
}

int xtListLUAddAt(struct xtListLU *list, unsigned long data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	list->data[index] = data;
	return 0;
}

int xtListZUAddAt(struct xtListZU *list, size_t data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	list->data[index] = data;
	return 0;
}

int xtListPAddAt(struct xtListP *list, void *data, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	if ((list->flags & XT_LIST_FREE_ITEM) && list->data[index])
		free(list->data[index]);
	list->data[index] = data;
	return 0;
}

#define func_clear(type) void type ## Clear(struct type *list) { list->count = 0; }

func_clear(xtListHD)
func_clear(xtListD )
func_clear(xtListU )
func_clear(xtListLU)
func_clear(xtListZU)

void xtListPClear(struct xtListP *list)
{
	if (list->flags & XT_LIST_FREE_ITEM)
		for (size_t i = 0, n = list->count; i < n; ++i)
			free(list->data[i]);
	list->count = 0;
}

#define list_init(this, data, cap) \
	this->data = data;\
	this->count = 0;\
	this->capacity = cap;\
	this->grow = -1;\
	this->flags = 0;

int xtListHDCreate(struct xtListHD *list, size_t capacity)
{
	void *data;
	if (capacity == 0)
		capacity = XT_LIST_CAPACITY_DEFAULT;
	int ret = _xtListCreate(&data, sizeof(short), capacity);
	if (ret)
		return ret;
	list_init(list, data, capacity);
	return 0;
}

int xtListDCreate(struct xtListD *list, size_t capacity)
{
	void *data;
	if (capacity == 0)
		capacity = XT_LIST_CAPACITY_DEFAULT;
	int ret = _xtListCreate(&data, sizeof(int), capacity);
	if (ret)
		return ret;
	list_init(list, data, capacity);
	return 0;
}

int xtListUCreate(struct xtListU *list, size_t capacity)
{
	void *data;
	if (capacity == 0)
		capacity = XT_LIST_CAPACITY_DEFAULT;
	int ret = _xtListCreate(&data, sizeof(unsigned), capacity);
	if (ret)
		return ret;
	list_init(list, data, capacity);
	return 0;
}

int xtListLUCreate(struct xtListLU *list, size_t capacity)
{
	void *data;
	if (capacity == 0)
		capacity = XT_LIST_CAPACITY_DEFAULT;
	int ret = _xtListCreate(&data, sizeof(unsigned long), capacity);
	if (ret)
		return ret;
	list_init(list, data, capacity);
	return 0;
}

int xtListZUCreate(struct xtListZU *list, size_t capacity)
{
	void *data;
	if (capacity == 0)
		capacity = XT_LIST_CAPACITY_DEFAULT;
	int ret = _xtListCreate(&data, sizeof(size_t), capacity);
	if (ret)
		return ret;
	list_init(list, data, capacity);
	return 0;
}

int xtListPCreate(struct xtListP *list, size_t capacity)
{
	void *data;
	if (capacity == 0)
		capacity = XT_LIST_CAPACITY_DEFAULT;
	int ret = _xtListCreate(&data, sizeof(void*), capacity);
	if (ret)
		return ret;
	list_init(list, data, capacity);
	return 0;
}

#define func_destroy(type) void type ## Destroy(struct type *list) { type ## Clear(list); if (list->data) { free(list->data); list->data = NULL; } }

func_destroy(xtListHD)
func_destroy(xtListD )
func_destroy(xtListU )
func_destroy(xtListLU)
func_destroy(xtListZU)
func_destroy(xtListP )

#define func_ensure_capacity(type) \
	int type ## EnsureCapacity(struct type *list, size_t minCapacity) { \
		return  list->capacity >= minCapacity ? 0 : type ## SetCapacity(list, minCapacity); \
	}

func_ensure_capacity(xtListHD)
func_ensure_capacity(xtListD )
func_ensure_capacity(xtListU )
func_ensure_capacity(xtListLU)
func_ensure_capacity(xtListZU)
func_ensure_capacity(xtListP )

#define func_get(type, arg) \
	int type ## Get(const struct type *list, size_t index, arg *data) { \
		return index >= list->count ? XT_EINVAL : (*data = list->data[index], 0); \
	}

func_get(xtListHD, short        )
func_get(xtListD , int          )
func_get(xtListU , unsigned     )
func_get(xtListLU, unsigned long)
func_get(xtListZU, size_t       )
func_get(xtListP , void*        )

#define func_get_capacity(type) size_t type ## GetCapacity(const struct type *t) { return t->capacity; }
#define func_get_count(type) size_t type ## GetCount(const struct type *t) { return t->count; }

func_get_capacity(xtListHD)
func_get_capacity(xtListD )
func_get_capacity(xtListU )
func_get_capacity(xtListLU)
func_get_capacity(xtListZU)
func_get_capacity(xtListP )

func_get_count(xtListHD)
func_get_count(xtListD )
func_get_count(xtListU )
func_get_count(xtListLU)
func_get_count(xtListZU)
func_get_count(xtListP )

#define func_get_flags(type) unsigned  type ## GetFlags(const struct type *t) { return t->flags; }

func_get_flags(xtListHD)
func_get_flags(xtListD )
func_get_flags(xtListU )
func_get_flags(xtListLU)
func_get_flags(xtListZU)
func_get_flags(xtListP )

#define func_get_growth_factor(type) int  type ## GetGrowthFactor(struct type *t) { return t->grow; }

func_get_growth_factor(xtListHD)
func_get_growth_factor(xtListD )
func_get_growth_factor(xtListU )
func_get_growth_factor(xtListLU)
func_get_growth_factor(xtListZU)
func_get_growth_factor(xtListP )

int xtListHDRemove(struct xtListHD *list, short data)
{
	size_t count = list->count;
	for (size_t i = 0; i < count; ++i) {
		if (list->data[i] == data) {
			xtListHDRemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListDRemove(struct xtListD *list, int data)
{
	size_t count = list->count;
	for (size_t i = 0; i < count; ++i) {
		if (list->data[i] == data) {
			xtListDRemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListURemove(struct xtListU *list, unsigned data)
{
	size_t count = list->count;
	for (size_t i = 0; i < count; ++i) {
		if (list->data[i] == data) {
			xtListURemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListLURemove(struct xtListLU *list, unsigned long data)
{
	size_t count = list->count;
	for (size_t i = 0; i < count; ++i) {
		if (list->data[i] == data) {
			xtListLURemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListZURemove(struct xtListZU *list, size_t data)
{
	size_t count = list->count;
	for (size_t i = 0; i < count; ++i) {
		if (list->data[i] == data) {
			xtListZURemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListPRemove(struct xtListP *list, void *data)
{
	size_t count = list->count;
	for (size_t i = 0; i < count; ++i) {
		if (list->data[i] == data) {
			xtListPRemoveAt(list, i);
			return 0;
		}
	}
	return XT_ENOENT;
}

int xtListHDRemoveAt(struct xtListHD *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	// If list is the last element, then there is no need to shift.
	// This trick greatly improves performance when destroying the list from the end!
	if (index == list->count - 1)
		list->data[index] = 0;
	else
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(short));
	--list->count;
	return 0;
}

int xtListDRemoveAt(struct xtListD *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	// If list is the last element, then there is no need to shift.
	// This trick greatly improves performance when destroying the list from the end!
	if (index == list->count - 1)
		list->data[index] = 0;
	else
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(int));
	--list->count;
	return 0;
}

int xtListURemoveAt(struct xtListU *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	if (index == list->count - 1)
		list->data[index] = 0;
	else
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(unsigned));
	--list->count;
	return 0;
}

int xtListLURemoveAt(struct xtListLU *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	if (index == list->count - 1)
		list->data[index] = 0;
	else
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(unsigned long));
	--list->count;
	return 0;
}

int xtListZURemoveAt(struct xtListZU *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	if (index == list->count - 1)
		list->data[index] = 0;
	else
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(size_t));
	--list->count;
	return 0;
}

int xtListPRemoveAt(struct xtListP *list, size_t index)
{
	if (index >= list->count)
		return XT_EINVAL;
	if (index == list->count - 1) {
		if ((list->flags & XT_LIST_FREE_ITEM) && list->data[index])
			free(list->data[index]);
		list->data[index] = NULL;
	} else
		memmove(&list->data[index], &list->data[index + 1], (list->count - index - 1) * sizeof(void*));
	--list->count;
	return 0;
}

int xtListHDSetCapacity(struct xtListHD *list, size_t capacity)
{
	short *temp;
	if (!(temp = realloc(list->data, capacity * sizeof(short))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity = capacity;
	if (list->count > capacity)
		list->count = capacity;
	return 0;
}

int xtListDSetCapacity(struct xtListD *list, size_t capacity)
{
	int *temp;
	if (!(temp = realloc(list->data, capacity * sizeof(int))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity = capacity;
	if (list->count > capacity)
		list->count = capacity;
	return 0;
}

int xtListUSetCapacity(struct xtListU *list, size_t capacity)
{
	unsigned *temp;
	if (!(temp = realloc(list->data, capacity * sizeof(unsigned))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity = capacity;
	if (list->count > capacity)
		list->count = capacity;
	return 0;
}

int xtListLUSetCapacity(struct xtListLU *list, size_t capacity)
{
	unsigned long *temp;
	if (!(temp = realloc(list->data, capacity * sizeof(unsigned long))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity = capacity;
	if (list->count > capacity)
		list->count = capacity;
	return 0;
}

int xtListZUSetCapacity(struct xtListZU *list, size_t capacity)
{
	size_t *temp;
	if (!(temp = realloc(list->data, capacity * sizeof(size_t))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity = capacity;
	if (list->count > capacity)
		list->count = capacity;
	return 0;
}

int xtListPSetCapacity(struct xtListP *list, size_t capacity)
{
	void *temp;
	if ((list->flags & XT_LIST_FREE_ITEM) && list->count > capacity)
		for (size_t i = capacity, n = list->count; i < n; ++i)
			free(list->data[i]);
	if (!(temp = realloc(list->data, capacity * sizeof(void*))))
		return XT_ENOMEM;
	list->data = temp;
	list->capacity = capacity;
	if (list->count > capacity)
		list->count = capacity;
	return 0;
}

#define func_set_flags(type) void type ## SetFlags(struct type *t, unsigned flags) { t->flags = flags; }

func_set_flags(xtListHD)
func_set_flags(xtListD )
func_set_flags(xtListU )
func_set_flags(xtListLU)
func_set_flags(xtListZU)
func_set_flags(xtListP )

#define func_set_growth_factor(type) void type ## SetGrowthFactor(struct type *t, int grow) { t->grow = grow; }

func_set_growth_factor(xtListHD)
func_set_growth_factor(xtListD )
func_set_growth_factor(xtListU )
func_set_growth_factor(xtListLU)
func_set_growth_factor(xtListZU)
func_set_growth_factor(xtListP )

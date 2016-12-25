// XT headers
#include <xt/stack.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>

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
	short *tmp = realloc(this->data, (this->capacity + grow) * sizeof(short));
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
	int *tmp = realloc(this->data, (this->capacity + grow) * sizeof(int));
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
	unsigned *tmp = realloc(this->data, (this->capacity + grow) * sizeof(unsigned));
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
	unsigned long *tmp = realloc(this->data, (this->capacity + grow) * sizeof(unsigned long));
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
	size_t *tmp = realloc(this->data, (this->capacity + grow) * sizeof(size_t));
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
func_get_size(xtStackD )
func_get_size(xtStackU )
func_get_size(xtStackLU)
func_get_size(xtStackZU)

func_get_cap(xtStackHD)
func_get_cap(xtStackD )
func_get_cap(xtStackU )
func_get_cap(xtStackLU)
func_get_cap(xtStackZU)

/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/queue.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>

#define func_init(type) void type ## Init(struct type *t) { t->data = NULL; }
#define func_set_grow(type) void type ## SetGrowthFactor(struct type *t, int grow) { t->grow = grow; }
#define func_get_grow(type) int  type ## GetGrowthFactor(struct type *t) { return t->grow; }

func_init(xtQueueHD)
func_init(xtQueueD )
func_init(xtQueueU )
func_init(xtQueueLU)
func_init(xtQueueZU)

func_set_grow(xtQueueHD)
func_set_grow(xtQueueD)
func_set_grow(xtQueueU)
func_set_grow(xtQueueLU)
func_set_grow(xtQueueZU)

func_get_grow(xtQueueHD)
func_get_grow(xtQueueD)
func_get_grow(xtQueueU)
func_get_grow(xtQueueLU)
func_get_grow(xtQueueZU)

#define queue_init(this, data, cap) \
	this->data = data; \
	this->count = this->front = this->rear = 0; \
	this->capacity = cap; \
	this->grow = -2;

static inline int queue_create(void **data, size_t elemsize, size_t n)
{
	if (!n)
		n = XT_QUEUE_CAPACITY_DEFAULT;
	if (!(*data = malloc(n * elemsize)))
		return XT_ENOMEM;
	return 0;
}

int xtQueueHDCreate(struct xtQueueHD *this, size_t capacity)
{
	void *data;
	int ret = queue_create(&data, sizeof(short), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity);
	return 0;
}

int xtQueueDCreate(struct xtQueueD *this, size_t capacity)
{
	void *data;
	int ret = queue_create(&data, sizeof(int), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity);
	return 0;
}

int xtQueueUCreate(struct xtQueueU *this, size_t capacity)
{
	void *data;
	int ret = queue_create(&data, sizeof(unsigned), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity);
	return 0;
}

int xtQueueLUCreate(struct xtQueueLU *this, size_t capacity)
{
	void *data;
	int ret = queue_create(&data, sizeof(unsigned long), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity);
	return 0;
}

int xtQueueZUCreate(struct xtQueueZU *this, size_t capacity)
{
	void *data;
	int ret = queue_create(&data, sizeof(size_t), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity);
	return 0;
}

int xtQueueHDPush(struct xtQueueHD *this, short value)
{
	if (this->count == this->capacity) {
		if (!this->grow)
			return XT_ENOBUFS;
		size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / (unsigned)-this->grow;
		// grow at least by one
		if (!grow) ++grow;
		int ret = xtQueueHDSetCapacity(this, this->capacity + grow);
		if (ret)
			return ret;
	}
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueDPush(struct xtQueueD *this, int value)
{
	if (this->count == this->capacity) {
		if (!this->grow)
			return XT_ENOBUFS;
		size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / (unsigned)-this->grow;
		// grow at least by one
		if (!grow) ++grow;
		int ret = xtQueueDSetCapacity(this, this->capacity + grow);
		if (ret)
			return ret;
	}
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueUPush(struct xtQueueU *this, unsigned value)
{
	if (this->count == this->capacity) {
		if (!this->grow)
			return XT_ENOBUFS;
		size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / (unsigned)-this->grow;
		// grow at least by one
		if (!grow) ++grow;
		int ret = xtQueueUSetCapacity(this, this->capacity + grow);
		if (ret)
			return ret;
	}
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueLUPush(struct xtQueueLU *this, unsigned long value)
{
	if (this->count == this->capacity) {
		if (!this->grow)
			return XT_ENOBUFS;
		size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / (unsigned)-this->grow;
		// grow at least by one
		if (!grow) ++grow;
		int ret = xtQueueLUSetCapacity(this, this->capacity + grow);
		if (ret)
			return ret;
	}
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueZUPush(struct xtQueueZU *this, size_t value)
{
	if (this->count == this->capacity) {
		if (!this->grow)
			return XT_ENOBUFS;
		size_t grow = this->grow > 0 ? (unsigned)this->grow : this->capacity / (unsigned)-this->grow;
		// grow at least by one
		if (!grow) ++grow;
		int ret = xtQueueZUSetCapacity(this, this->capacity + grow);
		if (ret)
			return ret;
	}
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

bool xtQueueHDPeek(struct xtQueueHD *this, short *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	return true;
}

bool xtQueueDPeek(struct xtQueueD *this, int *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	return true;
}

bool xtQueueUPeek(struct xtQueueU *this, unsigned *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	return true;
}

bool xtQueueLUPeek(struct xtQueueLU *this, unsigned long *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	return true;
}

bool xtQueueZUPeek(struct xtQueueZU *this, size_t *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	return true;
}


#define func_free(type) void type ## Destroy(struct type *t) { if (t->data) { free(t->data); t->data = NULL; } }

func_free(xtQueueHD)
func_free(xtQueueD )
func_free(xtQueueU )
func_free(xtQueueLU)
func_free(xtQueueZU)

bool xtQueueHDPop(struct xtQueueHD *this, short *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	this->rear = (this->rear + 1) % this->capacity;
	--this->count;
	return true;
}

bool xtQueueDPop(struct xtQueueD *this, int *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	this->rear = (this->rear + 1) % this->capacity;
	--this->count;
	return true;
}

bool xtQueueUPop(struct xtQueueU *this, unsigned *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	this->rear = (this->rear + 1) % this->capacity;
	--this->count;
	return true;
}

bool xtQueueLUPop(struct xtQueueLU *this, unsigned long *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	this->rear = (this->rear + 1) % this->capacity;
	--this->count;
	return true;
}

bool xtQueueZUPop(struct xtQueueZU *this, size_t *top)
{
	if (!this->count)
		return false;
	*top = this->data[this->rear];
	this->rear = (this->rear + 1) % this->capacity;
	--this->count;
	return true;
}

size_t xtQueueHDDrop(struct xtQueueHD *this)
{
	if (!this->count)
		return 0;
	this->rear = (this->rear + 1) % this->capacity;
	return --this->count;
}

size_t xtQueueDDrop(struct xtQueueD *this)
{
	if (!this->count)
		return 0;
	this->rear = (this->rear + 1) % this->capacity;
	return --this->count;
}

size_t xtQueueUDrop(struct xtQueueU *this)
{
	if (!this->count)
		return 0;
	this->rear = (this->rear + 1) % this->capacity;
	return --this->count;
}

size_t xtQueueLUDrop(struct xtQueueLU *this)
{
	if (!this->count)
		return 0;
	this->rear = (this->rear + 1) % this->capacity;
	return --this->count;
}

size_t xtQueueZUDrop(struct xtQueueZU *this)
{
	if (!this->count)
		return 0;
	this->rear = (this->rear + 1) % this->capacity;
	return --this->count;
}

#define func_get_size(type) size_t type ## GetSize(struct type *t) { return t->count; }
#define func_get_cap(type) size_t type ## GetCapacity(struct type *t) { return t->capacity; }

func_get_size(xtQueueHD)
func_get_size(xtQueueD )
func_get_size(xtQueueU )
func_get_size(xtQueueLU)
func_get_size(xtQueueZU)

func_get_cap(xtQueueHD)
func_get_cap(xtQueueD )
func_get_cap(xtQueueU )
func_get_cap(xtQueueLU)
func_get_cap(xtQueueZU)

int xtQueueHDSetCapacity(struct xtQueueHD *queue, size_t capacity)
{
	if (queue->capacity == capacity)
		return 0;
	struct xtQueueHD new;
	int ret = xtQueueHDCreate(&new, capacity);
	if (ret)
		return ret;
	size_t j = 0, n = queue->count;
	if (n > capacity)
		n = capacity;
	for (size_t i = queue->front, cap = queue->capacity; j < n; i = (i + 1) % cap)
		new.data[j++] = queue->data[i];
	new.front = j;
	new.count = n;
	xtQueueHDDestroy(queue);
	*queue = new;
	return 0;
}

int xtQueueDSetCapacity(struct xtQueueD *queue, size_t capacity)
{
	if (queue->capacity == capacity)
		return 0;
	struct xtQueueD new;
	int ret = xtQueueDCreate(&new, capacity);
	if (ret)
		return ret;
	size_t j = 0, n = queue->count;
	if (n > capacity)
		n = capacity;
	for (size_t i = queue->front, cap = queue->capacity; j < n; i = (i + 1) % cap)
		new.data[j++] = queue->data[i];
	new.front = j;
	new.count = n;
	xtQueueDDestroy(queue);
	*queue = new;
	return 0;
}

int xtQueueUSetCapacity(struct xtQueueU *queue, size_t capacity)
{
	if (queue->capacity == capacity)
		return 0;
	struct xtQueueU new;
	int ret = xtQueueUCreate(&new, capacity);
	if (ret)
		return ret;
	size_t j = 0, n = queue->count;
	if (n > capacity)
		n = capacity;
	for (size_t i = queue->front, cap = queue->capacity; j < n; i = (i + 1) % cap)
		new.data[j++] = queue->data[i];
	new.front = j;
	new.count = n;
	xtQueueUDestroy(queue);
	*queue = new;
	return 0;
}

int xtQueueLUSetCapacity(struct xtQueueLU *queue, size_t capacity)
{
	if (queue->capacity == capacity)
		return 0;
	struct xtQueueLU new;
	int ret = xtQueueLUCreate(&new, capacity);
	if (ret)
		return ret;
	size_t j = 0, n = queue->count;
	if (n > capacity)
		n = capacity;
	for (size_t i = queue->front, cap = queue->capacity; j < n; i = (i + 1) % cap)
		new.data[j++] = queue->data[i];
	new.front = j;
	new.count = n;
	xtQueueLUDestroy(queue);
	*queue = new;
	return 0;
}

int xtQueueZUSetCapacity(struct xtQueueZU *queue, size_t capacity)
{
	if (queue->capacity == capacity)
		return 0;
	struct xtQueueZU new;
	int ret = xtQueueZUCreate(&new, capacity);
	if (ret)
		return ret;
	size_t j = 0, n = queue->count;
	if (n > capacity)
		n = capacity;
	for (size_t i = queue->front, cap = queue->capacity; j < n; i = (i + 1) % cap)
		new.data[j++] = queue->data[i];
	new.front = j;
	new.count = n;
	xtQueueZUDestroy(queue);
	*queue = new;
	return 0;
}

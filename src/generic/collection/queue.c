// XT headers
#include <xt/queue.h>
#include <xt/error.h>

// STD headers
#include <stdlib.h>

#define func_init(type) void type ## Init(struct type *t) { t->data = NULL; }

func_init(xtQueueHD)
func_init(xtQueueD )
func_init(xtQueueU )
func_init(xtQueueLU)
func_init(xtQueueZU)

static inline int xt_queue_create(void **data, size_t elemsize, size_t n)
{
	if (!n)
		n = XT_QUEUE_CAPACITY_DEFAULT;
	if (!(*data = malloc(n * elemsize)))
		return XT_ENOMEM;
	return 0;
}

#define queue_init(this, data, cap, flags) \
	this->data = data; \
	this->count = this->front = this->rear = 0; \
	this->capacity = cap; \
	this->flags = flags;

int xtQueueHDCreate(struct xtQueueHD *this, size_t capacity, unsigned flags)
{
	void *data;
	int ret = xt_queue_create(&data, sizeof(short), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity, flags);
	return 0;
}

int xtQueueDCreate(struct xtQueueD *this, size_t capacity, unsigned flags)
{
	void *data;
	int ret = xt_queue_create(&data, sizeof(int), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity, flags);
	return 0;
}

int xtQueueUCreate(struct xtQueueU *this, size_t capacity, unsigned flags)
{
	void *data;
	int ret = xt_queue_create(&data, sizeof(unsigned), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity, flags);
	return 0;
}

int xtQueueLUCreate(struct xtQueueLU *this, size_t capacity, unsigned flags)
{
	void *data;
	int ret = xt_queue_create(&data, sizeof(unsigned long), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity, flags);
	return 0;
}

int xtQueueZUCreate(struct xtQueueZU *this, size_t capacity, unsigned flags)
{
	void *data;
	int ret = xt_queue_create(&data, sizeof(size_t), capacity);
	if (ret)
		return ret;
	queue_init(this, data, capacity, flags);
	return 0;
}

int xtQueueHDPush(struct xtQueueHD *this, short value)
{
	if (this->count == this->capacity)
		return XT_EINVAL;
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueDPush(struct xtQueueD *this, int value)
{
	if (this->count == this->capacity)
		return XT_EINVAL;
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueUPush(struct xtQueueU *this, unsigned value)
{
	if (this->count == this->capacity)
		return XT_EINVAL;
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueLUPush(struct xtQueueLU *this, unsigned long value)
{
	if (this->count == this->capacity)
		return XT_EINVAL;
	this->data[this->front] = value;
	this->front = (this->front + 1) % this->capacity;
	++this->count;
	return 0;
}

int xtQueueZUPush(struct xtQueueZU *this, size_t value)
{
	if (this->count == this->capacity)
		return XT_EINVAL;
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

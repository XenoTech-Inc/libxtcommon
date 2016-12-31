/**
 * @brief Circular fixed queue
 *
 * Insert and remove are constant: O(1).
 * NOTE: all queue operations are only documented for xtQueueD,
 * since all other types (xtQueueU, etc.) are exactly the same.
 * @file queue.h
 * @author Folkert van Verseveld
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_QUEUE_H
#define _XT_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>

#define XT_QUEUE_CAPACITY_DEFAULT 256

struct xtQueueHD {
	short *data;
	size_t count, capacity, front, rear;
	int grow;
};

struct xtQueueD {
	int *data;
	size_t count, capacity, front, rear;
	int grow;
};

struct xtQueueU {
	unsigned *data;
	size_t count, capacity, front, rear;
	int grow;
};

struct xtQueueLU {
	unsigned long *data;
	size_t count, capacity, front, rear;
	int grow;
};

struct xtQueueZU {
	size_t *data;
	size_t count, capacity, front, rear;
	int grow;
};

void xtQueueHDInit(struct xtQueueHD *queue);
void xtQueueDInit (struct xtQueueD  *queue);
void xtQueueUInit (struct xtQueueU  *queue);
void xtQueueLUInit(struct xtQueueLU *queue);
void xtQueueZUInit(struct xtQueueZU *queue);

int xtQueueHDCreate(struct xtQueueHD *queue, size_t capacity);
int xtQueueDCreate (struct xtQueueD  *queue, size_t capacity);
int xtQueueUCreate (struct xtQueueU  *queue, size_t capacity);
int xtQueueLUCreate(struct xtQueueLU *queue, size_t capacity);
int xtQueueZUCreate(struct xtQueueZU *queue, size_t capacity);
/**
 * Changes grow policy. Positive values indicate fixed growth.
 * Negative values is relative growth. Zero disables growth.
 * E.g. grow(-2) is 100% / 2 = 50%, grow(-5) is 100% / 5 = 20%
 */
void xtQueueDSetGrowthFactor (struct xtQueueD  *stack, int grow);
void xtQueueHDSetGrowthFactor(struct xtQueueHD *stack, int grow);
void xtQueueUSetGrowthFactor (struct xtQueueU  *stack, int grow);
void xtQueueLUSetGrowthFactor(struct xtQueueLU *stack, int grow);
void xtQueueZUSetGrowthFactor(struct xtQueueZU *stack, int grow);
/**
 * Get grow policy. Positive values indicate fixed growth.
 * Negative values is relative growth. Zero disables growth.
 * E.g. grow(-2) is 100% / 2 = 50%, grow(-5) is 100% / 5 = 20%
 */
int xtQueueDGetGrowthFactor (struct xtQueueD  *stack);
int xtQueueHDGetGrowthFactor(struct xtQueueHD *stack);
int xtQueueUGetGrowthFactor (struct xtQueueU  *stack);
int xtQueueLUGetGrowthFactor(struct xtQueueLU *stack);
int xtQueueZUGetGrowthFactor(struct xtQueueZU *stack);

int xtQueueHDPush(struct xtQueueHD *queue, short value);
int xtQueueDPush (struct xtQueueD  *queue, int value);
int xtQueueUPush (struct xtQueueU  *queue, unsigned value);
int xtQueueLUPush(struct xtQueueLU *queue, unsigned long value);
int xtQueueZUPush(struct xtQueueZU *queue, size_t value);

bool xtQueueHDPeek(struct xtQueueHD *queue, short *top);
bool xtQueueDPeek (struct xtQueueD  *queue, int *top);
bool xtQueueUPeek (struct xtQueueU  *queue, unsigned *top);
bool xtQueueLUPeek(struct xtQueueLU *queue, unsigned long *top);
bool xtQueueZUPeek(struct xtQueueZU *queue, size_t *top);

bool xtQueueHDPop(struct xtQueueHD *queue, short *top);
bool xtQueueDPop (struct xtQueueD  *queue, int *top);
bool xtQueueUPop (struct xtQueueU  *queue, unsigned *top);
bool xtQueueLUPop(struct xtQueueLU *queue, unsigned long *top);
bool xtQueueZUPop(struct xtQueueZU *queue, size_t *top);

void xtQueueHDDestroy(struct xtQueueHD *queue);
void xtQueueDDestroy (struct xtQueueD  *queue);
void xtQueueUDestroy (struct xtQueueU  *queue);
void xtQueueLUDestroy(struct xtQueueLU *queue);
void xtQueueZUDestroy(struct xtQueueZU *queue);

size_t xtQueueHDGetSize(struct xtQueueHD *queue);
size_t xtQueueDGetSize (struct xtQueueD  *queue);
size_t xtQueueUGetSize (struct xtQueueU  *queue);
size_t xtQueueLUGetSize(struct xtQueueLU *queue);
size_t xtQueueZUGetSize(struct xtQueueZU *queue);

size_t xtQueueHDGetCapacity(struct xtQueueHD *queue);
size_t xtQueueDGetCapacity (struct xtQueueD  *queue);
size_t xtQueueUGetCapacity (struct xtQueueU  *queue);
size_t xtQueueLUGetCapacity(struct xtQueueLU *queue);
size_t xtQueueZUGetCapacity(struct xtQueueZU *queue);

int xtQueueHDSetCapacity(struct xtQueueHD *queue, size_t capacity);
int xtQueueDSetCapacity (struct xtQueueD  *queue, size_t capacity);
int xtQueueUSetCapacity (struct xtQueueU  *queue, size_t capacity);
int xtQueueLUSetCapacity(struct xtQueueLU *queue, size_t capacity);
int xtQueueZUSetCapacity(struct xtQueueZU *queue, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif

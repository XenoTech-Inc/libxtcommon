/**
 * @brief First in first out stack container
 *
 * NOTE: all stack operations are only documented for xtStackD,
 * since all other types (xtStackU, etc.) are exactly the same.
 * @file stack.h
 * @author Folkert van Verseveld
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_STACK_H
#define _XT_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>

#define XT_STACK_CAPACITY_DEFAULT 256

struct xtStackHD {
	short *data;
	size_t count, capacity;
	int grow;
};

struct xtStackD {
	int *data;
	size_t count, capacity;
	int grow;
};

struct xtStackU {
	unsigned *data;
	size_t count, capacity;
	int grow;
};

struct xtStackLU {
	unsigned long *data;
	size_t count, capacity;
	int grow;
};

struct xtStackZU {
	size_t *data;
	size_t count, capacity;
	int grow;
};

void xtStackHDInit(struct xtStackHD *stack);
void xtStackDInit (struct xtStackD  *stack);
void xtStackUInit (struct xtStackU  *stack);
void xtStackLUInit(struct xtStackLU *stack);
void xtStackZUInit(struct xtStackZU *stack);

void xtStackHDDestroy(struct xtStackHD *stack);
void xtStackDDestroy (struct xtStackD  *stack);
void xtStackUDestroy (struct xtStackU  *stack);
void xtStackLUDestroy(struct xtStackLU *stack);
void xtStackZUDestroy(struct xtStackZU *stack);
/**
 * Creates a new stack. By default, automatic growth is enabled.
 * @param capacity Initial capacity (specify zero to use default value).
 * @return Zero if the stack has been created successfully, otherwise an error code.
 */
int xtStackDCreate (struct xtStackD  *stack, size_t capacity);
int xtStackHDCreate(struct xtStackHD *stack, size_t capacity);
int xtStackUCreate (struct xtStackU  *stack, size_t capacity);
int xtStackLUCreate(struct xtStackLU *stack, size_t capacity);
int xtStackZUCreate(struct xtStackZU *stack, size_t capacity);
/**
 * Changes grow policy. Positive values indicate fixed growth.
 * Negative values is relative growth. Zero disables growth.
 * E.g. grow(-2) is 100% / 2 = 50%, grow(-5) is 100% / 5 = 20%
 */
void xtStackDSetGrowthFactor (struct xtStackD  *stack, int grow);
void xtStackHDSetGrowthFactor(struct xtStackHD *stack, int grow);
void xtStackUSetGrowthFactor (struct xtStackU  *stack, int grow);
void xtStackLUSetGrowthFactor(struct xtStackLU *stack, int grow);
void xtStackZUSetGrowthFactor(struct xtStackZU *stack, int grow);
/**
 * Get grow policy. Positive values indicate fixed growth.
 * Negative values is relative growth. Zero disables growth.
 * E.g. grow(-2) is 100% / 2 = 50%, grow(-5) is 100% / 5 = 20%
 */
int xtStackDGetGrowthFactor (struct xtStackD  *stack);
int xtStackHDGetGrowthFactor(struct xtStackHD *stack);
int xtStackUGetGrowthFactor (struct xtStackU  *stack);
int xtStackLUGetGrowthFactor(struct xtStackLU *stack);
int xtStackZUGetGrowthFactor(struct xtStackZU *stack);
/**
 * Attempts to add \a value to stack.
 * The list will grow if configured to do so.
 * @return Zero if \a value has been added, otherwise an error code.
 */
int xtStackDPush (struct xtStackD  *stack, int value);
int xtStackHDPush(struct xtStackHD *stack, short value);
int xtStackUPush (struct xtStackU  *stack, unsigned value);
int xtStackLUPush(struct xtStackLU *stack, unsigned long value);
int xtStackZUPush(struct xtStackZU *stack, size_t value);
/**
 * Fetches last pushed item. Fails if stack is empty.
 */
bool xtStackDPeek (struct xtStackD  *stack, int *top);
bool xtStackHDPeek(struct xtStackHD *stack, short *top);
bool xtStackUPeek (struct xtStackU  *stack, unsigned *top);
bool xtStackLUPeek(struct xtStackLU *stack, unsigned long *top);
bool xtStackZUPeek(struct xtStackZU *stack, size_t *top);
/**
 * Free elements or do nothing if NULL has been specified.
 */
void xtStackDDestroy (struct xtStackD  *stack);
void xtStackHDDestroy(struct xtStackHD *stack);
void xtStackUDestroy (struct xtStackU  *stack);
void xtStackLUDestroy(struct xtStackLU *stack);
void xtStackZUDestroy(struct xtStackZU *stack);
/**
 * Stores and removes last pushed item. Fails if stack is empty.
 */
bool xtStackDPop (struct xtStackD  *stack, int *top);
bool xtStackHDPop(struct xtStackHD *stack, short *top);
bool xtStackUPop (struct xtStackU  *stack, unsigned *top);
bool xtStackLUPop(struct xtStackLU *stack, unsigned long *top);
bool xtStackZUPop(struct xtStackZU *stack, size_t *top);
/**
 * Removes last pushed item and return new count.
 * Fails if stack is empty (i.e. returns zero).
 */
size_t xtStackDDrop (struct xtStackD  *stack);
size_t xtStackHDDrop(struct xtStackHD *stack);
size_t xtStackUDrop (struct xtStackU  *stack);
size_t xtStackLUDrop(struct xtStackLU *stack);
size_t xtStackZUDrop(struct xtStackZU *stack);

size_t xtStackHDGetSize(struct xtStackHD *stack);
size_t xtStackDGetSize (struct xtStackD  *stack);
size_t xtStackUGetSize (struct xtStackU  *stack);
size_t xtStackLUGetSize(struct xtStackLU *stack);
size_t xtStackZUGetSize(struct xtStackZU *stack);

int xtStackHDSetCapacity(struct xtStackHD *stack, size_t capacity);
int xtStackDSetCapacity (struct xtStackD  *stack, size_t capacity);
int xtStackUSetCapacity (struct xtStackU  *stack, size_t capacity);
int xtStackLUSetCapacity(struct xtStackLU *stack, size_t capacity);
int xtStackZUSetCapacity(struct xtStackZU *stack, size_t capacity);

size_t xtStackHDGetCapacity(struct xtStackHD *stack);
size_t xtStackDGetCapacity (struct xtStackD  *stack);
size_t xtStackUGetCapacity (struct xtStackU  *stack);
size_t xtStackLUGetCapacity(struct xtStackLU *stack);
size_t xtStackZUGetCapacity(struct xtStackZU *stack);

#ifdef __cplusplus
}
#endif

#endif

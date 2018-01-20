/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/math.h>

bool xtMathIsPow2(uint64_t value)
{
	return value >= 2 && !(value & (value - 1));
}

void xtMathMemSplit(unsigned long long len, unsigned chunkSize, unsigned *restrict chunks, unsigned *restrict lastChunkSize)
{
	unsigned long long amountChunks = len / chunkSize;
	// Whatever is left for the last chunk
	unsigned long long remainder = len - (amountChunks * chunkSize);
	// Add a last chunk if the remainder is bigger than zero
	if (remainder > 0)
		++amountChunks;
	if (lastChunkSize)
		*lastChunkSize = remainder;
	if (chunks)
		*chunks = amountChunks - 1;
}

uint64_t xtMathNextPow2(uint64_t value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	return ++value;
}

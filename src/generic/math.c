// XT headers
#include <xt/math.h>

bool xtMathIsPow2(uint64_t v)
{
	return v >= 2 && !(v & (v - 1));
}

uint64_t xtMathNextPow2(uint64_t v)
{
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	return ++v;
}

void xtMathMemSplit(unsigned long long len, unsigned chunkSize, unsigned *chunks, unsigned *lastChunkSize)
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

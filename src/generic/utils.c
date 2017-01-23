// XT headers
#include <xt/utils.h>

// STD headers
#include <stdio.h>
#include <stdlib.h>

void xtDie(const char *msg)
{
	fprintf(stderr, "%s", msg);
	fflush(stderr);
	exit(EXIT_FAILURE);
}

void xtDieOn(bool cond, const char *msg)
{
	if (!cond)
		xtDie(msg);
}

uint64_t xtRandLLU(void)
{
	return (((uint64_t) (rand() + rand()) <<  0) & 0x000000000000FFFFLLU) |
		   (((uint64_t) (rand() + rand()) << 16) & 0x00000000FFFF0000LLU) |
		   (((uint64_t) (rand() + rand()) << 32) & 0x0000FFFF00000000LLU) |
		   (((uint64_t) (rand() + rand()) << 48) & 0xFFFF000000000000LLU);
}

void xtSRand(unsigned seed)
{
	srand(seed);
}

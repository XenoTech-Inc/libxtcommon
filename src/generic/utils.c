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

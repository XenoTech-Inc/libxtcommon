#include <xt/stack.h>
#include <xt/error.h>
#include <xt/os.h>
#include <xt/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(void)
{
	struct xtStackHD sthd;
	struct xtStackD  std ;
	struct xtStackU  stu ;
	struct xtStackLU stlu;
	struct xtStackZU stzu;
	xtStackHDInit(&sthd);
	xtStackDInit (&std );
	xtStackUInit (&stu );
	xtStackLUInit(&stlu);
	xtStackZUInit(&stzu);
	xtStackZUDestroy(&stzu);
	xtStackLUDestroy(&stlu);
	xtStackUDestroy (&stu );
	xtStackDDestroy (&std );
	xtStackHDDestroy(&sthd);
}

int push(void)
{
#define LISTSZ 666
#define STACKSZ 100
	int ret = 1;
	struct xtStackHD sthd;
	struct xtStackD  std ;
	struct xtStackU  stu ;
	struct xtStackLU stlu;
	struct xtStackZU stzu;
	xtStackHDInit(&sthd);
	xtStackDInit (&std );
	xtStackUInit (&stu );
	xtStackLUInit(&stlu);
	xtStackZUInit(&stzu);
	// create lists
	short         *listhd = NULL;
	int           *listd  = NULL;
	unsigned      *listu  = NULL;
	unsigned long *listlu = NULL;
	size_t        *listzu = NULL;
	if (!(listhd = malloc(LISTSZ * sizeof(short))))
		goto fail;
	if (!(listd  = malloc(LISTSZ * sizeof(int))))
		goto fail;
	if (!(listu  = malloc(LISTSZ * sizeof(unsigned))))
		goto fail;
	if (!(listlu = malloc(LISTSZ * sizeof(unsigned long))))
		goto fail;
	if (!(listzu = malloc(LISTSZ * sizeof(size_t))))
		goto fail;
	// create random data
	for (unsigned i = 0; i < LISTSZ; ++i) {
		listhd[i] = rand();
		listd [i] = rand();
		listu [i] = rand();
		listlu[i] = rand();
		listzu[i] = rand();
	}
	puts("Creating stacks");
	if (xtStackHDCreate(&sthd, STACKSZ)) {
		fputs("Failed to create StackHD\n", stderr);
		goto fail;
	}
	if (xtStackDCreate(&std, STACKSZ)) {
		fputs("Failed to create StackD\n", stderr);
		goto fail;
	}
	if (xtStackUCreate(&stu, STACKSZ)) {
		fputs("Failed to create StackU\n", stderr);
		goto fail;
	}
	if (xtStackLUCreate(&stlu, STACKSZ)) {
		fputs("Failed to create StackLU\n", stderr);
		goto fail;
	}
	if (xtStackZUCreate(&stzu, STACKSZ)) {
		fputs("Failed to create StackZU\n", stderr);
		goto fail;
	}
	puts("Pushing random data");
	for (unsigned i = 0; i < LISTSZ; ++i) {
		if (xtStackHDPush(&sthd, listhd[i])) {
			xtfprintf(stderr, "Failed to push %hd to StackHD\n", listhd[i]);
			goto fail;
		}
		if (xtStackDPush(&std, listd[i])) {
			xtfprintf(stderr, "Failed to push %d to StackD\n", listd[i]);
			goto fail;
		}
		if (xtStackUPush(&stu, listu[i])) {
			xtfprintf(stderr, "Failed to push %u to StackU\n", listu[i]);
			goto fail;
		}
		if (xtStackLUPush(&stlu, listlu[i])) {
			xtfprintf(stderr, "Failed to push %lu to StackLU\n", listlu[i]);
			goto fail;
		}
		if (xtStackZUPush(&stzu, listzu[i])) {
			xtfprintf(stderr, "Failed to push %zu to StackZU\n", listzu[i]);
			goto fail;
		}
	}
	puts("Verifying random data");
	for (unsigned i = 0; i < LISTSZ; ++i) {
		short hd;
		int d;
		unsigned u;
		unsigned long lu;
		size_t zu;
		if (!xtStackHDPop(&sthd, &hd)) {
			fputs("StackHD is broken\n", stderr);
			goto fail;
		}
		if (hd != listhd[LISTSZ - i - 1]) {
			fputs("StackHD is corrupted\n", stderr);
			xtfprintf(stderr, "Got %hd but expected %hd\n", hd, listhd[i]);
			goto fail;
		}
		if (!xtStackDPop(&std, &d)) {
			fputs("StackD is broken\n", stderr);
			goto fail;
		}
		if (d != listd[LISTSZ - i - 1]) {
			fputs("StackD is corrupted\n", stderr);
			xtfprintf(stderr, "Got %d but expected %d\n", d, listd[i]);
			goto fail;
		}
		if (!xtStackUPop(&stu, &u)) {
			fputs("StackU is broken\n", stderr);
			goto fail;
		}
		if (u != listu[LISTSZ - i - 1]) {
			fputs("StackU is corrupted\n", stderr);
			xtfprintf(stderr, "got %u but expected %u\n", u, listu[i]);
			goto fail;
		}
		if (!xtStackLUPop(&stlu, &lu)) {
			fputs("StackLU is broken\n", stderr);
			goto fail;
		}
		if (lu != listlu[LISTSZ - i - 1]) {
			fputs("StackLU is corrupted\n", stderr);
			xtfprintf(stderr, "got %lu but expected %lu\n", lu, listlu[i]);
			goto fail;
		}
		if (!xtStackZUPop(&stzu, &zu)) {
			fputs("StackZU is broken\n", stderr);
			goto fail;
		}
		if (zu != listzu[LISTSZ - i - 1]) {
			fputs("StackZU is corrupted\n", stderr);
			xtfprintf(stderr, "got %zu but expected %zu\n", zu, listzu[i]);
			goto fail;
		}
	}
	ret = 0;
fail:
	puts("Cleanup");
	if (listzu)
		free(listzu);
	if (listlu)
		free(listlu);
	if (listu)
		free(listu);
	if (listd)
		free(listd);
	if (listhd)
		free(listhd);
	xtStackHDDestroy(&sthd);
	xtStackDDestroy (&std );
	xtStackUDestroy (&stu );
	xtStackLUDestroy(&stlu);
	xtStackZUDestroy(&stzu);
	return ret;
}

int main(void)
{
	srand(time(NULL));
	xtConsoleFillLine("-");
	puts("-- STACK TEST");
	puts("Initialize all different types");
	init();
	xtConsoleFillLine("-");
	puts("-- PUSH TEST");
	if (push()) {
		xtConsoleFillLine("!");
		puts("Push test failed");
		return 1;
	}
	return 0;
}

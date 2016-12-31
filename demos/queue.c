#include <xt/queue.h>
#include <xt/error.h>
#include <xt/os.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(void)
{
	struct xtQueueHD qhd;
	struct xtQueueD  qd ;
	struct xtQueueU  qu ;
	struct xtQueueLU qlu;
	struct xtQueueZU qzu;
	xtQueueHDInit(&qhd);
	xtQueueDInit (&qd );
	xtQueueUInit (&qu );
	xtQueueLUInit(&qlu);
	xtQueueZUInit(&qzu);
	xtQueueZUDestroy(&qzu);
	xtQueueLUDestroy(&qlu);
	xtQueueUDestroy (&qu );
	xtQueueDDestroy (&qd );
	xtQueueHDDestroy(&qhd);
}

int push(void)
{
#define LISTSZ 666
#define QUEUESZ 100
	int ret = 1;
	struct xtQueueHD qhd;
	struct xtQueueD  qd ;
	struct xtQueueU  qu ;
	struct xtQueueLU qlu;
	struct xtQueueZU qzu;
	xtQueueHDInit(&qhd);
	xtQueueDInit (&qd );
	xtQueueUInit (&qu );
	xtQueueLUInit(&qlu);
	xtQueueZUInit(&qzu);
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
	puts("Creating queues");
	if (xtQueueHDCreate(&qhd, QUEUESZ)) {
		fputs("Failed to create QueueHD\n", stderr);
		goto fail;
	}
	if (xtQueueDCreate(&qd, QUEUESZ)) {
		fputs("Failed to create QueueD\n", stderr);
		goto fail;
	}
	if (xtQueueUCreate(&qu, QUEUESZ)) {
		fputs("Failed to create QueueU\n", stderr);
		goto fail;
	}
	if (xtQueueLUCreate(&qlu, QUEUESZ)) {
		fputs("Failed to create QueueLU\n", stderr);
		goto fail;
	}
	if (xtQueueZUCreate(&qzu, QUEUESZ)) {
		fputs("Failed to create QueueZU\n", stderr);
		goto fail;
	}
	puts("Pushing random data");
	for (unsigned i = 0; i < LISTSZ; ++i) {
		if (xtQueueHDPush(&qhd, listhd[i])) {
			fprintf(stderr, "Failed to push %hd to QueueHD\n", listhd[i]);
			goto fail;
		}
		if (xtQueueDPush(&qd, listd[i])) {
			fprintf(stderr, "Failed to push %d to QueueD\n", listd[i]);
			goto fail;
		}
		if (xtQueueUPush(&qu, listu[i])) {
			fprintf(stderr, "Failed to push %u to QueueU\n", listu[i]);
			goto fail;
		}
		if (xtQueueLUPush(&qlu, listlu[i])) {
			fprintf(stderr, "Failed to push %lu to QueueLU\n", listlu[i]);
			goto fail;
		}
		if (xtQueueZUPush(&qzu, listzu[i])) {
			fprintf(stderr, "Failed to push %zu to QueueZU\n", listzu[i]);
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
		if (!xtQueueHDPop(&qhd, &hd)) {
			fputs("QueueHD is broken\n", stderr);
			goto fail;
		}
		if (hd != listhd[i]) {
			fputs("QueueHD is corrupted\n", stderr);
			fprintf(stderr, "Got %hd but expected %hd\n", hd, listhd[i]);
			goto fail;
		}
		if (!xtQueueDPop(&qd, &d)) {
			fputs("QueueD is broken\n", stderr);
			goto fail;
		}
		if (d != listd[i]) {
			fputs("QueueD is corrupted\n", stderr);
			fprintf(stderr, "Got %d but expected %d\n", d, listd[i]);
			goto fail;
		}
		if (!xtQueueUPop(&qu, &u)) {
			fputs("QueueU is broken\n", stderr);
			goto fail;
		}
		if (u != listu[i]) {
			fputs("QueueU is corrupted\n", stderr);
			fprintf(stderr, "got %u but expected %u\n", u, listu[i]);
			goto fail;
		}
		if (!xtQueueLUPop(&qlu, &lu)) {
			fputs("QueueLU is broken\n", stderr);
			goto fail;
		}
		if (lu != listlu[i]) {
			fputs("QueueLU is corrupted\n", stderr);
			fprintf(stderr, "got %lu but expected %lu\n", lu, listlu[i]);
			goto fail;
		}
		if (!xtQueueZUPop(&qzu, &zu)) {
			fputs("QueueZU is broken\n", stderr);
			goto fail;
		}
		if (zu != listzu[i]) {
			fputs("QueueZU is corrupted\n", stderr);
			fprintf(stderr, "got %zu but expected %zu\n", zu, listzu[i]);
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
	xtQueueHDDestroy(&qhd);
	xtQueueDDestroy (&qd );
	xtQueueUDestroy (&qu );
	xtQueueLUDestroy(&qlu);
	xtQueueZUDestroy(&qzu);
	return ret;
}

int main(void)
{
	srand(time(NULL));
	xtConsoleFillLine("-");
	puts("-- QUEUE TEST");
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

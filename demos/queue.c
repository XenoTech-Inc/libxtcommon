/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/queue.h>
#include <xt/error.h>
#include <xt/os.h>
#include <xt/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

static struct stats stats;

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
	PASS("xtQueue*Init()");
	xtQueueZUDestroy(&qzu);
	xtQueueLUDestroy(&qlu);
	xtQueueUDestroy (&qu );
	xtQueueDDestroy (&qd );
	xtQueueHDDestroy(&qhd);
	PASS("xtQueue*Destroy()");
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
#define INIT_HD  1
#define INIT_D   2
#define INIT_U   4
#define INIT_LU  8
#define INIT_ZU 16
	unsigned init = 0;
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
	char buf[256];
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
	if (!xtQueueHDCreate(&qhd, QUEUESZ)) {
		init |= INIT_HD;
		PASS("xtQueueHDCreate()");
	} else
		FAIL("xtQueueHDCreate()");
	if (!xtQueueDCreate(&qd, QUEUESZ)) {
		init |= INIT_D;
		PASS("xtQueueDCreate()");
	} else
		FAIL("xtQueueDCreate()");
	if (!xtQueueUCreate(&qu, QUEUESZ)) {
		init |= INIT_U;
		PASS("xtQueueUCreate()");
	} else
		FAIL("xtQueueUCreate()");
	if (!xtQueueLUCreate(&qlu, QUEUESZ)) {
		init |= INIT_LU;
		PASS("xtQueueLUCreate()");
	} else
		FAIL("xtQueueLUCreate()");
	if (!xtQueueZUCreate(&qzu, QUEUESZ)) {
		init |= INIT_ZU;
		PASS("xtQueueZUCreate()");
	} else
		FAIL("xtQueueZUCreate()");
	puts("Pushing random data");
	if (init & INIT_HD) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtQueueHDPush(&qhd, listhd[i])) {
				xtsnprintf(buf, sizeof buf, "xtQueueHDPush(%hd)", listhd[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtQueueHDPush()");
	}
	if (init & INIT_D) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtQueueDPush(&qd, listd[i])) {
				xtsnprintf(buf, sizeof buf, "xtQueueDPush(%d)", listd[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtQueueDPush()");
	}
	if (init & INIT_U) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtQueueUPush(&qu, listu[i])) {
				xtsnprintf(buf, sizeof buf, "xtQueueUPush(%u)", listu[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtQueueUPush()");
	}
	if (init & INIT_LU) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtQueueLUPush(&qlu, listlu[i])) {
				xtsnprintf(buf, sizeof buf, "xtQueueLUPush(%lu)", listlu[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtQueueLUPush()");
	}
	if (init & INIT_ZU) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtQueueZUPush(&qzu, listzu[i])) {
				xtsnprintf(buf, sizeof buf, "xtQueueZUPush(%zu)", listzu[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtQueueZUPush()");
	}
	puts("Verifying random data");
	if (init & INIT_HD) {
		unsigned i;
		short hd;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtQueueHDPop(&qhd, &hd)) {
				FAIL("xtQueueHDPop()");
				break;
			}
			if (hd != listhd[i]) {
				xtsnprintf(buf, sizeof buf, "xtQueueHDPop() - Got %hd but expected %hd", hd, listhd[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtQueueHDPop()");
	}
	if (init & INIT_D) {
		unsigned i;
		int d;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtQueueDPop(&qd, &d)) {
				FAIL("xtQueueDPop()");
				break;
			}
			if (d != listd[i]) {
				xtsnprintf(buf, sizeof buf, "xtQueueDPop() - Got %d but expected %d", d, listd[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtQueueDPop()");
	}
	if (init & INIT_U) {
		unsigned i, u;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtQueueUPop(&qu, &u)) {
				FAIL("xtQueueUPop()");
				break;
			}
			if (u != listu[i]) {
				xtsnprintf(buf, sizeof buf, "xtQueueUPop() - Got %u but expected %u", u, listu[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtQueueUPop()");
	}
	if (init & INIT_LU) {
		unsigned i;
		unsigned long lu;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtQueueLUPop(&qlu, &lu)) {
				FAIL("xtQueueLUPop()");
				break;
			}
			if (lu != listlu[i]) {
				xtsnprintf(buf, sizeof buf, "xtQueueLUPop() - Got %lu but expected %lu", lu, listlu[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtQueueLUPop()");
	}
	if (init & INIT_ZU) {
		unsigned i;
		size_t zu;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtQueueZUPop(&qzu, &zu)) {
				FAIL("xtQueueZUPop()");
				break;
			}
			if (zu != listzu[i]) {
				xtsnprintf(buf, sizeof buf, "xtQueueZUPop() - Got %zu but expected %zu", zu, listzu[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtQueueZUPop()");
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
	stats_init(&stats, "queue");
	srand(time(NULL));
	puts("-- QUEUE TEST");
	puts("Initialize all different types");
	init();
	push();
	stats_info(&stats);
	return stats_status(&stats);
}

/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/stack.h>
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
	PASS("xtStack*Init()");
	xtStackZUDestroy(&stzu);
	xtStackLUDestroy(&stlu);
	xtStackUDestroy (&stu );
	xtStackDDestroy (&std );
	xtStackHDDestroy(&sthd);
	PASS("xtStack*Destroy()");
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
#define INIT_HD  1
#define INIT_D   2
#define INIT_U   4
#define INIT_LU  8
#define INIT_ZU 16
	unsigned init = 0;
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
	puts("Creating stacks");
	if (!xtStackHDCreate(&sthd, STACKSZ)) {
		init |= INIT_HD;
		PASS("xtStackHDCreate()");
	} else
		FAIL("xtStackHDCreate()");
	if (!xtStackDCreate(&std, STACKSZ)) {
		init |= INIT_D;
		PASS("xtStackDCreate()");
	} else
		FAIL("xtStackDCreate()");
	if (!xtStackUCreate(&stu, STACKSZ)) {
		init |= INIT_U;
		PASS("xtStackUCreate()");
	} else
		FAIL("xtStackUCreate()");
	if (!xtStackLUCreate(&stlu, STACKSZ)) {
		init |= INIT_LU;
		PASS("xtStackLUCreate()");
	} else
		FAIL("xtStackLUCreate()");
	if (!xtStackZUCreate(&stzu, STACKSZ)) {
		init |= INIT_ZU;
		PASS("xtStackZUCreate()");
	} else
		FAIL("xtStackZUCreate()");
	puts("Pushing random data");
	if (init & INIT_HD) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtStackHDPush(&sthd, listhd[i])) {
				xtsnprintf(buf, sizeof buf, "xtStackHDPush(%hd)", listhd[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtStackHDPush()");
	}
	if (init & INIT_D) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtStackDPush(&std, listd[i])) {
				xtsnprintf(buf, sizeof buf, "xtStackDPush(%d)", listd[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtStackDPush()");
	}
	if (init & INIT_U) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtStackUPush(&stu, listu[i])) {
				xtsnprintf(buf, sizeof buf, "xtStackUPush(%u)", listu[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtStackUPush()");
	}
	if (init & INIT_LU) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtStackLUPush(&stlu, listlu[i])) {
				xtsnprintf(buf, sizeof buf, "xtStackLUPush(%lu)", listlu[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtStackLUPush()");
	}
	if (init & INIT_ZU) {
		unsigned i;
		for (i = 0; i < LISTSZ; ++i)
			if (xtStackZUPush(&stzu, listzu[i])) {
				xtsnprintf(buf, sizeof buf, "xtStackZUPush(%zu)", listzu[i]);
				FAIL(buf);
				break;
			}
		if (i == LISTSZ)
			PASS("xtStackZUPush()");
	}
	puts("Verifying random data");
	if (init & INIT_HD) {
		unsigned i;
		short hd;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtStackHDPop(&sthd, &hd)) {
				FAIL("xtStackHDPop()");
				break;
			}
			if (hd != listhd[LISTSZ - i - 1]) {
				xtsnprintf(buf, sizeof buf, "xtStackHDPop() - Got %hd but expected %hd", hd, listhd[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtStackHDPop()");
	}
	if (init & INIT_D) {
		unsigned i;
		int d;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtStackDPop(&std, &d)) {
				FAIL("xtStackDPop()");
				break;
			}
			if (d != listd[LISTSZ - i - 1]) {
				xtsnprintf(buf, sizeof buf, "xtStackDPop() - Got %d but expected %d", d, listd[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtStackDPop()");
	}
	if (init & INIT_U) {
		unsigned i, u;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtStackUPop(&stu, &u)) {
				FAIL("xtStackUPop()");
				break;
			}
			if (u != listu[LISTSZ - i - 1]) {
				xtsnprintf(buf, sizeof buf, "xtStackUPop() - Got %u but expected %u", u, listu[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtStackUPop()");
	}
	if (init & INIT_LU) {
		unsigned i;
		unsigned long lu;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtStackLUPop(&stlu, &lu)) {
				FAIL("xtStackLUPop()");
				break;
			}
			if (lu != listlu[LISTSZ - i - 1]) {
				xtsnprintf(buf, sizeof buf, "xtStackLUPop() - Got %lu but expected %lu", lu, listlu[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtStackLUPop()");
	}
	if (init & INIT_ZU) {
		unsigned i;
		size_t zu;
		for (i = 0; i < LISTSZ; ++i) {
			if (!xtStackZUPop(&stzu, &zu)) {
				FAIL("xtStackZUPop()");
				break;
			}
			if (zu != listzu[LISTSZ - i - 1]) {
				xtsnprintf(buf, sizeof buf, "xtStackZUPop() - Got %zu but expected %zu", zu, listzu[i]);
				FAIL(buf);
				break;
			}
		}
		if (i == LISTSZ)
			PASS("xtStackZUPop()");
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
	stats_init(&stats, "stack");
	srand(time(NULL));
	puts("-- STACK TEST");
	puts("Initialize all different types");
	init();
	push();
	stats_info(&stats);
	return stats_status(&stats);
}

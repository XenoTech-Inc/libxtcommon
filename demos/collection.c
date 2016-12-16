#include <xt/stack.h>
#include <xt/error.h>
#include <stdio.h>

int main(void)
{
	int ret = 1;
	struct xtStackD st;
	xtStackDInit(&st);
	if ((ret = xtStackDCreate(&st, 4))) {
		fprintf(stderr, "create failed: %s\n", xtGetErrorStr(ret));
		goto fail;
	}
	xtStackDPush(&st, -9);
	xtStackDPush(&st, 43);
	xtStackDPush(&st, 21);
	xtStackDPush(&st, -2);
	while (xtStackDGetSize(&st)) {
		int top;
		xtStackDPop(&st, &top);
		printf(" %d", top);
	}
	xtStackDPush(&st, 100);
	xtStackDPush(&st, 200);
	xtStackDPush(&st, 300);
	xtStackDPush(&st, 400);
	xtStackDPush(&st, 500);
	putchar('\n');
	printf("cap: %zu, grow: %d\n", xtStackDGetCapacity(&st), xtStackDGetGrow(&st));
	ret = 0;
fail:
	xtStackDDestroy(&st);
	return ret;
}

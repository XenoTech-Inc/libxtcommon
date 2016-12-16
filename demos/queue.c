#include <xt/queue.h>
#include <xt/error.h>
#include <stdio.h>

int main(void)
{
	puts("--------------------------------------------------------------------------------\n-- QUEUE TEST");
	const int data[] = { 4, -20, 307, 5, -1, 1520 };
	int ret = 1;
	struct xtQueueD q;
	xtQueueDInit(&q);
	if ((ret = xtQueueDCreate(&q, sizeof(data) / sizeof(data[0]), 0))) {
		fprintf(stderr, "Queue creation failed: %s\n", xtGetErrorStr(ret));
		goto fail;
	}
	for (unsigned i = 0; i < sizeof(data) / sizeof(data[0]); ++i)
		printf("Pushing number %d,\tretval: %d\n", data[i], xtQueueDPush(&q, data[i]));
	while (xtQueueDGetSize(&q)) {
		int top;
		ret = xtQueueDPop(&q, &top);
		printf("Popping, value: %d,\tretval: %d\n", top, ret);
	}
	ret = 0;
fail:
	xtQueueDDestroy(&q);
	return ret;
}

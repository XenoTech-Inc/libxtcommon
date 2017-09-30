#include <xt/string.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TEST_SIZE (64*1048576)

static size_t xtBase64GetEncodedSize(size_t buflen)
{
	return 4 * ((buflen + 2) / 3);
}

static size_t xtBase64GetDecodedSize(const void *buf, size_t buflen)
{
	unsigned const char *xbuf = buf;
	size_t outputlen = buflen / 4 * 3; // XXX should be * 3 / 4 ?
	if (xbuf[buflen - 1] == '=')
		--outputlen;
	if (xbuf[buflen - 2] == '=')
		--outputlen;
	return outputlen;
}

int main(void)
{
	char sbuf[256];
	char *data = NULL, *data2 = NULL;
	char *dest = NULL;
	size_t destSize = xtBase64GetEncodedSize(TEST_SIZE) + 1;
	int ret = 1;
	struct xtTimestamp start, end;

	srand(time(NULL));

	data = malloc(TEST_SIZE);
	if (!data)
		goto fail;
	data2 = malloc(destSize);
	if (!data2)
		goto fail;
	dest = malloc(destSize);
	if (!data)
		goto fail;

	/* fill data with random shit */
	for (size_t i = 0; i < TEST_SIZE; ++i)
		data[i] = rand();

	xtClockGetTime(&start, XT_CLOCK_MONOTONIC);
	/* Encode data and Decode back */
	xtBase64Encode(dest, destSize, data, TEST_SIZE);
	xtBase64Decode(data2, destSize, dest, destSize);

	xtClockGetTime(&end, XT_CLOCK_MONOTONIC);

	xtFormatTimeDuration(sbuf, sizeof sbuf, "%G", &start, &end);

	printf("Elapsed time: %s\n", sbuf);

	int good = 1;
	for (size_t i = 0; i < TEST_SIZE; ++i)
		if (data2[i] != data[i]) {
			good = 0;
			break;
		}

	if (good)
		puts("Success");
	else
		fputs("Fail\n", stderr);

	ret = 0;
fail:
	if (dest)
		free(dest);
	if (data2)
		free(data2);
	if (data)
		free(data);
	return ret;
}

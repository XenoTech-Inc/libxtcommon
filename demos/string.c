#include <xt/error.h>
#include <xt/string.h>
#include <xt/os.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void trim(void)
{
	xtConsoleFillLine("-");
	puts("-- TRIM TEST");
	char test[] = "   Mah \t\t fat BOI\t\t  s\t ";
	xtStringTrim(test);
	printf("Trim: \"%s\"\n", test);
	if (isspace(test[0]) || isspace(test[strlen(test) - 1]))
		goto fail;
	char test2[] = "     What's the    most \t fucking  important   ASSpect?";
	xtStringTrim(test2);
	printf("Trim: \"%s\"\n", test2);
	if (isspace(test[0]) || isspace(test[strlen(test) - 1]))
		goto fail;
	char test3[] = "This is   againsed      \t law, reported   \t ";
	xtStringTrim(test3);
	printf("Trim: \"%s\"\n", test3);
	if (isspace(test[0]) || isspace(test[strlen(test) - 1]))
		goto fail;
	return;
fail:
	fputs("Trim failed\n", stderr);
	exit(EXIT_FAILURE);
}

static void trimWords(void)
{
	xtConsoleFillLine("-");
	puts("-- TRIM WORDS TEST");
	const char *orig = "     What's the    most \t fucking  important   ASSpect?";
	char buf[256];
	printf("Original: \"%s\"\n", orig);
	strcpy(buf, orig);
	xtStringTrim(buf);
	printf("Trimmed: \"%s\"\n", buf);
	strcpy(buf, orig);
	xtStringTrimWords(buf);
	printf("Trim words: \"%s\"\n", buf);
}

static void rot13(void)
{
	xtConsoleFillLine("-");
	puts("-- ROT13 TEST");
	const char *test = "FuCkEd CaSe";
	char buf[256];
	strcpy(buf, test);
	xtRot13(buf, strlen(buf));
	puts(buf);
	xtRot13(buf, strlen(buf));
	puts(buf);
	if (strcmp(test, buf)) {
		fputs("ROT13 failed\n", stderr);
		exit(EXIT_FAILURE);
	}
}

static void reverse(void)
{
	xtConsoleFillLine("-");
	puts("-- REVERSE TEST");
	const char *test = "Mah boi, this is what all true warriors strive for";
	char buf[256];
	strcpy(buf, test);
	xtStringReverse(buf);
	puts(buf);
	xtStringReverse(buf);
	puts(buf);
	if (strcmp(buf, test)) {
		fputs("Reverse failed\n", stderr);
		exit(EXIT_FAILURE);
	}
}

static void contains(void)
{
	xtConsoleFillLine("-");
	puts("-- CONTAINS TEST");
	puts("Contains end test");
	if (!xtStringContains("mah fatboi", "boi"))
		goto fail;
	puts("Contains start test");
	if (!xtStringContains("mah fatboi", "mah"))
		goto fail;
	puts("Contains middle test");
	if (!xtStringContains("mah fatboi", "fat"))
		goto fail;
	return;
fail:
	fputs("String contains failed\n", stderr);
	exit(EXIT_FAILURE);
}

static void start(void)
{
	xtConsoleFillLine("-");
	puts("-- STARTS WITH TEST");
	if (!xtStringStartsWith("Mah boi", "Mah ")) {
		fputs("Starts with failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (!xtStringStartsWith("Mah boi", "Mah boi")) {
		fputs("Starts with failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	puts("Success");
}

static void end(void)
{
	xtConsoleFillLine("-");
	puts("-- ENDS WITH TEST");
	if (!xtStringEndsWith("Mah boi", " boi")) {
		fputs("Ends with failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (!xtStringEndsWith("Mah boi", "Mah boi")) {
		fputs("Ends with failed\n", stderr);
		exit(EXIT_FAILURE);
	}
	puts("Success");
}

static void formatSI(void)
{
	xtConsoleFillLine("-");
	puts("-- FORMAT SI TEST");
	char buf[256];
	puts(" D B  Number                Value");
	puts("----------------------------------------------");
	for (unsigned i = 0; i < 16; ++i) {
		size_t num = rand();
		unsigned decimals = 1 + (rand() % 3);
		unsigned bin = rand() & 1;
		unsigned dummy;
		xtFormatBytesSI(buf, sizeof buf, num, decimals, bin, &dummy);
		xtprintf("(%u,%u) %-20zu  %s\n", decimals, bin, num, buf);
	}
}

static void printFormat(void)
{
	xtConsoleFillLine("-");
	puts("-- PRINT FORMAT TEST");
	char buf[256];
	xtsnprintf(buf, sizeof buf, "%s normal & simple %10s", "perfectly", "format");
	puts(buf);
	long long lnum = 3419803901L;
	unsigned unum = 30;
	size_t znum = ~0xcafebabe;
	xtsnprintf(buf, sizeof buf, "wide test: %lld %04u %20zu", lnum, unum, znum);
	puts(buf);
	xtsnprintf(buf, sizeof buf, "float test: %+.2f %g %lf", .42f, -4e3 + 1.0, 0.0001);
	puts(buf);
	xtsnprintf(buf, sizeof buf, "int_t variants:"
		"% +6I8d %I8u\n"
		"%I16d %08I16o\n"
		"%I32d %#-I32x\n"
		"%30I64d %I64u",
		INT8_MIN, UINT8_MAX,
		INT16_MIN, INT16_MAX,
		INT32_MIN, INT32_MAX,
		INT64_MIN, UINT64_MAX
	);
	puts(buf);
	xtprintf("Custom error format: message=%M, code=%d\n", errno);
}

int main(void)
{
	srand(time(NULL));
	xtConsoleFillLine("-");
	puts("-- STRING TEST");
	start();
	end();
	contains();
	reverse();
	rot13();
	trim();
	trimWords();
	printFormat();
	formatSI();
	return EXIT_SUCCESS;
}

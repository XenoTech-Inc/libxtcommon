/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/error.h>
#include <xt/string.h>
#include <xt/time.h>
#include <xt/os.h>
#include <xt/os_macros.h>
#include <xt/utils.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

static struct stats stats;

static void trim(void)
{
	char test[] = "   Mah \t\t fat BOI\t\t  s\t ";
	const char *ttest = "Mah \t\t fat BOI\t\t  s";
	xtStringTrim(test);
	if (strcmp(test, ttest)) {
		FAIL("xtStringTrim() - leading & trailing whitespace");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", ttest, test);
	} else
		PASS("xtStringTrim() - leading & trailing whitespace");
	char test2[] = "     What's the    most \t fucking  important   ASSpect?";
	const char *ttest2 = "What's the    most \t fucking  important   ASSpect?";
	xtStringTrim(test2);
	if (strcmp(test2, ttest2)) {
		FAIL("xtStringTrim() - leading whitespace xtStringTrim()");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", ttest2, test2);
	} else
		PASS("xtStringTrim() - leading whitespace xtStringTrim()");
	char test3[] = "This is   againsed      \t law, reported   \t ";
	const char *ttest3 = "This is   againsed      \t law, reported";
	xtStringTrim(test3);
	if (strcmp(test3, ttest3)) {
		FAIL("xtStringTrim() - trailing whitespace");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", ttest3, test3);
	} else
		PASS("xtStringTrim() - trailing whitespace");
	char test4[] = "";
	const char *ttest4 = "";
	xtStringTrim(test4);
	if (strcmp(test4, ttest4)) {
		FAIL("xtStringTrim() - empty string");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", ttest4, test4);
	} else
		PASS("xtStringTrim() - empty string");
}

static void trimWords(void)
{
	const char *orig = "     What's the    most \t fucking  important   ASSpect?";
	const char *trim = "What's the most fucking important ASSpect?";
	char buf[256];
	strcpy(buf, orig);
	xtStringTrimWords(buf);
	if (strcmp(buf, trim)) {
		FAIL("xtStringTrimWords()");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", trim, buf);
	} else
		PASS("xtStringTrimWords()");
}

#define SPACE_DELIM " \f\n\r\t\v"

static void split(void)
{
	xtConsoleFillLine("-");
	puts("-- SPLIT TEST");
	char dope[] = "Mah   	Boi,  this 	piece	is what	all true warriors\nstrive		for";
	char *tokens[12];
	unsigned num = 12;
	xtStringSplit(dope, SPACE_DELIM, tokens, &num);
	printf("num: %u\n", num);
	for (unsigned i = 0; i < num; ++i)
		puts(tokens[i]);
}

static void rot13(void)
{
	const char *test = "FuCkEd CaSe";
	char buf[256];
	strcpy(buf, test);
	xtRot13(buf, strlen(buf));
	const char *testr = "ShPxRq PnFr";
	if (strcmp(buf, testr)) {
		FAIL("xtRot13() - encode");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", testr, buf);
	} else
		PASS("xtRot13() - encode");
	xtRot13(buf, strlen(buf));
	if (strcmp(buf, test)) {
		FAIL("xtRot13() - decode");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", test, buf);
	} else
		PASS("xtRot13() - decode");
}

static void reverse(void)
{
	const char *test = "Mah boi, this is what all true warriors strive for";
	const char *rtest = "rof evirts sroirraw eurt lla tahw si siht ,iob haM";
	char buf[256];
	strcpy(buf, test);
	xtStringReverse(buf);
	if (strcmp(buf, rtest)) {
		FAIL("xtStringReverse()");
		fprintf(stderr, "expected  \"%s\", but got \"%s\"\n", rtest, buf);
	} else
		PASS("xtStringReverse()");
	xtStringReverse(buf);
	if (strcmp(buf, test)) {
		FAIL("xtStringReverse() - reverse");
		fprintf(stderr, "expected  \"%s\", but got \"%s\"\n", test, buf);
	} else
		PASS("xtStringReverse() - reverse");
}

static void contains(void)
{
	if (xtStringContains("mah fatboi", "boi"))
		PASS("xtStringContains() - ending");
	else
		FAIL("xtStringContains() - ending");
	if (xtStringContains("mah fatboi", "mah"))
		PASS("xtStringContains() - starting");
	else
		FAIL("xtStringContains() - starting");
	if (xtStringContains("mah fatboi", "fat"))
		PASS("xtStringContains() - middle");
	else
		FAIL("xtStringContains() - middle");
}

static void start(void)
{
	if (xtStringStartsWith("Mah boi", "Mah "))
		PASS("xtStringStartsWith() - partial");
	else
		FAIL("xtStringStartsWith() - partial");
	if (xtStringStartsWith("Mah boi", "Mah boi"))
		PASS("xtStringStartsWith() - full");
	else
		FAIL("xtStringStartsWith() - full");
}

static void end(void)
{
	if (xtStringEndsWith("Mah boi", " boi"))
		PASS("xtStringEndsWith() - partial");
	else
		FAIL("xtStringEndsWith() - partial");
	if (xtStringEndsWith("Mah boi", "Mah boi"))
		PASS("xtStringEndsWith() - full");
	else
		FAIL("xtStringEndsWith() - full");
}

static void copy(void)
{
	char small[8];
	const char *str9 = "1234567!"; // strlen > 8
	xtstrncpy(small, str9, sizeof small);
	if (strcmp(small, "1234567"))
		FAIL("xtstrncpy");
	else
		PASS("xtstrncpy");
}

static void formatSI(void)
{
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
	char buf[256];
	const char *match = "perfectly normal & simple     format";
	xtsnprintf(buf, sizeof buf, "%s normal & simple %10s", "perfectly", "format");
	if (strcmp(buf, match)) {
		FAIL("xtvsnprintf() - strings");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtvsnprintf() - strings");
	long long lnum = 3419803901L;
	unsigned unum = 30;
	size_t znum = ~0xcafebabe;
	xtsnprintf(buf, sizeof buf, "wide test: %lld %04u %20zu", lnum, unum, znum);
	match = "wide test: 3419803901 0030            889275713";
	if (strcmp(buf, match)) {
#if XT_IS_WINDOWS
		XFAIL("xtvsnprintf() - (wide) integers");
#else
		FAIL("xtvsnprintf() - (wide) integers");
#endif
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtvsnprintf() - (wide) integers");
	xtsnprintf(buf, sizeof buf, "float test: %+.2f %g %lf", .42f, -4e3 + 1.0, 0.0001);
	match = "float test: +0.42 -3999 0.000100";
	if (strcmp(buf, match)) {
		FAIL("xtvsnprintf() - float");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtvsnprintf() - float");
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
	match =
		"int_t variants:  -128 255\n"
		"-32768 00077777\n"
		"-2147483648 0x7fffffff\n"
		"          -9223372036854775808 18446744073709551615";
	if (strcmp(buf, match)) {
		FAIL("xtvsnprintf() - intXX_t");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtvsnprintf() - intXX_t");
}

static void formatTime(void)
{
	char buf[1024];
	struct xtTimestamp start, end;
	start.sec  = 432840;
	start.nsec = 37942808LLU;
	end.sec    = 723480;
	end.nsec   = 347298107LLU;
	xtFormatTimeDuration(buf, sizeof buf, "diff.sec : %w %d %h %m %s", &start, &end);
	const char *match = "diff.sec : 0 weeks 3 days 8 hours 44 minutes 0 seconds";
	if (strcmp(buf, match)) {
		FAIL("xtFormatTimeDuration() - diff.sec");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtFormatTimeDuration() - diff.sec");
	xtFormatTimeDuration(buf, sizeof buf, "diff.nsec: %t.%u.%n", &start, &end);
	match = "diff.nsec: 309 milliseconds.355 microseconds.299 nanoseconds";
	if (strcmp(buf, match)) {
		FAIL("xtFormatTimeDuration() - diff.nsec");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtFormatTimeDuration() - diff.nsec");
	xtFormatTimeDuration(buf, sizeof buf, "diff     : %g", &start, &end);
	match = "diff     : 3 days 8 hours 44 minutes 0 seconds 309 milliseconds 355 microseconds 299 nanoseconds";
	if (strcmp(buf, match)) {
		FAIL("xtFormatTimeDuration() - diff");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtFormatTimeDuration() - diff");
	xtFormatTimeDuration(buf, sizeof buf, "DIFF     : %G", &start, &end);
	match = "DIFF     : 3 days 8 hours 44 minutes 309 milliseconds 355 microseconds 299 nanoseconds";
	if (!strcmp(buf, match)) {
		FAIL("xtFormatTimeDuration() - DIFF");
		fprintf(stderr, "expected \"%s\", but got \"%s\"\n", match, buf);
	} else
		PASS("xtFormatTimeDuration() - DIFF");
	end.sec  = xtRandLLU() % (604800U * 52);
	end.nsec = xtRandLLU() % 1000000000LLU;
	xtFormatTimeDuration(buf, sizeof buf, "Random time: %G", &start, &end);
	puts(buf);
	xtFormatTimeDuration(buf, sizeof buf, "Random time: %W-%D %H:%M:%S %T.%U.%N", &start, &end);
	puts(buf);
}

static void putString(void)
{
	const char *text = "Testerdetest\nWhoah, this is me, teh KING";
	xtConsoleFillLine("-");
	puts("-- PUT STRING TEST");

	xtConsolePutString(text, 0, false, XT_CONSOLE_ALIGN_CENTER);
	xtConsolePutString(text, 0, false, XT_CONSOLE_ALIGN_LEFT);
	xtConsolePutString(text, 0, false, XT_CONSOLE_ALIGN_RIGHT);

	xtConsolePutString(text, 14, false, XT_CONSOLE_ALIGN_LEFT);
	xtConsolePutString(text, 14, false, XT_CONSOLE_ALIGN_CENTER);
	xtConsolePutString(text, 14, false, XT_CONSOLE_ALIGN_RIGHT);

	xtConsolePutString(text, 6, true, XT_CONSOLE_ALIGN_LEFT);
	xtConsolePutString(text, 6, true, XT_CONSOLE_ALIGN_CENTER);
	xtConsolePutString(text, 6, true, XT_CONSOLE_ALIGN_RIGHT);
}

int main(void)
{
	stats_init(&stats, "string");
	srand(time(NULL));
	puts("-- STRING TEST");
	start();
	end();
	copy();
	contains();
	reverse();
	rot13();
	trim();
	trimWords();
	split();
	printFormat();
	formatSI();
	formatTime();
	putString();
	stats_info(&stats);
	return stats_status(&stats);
}

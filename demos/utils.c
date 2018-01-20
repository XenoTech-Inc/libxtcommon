/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include "utils.h"
#include <stdio.h>
#include <xt/os.h>
#include <xt/utils.h>

void stats_init(struct stats *s, const char *name)
{
	s->run = s->total = s->pass = s->fail = s->xfail = s->skip = 0;
	s->name = name;
}

void stats_info(const struct stats *s)
{
	xtConsoleFillLine("-");
	printf(
		"STATUS REPORT - %s\n"
		"Number of tests: %u\n"
		"Test count: %u\n"
		"Test coverage: %.2f%%\n"
		"# PASS : %u\n"
		"# FAIL : %u\n"
		"# XFAIL: %u\n"
		"# SKIP : %u\n",
		s->name, s->run, s->total, 100.0 * s->run / s->total,
		s->pass, s->fail, s->xfail, s->skip
	);
}

void stats_pass(struct stats *s, const char *str)
{
	++s->pass;
	++s->total;
	++s->run;
	printf("PASS  %s\n", str);
}

void stats_fail(struct stats *s, const char *str)
{
	++s->fail;
	++s->total;
	++s->run;
	fprintf(stderr, "FAIL  %s\n", str);
}

void stats_xfail(struct stats *s, const char *str)
{
	++s->xfail;
	++s->total;
	++s->run;
	printf("XFAIL %s\n", str);
}

void stats_skip(struct stats *s, const char *str)
{
	++s->skip;
	++s->total;
	fprintf(stderr, "SKIP  %s\n", str);
}

int stats_status(const struct stats *s)
{
	if (s->fail)
		return 1;
	return 0;
}

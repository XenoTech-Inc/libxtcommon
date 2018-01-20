/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#ifndef DEMOS_UTILS_H
#define DEMOS_UTILS_H

#define PASS(x) stats_pass(&stats, x)
#define FAIL(x) stats_fail(&stats, x)
#define XFAIL(x) stats_xfail(&stats, x)
#define SKIP(x) stats_skip(&stats, x)

struct stats {
	unsigned pass, fail, xfail, skip;
	unsigned run, total;
	const char *name;
};

void stats_init(struct stats *s, const char *name);
void stats_info(const struct stats *s);
void stats_pass(struct stats *s, const char *str);
void stats_fail(struct stats *s, const char *str);
void stats_xfail(struct stats *s, const char *str);
void stats_skip(struct stats *s, const char *str);
int stats_status(const struct stats *s);

#endif

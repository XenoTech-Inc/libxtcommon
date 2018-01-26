/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/endian.h>
#include <xt/string.h>

#include <stdio.h>
#include <stdint.h>
#include "utils.h"

static struct stats stats;

static void swap(void)
{
	uint16_t dw = 0xCAFE, dw2 = 0xFECA;
	uint32_t dd = 0xCAFEBABE, dd2 = 0xBEBAFECA;
	uint64_t dq = 0x0BADB002DEADFACE, dq2 = 0xCEFAADDE02B0AD0B;

	if (_xt_swap16(dw) != dw2 || _xt_swap16(dw2) != dw) {
		FAIL("_xt_swap16()");
		if (_xt_swap16(dw) != dw2)
			xtfprintf(stderr, "Expected: %I16X, but got: %I16X\n", dw2, _xt_swap16(dw));
		else
			xtfprintf(stderr, "Expected: %I16X, but got: %I16X\n", dw, _xt_swap16(dw2));
	} else
		PASS("_xt_swap16()");

	if (_xt_swap32(dd) != dd2 || _xt_swap32(dd2) != dd) {
		FAIL("_xt_swap32()");
		if (_xt_swap32(dd) != dd2)
			xtfprintf(stderr, "Expected: %I32X, but got: %I32X\n", dd2, _xt_swap32(dd));
		else
			xtfprintf(stderr, "Expected: %I32X, but got: %I32X\n", dd, _xt_swap32(dd2));
	} else
		PASS("_xt_swap32()");

	if (_xt_swap64(dq) != dq2 || _xt_swap64(dq2) != dq) {
		FAIL("_xt_swap64()");
		if (_xt_swap64(dq) != dq2)
			xtfprintf(stderr, "Expected: %I64X, but got: %I64X\n", dq2, _xt_swap64(dq));
		else
			xtfprintf(stderr, "Expected: %I64X, but got: %I64X\n", dq, _xt_swap64(dq2));
	} else
		PASS("_xt_swap64()");
}

int main(void)
{
	stats_init(&stats, "endian");
	puts("-- ENDIAN TEST");
	puts("Platform info:\nByte order: "
#ifdef _XT_BIG_ENDIAN
		"big"
#else
		"little"
#endif
		"\nHas unsigned long long: "
#ifdef _XT_HAS_LLU
		"yes"
#else
		"no"
#endif
	);
	swap();
	stats_info(&stats);
	return stats_status(&stats);
}

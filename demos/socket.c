/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/socket.h>
#include <xt/string.h>

#include "utils.h"

static struct stats stats;
static bool socket_init = false;

int main(void)
{
	stats_init(&stats, "socket");
	puts("-- SOCKET TEST");
	if (!xtSocketInit()) {
		FAIL("xtSocketInit()");
		goto fail;
	}
	PASS("xtSocketInit()");
	socket_init = true;
fail:
	if (socket_init)
		xtSocketDestruct();
	stats_info(&stats);
	return stats_status(&stats);
}

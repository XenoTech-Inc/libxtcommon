/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/socket.h>
#include <xt/string.h>
#include <xt/thread.h>
#include <xt/error.h>

#include <stdbool.h>

#include "utils.h"

static struct stats stats;
static bool socket_init = false;

struct sock_thread {
	xtSocket sock;
	int port;
	int err;
	bool connected;
};

static int sock_thread_init(struct sock_thread *s)
{
	int err;
	// Setup some dummy values
	s->sock = XT_SOCKET_INVALID_FD;
	s->port = 25659;
	s->err = 0;
	s->connected = false;

	// Setup socket
	err = xtSocketCreate(&s->sock, XT_SOCKET_PROTO_TCP);
	if (err) {
		xtPerror("Could not create socket", err);
		goto fail;
	}
	err = xtSocketSetSoReuseAddress(s->sock, true);
	if (err) {
		xtPerror("Could not reuse address", err);
		goto fail;
	}
fail:
	return err;
}

static void sock_thread_free(struct sock_thread *s)
{
	int err;
	if (s->sock != XT_SOCKET_INVALID_FD) {
		err = xtSocketClose(&s->sock);
		if (err)
			xtPerror("sock_thread_free", err);
		s->sock = XT_SOCKET_INVALID_FD;
	}
}

static void *wait_master(struct xtThread *t, void *arg)
{
	int err;
	struct sock_thread *info = arg;
	xtSocket peer;
	struct xtSockaddr peerAddr;
	(void)t;

	err = xtSocketBindToAny(info->sock, info->port);
	if (err) {
		xtPerror("Master: Could not bind to interface", err);
		goto fail;
	}
	err = xtSocketListen(info->sock, 2);
	if (err) {
		xtPerror("Master: Could not listen on socket", err);
		goto fail;
	}
	err = xtSocketTCPAccept(info->sock, &peer, &peerAddr);
	if (err) {
		xtPerror("Master: Could not accept peer", err);
		goto fail;
	}
	info->connected = true;
	puts("Master: Got my bitch");
fail:
	if (err)
		info->err = err;
	sock_thread_free(info);
	return info;
}

static void *wait_slave(struct xtThread *t, void *arg)
{
	int err;
	struct sock_thread *info = arg;
	struct xtSockaddr sa;
	(void)t;

	if (!xtSockaddrFromAddr(&sa, xtSockaddrGetAddressLocalHost(), info->port)) {
		fputs("Slave: Could not find server address\n", stderr);
		goto fail;
	}
	// Try a couple of times before giving up
	for (unsigned i = 0; i < 5; ++i) {
		err = xtSocketConnect(info->sock, &sa);
		if (err)
			xtSleepMS(50);
		else
			break;
	}
	if (err) {
		xtPerror("Slave: Could not connect to master", err);
		goto fail;
	}
	info->connected = true;
	puts("Slave: Hello master");
fail:
	if (err)
		info->err = err;
	sock_thread_free(info);
	return info;
}

static int connect_loop(void)
{
	struct xtThread master, slave;
	struct sock_thread info_master, info_slave;

	// Initialize thread info
	if (sock_thread_init(&info_master) || sock_thread_init(&info_slave))
		return 1;

	// Spawn threads
	if (xtThreadCreate(&master, wait_master, &info_master, 0, 0)) {
		fputs("Failed to create master thread\n", stderr);
		return 1;
	}
	if (xtThreadCreate(&slave, wait_slave, &info_slave, 0, 0)) {
		fputs("Failed to create slave thread\n", stderr);
		return 1;
	}

	// Try a couple of times before giving up.
	bool connected = false;
	for (unsigned i = 0; i < 20; ++i) {
		xtSleepMS(50);
		if (info_master.connected && info_slave.connected) {
			connected = true;
			break;
		}
	}
	return connected ? 0 : 1;
}

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
	if (connect_loop())
		FAIL("connect_loop");
	else
		PASS("connect_loop");
fail:
	if (socket_init)
		xtSocketDestruct();
	stats_info(&stats);
	return stats_status(&stats);
}

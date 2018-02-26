/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/socket.h>
#include <xt/string.h>
#include <xt/thread.h>
#include <xt/error.h>
#include <xt/hash.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "utils.h"

#define TCP_IO_TRIES 16
#define DATA_PAGES (8*PAGESIZE)
#define PAGESIZE 4096

static struct stats stats;
static bool socket_init = false;

struct sock_thread {
	xtSocket sock, peer;
	struct xtSockaddr peerAddr;
	int port;
	int err;
	bool connected;
	void *ret;
	// Dynamic buffer for random data test
	void *buf;
	// The test to run after socket is connected.
	void *(*func)(struct xtThread *t, void *arg);
	// Tests whether the test has passed.
	int (*check)(struct xtThread *t, struct sock_thread *info);
};

static struct xtThread master, slave;
static struct sock_thread info_master, info_slave;

/* Some text and binary data for the TCP tests. */
static const char *tcp_text = "Mah BOI, this is what all true warriors strive for!";
static const unsigned char tcp_binary[] = {
	0xCA, 0xFE, 0xBA, 0xBE,
	0xDE, 0xAD, 0xBE, 0xEF,
	0x13, 0x37, 0x13, 0x37,
};

static int check_connected(struct xtThread *t, struct sock_thread *info)
{
	(void)t;
	return info->connected ? 0 : 1;
}

static void *dummy_func(struct xtThread *t, void *arg)
{
	(void)t;
	return arg;
}

static int sock_thread_join(struct xtThread *t, struct sock_thread *info)
{
	return xtThreadJoin(t, &info->ret);
}

static void *master_tcp_test(struct xtThread *t, void *arg)
{
	int err;
	char buf[256];
	uint16_t ioDummy;
	struct sock_thread *info = arg;

	(void)t;
	info->err = XT_EAGAIN;

	memset(buf, 0, sizeof buf);

	err = xtSocketTCPWriteFully(info->peer, tcp_text, strlen(tcp_text) + 1, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Master: Output error", err);
		goto fail;
	}

	err = xtSocketTCPReadFully(info->peer, buf, sizeof tcp_binary, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Master: Input error", err);
		goto fail;
	}

	if (memcmp(tcp_binary, buf, sizeof tcp_binary)) {
		fputs("Master: Slave packet is corrupt\n", stderr);
		err = XT_EIO;
	}

	// Create a block of random data
	info->buf = malloc(DATA_PAGES);
	if (!info->buf) {
		fputs("Master: Out of memory\n", stderr);
		err = XT_ENOMEM;
		goto fail;
	}

	xtprintf("Master: Generating %zu bytes random data\n", DATA_PAGES);

	unsigned char *ptr = info->buf, *end = ptr + DATA_PAGES;
	while (ptr < end)
		*ptr++ = rand();
	uint32_t checksum = xtHashCRC32(0, info->buf, DATA_PAGES);

	xtprintf("Master: Block generated. Checksum: %I32X\n", checksum);

	// Send block to client
	err = xtSocketTCPWriteFully(info->peer, info->buf, DATA_PAGES, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Master: Output error", err);
		goto fail;
	}

	uint32_t verify;
	err = xtSocketTCPReadFully(info->peer, &verify, sizeof verify, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Master: Input error", err);
		goto fail;
	}

	if (verify != checksum) {
		fputs("Master: Checksum differs\n", stderr);
		err = XT_EUNKNOWN;
		goto fail;
	}

fail:
	info->err = err;
	return arg;
}

static void *slave_tcp_test(struct xtThread *t, void *arg)
{
	int err;
	char buf[256];
	uint16_t ioDummy;
	struct sock_thread *info = arg;

	(void)t;
	info->err = XT_EAGAIN;

	memset(buf, 0, sizeof buf);

	err = xtSocketTCPReadFully(info->sock, buf, strlen(tcp_text) + 1, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Slave: Input error", err);
		goto fail;
	}

	printf("Slave: Master said \"%s\"\n", buf);
	if (strcmp(tcp_text, buf)) {
		fputs("Slave: Master packet is corrupt\n", stderr);
		err = XT_EIO;
	}

	err = xtSocketTCPWriteFully(info->sock, tcp_binary, sizeof tcp_binary, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Slave: Output error", err);
		goto fail;
	}

	xtprintf("Slave: Preparing to read %zu bytes random data\n", DATA_PAGES);

	info->buf = malloc(DATA_PAGES);
	if (!info->buf) {
		fputs("Slave: Out of memory\n", stderr);
		err = XT_ENOMEM;
		goto fail;
	}

	// Read block from master
	err = xtSocketTCPReadFully(info->sock, info->buf, DATA_PAGES, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Slave: Input error", err);
		goto fail;
	}

	uint32_t verify = xtHashCRC32(0, info->buf, DATA_PAGES);
	xtprintf("Slave: Block received. Checksum: %I32X\n", verify);
	err = xtSocketTCPWriteFully(info->sock, &verify, sizeof verify, &ioDummy, TCP_IO_TRIES);
	if (err) {
		xtPerror("Slave: Output error", err);
		goto fail;
	}

fail:
	info->err = err;
	return arg;
}

static int sock_thread_init(struct sock_thread *s)
{
	int err;
	// Setup some dummy values
	s->sock = XT_SOCKET_INVALID_FD;
	s->peer = XT_SOCKET_INVALID_FD;
	s->port = 25659;
	s->err = 0;
	s->connected = false;
	s->func = dummy_func;
	s->check = check_connected;
	s->buf = NULL;

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

	free(s->buf);

	if (s->peer != XT_SOCKET_INVALID_FD) {
		err = xtSocketClose(&s->peer);
		if (err)
			xtPerror("sock_thread_free", err);
		s->peer = XT_SOCKET_INVALID_FD;
	}
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
	err = xtSocketTCPAccept(info->sock, &info->peer, &info->peerAddr);
	if (err) {
		xtPerror("Master: Could not accept peer", err);
		goto fail;
	}

	info->connected = true;
	puts("Master: Got my bitch");

	// Run the real test.
	info->func(t, arg);

fail:
	if (err)
		info->err = err;
	else if (!info->err)
		info->err = info->check(t, info);

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

	// Run the real test.
	info->func(t, arg);

fail:
	if (err)
		info->err = err;
	else if (!info->err)
		info->err = info->check(t, info);

	sock_thread_free(info);
	return info;
}

static int connect_test(
	void *(*master_func)(struct xtThread*, void*),
	int (*master_check)(struct xtThread*, struct sock_thread*),
	void *(*slave_func)(struct xtThread*, void*),
	int (*slave_check)(struct xtThread*, struct sock_thread*)
)
{
	int err;

	// Initialize thread info
	if (sock_thread_init(&info_master) || sock_thread_init(&info_slave))
		return 1;
	info_master.func = master_func;
	info_master.check = master_check;
	info_slave.func = slave_func;
	info_slave.check = slave_check;

	// Spawn threads
	if (xtThreadCreate(&master, wait_master, &info_master, 0, 0)) {
		fputs("Failed to create master thread\n", stderr);
		return 1;
	}
	if (xtThreadCreate(&slave, wait_slave, &info_slave, 0, 0)) {
		fputs("Failed to create slave thread\n", stderr);
		return 1;
	}

	// Wait till both have finished.
	err = sock_thread_join(&master, &info_master);
	if (err) {
		xtPerror("Failed to join master thread", err);
		return 1;
	}
	err = sock_thread_join(&slave, &info_slave);
	if (err) {
		xtPerror("Failed to join slave thread", err);
		return 1;
	}

	// Check if both threads are run successfully.
	if (info_master.err)
		return info_master.err;
	if (info_slave.err)
		return info_slave.err;
	return 0;
}

int main(void)
{
	stats_init(&stats, "socket");
	srand(time(NULL));
	puts("-- SOCKET TEST");

	if (!xtSocketInit()) {
		FAIL("xtSocketInit()");
		goto fail;
	}
	PASS("xtSocketInit()");
	socket_init = true;

	// Generic tests
	if (connect_test(dummy_func, check_connected, dummy_func, check_connected))
		FAIL("connect_loop()");
	else
		PASS("connect_loop()");
	if (connect_test(master_tcp_test, check_connected, slave_tcp_test, check_connected))
		FAIL("tcp_test()");
	else
		PASS("tcp_test()");

fail:
	if (socket_init)
		xtSocketDestruct();

	stats_info(&stats);
	return stats_status(&stats);
}

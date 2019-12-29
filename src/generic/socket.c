/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */
#include <_xt/socket.h>

// Default retry count for TCP I/O
#define XT_SOCKET_TCP_IO_TRIES 16

/**
 * Only initializes the sin_family field in the address. This is VERY IMPORTANT!!!
 * All other fields will be left untouched.
 */
static void sockaddr_init(struct xtSocketAddress *sa)
{
	((struct sockaddr_in*)sa)->sin_family = AF_INET;
}

bool xtSocketAddressEquals(const struct xtSocketAddress *sa1, const struct xtSocketAddress * sa2)
{
	// DO NOT just check the full memory! It is possible that only the address and port match, which is what we want to check for.
	return ((struct sockaddr_in*)sa1)->sin_addr.s_addr == ((struct sockaddr_in*)sa2)->sin_addr.s_addr &&
		((struct sockaddr_in*)sa1)->sin_port == ((struct sockaddr_in*)sa2)->sin_port;
}

bool xtSocketAddressFromAddr(struct xtSocketAddress *sa, uint32_t addr, uint16_t port)
{
	xtSocketAddressSetAddress(sa, addr);
	xtSocketAddressSetPort(sa, port);
	sockaddr_init(sa); // Init this to be safe
	return true;
	// Suppress unused function warning
	(void)socket_proto_to_native_proto;
	(void)socket_native_proto_to_proto;
}

bool xtSocketAddressFromString(struct xtSocketAddress *restrict sa, const char *restrict addr, uint16_t port)
{
	char buf[32];
	char *sep = strchr(addr, ':');
	if (sep) { // Copy the string. Otherwise inet_pton won't work, since it tries to read the whole string
		if (sizeof buf <= (unsigned)(sep - addr)) // The string is too long
			return false;
		strncpy(buf, addr, sep - addr);
		buf[sep - addr] = '\0'; // To be safe. We need that NULL terminator.
	} else {
		strncpy(buf, addr, sizeof buf);
		buf[sizeof buf - 1] = '\0'; // To be safe, incase an invalid string is passed
	}
	if (inet_pton(AF_INET, buf, &((struct sockaddr_in*) sa)->sin_addr) != 1)
		return false;
	xtSocketAddressSetPort(sa, sep ? (unsigned short)strtoul(++sep, NULL, 10) : port);
	sockaddr_init(sa); // Init this to be safe
	return true;
}

uint32_t xtSocketAddressGetAddress(const struct xtSocketAddress *sa)
{
	return ((struct sockaddr_in*) sa)->sin_addr.s_addr;
}

uint32_t xtSocketAddressGetAddressAny(void)
{
	return INADDR_ANY;
}

uint32_t xtSocketAddressGetAddressLocalHost(void)
{
	return 16777343;
}

uint16_t xtSocketAddressGetPort(const struct xtSocketAddress *sa)
{
	return xthtobe16(((struct sockaddr_in*) sa)->sin_port);
}

void xtSocketAddressInit(struct xtSocketAddress *sa)
{
	sockaddr_init(sa);
}

char *xtSocketAddressToString(const struct xtSocketAddress *restrict sa, char *restrict buf, size_t buflen)
{
	char sbuf[32];
	if (!inet_ntop(AF_INET, &((struct sockaddr_in*)sa)->sin_addr, sbuf, INET_ADDRSTRLEN))
		return NULL;
	snprintf(buf, buflen, "%s:%hu", sbuf, xtSocketAddressGetPort(sa));
	return buf;
}

int xtSocketGetLocalSocketAddress(xtSocket sock, struct xtSocketAddress *sa)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getsockname(sock, (struct sockaddr*)sa, &addrlen) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

uint16_t xtSocketGetLocalPort(xtSocket sock)
{
	struct xtSocketAddress sa;
	if (xtSocketGetLocalSocketAddress(sock, &sa) == 0)
		return xtSocketAddressGetPort(&sa);
	return 0;
}

int xtSocketBindTo(xtSocket sock, const struct xtSocketAddress *sa)
{
	if (bind(sock, (const struct sockaddr*)sa, sizeof(struct sockaddr_in)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketBindToAny(xtSocket sock, uint16_t port)
{
	struct xtSocketAddress sa;
	xtSocketAddressSetAddress(&sa, xtSocketAddressGetAddressAny());
	xtSocketAddressSetPort(&sa, port);
	return xtSocketBindTo(sock, &sa);
}

int xtSocketConnect(xtSocket sock, const struct xtSocketAddress *dest)
{
	if (connect(sock, (const struct sockaddr*)dest, sizeof(struct sockaddr_in)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketClose(xtSocket *sock)
{
	if (xtSocketIsClosed(*sock))
		return XT_EBADF;
	if (xtSocketGetProtocol(*sock) == XT_SOCKET_PROTO_TCP)
		shutdown(*sock, SHUT_RDWR);
	close(*sock);
	// Very important!!! The FD is used for detecting if the socket is valid!
	*sock = XT_SOCKET_INVALID_FD;
	return 0;
}

void xtSocketAddressSetAddress(struct xtSocketAddress *sa, uint32_t addr)
{
	((struct sockaddr_in*)sa)->sin_addr.s_addr = addr;
	sockaddr_init(sa); // Init this to be safe
}

void xtSocketAddressSetPort(struct xtSocketAddress *sa, uint16_t port)
{
	((struct sockaddr_in*)sa)->sin_port = xthtobe16(port);
	sockaddr_init(sa); // Init this to be safe
}

int xtSocketTcpReadFully(xtSocket sock, void *restrict buf, uint16_t buflen, uint16_t *restrict bytesRead, unsigned retryCount)
{
	int err;
	uint16_t size = 0, in = 0, rem = buflen;
	unsigned char *ptr = buf;

	if (!retryCount)
		retryCount = XT_SOCKET_TCP_IO_TRIES;

	for (unsigned i = 0; i < retryCount; ++i) {
		err = xtSocketTcpRead(sock, ptr, rem, &size);
		if (err)
			goto fail;

		in += size;
		if (in >= buflen)
			break;

		rem -= buflen;
		ptr += buflen;
	}

	if (!err && in < buflen)
		err = XT_EAGAIN;
fail:
	*bytesRead = in;
	return err;
}

int xtSocketTcpWriteFully(xtSocket sock, const void *restrict buf, uint16_t buflen, uint16_t *restrict bytesSent, unsigned retryCount)
{
	int err;
	uint16_t size = 0, out = 0, rem = buflen;
	const unsigned char *ptr = buf;

	if (!retryCount)
		retryCount = XT_SOCKET_TCP_IO_TRIES;

	for (unsigned i = 0; i < retryCount; ++i) {
		err = xtSocketTcpWrite(sock, ptr, rem, &size);
		if (err)
			goto fail;

		out += size;
		if (out >= buflen)
			break;

		rem -= out;
		ptr += out;
	}

	if (!err && out < buflen)
		err = XT_EAGAIN;
fail:
	*bytesSent = out;
	return err;
}

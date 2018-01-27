/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */
#include <_xt/socket.h>

/**
 * Only initializes the sin_family field in the address. This is VERY IMPORTANT!!!
 * All other fields will be left untouched.
 */
static void sockaddr_init(struct xtSockaddr *sa)
{
	((struct sockaddr_in*)sa)->sin_family = AF_INET;
}

bool xtSockaddrEquals(const struct xtSockaddr *sa1, const struct xtSockaddr * sa2)
{
	// DO NOT just check the full memory! It is possible that only the address and port match, which is what we want to check for.
	return ((struct sockaddr_in*)sa1)->sin_addr.s_addr == ((struct sockaddr_in*)sa2)->sin_addr.s_addr &&
		((struct sockaddr_in*)sa1)->sin_port == ((struct sockaddr_in*)sa2)->sin_port;
}

bool xtSockaddrFromAddr(struct xtSockaddr *sa, uint32_t addr, uint16_t port)
{
	xtSockaddrSetAddress(sa, addr);
	xtSockaddrSetPort(sa, port);
	sockaddr_init(sa); // Init this to be safe
	return true;
	// Suppress unused function warning
	(void)socket_proto_to_native_proto;
	(void)socket_native_proto_to_proto;
}

bool xtSockaddrFromString(struct xtSockaddr *restrict sa, const char *restrict addr, uint16_t port)
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
	xtSockaddrSetPort(sa, sep ? (unsigned short)strtoul(++sep, NULL, 10) : port);
	sockaddr_init(sa); // Init this to be safe
	return true;
}

uint32_t xtSockaddrGetAddress(const struct xtSockaddr *sa)
{
	return ((struct sockaddr_in*) sa)->sin_addr.s_addr;
}

uint32_t xtSockaddrGetAddressAny(void)
{
	return INADDR_ANY;
}

uint32_t xtSockaddrGetAddressLocalHost(void)
{
	return 16777343;
}

uint16_t xtSockaddrGetPort(const struct xtSockaddr *sa)
{
	return xthtobe16(((struct sockaddr_in*) sa)->sin_port);
}

void xtSockaddrInit(struct xtSockaddr *sa)
{
	sockaddr_init(sa);
}

char *xtSockaddrToString(const struct xtSockaddr *restrict sa, char *restrict buf, size_t buflen)
{
	char sbuf[32];
	if (!inet_ntop(AF_INET, &((struct sockaddr_in*)sa)->sin_addr, sbuf, INET_ADDRSTRLEN))
		return NULL;
	snprintf(buf, buflen, "%s:%hu", sbuf, xtSockaddrGetPort(sa));
	return buf;
}

int xtSocketGetLocalSocketAddress(xtSocket sock, struct xtSockaddr *sa)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getsockname(sock, (struct sockaddr*)sa, &addrlen) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

uint16_t xtSocketGetLocalPort(xtSocket sock)
{
	struct xtSockaddr sa;
	if (xtSocketGetLocalSocketAddress(sock, &sa) == 0)
		return xtSockaddrGetPort(&sa);
	return 0;
}

int xtSocketBindTo(xtSocket sock, const struct xtSockaddr *sa)
{
	if (bind(sock, (const struct sockaddr*)sa, sizeof(struct sockaddr_in)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketBindToAny(xtSocket sock, uint16_t port)
{
	struct xtSockaddr sa;
	xtSockaddrSetAddress(&sa, xtSockaddrGetAddressAny());
	xtSockaddrSetPort(&sa, port);
	return xtSocketBindTo(sock, &sa);
}

int xtSocketConnect(xtSocket sock, const struct xtSockaddr *dest)
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

void xtSockaddrSetAddress(struct xtSockaddr *sa, uint32_t addr)
{
	((struct sockaddr_in*)sa)->sin_addr.s_addr = addr;
	sockaddr_init(sa); // Init this to be safe
}

void xtSockaddrSetPort(struct xtSockaddr *sa, uint16_t port)
{
	((struct sockaddr_in*)sa)->sin_port = xthtobe16(port);
	sockaddr_init(sa); // Init this to be safe
}

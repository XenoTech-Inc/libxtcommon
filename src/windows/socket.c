// XT headers
#include <xt/socket.h>
#include <xt/endian.h>
#include <xt/error.h>

// System headers
#include <ws2tcpip.h> // Necessary for a hell lot of stuff
#include <windows.h> // Do this after including winsock (ws2tcpip includes it for us)

// STD headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Returns the native sockaddr_in representation. 
 * This function is intended for private access only.
 */
static inline struct sockaddr_in *_xtSockaddrGetNativeImpl(xtSockaddr *sa)
{
	return (struct sockaddr_in*) &sa->sa;
}
/**
 * Returns the native sockaddr_in representation as const.
 * This function is intended tfor private access only.
 */
static inline const struct sockaddr_in *_xtSockaddrGetNativeImplConst(const xtSockaddr *sa)
{
	return (const struct sockaddr_in*) &sa->sa;
}
/**
 * Only initializes the sin_family field in the address. This is VERY IMPORTANT!!! 
 * All other fields will be left untouched.
 */
static inline void _xtSockaddrInit(xtSockaddr *sa)
{
	struct sockaddr_in *nsa = _xtSockaddrGetNativeImpl(sa);
	nsa->sin_family = AF_INET;
}

bool xtSockaddrEquals(const xtSockaddr *sa1, const xtSockaddr *sa2)
{
	// DO NOT just check the full memory! It is possible that only the address and port match, which is what we want to check for.
	return xtSockaddrGetAddress(sa1) == xtSockaddrGetAddress(sa2) && xtSockaddrGetPort(sa1) == xtSockaddrGetPort(sa2);
}

bool xtSockaddrFromString(xtSockaddr *sa, const char *addr, uint16_t port)
{
	char buf[32];
	char *sep = strchr(addr, ':');
	if (sep) { // Copy the string. Otherwise inet_pton won't work, since it tries to read the whole string
		if (sizeof(buf) <= (unsigned) (sep - addr)) // The string is too long
			return false;
		strncpy(buf, addr, sep - addr);
		buf[sep - addr] = '\0'; // To be safe. We need that NULL terminator.
	} else {
		strncpy(buf, addr, sizeof(buf));
		buf[sizeof(buf) - 1] = '\0'; // To be safe, incase an invalid string is passed
	}
	struct sockaddr_in *nsa = _xtSockaddrGetNativeImpl(sa);
	if (inet_pton(AF_INET, buf, &nsa->sin_addr) != 1)
		return false;
	xtSockaddrSetPort(sa, sep ? atoi(++sep) : port);
	_xtSockaddrInit(sa); // Init this to be safe
	return true;
}

bool xtSockaddrFromAddr(xtSockaddr *sa, uint32_t addr, uint16_t port)
{
	xtSockaddrSetAddress(sa, addr);
	xtSockaddrSetPort(sa, port);
	_xtSockaddrInit(sa); // Init this to be safe
	return true;
}

uint32_t xtSockaddrGetAddress(const xtSockaddr *sa)
{
	return _xtSockaddrGetNativeImplConst(sa)->sin_addr.s_addr;
}

uint32_t xtSockaddrGetAddressAny(void)
{
	return INADDR_ANY;
}

uint32_t xtSockaddrGetAddressLocalHost(void)
{
	return 16777343;
}

void *xtSockaddrGetNativeImpl(xtSockaddr *sa)
{
	return _xtSockaddrGetNativeImpl(sa);
}

const void *xtSockaddrGetNativeImplConst(const xtSockaddr *sa)
{
	return _xtSockaddrGetNativeImplConst(sa);
}

uint16_t xtSockaddrGetPort(const xtSockaddr *sa)
{
	return htobe16(_xtSockaddrGetNativeImplConst(sa)->sin_port);
}

void xtSockaddrInit(xtSockaddr *sa)
{
	_xtSockaddrInit(sa);
}

void xtSockaddrSetAddress(xtSockaddr *sa, uint32_t addr)
{
	struct sockaddr_in *nsa = _xtSockaddrGetNativeImpl(sa);
	nsa->sin_addr.s_addr = addr;
	_xtSockaddrInit(sa); // Init this to be safe
}

void xtSockaddrSetAll(xtSockaddr *sa, const void *nsa)
{
	struct sockaddr_in *dst = _xtSockaddrGetNativeImpl(sa);
	const struct sockaddr_in *src = nsa;
	*dst = *src;
	_xtSockaddrInit(sa);
}

void xtSockaddrSetPort(xtSockaddr *sa, uint16_t port)
{
	_xtSockaddrGetNativeImpl(sa)->sin_port = htobe16(port);
	_xtSockaddrInit(sa); // Init this to be safe
}

char *xtSockaddrToString(const xtSockaddr *sa, char *buf, size_t buflen)
{
	char sbuf[32];
	struct sockaddr_in nsa = *_xtSockaddrGetNativeImplConst(sa); // Copy the thing
	if (!inet_ntop(AF_INET, &nsa.sin_addr, sbuf, INET_ADDRSTRLEN))
		return NULL;
	snprintf(buf, buflen, "%s:%hu", sbuf, xtSockaddrGetPort(sa));
	return buf;
}

// Some macros that spare us a lot of typing
#define XT_SOCKET_INVALID_FD (unsigned int) SOCKET_ERROR
#define XT_SOCKET_LAST_ERROR WSAGetLastError()
#define close closesocket
#define SHUT_RDWR SD_BOTH
/**
 * Sets the socket protocol and timeout value. This function should always be called before using a socket.
 */
static inline void _xtSocketCreate(xtSocket *sock, xtSocketProto proto)
{
	(void) sock;
	(void) proto;
}
/**
 * Sets the file descriptor to the "invalid" value. This way we can see if the socket is closed in a legit way.
 */
static inline void _xtSocketClose(xtSocket *sock)
{
	// Very important!!! The FD is used for detecting if the socket is valid!
	*sock = XT_SOCKET_INVALID_FD;
}
/**
 * Converts a native socket protocol to it's xt representation.
 * XT_SOCKET_PROTO_UNKNOWN is returned if the protocol is unsupported.
 */
static xtSocketProto _xtSocketNativeProtoToProto(int nativeProto)
{
	switch (nativeProto) {
	case IPPROTO_TCP: return XT_SOCKET_PROTO_TCP;
	case IPPROTO_UDP: return XT_SOCKET_PROTO_UDP;
	default:          return XT_SOCKET_PROTO_UNKNOWN;
	}
}
/**
 * Converts an xtSocketProto to the native representation.
 * @return True of the conversion was successful, false otherwise.
 */
static bool _xtSocketProtoToNativeProto(xtSocketProto proto, int *nativeType, int *nativeProto)
{
	switch (proto) {
	case XT_SOCKET_PROTO_TCP:
		*nativeType = SOCK_STREAM;
		*nativeProto = IPPROTO_TCP;
		return true;
	case XT_SOCKET_PROTO_UDP:
		*nativeType = SOCK_DGRAM;
		*nativeProto = IPPROTO_UDP;
		return true;
	default:
		return false;
	}
}

int xtSocketAccept(xtSocket sock, xtSocket *peerSock, xtSockaddr *peerAddr)
{
	struct sockaddr_in nsa;
	socklen_t dummyLen = sizeof(nsa);
	// Something is happening!
	*peerSock = accept(sock, (struct sockaddr*) &nsa, &dummyLen);
	// Check if this socket suddenly closed, but is now woken up
	// Also check if the peersocket has been accepted
	if (xtSocketIsClosed(sock) || *peerSock == XT_SOCKET_INVALID_FD)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	// Initialize the socket. very important!!!
	_xtSocketCreate(peerSock, XT_SOCKET_PROTO_TCP);
	xtSockaddrSetAll(peerAddr, &nsa);
	return 0;
}

int xtSocketBindTo(xtSocket sock, const xtSockaddr *sa)
{
	if (bind(sock, (const struct sockaddr*) sa, sizeof(struct sockaddr_in)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketBindToAny(xtSocket sock, uint16_t port)
{
	xtSockaddr sa;
	xtSockaddrSetAddress(&sa, xtSockaddrGetAddressAny());
	xtSockaddrSetPort(&sa, port);
	return xtSocketBindTo(sock, &sa);
}

int xtSocketClose(xtSocket *sock)
{
	if (xtSocketIsClosed(*sock))
		return XT_EBADF;
	shutdown(*sock, SHUT_RDWR);
	close(*sock);
	_xtSocketClose(sock);
	return 0;
}

int xtSocketConnect(xtSocket sock, const xtSockaddr *dest)
{
	if (connect(sock, (const struct sockaddr*) &dest->sa, sizeof(struct sockaddr_in)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketCreate(xtSocket *sock, xtSocketProto proto)
{
	int nativeType, nativeProto;
	if (!_xtSocketProtoToNativeProto(proto, &nativeType, &nativeProto))
		return XT_EPROTONOSUPPORT;
	*sock = socket(AF_INET, nativeType, nativeProto);
	if (*sock == XT_SOCKET_INVALID_FD)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	_xtSocketCreate(sock, proto);
	return 0;
}

void xtSocketDestruct(void)
{
	// 0 is success, but we dont care, we are terminating anyways
	WSACleanup();
}

inline bool xtSocketEquals(xtSocket sock1, xtSocket sock2)
{
	return sock1 == sock2;
}

int xtSocketGetLocalSocketAddress(xtSocket sock, xtSockaddr *sa)
{
	struct sockaddr_in *nsa = (struct sockaddr_in*) &sa->sa;
	socklen_t addrlen = sizeof(*nsa);
	if (getsockname(sock, (struct sockaddr*) nsa, &addrlen) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

uint16_t xtSocketGetLocalPort(xtSocket sock)
{
	xtSockaddr sa;
	if (xtSocketGetLocalSocketAddress(sock, &sa) == 0)
		return xtSockaddrGetPort(&sa);
	return 0;
}

xtSocketProto xtSocketGetProtocol(const xtSocket sock)
{
	WSAPROTOCOL_INFO val;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_PROTOCOL_INFO, (char*) &val, &len) == 0)
		return _xtSocketNativeProtoToProto(val.iProtocol);
	return XT_SOCKET_PROTO_UNKNOWN;
}

int xtSocketGetRemoteSocketAddress(const xtSocket sock, xtSockaddr *sa)
{
	struct sockaddr_in *nsa = xt
	socklen_t addrlen = sizeof(*nsa);
	if (getpeername(sock, (struct sockaddr*) nsa, &addrlen) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoKeepAlive(const xtSocket sock, bool *flag)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*) &val, &len) == 0) {
		*flag = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoLinger(const xtSocket sock, bool *on, int *linger)
{
	struct linger val;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_LINGER, (char*) &val, &len) == 0) {
		*on = val.l_onoff == 1 ? true : false;
		*linger = val.l_linger;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoReuseAddress(const xtSocket sock, bool *flag)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &val, &len) == 0) {
		*flag = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetTCPNoDelay(const xtSocket sock, bool *flag)
{
	int val;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*) &val, &len) == 0) {
		*flag = val == 1;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

bool xtSocketInit(void)
{
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	return WSAStartup(wVersionRequested, &wsaData) == 0;
}

inline bool xtSocketIsClosed(const xtSocket sock)
{
	return !xtSocketIsOpen(sock);
}

inline bool xtSocketIsOpen(const xtSocket sock)
{
	return sock != XT_SOCKET_INVALID_FD;
}

int xtSocketListen(xtSocket sock, unsigned backlog)
{
	// Cannot allow zero as backlog. This has different meanings across implementations
	// Also take care of possible overflow issues from unsigned -> signed
	if (listen(sock, (int) backlog <= 0 ? SOMAXCONN : (int) backlog) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketTCPRead(xtSocket sock, void *buf, uint16_t buflen, uint16_t *bytesRead)
{
	ssize_t ret;
	ret = recv(sock, buf, buflen, 0);
	if (ret == -1) {
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	} else if (ret == 0) { // Gracefull shutdown
		*bytesRead = 0;
		return XT_ESHUTDOWN;
	} else {
		*bytesRead = ret;
	}
	return 0;
}

int xtSocketUDPRead(xtSocket sock, void *buf, uint16_t buflen, uint16_t *bytesRead, xtSockaddr *sender)
{
	socklen_t dummyLen = sizeof(struct sockaddr_in);
	ssize_t ret;
	ret = recvfrom(sock, buf, buflen, 0, (struct sockaddr*) sender, &dummyLen);
	if (ret == -1) {
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	} else if (ret == 0) { // Gracefull shutdown
		*bytesRead = 0;
		return XT_ESHUTDOWN;
	} else {
		*bytesRead = ret;
	}
	return 0;
}

int xtSocketSetBlocking(xtSocket sock, bool flag)
{
	unsigned long mode = flag ? 0 : 1;
	if (ioctlsocket(sock, FIONBIO, &mode) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoKeepAlive(xtSocket sock, bool flag)
{
	int val = (flag) ? 1 : 0;
	if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char*) &val, sizeof(val)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoLinger(xtSocket sock, bool on, int linger)
{
	struct linger val;
	val.l_onoff = on ? 1 : 0;
	val.l_linger = linger;
	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*) &val, sizeof(val)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetTCPNoDelay(xtSocket sock, bool flag)
{
	int val = (flag) ? 1 : 0;
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*) &val, sizeof(val)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoReuseAddress(xtSocket sock, bool flag)
{
	int val = flag ? 1 : 0;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &val, sizeof(val)) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketTCPWrite(xtSocket sock, const void *buf, uint16_t buflen, uint16_t *bytesSent)
{
	ssize_t ret;
	ret = send(sock, (const char*) buf, buflen, 0);
	if (ret == -1) {
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	} else {
		*bytesSent = ret;
	}
	return 0;
}

int xtSocketUDPWrite(xtSocket sock, const void *buf, uint16_t buflen, uint16_t *bytesSent, const xtSockaddr *dest)
{
	ssize_t ret;
	ret = sendto(sock, buf, buflen, 0, (const struct sockaddr*) dest, sizeof(struct sockaddr_in));
	if (ret == -1) {
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	} else {
		*bytesSent = ret;
	}
	return 0;
}

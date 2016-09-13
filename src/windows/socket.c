// XT headers
#include <xt/socket.h>
#include <xt/endian.h>
#include <xt/error.h> // htobe16
#include <xt/time.h> // sleep

// System headers
#include <ws2tcpip.h> // Necessary for a hell lot of stuff
#include <windows.h> // Do this after including winsock (ws2tcpip includes it for us)

// STD headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Only initializes the sin_family field in the address. This is VERY IMPORTANT!!! 
 * All other fields will be left untouched.
 */
static void _xtSockaddrInit(xtSockaddr *sa)
{
	((struct sockaddr_in*) sa)->sin_family = AF_INET;
}

bool xtSockaddrEquals(const xtSockaddr *sa1, const xtSockaddr *sa2)
{
	// DO NOT just check the full memory! It is possible that only the address and port match, which is what we want to check for.
	return ((struct sockaddr_in*) sa1)->sin_addr.s_addr == ((struct sockaddr_in*) sa2)->sin_addr.s_addr && 
		((struct sockaddr_in*) sa1)->sin_port == ((struct sockaddr_in*) sa2)->sin_port;
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
	if (inet_pton(AF_INET, buf, &((struct sockaddr_in*) sa)->sin_addr) != 1)
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

uint16_t xtSockaddrGetPort(const xtSockaddr *sa)
{
	return htobe16(((struct sockaddr_in*) sa)->sin_port);
}

void xtSockaddrInit(xtSockaddr *sa)
{
	_xtSockaddrInit(sa);
}

void xtSockaddrSetAddress(xtSockaddr *sa, uint32_t addr)
{
	((struct sockaddr_in*) sa)->sin_addr.s_addr = addr;
	_xtSockaddrInit(sa); // Init this to be safe
}

void xtSockaddrSetPort(xtSockaddr *sa, uint16_t port)
{
	((struct sockaddr_in*) sa)->sin_port = htobe16(port);
	_xtSockaddrInit(sa); // Init this to be safe
}

char *xtSockaddrToString(const xtSockaddr *sa, char *buf, size_t buflen)
{
	char sbuf[32];
	if (!inet_ntop(AF_INET, &((struct sockaddr_in*) sa)->sin_addr, sbuf, INET_ADDRSTRLEN))
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
	// Very important!!! The FD is used for detecting if the socket is valid!
	*sock = XT_SOCKET_INVALID_FD;
	return 0;
}

int xtSocketConnect(xtSocket sock, const xtSockaddr *dest)
{
	if (connect(sock, (const struct sockaddr*) dest, sizeof(struct sockaddr_in)) == 0)
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
	return 0;
}

void xtSocketDestruct(void)
{
	// 0 is success, but we dont care, we are terminating anyways
	WSACleanup();
}

int xtSocketGetLocalSocketAddress(xtSocket sock, xtSockaddr *sa)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getsockname(sock, (struct sockaddr*) sa, &addrlen) == 0)
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
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getpeername(sock, (struct sockaddr*) sa, &addrlen) == 0)
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

int xtSocketGetSoReceiveBufferSize(xtSocket sock, unsigned *size)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &val, &len) == 0) {
		*size = val;
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

int xtSocketGetSoSendBufferSize(xtSocket sock, unsigned *size)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*) &val, &len) == 0) {
		*size = val;
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

bool xtSocketIsClosed(const xtSocket sock)
{
	return !xtSocketIsOpen(sock);
}

bool xtSocketIsOpen(const xtSocket sock)
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

int xtSocketSetSoReceiveBufferSize(xtSocket sock, unsigned size)
{
	int val = size;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*) &val, sizeof(val)) == 0)
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

int xtSocketSetSoSendBufferSize(xtSocket sock, unsigned size)
{
	int val = size;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*) &val, sizeof(val)) == 0)
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

int xtSocketTCPAccept(xtSocket sock, xtSocket *peerSock, xtSockaddr *peerAddr)
{
	socklen_t dummyLen = sizeof(struct sockaddr_in*);
	// Something is happening!
	*peerSock = accept(sock, (struct sockaddr*) peerAddr, &dummyLen);
	// Check if this socket suddenly closed, but is now woken up
	// Also check if the peersocket has been accepted
	if (xtSocketIsClosed(sock) || *peerSock == XT_SOCKET_INVALID_FD)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	return 0;
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

struct _xt_poll_data {
	xtSocket fd;
	void *data;
};

struct xtSocketPoll {
	struct _xt_poll_data *data;
	struct _xt_poll_data *readyData;
	struct pollfd *fds;
	unsigned size, count, socketsReady;
};

int xtSocketPollAdd(xtSocketPoll *p, xtSocket sock, void *data)
{
	unsigned index = p->count;
	if (index == p->size)
		return XT_ENOBUFS;
	p->fds[index].fd = sock;
	// Assign the events we want to be informed of, errors are added automatically
	p->fds[index].events = POLLRDNORM | POLLRDBAND;
	// Reset it, or else we get unwanted results
	p->fds[index].revents = 0;
	p->data[index].fd = sock;
	p->data[index].data = data;
	++p->count;
	return 0;
}

int xtSocketPollCreate(xtSocketPoll **p, unsigned size)
{
	if (size == 0)
		return XT_EINVAL;
	xtSocketPoll *_p = malloc(sizeof(xtSocketPoll));
	if (!_p)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	_p->fds = NULL;
	_p->data = NULL;
	_p->readyData = NULL;
	// Overflow control
	size = (int) size <= 0 ? 64 : size;
	_p->fds = malloc(size * sizeof(struct pollfd));
	if (!_p->fds)
		goto error;
	_p->data = malloc(size * sizeof(struct _xt_poll_data));
	if (!_p->data)
		goto error;
	_p->readyData = malloc(size * sizeof(struct _xt_poll_data));
	if (!_p->readyData)
		goto error;
	// Prepare the array for usage
	for (unsigned i = 0; i < size; ++i) {
		_p->fds[i].fd = XT_SOCKET_INVALID_FD;
		// Assign the events we want to be informed of, errors are added automatically
		_p->fds[i].events = POLLRDNORM | POLLRDBAND;
		// Reset it, or we get unwanted results
		_p->fds[i].revents = 0;
		_p->data[i].fd = XT_SOCKET_INVALID_FD;
		_p->data[i].data = NULL;
		_p->readyData[i].fd = XT_SOCKET_INVALID_FD;
		_p->readyData[i].data = NULL;
	}
	_p->size = size;
	_p->count = 0;
	_p->socketsReady = 0;
	*p = _p;
	return 0;
error:
	free(_p->fds);
	free(_p->data);
	free(_p->readyData);
	free(_p);
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

void xtSocketPollDestroy(xtSocketPoll *p)
{
	if (!p)
		return;
	free(p->fds);
	free(p->data);
	free(p->readyData);
	free(p);
}

unsigned xtSocketPollGetCount(const xtSocketPoll *p)
{
	return p->count;
}

void *xtSocketPollGetData(xtSocketPoll *p, unsigned index)
{
	
	return p->readyData[index].data;
}

unsigned xtSocketPollGetSize(xtSocketPoll *p)
{
	return p->size;
}

xtSocket xtSocketPollGetSocket(xtSocketPoll *p, unsigned index)
{
	return p->readyData[index].fd;
}

bool xtSocketPollRemove(xtSocketPoll *p, xtSocket socket)
{
	// Find the socket
	// Clean up possible sensitive data
	unsigned index = UINT_MAX;
	for (unsigned i = 0; i < p->size; ++i) {
		if (p->fds[i].fd == socket) {
			index = i;
			break;
		}
	}
	if (index == UINT_MAX)
		return false;
	if (index == p->size) {
		p->fds[index].fd = XT_SOCKET_INVALID_FD;
		p->data[index].fd = XT_SOCKET_INVALID_FD;
		p->data[index].data = NULL;
	} else {
		// Use memmmove, which is much faster than moving everything with a for loop
		// Shift all elements back one position
		memmove(&p->fds[index], &p->fds[index + 1], (p->count - index) * sizeof(struct pollfd));
		memmove(&p->data[index], &p->data[index + 1], (p->count - index) * sizeof(struct _xt_poll_data));
	}
	--p->count;
	return true;
}

int xtSocketPollWait(xtSocketPoll *p, int timeout, unsigned *socketsReady)
{
	int eventCount = WSAPoll(p->fds, p->count, timeout);
	if (eventCount == -1) {
		int syserr = XT_SOCKET_LAST_ERROR;
		// Handle this bug in the windows poll. If no sockets are present, it will return immidiately
		if (syserr == WSAEINVAL && p->count == 0) {
			// Simulate the timeout
			if (timeout > 0)
				xtSleepMS(timeout);
			if (socketsReady)
				*socketsReady = 0;
			return 0; // All good
		} else
			return _xtTranslateSysError(syserr); // Legit error
	}
	p->socketsReady = eventCount;
	if (socketsReady)
		*socketsReady = eventCount;
	for (unsigned i = 0, socketsHandled = 0; i < p->size && socketsHandled < (unsigned) eventCount; ++i) {
		if (p->fds[i].revents & (POLLRDNORM | POLLRDBAND)) {
			p->fds[i].revents = 0;
			p->readyData[socketsHandled].fd = p->fds[i].fd;
			p->readyData[socketsHandled].data = p->data[i].data;
			++socketsHandled;
			continue;
		}
	}
	return 0;
}

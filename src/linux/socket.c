// XT headers
#include <xt/socket.h>
#include <xt/endian.h> // htobe16
#include <xt/error.h>

// System headers
#include <arpa/inet.h> // sockaddr_in struct and functions to format IP addresses and ports
#include <errno.h> // for the error macros
#include <fcntl.h> // fcntl function
#include <netdb.h> // hostent struct, gethostbyname(), needed to convert a char* to in_addr
#include <netinet/tcp.h> // For TCP_NODELAY and such
#include <sys/epoll.h> // for epoll
#include <sys/socket.h> // for the socket function
#include <unistd.h> // close function

// STD headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * When porting to Windows, put these macros in the file. Overwrite any old ones
#define XT_SOCKET_INVALID_FD (unsigned int) SOCKET_ERROR
#define XT_SOCKET_LAST_ERROR WSAGetLastError()
#define close closesocket
#define SHUT_RDWR SD_BOTH
 * Overwrite the old functions with these:
 * xtSocketSetBlocking -> {
	unsigned long mode = flag ? 0 : 1;
	if (ioctlsocket(sock->fd, FIONBIO, &mode) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}
 */
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
#define XT_SOCKET_INVALID_FD -1
#define XT_SOCKET_LAST_ERROR errno
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
	if (xtSocketGetProtocol(*sock) == XT_SOCKET_PROTO_TCP)
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
	// Nothing to be done on Linux
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
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_PROTOCOL, (char*) &val, &len) == 0)
		return _xtSocketNativeProtoToProto(val);
	return XT_SOCKET_PROTO_UNKNOWN;
}

int xtSocketGetRemoteSocketAddress(const xtSocket sock, xtSockaddr *sa)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getpeername(sock, (struct sockaddr*) sa, &addrlen) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoError(const xtSocket sock, int *errnum)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*) &val, &len) == 0) {
		*errnum = _xtTranslateSysError(val);
		return 0;
	}
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

int xtSocketGetSoOOBInline(xtSocket sock, bool *flag)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_OOBINLINE, (char*) &val, &len) == 0) {
		*flag = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoReceiveBufferSize(xtSocket sock, unsigned *size)
{
	int val = 0;
	socklen_t len = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &val, &len) == 0) {
		*size = val / 2; // Divided by two, since the kernel doubles it
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
		*size = val / 2; // Divided by two, since the kernel doubles it
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
	// Nothing to be done on Linux
	return true;
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
	// Do not clobber other flags
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	flags = flag ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
	if (fcntl(sock, F_SETFL, flags) == 0)
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

int xtSocketSetSoOOBInline(xtSocket sock, bool flag)
{
	int val = flag ? 1 : 0;
	if (setsockopt(sock, SOL_SOCKET, SO_OOBINLINE, (const char*) &val, sizeof(val)) == 0)
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
	socklen_t dummyLen = sizeof(struct sockaddr_in);
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
	} else if (ret == 0) { // Graceful shutdown
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

int xtSocketTCPWriteOOB(xtSocket sock, uint8_t buf)
{
	ssize_t ret;
	ret = send(sock, (const char*) &buf, 1, MSG_OOB);
	if (ret == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	return 0;
}

int xtSocketUDPRead(xtSocket sock, void *buf, uint16_t buflen, uint16_t *bytesRead, xtSockaddr *sender)
{
	socklen_t dummyLen = sizeof(struct sockaddr_in);
	ssize_t ret;
	ret = recvfrom(sock, buf, buflen, 0, (struct sockaddr*) sender, &dummyLen);
	// When sending UDP packets on Windows, and they destination reports that it's not 
	// listening on that port, Windows will report WSAECONNRESET. You should really just ignore it 
	// because it's bullshit. And ignoring it here is exactly what we're doing. This Windows behavior 
	// should be disabled when we're creating new sockets, but still I mention the problem here.
	if (ret == -1) {
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	}
	*bytesRead = ret;
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
	struct epoll_event *events;
	int epollfd;
	unsigned size, count, socketsReady;
};
/**
 * Filters out or adds flags. This is to have consistent cross platform behavior.
 */
static uint32_t _xtSocketPollEventFixSysFlags(uint32_t sysevents)
{
	// Always add these on Linux!
	sysevents |= EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	return sysevents;
}
/**
 * Translates an xtSocketPollEvent to it's native countpart.
 */
static uint32_t _xtSocketPollEventFlagsToSys(xtSocketPollEvent events)
{
	uint32_t newEvents = 0;
	if (events & XT_POLLIN)
		newEvents |= EPOLLIN;
	if (events & XT_POLLPRI)
		newEvents |= EPOLLPRI;
	if (events & XT_POLLOUT)
		newEvents |= EPOLLOUT;
	if (events & XT_POLLERR)
		newEvents |= EPOLLERR;
	if (events & XT_POLLHUP)
		newEvents |= EPOLLHUP | EPOLLRDHUP;
	return newEvents;
}
/**
 * Translates native epoll flags to it's xt counterpart.
 */
static xtSocketPollEvent _xtSocketPollEventSysToFlags(uint32_t sysevents)
{
	xtSocketPollEvent newEvents = 0;
	if (sysevents & EPOLLIN)
		newEvents |= XT_POLLIN;
	if (sysevents & EPOLLPRI)
		newEvents |= XT_POLLPRI;
	if (sysevents & EPOLLOUT)
		newEvents |= XT_POLLOUT;
	if (sysevents & EPOLLERR)
		newEvents |= XT_POLLERR;
	if (sysevents & EPOLLHUP || sysevents & EPOLLRDHUP)
		newEvents |= XT_POLLHUP;
	return newEvents;
}

int xtSocketPollAdd(xtSocketPoll *p, xtSocket sock, void *data, xtSocketPollEvent events)
{
	if (p->count == p->size)
		return XT_ENOBUFS;
	struct epoll_event event;
	memset(&event, 0, sizeof(event)); // Prevent "uninitialised value(s)" warnings in Valgrind
	event.events = _xtSocketPollEventFixSysFlags(_xtSocketPollEventFlagsToSys(events));
	// Search for a free index
	for (unsigned i = 0; i < p->size; ++i) {
		if (p->data[i].fd == XT_SOCKET_INVALID_FD) {
			event.data.ptr = &p->data[i];
			if (epoll_ctl(p->epollfd, EPOLL_CTL_ADD, sock, &event) == 0) {
				p->data[i].fd = sock;
				p->data[i].data = data;
				++p->count;
				return 0;
			}
			break;
		}
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketPollCreate(xtSocketPoll **p, unsigned size)
{
	if (size == 0)
		return XT_EINVAL;
	xtSocketPoll *_p = malloc(sizeof(xtSocketPoll));
	if (!_p)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	_p->data = NULL;
	_p->events = NULL;
	// Overflow control
	size = (int) size <= 0 ? 64 : size;
	if (!(_p->events = malloc(sizeof(struct epoll_event) * size)))
		goto error;
	if (!(_p->data = malloc(sizeof(struct _xt_poll_data) * size)))
		goto error;
	int epollfd = epoll_create(size);
	if (epollfd == -1)
		goto error;
	// Prepare the array for usage
	for (unsigned i = 0; i < size; ++i) {
		_p->data[i].fd = XT_SOCKET_INVALID_FD;
		_p->data[i].data = NULL;
	}
	_p->epollfd = epollfd;
	_p->size = size;
	_p->count = 0;
	_p->socketsReady = 0;
	*p = _p;
	return 0;
error:
	free(_p->data);
	free(_p->events);
	free(_p);
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

void xtSocketPollDestroy(xtSocketPoll *p)
{
	if (!p)
		return;
	close(p->epollfd);
	// Clean up possible sensitive data
	for (unsigned i = 0; i < p->size; ++i) {
		p->data[i].fd = XT_SOCKET_INVALID_FD;
		p->data[i].data = NULL;
	}
	free(p->data);
	free(p->events);
	free(p);
}

unsigned xtSocketPollGetCount(const xtSocketPoll *p)
{
	return p->count;
}

void *xtSocketPollGetData(const xtSocketPoll *p, unsigned index)
{
	return ((struct _xt_poll_data*) p->events[index].data.ptr)->data;
}

xtSocketPollEvent xtSocketPollGetEvent(const xtSocketPoll *p, unsigned index)
{
	return _xtSocketPollEventSysToFlags(p->events[index].events);
}

unsigned xtSocketPollGetSize(const xtSocketPoll *p)
{
	return p->size;
}

xtSocket xtSocketPollGetSocket(const xtSocketPoll *p, unsigned index)
{
	return ((struct _xt_poll_data*) p->events[index].data.ptr)->fd;
}

int xtSocketPollMod(xtSocketPoll *p, xtSocket sock, xtSocketPollEvent events)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(event)); // Prevent "uninitialised value(s)" warnings in Valgrind
	event.events = _xtSocketPollEventFixSysFlags(_xtSocketPollEventFlagsToSys(events));
	for (unsigned i = 0; i < p->size; ++i) {
		if (p->data[i].fd == sock) {
			event.data.ptr = &p->data[i];
			if (epoll_ctl(p->epollfd, EPOLL_CTL_MOD, sock, &event) == 0)
				return 0;
			break;
		}
	}
	return XT_EINVAL;
}

int xtSocketPollRemove(xtSocketPoll *p, xtSocket sock)
{
	if (epoll_ctl(p->epollfd, EPOLL_CTL_DEL, sock, NULL) != 0)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	// Clean up the data in the data array
	for (unsigned i = 0; i < p->size; ++i) {
		if (p->data[i].fd == sock) {
			p->data[i].fd = XT_SOCKET_INVALID_FD;
			p->data[i].data = NULL;
			break;
		}
	}
	--p->count;
	return 0;
}

int xtSocketPollSetEvent(xtSocketPoll *p, xtSocket sock, xtSocketPollEvent events)
{
	for (unsigned i = 0; i < p->socketsReady; ++i) {
		if (((struct _xt_poll_data*) p->events[i].data.ptr)->fd == sock) {
			p->events[i].events = _xtSocketPollEventFlagsToSys(events);
			return 0;
		}
	}
	return XT_EINVAL;
}

int xtSocketPollWait(xtSocketPoll *p, int timeout, unsigned *socketsReady)
{
	int eventCount = epoll_wait(p->epollfd, p->events, p->size, timeout);
	if (eventCount == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	p->socketsReady = eventCount;
	*socketsReady = eventCount;
	return 0;
}

/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <_xt/socket.h>

int xtSocketCreate(xtSocket *sock, enum xtSocketProto proto)
{
	int nativeType, nativeProto;
	if (!socket_proto_to_native_proto(proto, &nativeType, &nativeProto))
		return XT_EPROTONOSUPPORT;
	*sock = socket(AF_INET, nativeType, nativeProto);
	if (*sock == XT_SOCKET_INVALID_FD)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	if (proto == XT_SOCKET_PROTO_UDP) {
		// Taken from: https://stackoverflow.com/questions/10332630/connection-reset-on-receiving-packet-in-udp-server
		// and: https://support.microsoft.com/en-us/kb/263823
		// Black magic. This disables the stupid Windows behavior of reporting WSAECONNRESET when the destination is
		// actively reporting that it's not listening on the destination port.
		//
		// This macro is defined with a magic value because the original M$ macro is not to be found in
		// the headers that we use
		#define _XT_SIO_UDP_CONNRESET 0x9800000C
		DWORD dwBytesReturned = 0;
		BOOL bNewBehavior = FALSE;
		int status;
		// disable new behavior using
		// IOCTL: SIO_UDP_CONNRESET
		status = WSAIoctl(*sock, _XT_SIO_UDP_CONNRESET,
			&bNewBehavior, sizeof bNewBehavior,
			NULL, 0, &dwBytesReturned,
			NULL, NULL);
		if (status == SOCKET_ERROR)
			return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
		#undef _XT_SIO_UDP_CONNRESET
	}
	return 0;
}

void xtSocketDestruct(void)
{
	// 0 is success, but we dont care, we are terminating anyways
	WSACleanup();
}

enum xtSocketProto xtSocketGetProtocol(const xtSocket sock)
{
	WSAPROTOCOL_INFO val;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&val, &len) == 0)
		return socket_native_proto_to_proto(val.iProtocol);
	return XT_SOCKET_PROTO_UNKNOWN;
}

int xtSocketGetRemoteSocketAddress(const xtSocket sock, struct xtSocketAddress *sa)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getpeername(sock, (struct sockaddr*)sa, &addrlen) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoError(const xtSocket sock, int *errnum)
{
	int val = 0;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&val, &len) == 0) {
		*errnum = _xtTranslateSysError(val);
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoKeepAlive(const xtSocket sock, bool *flag)
{
	int val = 0;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, &len) == 0) {
		*flag = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoLinger(const xtSocket sock, bool *restrict on, int *restrict linger)
{
	struct linger val;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&val, &len) == 0) {
		*on = val.l_onoff == 1 ? true : false;
		*linger = val.l_linger;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoReceiveBufferSize(xtSocket sock, unsigned *size)
{
	int val = 0;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&val, &len) == 0) {
		*size = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoReuseAddress(const xtSocket sock, bool *flag)
{
	int val = 0;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&val, &len) == 0) {
		*flag = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetSoSendBufferSize(xtSocket sock, unsigned *size)
{
	int val = 0;
	socklen_t len = sizeof val;
	if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&val, &len) == 0) {
		*size = val;
		return 0;
	}
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketGetTCPNoDelay(const xtSocket sock, bool *flag)
{
	int val;
	socklen_t len = sizeof val;
	if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&val, &len) == 0) {
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
	if (listen(sock, (int)backlog <= 0 ? SOMAXCONN : (int)backlog) == 0)
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
	int val = flag ? 1 : 0;
	if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof val) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoLinger(xtSocket sock, bool on, int linger)
{
	struct linger val;
	val.l_onoff = on ? 1 : 0;
	val.l_linger = linger;
	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&val, sizeof val) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoReceiveBufferSize(xtSocket sock, unsigned size)
{
	int val = size;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&val, sizeof val) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoReuseAddress(xtSocket sock, bool flag)
{
	int val = flag ? 1 : 0;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof val) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetSoSendBufferSize(xtSocket sock, unsigned size)
{
	int val = size;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&val, sizeof val) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketSetTCPNoDelay(xtSocket sock, bool flag)
{
	int val = (flag) ? 1 : 0;
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof val) == 0)
		return 0;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

int xtSocketTcpAccept(xtSocket sock, xtSocket *restrict peerSock, struct xtSocketAddress *restrict peerAddr)
{
	socklen_t dummyLen = sizeof(struct sockaddr_in);
	// Something is happening!
	*peerSock = accept(sock, (struct sockaddr*)peerAddr, &dummyLen);
	// Check if this socket suddenly closed, but is now woken up
	// Also check if the peersocket has been accepted
	if (xtSocketIsClosed(sock) || *peerSock == XT_SOCKET_INVALID_FD)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	return 0;
}

int xtSocketTcpRead(xtSocket sock, void *restrict buf, uint16_t buflen, uint16_t *restrict bytesRead)
{
	ssize_t ret;
	ret = recv(sock, buf, buflen, 0);
	if (ret == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	else if (ret == 0) { // Gracefull shutdown
		*bytesRead = 0;
		return XT_ESHUTDOWN;
	} else
		*bytesRead = ret;
	return 0;
}

int xtSocketTcpWrite(xtSocket sock, const void *restrict buf, uint16_t buflen, uint16_t *restrict bytesSent)
{
	ssize_t ret;
	ret = send(sock, (const char*)buf, buflen, 0);
	if (ret == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	else
		*bytesSent = ret;
	return 0;
}

int xtSocketUdpRead(xtSocket sock, void *restrict buf, uint16_t buflen, uint16_t *restrict bytesRead, struct xtSocketAddress *restrict sender)
{
	socklen_t dummyLen = sizeof(struct sockaddr_in);
	ssize_t ret;
	ret = recvfrom(sock, buf, buflen, 0, (struct sockaddr*)sender, &dummyLen);
	// When sending UDP packets on Windows, and they destination reports that it's not
	// listening on that port, Windows will report WSAECONNRESET. You should really just ignore it
	// because it's bullshit. And ignoring it here is exactly what we're doing. This Windows behavior
	// should be disabled when we're creating new sockets, but still I mention the problem here.
	if (ret == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	*bytesRead = ret;
	return 0;
}

int xtSocketUdpWrite(xtSocket sock, const void *restrict buf, uint16_t buflen, uint16_t *restrict bytesSent, const struct xtSocketAddress *restrict dest)
{
	ssize_t ret;
	ret = sendto(sock, buf, buflen, 0, (const struct sockaddr*)dest, sizeof(struct sockaddr_in));
	if (ret == -1)
		return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
	else
		*bytesSent = ret;
	return 0;
}

struct _xt_poll_data {
	xtSocket fd;
	void *data;
	short events;
};

struct xtSocketPoll {
	struct _xt_poll_data *data;
	struct _xt_poll_data *readyData;
	struct pollfd *fds;
	unsigned capacity, count, socketsReady;
};
/**
 * Filters out or adds flags. This is to have consistent cross platform behavior.
 */
static short socket_poll_event_fix_sys_flags(short sysevents)
{
	// Always remove these flags on Windows! They are not allowed to be passed to WSAPoll
	sysevents &= ~(POLLERR | POLLHUP);
	return sysevents;
}
/**
 * Translates an xtSocketPollEvent to it's native countpart.
 */
static short socket_poll_event_flags_to_sys(enum xtSocketPollEvent events)
{
	short newEvents = 0;
	if (events & XT_POLLIN)
		newEvents |= POLLRDNORM;
	if (events & XT_POLLOUT)
		newEvents |= POLLWRNORM;
	if (events & XT_POLLERR)
		newEvents |= POLLERR;
	if (events & XT_POLLHUP)
		newEvents |= POLLHUP;
	return newEvents;
}
/**
 * Translates native poll flags to it's xt counterpart.
 */
static enum xtSocketPollEvent socket_poll_event_sys_to_flags(short sysevents)
{
	enum xtSocketPollEvent newEvents = 0;
	if (sysevents & POLLIN)
		newEvents |= XT_POLLIN;
	if (sysevents & POLLOUT)
		newEvents |= XT_POLLOUT;
	if (sysevents & POLLERR || sysevents & POLLNVAL)
		newEvents |= XT_POLLERR;
	if (sysevents & POLLHUP)
		newEvents |= XT_POLLHUP;
	return newEvents;
}

int xtSocketPollAdd(struct xtSocketPoll *p, xtSocket sock, void *restrict data, enum xtSocketPollEvent events)
{
	int index = p->count;
	if (index == (int)p->capacity)
		return XT_ENOBUFS;
	p->fds[index].fd = sock;
	p->fds[index].events = socket_poll_event_fix_sys_flags(socket_poll_event_flags_to_sys(events));
	// Reset it, or else we get unwanted results. Only do this once here
	p->fds[index].revents = 0;
	p->data[index].fd = sock;
	p->data[index].data = data;
	++p->count;
	return 0;
}

int xtSocketPollCreate(struct xtSocketPoll **p, size_t capacity)
{
	struct xtSocketPoll *sp = malloc(sizeof **p);
	if (!sp)
		goto error;
	sp->data = NULL;
	sp->fds = NULL;
	sp->readyData = NULL;
	if (capacity == 0)
		capacity = XT_SOCKET_POLL_CAPACITY_DEFAULT;
	else if (capacity > INT_MAX)
		capacity = INT_MAX;
	if (!(sp->data = malloc(sizeof *sp->data * capacity)))
		goto error;
	if (!(sp->fds = malloc(sizeof *sp->fds * capacity)))
		goto error;
	if (!(sp->readyData = malloc(sizeof *sp->readyData * capacity)))
		goto error;
	// Prepare the array for usage
	for (int i = 0; i < (int)capacity; ++i) {
		sp->fds[i].fd = XT_SOCKET_INVALID_FD;
		// Assign the events we want to be informed of, errors are added automatically
		sp->fds[i].events = 0;
		// Reset it, or we get unwanted results
		sp->fds[i].revents = 0;
		sp->data[i].fd = XT_SOCKET_INVALID_FD;
		sp->data[i].data = NULL;
		sp->readyData[i].fd = XT_SOCKET_INVALID_FD;
		sp->readyData[i].data = NULL;
		sp->readyData[i].events = 0;
	}
	sp->capacity = capacity;
	sp->count = 0;
	sp->socketsReady = 0;
	*p = sp;
	return 0;
error:
	if (sp) {
		free(sp->data);
		free(sp->fds);
		free(sp->readyData);
		free(sp);
	}
	*p = NULL;
	return _xtTranslateSysError(XT_SOCKET_LAST_ERROR);
}

void xtSocketPollDestroy(struct xtSocketPoll **p)
{
	struct xtSocketPoll *sp = *p;
	if (!sp)
		return;
	free(sp->fds);
	free(sp->data);
	free(sp->readyData);
	free(sp);
	*p = NULL;
}

size_t xtSocketPollGetCapacity(const struct xtSocketPoll *p)
{
	return p->capacity;
}

size_t xtSocketPollGetCount(const struct xtSocketPoll *p)
{
	return p->count;
}

void *xtSocketPollGetData(const struct xtSocketPoll *p, size_t index)
{
	return p->data[index].data;
}

void *xtSocketPollGetReadyData(const struct xtSocketPoll *p, size_t index)
{
	return p->readyData[index].data;
}

enum xtSocketPollEvent xtSocketPollGetReadyEvent(const struct xtSocketPoll *p, size_t index)
{
	return p->readyData[index].events;
}

xtSocket xtSocketPollGetReadySocket(const struct xtSocketPoll *p, size_t index)
{
	return p->readyData[index].fd;
}

xtSocket xtSocketPollGetSocket(const struct xtSocketPoll *p, size_t index)
{
	return p->data[index].fd;
}

int xtSocketPollMod(struct xtSocketPoll *p, xtSocket sock, enum xtSocketPollEvent events)
{
	int capacity = p->capacity;
	for (int i = 0; i < capacity; ++i)
		if (p->fds[i].fd == sock) {
			p->fds[i].events = socket_poll_event_fix_sys_flags(socket_poll_event_flags_to_sys(events));
			return 0;
		}
	return XT_EINVAL;
}

int xtSocketPollRemove(struct xtSocketPoll *p, xtSocket sock)
{
	// Find the socket
	// Clean up possible sensitive data
	int index = -1;
	int capacity = p->capacity;
	for (int i = 0; i < capacity; ++i)
		if (p->fds[i].fd == sock) {
			index = i;
			break;
		}
	if (index == -1)
		return XT_ENOENT;
	return xtSocketPollRemoveByIndex(p, index);
}

int xtSocketPollRemoveByIndex(struct xtSocketPoll *p, size_t index)
{
	int capacity = p->capacity;
	if (index >= (size_t)capacity)
		return XT_EINVAL;
	xtSocket sock = p->fds[index].fd;
	// Clean up the data in the data array
	if (index - 1 == (size_t)capacity) {
		p->fds[index].fd = XT_SOCKET_INVALID_FD;
		p->data[index].fd = XT_SOCKET_INVALID_FD;
		p->data[index].data = NULL;
	} else {
		// Use memmmove, which is much faster than moving everything with a for loop
		// Shift all elements back one position
		memmove(&p->fds[index], &p->fds[index + 1], (p->count - index) * sizeof(struct pollfd));
		memmove(&p->data[index], &p->data[index + 1], (p->count - index) * sizeof(struct _xt_poll_data));
	}
	int socketsReady = p->socketsReady;
	// Locate the socket in the ready array
	for (int i = 0; i < socketsReady; ++i)
		if (p->readyData[i].fd == sock) {
			p->readyData[i].fd = XT_SOCKET_INVALID_FD;
			p->readyData[i].data = NULL;
			break;
		}
	--p->count;
	return 0;
}

int xtSocketPollSetEvent(struct xtSocketPoll *p, xtSocket sock, enum xtSocketPollEvent events)
{
	int socketsReady = p->socketsReady;
	for (int i = 0; i < socketsReady; ++i)
		if (p->readyData[i].fd == sock) {
			p->readyData[i].events = socket_poll_event_flags_to_sys(events);
			return 0;
		}
	return XT_EINVAL;
}

int xtSocketPollWait(struct xtSocketPoll *restrict p, int timeout, size_t *restrict socketsReady)
{
	int eventCount = WSAPoll(p->fds, p->count, timeout);
	if (eventCount == -1) {
		int syserr = XT_SOCKET_LAST_ERROR;
		// Handle this bug in the windows poll. If no sockets are present, it will return immediately
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
	*socketsReady = eventCount;
	for (int i = 0, socketsHandled = 0, capacity = p->capacity; i < capacity && socketsHandled < eventCount; ++i)
		if (p->fds[i].revents != 0) {
			p->readyData[socketsHandled].fd = p->fds[i].fd;
			p->readyData[socketsHandled].data = p->data[i].data;
			p->readyData[socketsHandled].events = socket_poll_event_sys_to_flags(p->fds[i].revents);
			// Resetting it is for WSAPoll not necessary, but it is for us so that we
			// know if something new really happened
			p->fds[i].revents = 0;
			++socketsHandled;
			continue;
		}
	return 0;
}

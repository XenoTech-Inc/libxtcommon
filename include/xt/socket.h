/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @brief Cross platform sockets.
 *
 * My beloved socket library, born in 2014. It is only intended for IPv4.
 * It works on Linux and Windows, not on Mac! Haha.
 * This library makes working with sockets a lot easier, since under the hood
 * a lot of work is done for you, but still its a thousand times faster than Java.
 * No strange things are done like Java does. These sockets are made for speed!
 *
 * All functions are NOT thread safe. Unless otherwise noted.
 * You should treat all structures that are part of the sockets as if they were opaque.
 * Unless otherwise noted.
 * @file socket.h
 * @author Tom Everaarts
 * @date 2014
 * @copyright LGPL v3.0.
 */

#ifndef _XT_SOCKET_H
#define _XT_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/_base.h>
#include <xt/os_macros.h>

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Contains the IP + Port number of a device on the grid.
 *
 * This is a wrapper for struct sockaddr_in, which makes it very easy to use. Using the raw struct sockaddr_in can be a real pain.
 * This struct is POD. It is different across platforms though.
 */
struct xtSockaddr {
#if XT_IS_LINUX
	short sin_family;
	unsigned short sin_port;
	struct {
		unsigned long s_addr;
	} sin_addr;
	char sin_zero[8];
#elif XT_IS_WINDOWS
	short sin_family;
	unsigned short sin_port;
	struct {
		union {
			struct { unsigned char  s_b1, s_b2, s_b3, s_b4; } S_un_b;
			struct { unsigned short s_w1, s_w2; } S_un_w;
			unsigned long S_addr;
		} S_un;
	} sin_addr;
	char sin_zero[8];
#endif
};
/**
 * Checks if two addresses are equal.
 */
bool xtSockaddrEquals(const struct xtSockaddr *sa1, const struct xtSockaddr *sa2);
/**
 * A constructor which translates the IP address from the provided string.
 * The string MAY contain the port to set. If the string contains the port, then that port value is used. If the string does not contain a port, the value of \a port is used.
 * If the translation fails for whatever reason, then the address is left untouched.
 * String IP format : [IP address] OR [IP address]:[Port]
 */
bool xtSockaddrFromString(struct xtSockaddr *restrict sa, const char *restrict addr, uint16_t port);
/**
 * Sets the address from an ipv4 address represented as decimals. The port is always set by \a port.
 */
bool xtSockaddrFromAddr(struct xtSockaddr *sa, uint32_t addr, uint16_t port);
/**
 * Returns the IP address as raw 4 byte number.
 */
uint32_t xtSockaddrGetAddress(const struct xtSockaddr *sa);
/**
 * This address is used when you don't need to bind a socket to a specific IP.
 * When you use this value as the address when binding a socket, the socket accepts connections to all the IPs of the machine.
 */
uint32_t xtSockaddrGetAddressAny(void);
/**
 * The localhost address as we know it: 127.0.0.1. This address is already prepared for direct use by sockets (Correct endianness).
 * @remarks When wanting to bind to all interfaces, this is NOT the address to use.
 * Instead use xtSockaddrGetAddressAny() for that!
 */
uint32_t xtSockaddrGetAddressLocalHost(void);
uint16_t xtSockaddrGetPort(const struct xtSockaddr *sa);
/**
 * Initializes a sockaddr. This MUST be done before it can be used by any sockets.
 * All xtSockaddr functions which SET a value in the sockaddr do this automatically to be safe. By calling this function,
 * you can do it manually incase that is desired.
 */
void xtSockaddrInit(struct xtSockaddr *sa);
void xtSockaddrSetAddress(struct xtSockaddr *sa, uint32_t addr);
void xtSockaddrSetPort(struct xtSockaddr *sa, uint16_t port);
/**
 * Returns this address as a string formatted as : [IP]:[PORT].
 * A NULL pointer is returned on failure to translate the address.
 */
char *xtSockaddrToString(const struct xtSockaddr *restrict sa, char *restrict buf, size_t buflen);
/**
 * @brief All protocols that are supported by the xtSockets.
 */
enum xtSocketProto {
	XT_SOCKET_PROTO_UNKNOWN, XT_SOCKET_PROTO_TCP, XT_SOCKET_PROTO_UDP
};
/**
 * @brief Super speedy sockets.
 *
 * The sockets contains a nice amount of easy to use functions and are optimized for speed.
 * Non-blocking operations are supported!
 */
#if XT_IS_LINUX
	typedef int xtSocket;
	#define XT_SOCKET_INVALID_FD (-1)
#elif XT_IS_WINDOWS
	typedef uintptr_t xtSocket;
	#define XT_SOCKET_INVALID_FD ((xtSocket) (~0))
#endif
/**
 * The maximum payload size of an ipv4 TCP packet.\n
 * Keep in mind that using this ridiculously huge size is not a good idea since the MTU on most systems is just 1500 bytes.
 */
#define XT_SOCKET_TCP_MAXIMUM_PAYLOAD_SIZE 65535
/**
 * The maximum payload size of an ipv4 UDP packet.\n
 * Keep in mind that using this ridiculously huge size is not a good idea since the MTU on most systems is just 1500 bytes.
 * This value is the ABSOLUTE limit, given that a standard UDP header is 20 bytes. The UDP header can be extended to 60 bytes, which would make the max payload size 40 bytes smaller than this value.
 * However, no normal packet should have this huge header, or a payload of this size.
 */
#define XT_SOCKET_UDP_MAXIMUM_PAYLOAD_SIZE 65507
/**
 * Binds the socket to the specified interface.
 * @param port - The port to bind to. Port 0 lets the kernel pick a random port.
 * @return Zero if the socket has been bound successfully, otherwise an error code.
 * @remarks Ports below 1024 may require admin rights.
 */
int xtSocketBindTo(xtSocket sock, const struct xtSockaddr *sa);
/**
 * Binds the socket to the specified interface.
 * @param port - The port to bind to. Port 0 lets the kernel pick a random port.
 * @return Zero if the socket has been bound successfully, otherwise an error code.
 * @remarks Ports below 1024 may require admin rights.
 */
int xtSocketBindToAny(xtSocket sock, uint16_t port);
/**
 * Closes the socket and releases all system resources associated with it. the socket is unuseable after calling this function.
 * Any threads waiting on the socket for a blocking operation should wake up.
 * @return Zero if the socket has been closed, otherwise an error code.
 */
int xtSocketClose(xtSocket *sock);
/**
 * This function has multiple usages. They differ both for TCP & UDP.\n
 * TCP : Simply connects the socket to \a dest. This will fail if the remote device is offline.\n
 * UDP : After a successful call to this function, the UDP socket will ONLY accept data that came from \a dest.
 * Data that came from other devices is now simply discarded, as if it never existed. The kernel takes care of this for us.
 * @return Zero if the socket has connected successfully, otherwise an error code.
 */
int xtSocketConnect(xtSocket sock, const struct xtSockaddr *dest);
/**
 * Creates a new socket which is directly available for use.
 * This socket must always be closed by xtSocketClose().
 * @param sock - A pointer to the socket which is to be initialized.
 * @param proto - The protocol that the socket should be using.
 * @return Zero if the socket has been created, otherwise an error code.
 */
int xtSocketCreate(xtSocket *sock, enum xtSocketProto proto);
/**
 * Cleans up any resources that were necessary for the sockets to function.
 * Failure to call this function may lead to the leakage of system resources.
 * After calling this function, you must initialize the socket system again before attempting
 * to use any socket functionality.
 */
void xtSocketDestruct(void);
/**
 * Tells you the address of the interface where \a sock is bound to.
 * Even if the socket is not bound yet, this function will succeed.
 * @param sa - A pointer to the structure which will receive the address of the interface.
 * @return Zero if the address has been retrieved, otherwise an error code.
 */
int xtSocketGetLocalSocketAddress(xtSocket sock, struct xtSockaddr *sa);
/**
 * Tells you the port of the interface where \a sock is bound to.
 * On error, zero is returned.
 */
uint16_t xtSocketGetLocalPort(xtSocket sock);
/**
 * Returns the protocol that is associated with this socket. If any error occurs,
 * XT_SOCKET_PROTO_UNKNOWN is returned.
 */
enum xtSocketProto xtSocketGetProtocol(const xtSocket sock);
/**
 * Tells you the address of the peer connected to \a sock.
 * @return Zero if the address has been retrieved, otherwise an error code.
 */
int xtSocketGetRemoteSocketAddress(const xtSocket sock, struct xtSockaddr *sa);
/**
 * Tells you if an error has occurred on \a sock. After a successful call to this function,
 * the error code is cleared.
 * @return Zero if the property has been fetched successfully, otherwise an error code.
 */
int xtSocketGetSoError(const xtSocket sock, int *errnum);
/**
 * Tells you if the socket has it's SO_KEEPALIVE option enabled or disabled.
 * @param flag - Will receive the result of the property on success.
 * @return Zero if the property has been fetched successfully, otherwise an error code.
 */
int xtSocketGetSoKeepAlive(const xtSocket sock, bool *flag);
/**
 * Tells you the current linger settings for the socket.
 * @return Zero if the properties have been fetched successfully, otherwise an error code.
 */
int xtSocketGetSoLinger(const xtSocket sock, bool *restrict on, int *restrict linger);
/**
 * Tells you the current SO_RCVBUF size in bytes for the specified socket.
 * @return Zero if the property has been fetched successfully, otherwise an error code.
 */
int xtSocketGetSoReceiveBufferSize(xtSocket sock, unsigned *size);
/**
 * Tells you if the socket has it's SO_REUSEADDR option enabled or disabled.
 * @param flag - Will receive the result of the property on success.
 * @return Zero if the property has been fetched successfully, otherwise an error code.
 */
int xtSocketGetSoReuseAddress(const xtSocket sock, bool *flag);
/**
 * Tells you the current SO_SNDBUF size in bytes for the specified socket.
 * @return Zero if the property has been fetched successfully, otherwise an error code.
 */
int xtSocketGetSoSendBufferSize(xtSocket sock, unsigned *size);
/**
 * Tells you if the socket has it's TCP_NODELAY option enabled or disabled.
 * @param flag - Will receive the result of the property on success.
 * @return Zero if the property has been fetched successfully, otherwise an error code.
 */
int xtSocketGetTCPNoDelay(const xtSocket sock, bool *flag);
/**
 * Initializes the socket system to enable socket functionality useage. You should call
 * xtSocketDestruct() after socket functionality is no longer needed.
 * @return True if the socket system has been initialized or is already initialized, false otherwise.
 * The reason for failure can vary across systems, so there is no error code for this.
 */
bool xtSocketInit(void);
/**
 * Tells you if the specified socket is closed. A socket is only closed when it has been closed explicitly.
 */
bool xtSocketIsClosed(const xtSocket sock);
/**
 * Tells you if the specified socket is still open. A socket is still open when it has not yet been closed explicitly.
 */
bool xtSocketIsOpen(const xtSocket sock);
/**
 * Marks the specified socket as a passive socket willing to accept connections to it.
 * If this function succeeds, you will not be able to use the socket for any other purpose
 * then accepting connections.
 * @param sock - A pointer to the socket to use. It must be a TCP socket. Also the should already be bound
 * prior to calling this function . Otherwise you will get problems later on.
 * @param backlog - The maximum length to which the queue of pending connections for \a sock may grow.
 * If a connection request arrives when the queue is full, the client may receive an error with
 * an indication of connection refused. Leave this parameter zero to default to a recommended value.
 * @return Zero if the socket has been put into listen mode, otherwise an error code.
 */
int xtSocketListen(xtSocket sock, unsigned backlog);
/**
 * Sets a socket to blocking mode or non-blocking mode. Sockets are by default always in blocking mode.
 * This means that when socket functions are called, the calling thread is paused until the kernel has
 * processed the request. With non-blocking sockets functions can return immediately without blocking.
 * @param flag - Specify true to go to blocking mode. Specify false to go to non-blocking mode.
 * @return Zero if the option has been changed successfully, otherwise an error code.
 */
int xtSocketSetBlocking(xtSocket sock, bool flag);
/**
 * Enables or disables SO_KEEPALIVE.
 * When this socket option is enabled, the TCP stack sends keep-alive packets when no data or acknowledgement packets have been
 * received for the connection within an interval to detect if the link between two sockets is broken.
 * This option is off by default.
 * @return Zero if the option has been changed successfully, otherwise an error code.
 */
int xtSocketSetSoKeepAlive(xtSocket sock, bool flag);
/**
 * Enables or disables SO_LINGER.
 * This value decides how the socket behaves after it is closed. When lingering is enabled, the kernel keeps the socket in a TIME_WAIT state, to ensure
 * that any remaining data will be sent to the remote side, and the close is acknowledged. Normally you should leave this option alone, as you generally want this to be on.
 * However, when dealing with thousands of sockets, you may want them to dissipate immediately after closing, so you should disable lingering on those sockets.
 * Remember : Lingering sockets still occupy a port, and eat system resources. Even if your program has already terminated!
 * @param on - If lingering should be enabled or not.
 * @param linger - The timeout value for SO_LINGER in seconds. The maximum timeout value is platform specific.
 * @return Zero if the option has been changed successfully, otherwise an error code.
 */
int xtSocketSetSoLinger(xtSocket sock, bool on, int linger);
/**
 * Sets the SO_RCVBUF option to the specified value for this socket.
 * The SO_RCVBUF option is used by the platform's networking code as a hint for the size to set the receive buffer of the underlying I/O buffers.
 * Because this is a just a hint to the implementation, you should check the buffers afterwards by calling xtSocketGetSoReceiveBufferSize().
 * @remarks It is best practice to call this function before connecting or binding the socket. This prevents certain problems.
 */
int xtSocketSetSoReceiveBufferSize(xtSocket sock, unsigned size);
/**
 * Enables or disables SO_REUSEADDR.
 * When a socket is closed the connection may remain in a timeout state for a period of time after the connection is closed.
 * For applications using a well known socket address or port it may not be possible to bind a socket to that interface if there is a connection in the timeout state involving the socket address or port.
 * Enabling SO_REUSEADDR prior to binding the socket allows the socket to be bound even though a previous connection is in a timeout state.\n
 * When a socket is created, this option is off by default.
 * @return Zero if the option has been changed successfully, otherwise an error code.
 * @remarks Execute this function PRIOR to binding the socket! Otherwise this function will have no effect.
 */
int xtSocketSetSoReuseAddress(xtSocket sock, bool flag);
/**
 * Sets the SO_SNDBUF option to the specified value for this socket.
 * The SO_SNDBUF option is used by the platform's networking code as a hint for the size to set the send buffer of the underlying I/O buffers.
 * Because this is a just a hint to the implementation, you should check the buffers afterwards by calling xtSocketGetSoSendBufferSize().
 * @return Zero if the option has been changed successfully, otherwise an error code.
 * @remarks It is best practice to call this function before connecting or binding the socket. This prevents certain problems.
 */
int xtSocketSetSoSendBufferSize(xtSocket sock, unsigned size);
/**
 * Enables or disables TCP_NODELAY.\n
 * \a flag = true : Send the data (partial frames) the moment you get them, regardless if you have enough frames for a full network packet.\n
 * \a flag = false : Enable Nagle's algorithm  which means send the data when it is bigger than the MSS or waiting for the receiving acknowledgement before sending data which is smaller.\n
 * @return Zero if the option has been changed successfully, otherwise an error code.
 */
int xtSocketSetTCPNoDelay(xtSocket sock, bool flag);
/**
 * Blocks until an incoming TCP connection is accepted or until the socket is closed, or if the socket is non-blocking,
 * it will return immediately.
 * @param peerSock - This parameter will be filled with the socket of the peer that has connected.
 * @param peerAddr - This parameter will be filled with the address of the peer socket.
 * @return Zero if a peer has connected successfully, otherwise an error code.
 * @remarks The socket must be in listen mode for this function to work.
 */
int xtSocketTCPAccept(xtSocket sock, xtSocket *restrict peerSock, struct xtSockaddr *restrict peerAddr);
/**
 * Blocks until "some" data has been read on the socket. This does not necessarily have to be the size of \a buflen.
 * @param bytesRead - Receives the amount of bytes that have been read.
 * @returns Zero if the operation has succeeded, otherwise an error code.
 */
int xtSocketTCPRead(xtSocket sock, void *restrict buf, uint16_t buflen, uint16_t *restrict bytesRead);
/**
 * Writes the data in \a buf to the connected remote socket.
 * @param bytesSent - Receives the amount of bytes that have been sent.
 * @returns Zero if the operation has succeeded, otherwise an error code.
 */
int xtSocketTCPWrite(xtSocket sock, const void *restrict buf, uint16_t buflen, uint16_t *restrict bytesSent);
/**
 * Blocks until "some" data has been read on the socket. This does not necessarily have to be the size of \a buflen.
 * @param bytesRead - Receives the amount of bytes that have been read.
 * @param sender - Receives the address of the sender.
 * @returns Zero if the operation has succeeded, otherwise an error code.
 */
int xtSocketUDPRead(xtSocket sock, void *restrict buf, uint16_t buflen, uint16_t *restrict bytesRead, struct xtSockaddr *restrict sender);
/**
 * Writes the data in \a buf to the address of \a dest.
 * @param bytesSent - Receives the amount of bytes that have been sent.
 * @param dest - Contains the address of the destination. For a connected UDP socket, you can specify a NULL pointer.
 * @returns Zero if the operation has succeeded, otherwise an error code.
 */
int xtSocketUDPWrite(xtSocket sock, const void *restrict buf, uint16_t buflen, uint16_t *restrict bytesSent, const struct xtSockaddr *restrict dest);

#define XT_SOCKET_POLL_CAPACITY_DEFAULT 1024
/**
 * @brief Declaration for an opaque pointer.
 *
 * Any sockets which have events waiting on them, are referenced to as "ready sockets".
 * These ready sockets are stored in seperate arrays, hence there are several other
 * functions for them.
 */
struct xtSocketPoll;
/**
 * @brief All types of events that can occur on a socket.
 */
enum xtSocketPollEvent {
	/** No event has occurred. */
	XT_POLLNONE = 0x01,
	/** Normal data can be read without blocking. */
	XT_POLLIN   = 0x02,
	/** Normal data can be written without blocking. */
	XT_POLLOUT  = 0x04,
	/** An error has occurred. */
	XT_POLLERR  = 0x08,
	/** A stream-oriented connection was either disconnected or aborted. */
	XT_POLLHUP  = 0x10
};
/**
 * Adds a socket for monitoring.
 * After a successful call to this function, the socket will be monitored for the
 * specified events by the system.
 * XT_POLLERR and XT_POLLHUP are always added implicitly.
 * @param data - The data to associate with the socket.
 * @param events - The events which are to be monitored.
 */
int xtSocketPollAdd(struct xtSocketPoll *restrict p, xtSocket sock, void *restrict data, enum xtSocketPollEvent events);
/**
 * Initiates the poll structure for socket monitoring.
 * @param capacity - The amount of sockets that will fit into the structure.
 */
int xtSocketPollCreate(struct xtSocketPoll **p, size_t capacity);
/**
 * Destroys the structure and cleans up all resources.
 * The structure is rendered unuseable after calling this function.
 * The sockets remain unaffected.
 */
void xtSocketPollDestroy(struct xtSocketPoll **p);

size_t xtSocketPollGetCapacity(const struct xtSocketPoll *p);

size_t xtSocketPollGetCount(const struct xtSocketPoll *p);
/**
 * Returns the data that is associated with the socket at \a index.
 * @remarks No bounds checking is performed. Specifying a too high index
 * results in undefined behavior.
 */
void *xtSocketPollGetData(const struct xtSocketPoll *p, size_t index);
/**
 * Returns the data that is associated with the ready socket at \a index.
 * @remarks No bounds checking is performed. Specifying a too high index
 * results in undefined behavior.
 */
void *xtSocketPollGetReadyData(const struct xtSocketPoll *p, size_t index);
/**
 * Returns the current event that is happening on the ready socket at \a index.
 * @remarks No bounds checking is performed. Specifying a too high index
 * results in undefined behavior.
 */
enum xtSocketPollEvent xtSocketPollGetReadyEvent(const struct xtSocketPoll *p, size_t index);
/**
 * Returns the ready socket at \a index.
 * @remarks No bounds checking is performed. Specifying a too high index
 * results in undefined behavior.
 */
xtSocket xtSocketPollGetReadySocket(const struct xtSocketPoll *p, size_t index);
/**
 * Returns the socket at \a index.
 * @remarks No bounds checking is performed. Specifying a too high index
 * results in undefined behavior.
 */
xtSocket xtSocketPollGetSocket(const struct xtSocketPoll *p, size_t index);
/**
 * Modifies the events for which a socket will be monitored. This will take effect
 * on the next call to xtSocketPollWait().
 * @returns Zero if the socket was found and modified, otherwise an error code.
 */
int xtSocketPollMod(struct xtSocketPoll *p, xtSocket sock, enum xtSocketPollEvent events);
/**
 * Removes the specified socket from monitoring. The socket will be invalidated in the ready array.
 * It's file descriptor will be set to XT_SOCKET_INVALID_FD and it's data set to NULL.
 * Do note that this means it is still present in the ready array until the next call to xtSocketPollWait().
 * @returns Zero if the socket was found and is removed, otherwise an error code.
 */
int xtSocketPollRemove(struct xtSocketPoll *p, xtSocket sock);
/**
 * Same as xtSocketPollRemove(), only then a bit faster because you already know the index
 * of the socket. Bounds checking is performed.
 */
int xtSocketPollRemoveByIndex(struct xtSocketPoll *p, size_t index);
/**
 * Sets the current event(s) for a ready socket.
 */
int xtSocketPollSetEvent(struct xtSocketPoll *p, xtSocket sock, enum xtSocketPollEvent event);
/**
 * Determines the status of one or more sockets.
 * All sockets are automically rearmed for the next call to this function.
 * This means that if some sockets have data waiting to be read, and you skip reading
 * it, the next call to this function will return immediately with those same sockets.
 * @param timeout - The time to wait at maximum before returning in milliseconds.
 * Different values are accepted.
 * -1: Block indefinitely.
 * 0 : Return immediately.
 * >1: Block for that amount of time at maximum.
 * @param socketsReady - Will receive the amount of sockets which are ready. This is
 * left untouched on error.
 * @return Zero is the function has executed successfully, otherwise an error code.
 */
int xtSocketPollWait(struct xtSocketPoll *restrict p, int timeout, size_t *restrict socketsReady);

#ifdef __cplusplus
}
#endif

#endif

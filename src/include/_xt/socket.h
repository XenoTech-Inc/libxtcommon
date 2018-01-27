/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#ifndef __XT_SOCKET_H
#define __XT_SOCKET_H

// XT headers
#include <_xt/error.h>
#include <xt/endian.h>
#include <xt/error.h>
#include <xt/socket.h>
#include <xt/os_macros.h>

#if XT_IS_LINUX

// System headers
#include <arpa/inet.h> // sockaddr_in struct and functions to format IP addresses and ports
#include <errno.h> // for the error macros
#include <fcntl.h> // fcntl function
#include <netdb.h> // hostent struct, gethostbyname(), needed to convert a char* to in_addr
#include <netinet/tcp.h> // For TCP_NODELAY and such
#include <sys/epoll.h> // for epoll
#include <sys/socket.h> // for the socket function
#include <unistd.h> // close function

// Some macros that spare us a lot of typing
#define XT_SOCKET_LAST_ERROR errno

#else

// XT headers
#include <xt/time.h> // sleep

// System headers
#include <ws2tcpip.h> // Necessary for a hell lot of stuff
#include <windows.h> // Do this after including winsock (ws2tcpip includes it for us)

// Some macros that spare us a lot of typing
#define XT_SOCKET_LAST_ERROR WSAGetLastError()
#define close closesocket
#define SHUT_RDWR SD_BOTH

#endif

// STD headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Converts an xtSocketProto to the native representation.
 * @return True of the conversion was successful, false otherwise.
 */
static bool socket_proto_to_native_proto(enum xtSocketProto proto, int *restrict nativeType, int *restrict nativeProto)
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

/**
 * Converts a native socket protocol to it's xt representation.
 * XT_SOCKET_PROTO_UNKNOWN is returned if the protocol is unsupported.
 */
static enum xtSocketProto socket_native_proto_to_proto(int nativeProto)
{
	switch (nativeProto) {
	case IPPROTO_TCP: return XT_SOCKET_PROTO_TCP;
	case IPPROTO_UDP: return XT_SOCKET_PROTO_UDP;
	default:          return XT_SOCKET_PROTO_UNKNOWN;
	}
}

#endif

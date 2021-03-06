/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @brief Used for error handling.
 *
 * @file error.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright LGPL v3.0.
 */

#ifndef _XT_ERROR_H
#define _XT_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/_base.h>

// STD headers
#include <stddef.h>

#define XT_EPERM           1   /* Operation not permitted */
#define XT_EINTR           2   /* Interrupted system call */
#define XT_EIO             3   /* I/O error */
#define XT_EAGAIN          4   /* Try again / Operation would block */
#define XT_ENOMEM          5   /* Out of memory */
#define XT_EACCES          6   /* Permission denied */
#define XT_EINVAL          7   /* Invalid argument */
#define XT_EBADF           8   /* Bad file descriptor */
#define XT_EMFILE          9   /* Too many open files */
#define XT_ETIMEDOUT       10  /* Timeout expired */
#define XT_EOVERFLOW       11  /* Value too large for defined data type */

#define XT_EDESTADDRREQ    12  /* Destination address required */
#define XT_EMSGSIZE        13  /* Message too long */
#define XT_ESOCKTNOSUPPORT 14  /* Socket type not supported */
#define XT_EOPNOTSUPP      15  /* Operation not supported on transport endpoint */
#define XT_EADDRINUSE      16  /* Address already in use */
#define XT_EADDRNOTAVAIL   17  /* Cannot assign requested address */
#define XT_ENETDOWN        18  /* Network is down */
#define XT_ENETUNREACH     19  /* Network is unreachable */
#define XT_ENETRESET       20  /* Network dropped connection because of reset */
#define XT_ECONNABORTED    21  /* Software caused connection abort */
#define XT_ECONNRESET      22  /* Connection reset by peer */
#define XT_ENOBUFS         23  /* No buffer space available */
#define XT_EISCONN         24  /* Transport endpoint is already connected */
#define XT_ENOTCONN        25  /* Transport endpoint is not connected */
#define XT_ESHUTDOWN       26  /* Cannot send after transport endpoint shutdown */
#define XT_ECONNREFUSED    27  /* Connection refused */
#define XT_EHOSTDOWN       28  /* Host is down */
#define XT_EHOSTUNREACH    29  /* No route to host */
#define XT_EALREADY        30  /* Operation already in progress */
#define XT_EINPROGRESS     31  /* Operation now in progress */
#define XT_EPIPE           32  /* Broken pipe */
#define XT_EBUSY           33  /* Device or resource busy */
#define XT_EEXIST          34  /* File exists */
#define XT_EFAULT          35  /* Bad address */
#define XT_ENOENT          36  /* No such file or directory */
#define XT_ENOTEMPTY       37  /* Directory not empty */
#define XT_E2BIG           38  /* Arg list too long */
#define XT_ENOTDIR         39  /* Not a directory */
#define XT_EISDIR          40  /* Is a directory */
#define XT_ENAMETOOLONG    41  /* File name too long */
#define XT_EAFNOSUPPORT    42  /* Address family not supported by protocol */
#define XT_EPROTONOSUPPORT 43  /* Protocol not supported */
#define XT_ELOOP           44  /* Too many symbolic links encountered */
#define XT_ENOSPC          45  /* No space left on device */
#define XT_ENOTSOCK        46  /* Socket operation on non-socket */
#define XT_EXDEV           47  /* Cross-device link */
#define XT_EUNKNOWN        48  /* Unknown error */
#define XT_EMAXRANGE       XT_EUNKNOWN

const char *xtGetErrorStr(int errnum);
/**
 * Acts exactly the same as perror() on Linux.
 * The error is printed to stderr, along with an optional error message.
 * @param msg - Optional message to be printed. This may be a NULL pointer.
 * @param errnum - An XT error code.
 */
void xtPerror(const char *msg, int errnum);
/**
 * Acts like strerror() on posix but you have to provide the buffer.
 * The provided buffer is also returned.
 * @param buf - The provided buffer
 * @param buflen - The provided buffer length including null terminator
 * @param errnum - An XT error code
 * @return The provided buffer
 */
char *xtStrError(char *buf, size_t buflen, int errnum);

#ifdef __cplusplus
}
#endif

#endif

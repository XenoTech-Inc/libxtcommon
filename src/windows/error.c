// XT headers
#include <xt/error.h>

// System headers
#include <winerror.h>

// STD headers
#include <stdio.h>
#include <string.h>

/**
 * struct errormap - map string errors from Plan 9 to Linux numeric ids
 * @name: string sent over 9P
 * @val: numeric id most closely representing @name
 * @namelen: length of string
 * @list: hash-table list for string lookup
 */
typedef struct xtErrorMap {
	const char *name;
	int val;
} xtErrorMap;

static const xtErrorMap errorMap[] = {
	{ "Success", 0 },
	{ "Operation not permitted", XT_EPERM },
	{ "Interrupted system call", XT_EINTR },
	{ "I/O error", XT_EIO },
	{ "Try again / Operation would block", XT_EAGAIN },
	{ "Out of memory", XT_ENOMEM },
	{ "Permission denied", XT_EACCES },
	{ "Invalid argument", XT_EINVAL },
	{ "Bad file descriptor", XT_EBADF },
	{ "Too many open files", XT_EMFILE },
	{ "Timeout expired", XT_ETIMEDOUT },
	{ "Value too large for defined data type", XT_EOVERFLOW },
	{ "Destination address required", XT_EDESTADDRREQ },
	{ "Message too long", XT_EMSGSIZE },
	{ "Socket type not supported", XT_ESOCKTNOSUPPORT },
	{ "Operation not supported on transport endpoint", XT_EOPNOTSUPP },
	{ "Address already in use", XT_EADDRINUSE },
	{ "Cannot assign requested address", XT_EADDRNOTAVAIL },
	{ "Network is down", XT_ENETDOWN },
	{ "Network is unreachable", XT_ENETUNREACH },
	{ "Network dropped connection because of reset", XT_ENETRESET },
	{ "Software caused connection abort", XT_ECONNABORTED },
	{ "Connection reset by peer", XT_ECONNRESET },
	{ "No buffer space available", XT_ENOBUFS },
	{ "Transport endpoint is already connected", XT_EISCONN },
	{ "Transport endpoint is not connected", XT_ENOTCONN },
	{ "Cannot send after transport endpoint shutdown", XT_ESHUTDOWN },
	{ "Connection refused", XT_ECONNREFUSED },
	{ "Host is down", XT_EHOSTDOWN },
	{ "No route to host", XT_EHOSTUNREACH },
	{ "Operation already in progress", XT_EALREADY },
	{ "Operation now in progress", XT_EINPROGRESS },
	{ "Broken pipe", XT_EPIPE },
	{ "Device or resource busy", XT_EPIPE },
	{ "File exists", XT_EEXIST },
	{ "Bad address", XT_EFAULT },
	{ "No such file or directory", XT_ENOENT },
	{ "Directory not empty", XT_ENOTEMPTY },
	{ "Arg list too long", XT_E2BIG },
	{ "Not a directory", XT_ENOTDIR },
	{ "Is a directory", XT_EISDIR },
	{ "File name too long", XT_ENAMETOOLONG },
	{ "Address family not supported by protocol", XT_EAFNOSUPPORT },
	{ "Protocol not supported", XT_EPROTONOSUPPORT },
	{ "Too many symbolic links encountered", XT_ELOOP },
	{ "No space left on device", XT_ENOSPC },
	{ "Socket operation on non-socket", XT_ENOTSOCK },
	{ "Cross-device link", XT_EXDEV },
	{ "Unknown error", XT_EUNKNOWN }
};

const char *xtGetErrorStr(int errnum)
{
	errnum = errnum >= XT_EMAXRANGE || errnum < 0 ? XT_EUNKNOWN : errnum;
	return errorMap[errnum].name;
}

void xtPerror(const char *msg, int errnum)
{
	if (msg && msg[0] != '\0') { // If the string is longer than zero characters also!
		if (errnum >= XT_EMAXRANGE || errnum < 0)
			fprintf(stderr, "%s: %s %d\n", msg, xtGetErrorStr(errnum), errnum);
		else
			fprintf(stderr, "%s: %s\n", msg, xtGetErrorStr(errnum));
	} else {
		if (errnum >= XT_EMAXRANGE || errnum < 0)
			fprintf(stderr, "%s %d\n", xtGetErrorStr(errnum), errnum);
		else
			fprintf(stderr, "%s\n", xtGetErrorStr(errnum));
	}
}

char *xtStrError(char *buf, size_t buflen, int errnum)
{
	if (errnum >= XT_EMAXRANGE || errnum < 0)
		snprintf(buf, buflen, "%s %d", xtGetErrorStr(errnum), errnum);
	else
		snprintf(buf, buflen, "%s", xtGetErrorStr(errnum));
	return buf;
}

int _xtTranslateSysError(int syserrnum)
{
	switch (syserrnum) {
	case 0:                                 return 0;
	case ERROR_NOACCESS:
	case WSAEACCES:                         return XT_EACCES;
	case ERROR_ADDRESS_ALREADY_ASSOCIATED:
	case WSAEADDRINUSE:                     return XT_EADDRINUSE;
	case WSAEADDRNOTAVAIL:                  return XT_EADDRNOTAVAIL;
	case WSAEAFNOSUPPORT:                   return XT_EAFNOSUPPORT;
	case WSAEWOULDBLOCK:                    return XT_EAGAIN;
	case WSAEALREADY:                       return XT_EALREADY;
	case ERROR_INVALID_FLAGS:
	case ERROR_INVALID_HANDLE:              return XT_EBADF;
	case ERROR_LOCK_VIOLATION:
	case ERROR_PIPE_BUSY:
	case ERROR_SHARING_VIOLATION:           return XT_EBUSY;
	case ERROR_OPERATION_ABORTED:
	case WSAEINTR:                          return XT_EINTR;
	case ERROR_CONNECTION_ABORTED:
	case WSAECONNABORTED:                   return XT_ECONNABORTED;
	case ERROR_CONNECTION_REFUSED:
	case WSAECONNREFUSED:                   return XT_ECONNREFUSED;
	case ERROR_NETNAME_DELETED:
	case WSAECONNRESET:                     return XT_ECONNRESET;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:                 return XT_EEXIST;
	case ERROR_BUFFER_OVERFLOW:
	case WSAEFAULT:                         return XT_EFAULT;
	case ERROR_HOST_UNREACHABLE:
	case WSAEHOSTUNREACH:                   return XT_EHOSTUNREACH;
	case ERROR_INSUFFICIENT_BUFFER:
	case ERROR_INVALID_DATA:
	case ERROR_INVALID_PARAMETER:
	case WSAEINVAL:                         return XT_EINVAL;
	case WSAESOCKTNOSUPPORT:                return XT_ESOCKTNOSUPPORT;
	case ERROR_BEGINNING_OF_MEDIA:
	case ERROR_BUS_RESET:
	case ERROR_CRC:
	case ERROR_DEVICE_DOOR_OPEN:
	case ERROR_DEVICE_REQUIRES_CLEANING:
	case ERROR_DISK_CORRUPT:
	case ERROR_EOM_OVERFLOW:
	case ERROR_FILEMARK_DETECTED:
	case ERROR_GEN_FAILURE:
	case ERROR_INVALID_BLOCK_LENGTH:
	case ERROR_IO_DEVICE:
	case ERROR_NO_DATA_DETECTED:
	case ERROR_NO_SIGNAL_SENT:
	case ERROR_OPEN_FAILED:
	case ERROR_SETMARK_DETECTED:
	case ERROR_SIGNAL_REFUSED:              return XT_EIO;
	case WSAEISCONN:                        return XT_EISCONN;
	case ERROR_CANT_RESOLVE_FILENAME:       return XT_ELOOP;
	case ERROR_TOO_MANY_OPEN_FILES:
	case WSAEMFILE:                         return XT_EMFILE;
	case WSAEMSGSIZE:                       return XT_EMSGSIZE;
	case ERROR_FILENAME_EXCED_RANGE:        return XT_ENAMETOOLONG;
	case ERROR_NETWORK_UNREACHABLE:
	case WSAENETUNREACH:                    return XT_ENETUNREACH;
	case WSAENETDOWN:                       return XT_ENETDOWN;
	case WSAENETRESET:                      return XT_ENETRESET;
	case WSAENOBUFS:                        return XT_ENOBUFS;
	case ERROR_BAD_PATHNAME:
	case ERROR_DIRECTORY:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_INVALID_NAME:
	case ERROR_INVALID_DRIVE:
	case ERROR_INVALID_REPARSE_DATA:
	case ERROR_MOD_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case WSAHOST_NOT_FOUND:
	case WSANO_DATA:                        return XT_ENOENT;
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:                 return XT_ENOMEM;
	case ERROR_CANNOT_MAKE:
	case ERROR_DISK_FULL:
	case ERROR_EA_TABLE_FULL:
	case ERROR_END_OF_MEDIA:
	case ERROR_HANDLE_DISK_FULL:            return XT_ENOSPC;
	case ERROR_NOT_CONNECTED:
	case WSAENOTCONN:                       return XT_ENOTCONN;
	case ERROR_DIR_NOT_EMPTY:               return XT_ENOTEMPTY;
	case WSAENOTSOCK:                       return XT_ENOTSOCK;
	case ERROR_NOT_SUPPORTED:               return XT_EOPNOTSUPP;
	case ERROR_BROKEN_PIPE:                 return XT_EPIPE; // EOF?
	case ERROR_ACCESS_DENIED:
	case ERROR_PRIVILEGE_NOT_HELD:          return XT_EPERM;
	case ERROR_BAD_PIPE:
	case ERROR_NO_DATA:
	case ERROR_PIPE_NOT_CONNECTED:
	case WSAEPROTONOSUPPORT:                return XT_EPROTONOSUPPORT;
	case ERROR_SEM_TIMEOUT:
	case WSAETIMEDOUT:                      return XT_ETIMEDOUT;
	case ERROR_NOT_SAME_DEVICE:             return XT_EXDEV;
	case ERROR_INVALID_FUNCTION:            return XT_EISDIR;
	case ERROR_META_EXPANSION_TOO_LONG:     return XT_E2BIG;
	case WSAEINPROGRESS:                    return XT_EINPROGRESS;
	case WSAESHUTDOWN:                      return XT_ESHUTDOWN;
	default:                                return XT_EUNKNOWN;
	}
}

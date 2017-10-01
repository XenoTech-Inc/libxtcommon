// XT headers
#include <xt/error.h>

// System headers
#include <errno.h>

// STD headers
#include <stdio.h>
#include <string.h>

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

int _xtTranslateSysError(int syserrnum)
{
	switch (syserrnum) {
	case 0:                              return 0;
	case EACCES:                         return XT_EACCES;
	case EADDRINUSE:                     return XT_EADDRINUSE;
	case EADDRNOTAVAIL:                  return XT_EADDRNOTAVAIL;
	case EAFNOSUPPORT:                   return XT_EAFNOSUPPORT;
	case EALREADY:                       return XT_EALREADY;
	case EBADFD:
	case EBADF:                          return XT_EBADF;
	case EBUSY:                          return XT_EBUSY;
	case EINTR:                          return XT_EINTR;
	case ECONNABORTED:                   return XT_ECONNABORTED;
	case ECONNREFUSED:                   return XT_ECONNREFUSED;
	case ECONNRESET:                     return XT_ECONNRESET;
	case EEXIST:                         return XT_EEXIST;
	case EFAULT:                         return XT_EFAULT;
	case EHOSTUNREACH:                   return XT_EHOSTUNREACH;
	case ESOCKTNOSUPPORT:                return XT_ESOCKTNOSUPPORT;
	case EINVAL:                         return XT_EINVAL;
	case EIO:                            return XT_EIO;
	case EISCONN:                        return XT_EISCONN;
	case ELOOP:                          return XT_ELOOP;
	case ENFILE:
	case EMFILE:                         return XT_EMFILE;
	case EMSGSIZE:                       return XT_EMSGSIZE;
	case ENAMETOOLONG:                   return XT_ENAMETOOLONG;
	case ENETUNREACH:                    return XT_ENETUNREACH;
	case ENETDOWN:                       return XT_ENETDOWN;
	case ENETRESET:                      return XT_ENETRESET;
	case ENOBUFS:                        return XT_ENOBUFS;
	case ENOENT:                         return XT_ENOENT;
	case ENOMEM:                         return XT_ENOMEM;
	case ENOSPC:                         return XT_ENOSPC;
	case ENOTCONN:                       return XT_ENOTCONN;
	case ENOTEMPTY:                      return XT_ENOTEMPTY;
	case ENOTSOCK:                       return XT_ENOTSOCK;
	case EOPNOTSUPP:                     return XT_EOPNOTSUPP;
	case EPIPE:                          return XT_EPIPE;
	case EPERM:                          return XT_EPERM;
	case EPROTONOSUPPORT:                return XT_EPROTONOSUPPORT;
	case ETIME:
	case ETIMEDOUT:                      return XT_ETIMEDOUT;
	case EXDEV:                          return XT_EXDEV;
	case ENOTDIR:                        return XT_ENOTDIR;
	case EISDIR:                         return XT_EISDIR;
	case E2BIG:                          return XT_E2BIG;
	case EINPROGRESS:                    return XT_EINPROGRESS;
	case ESHUTDOWN:                      return XT_ESHUTDOWN;
	default:                             return (syserrnum == EAGAIN || syserrnum == EWOULDBLOCK) ? XT_EAGAIN : XT_EUNKNOWN;
	}
}

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

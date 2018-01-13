// XT headers
#include <xt/file.h>
#include <_xt/error.h>
#include <xt/error.h>
#include <xt/sort.h>
#include <xt/string.h>

// System headers
#include <dirent.h>
#include <errno.h>
#include <strings.h>
#include <sys/stat.h> // for the "stat" call, to obtain a file's size
#include <unistd.h> // necessary for the stat struct

// STD headers
#include <stdlib.h>
#include <string.h>

int xtFileIteratorStart(
	struct xtFileIterator *restrict handle,
	const char *restrict path)
{
	if (!(handle->dir = opendir(path)))
		return _xtTranslateSysError(errno);
	handle->fileCount = 0;
	return 0;
}

int xtFileIteratorNext(struct xtFileIterator *restrict handle, char *restrict buf, size_t buflen)
{
	errno = 0; // We must clear the error flag on every iteration
	if (!(handle->entry = readdir(handle->dir))) {
		// Cache error code because it can change.
		int errnoTemp = errno;
		xtFileIteratorEnd(handle);
		return errnoTemp == 0 ? XT_ENOENT : _xtTranslateSysError(errnoTemp);
	}
	xtstrncpy(buf, handle->entry->d_name, buflen);
	++handle->fileCount;
	return 0;
}

int xtFileIteratorEnd(struct xtFileIterator *handle)
{
	return closedir(handle->dir) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileAccess(const char *path, enum xtFileAccessMode mode)
{
	int flags = 0;
	if (mode & XT_FILE_F_OK) flags |= F_OK;
	if (mode & XT_FILE_R_OK) flags |= R_OK;
	if (mode & XT_FILE_W_OK) flags |= W_OK;
	if (mode & XT_FILE_X_OK) flags |= X_OK;
	return access(path, flags) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileCopy(const char *restrict src, const char *restrict dst)
{
	if (!src || !dst)
		return XT_EINVAL;
	FILE *fsrc = fopen(src, "rb");
	if (!fsrc)
		return _xtTranslateSysError(errno);
	FILE *fdst = fopen(dst, "w+");
	if (!fdst) {
		fclose(fsrc);
		return _xtTranslateSysError(errno);
	}
	int ret = xtFileCopyByHandle(fsrc, fdst);
	fclose(fsrc);
	fclose(fdst);
	return ret;
}

int xtFileCopyByHandle(FILE *restrict src, FILE *restrict dst)
{
	if (!src || !dst)
		return XT_EINVAL;
	if (fseek(src, 0, SEEK_SET) != 0 || fseek(dst, 0, SEEK_SET) != 0)
		return _xtTranslateSysError(errno);
	char buf[8192];
	size_t len;
	while (true) {
		len = fread(buf, 1, sizeof buf, src);
		if (len == 0)
			break;
		if (fwrite(buf, len, 1, dst) != 1)
			return _xtTranslateSysError(errno);
	}
	fseek(dst, 0, SEEK_SET);
	fseek(src, 0, SEEK_SET);
	return 0;
}

int xtFileCreateDir(const char *path)
{
	return mkdir(path, S_IRWXU) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileExecute(const char *path)
{
	int ret = xtFileAccess(path, XT_FILE_R_OK);
	if (ret != 0)
		return ret;
	char buf[FILENAME_MAX];
	snprintf(buf, FILENAME_MAX, "/usr/bin/xdg-open \"%s\"", path);
	return system(buf) == 0 ? 0 : XT_EUNKNOWN; // We can't know what the retval means
}

const char *xtFileGetBaseName(const char *path)
{
	if (!path || strlen(path) == 0)
		return NULL;
	const char *lastPos = path;
	for (unsigned i = 0; i < strlen(path); ++i) {
		const char *slash = strrchr(lastPos, '/');
		if (!slash)
			break;
		lastPos = ++slash;
	}
	return lastPos;
}

int xtFileGetCWD(char *buf, size_t buflen)
{
	return getcwd(buf, buflen) ? 0 : _xtTranslateSysError(errno);
}

int xtFileGetExecutablePath(char *buf, size_t buflen)
{
	char cmdbuf[64];
	snprintf(cmdbuf, sizeof cmdbuf, "/bin/readlink /proc/%zu/exe 2> /dev/null", (size_t) getpid());
	FILE *fp = popen(cmdbuf, "r");
	if (!fp)
		return _xtTranslateSysError(errno);
	if (!xtStringReadLine(buf, buflen, NULL, fp)) {
		pclose(fp);
		return XT_EIO;
	}
	pclose(fp);
	return 0;
}

const char *xtFileGetExtension(const char *path)
{
	// First skip to the final /
	const char *fixedPath = xtFileGetBaseName(path);
	if (!fixedPath)
		return NULL;
	const char *dotPlus = strrchr(fixedPath, '.');
	if (!dotPlus)
		return NULL;
	++dotPlus;
	return *dotPlus != '\0' ? dotPlus : NULL;
}

char *xtFileGetHomeDir(char *buf, size_t buflen)
{
	char *dir = getenv("HOME");
	if (!dir)
		return NULL;
	snprintf(buf, buflen, "%s", dir);
	return buf;
}

static enum xtFileType ftype_to_xt(mode_t st_mode)
{
	// It seems that a file on Linux can only be one type at a time
	switch (st_mode & S_IFMT) {
	case S_IFBLK:  return XT_FILE_BLK;
	case S_IFCHR:  return XT_FILE_CHR;
	case S_IFDIR:  return XT_FILE_DIR;
	case S_IFIFO:  return XT_FILE_FIFO;
	case S_IFLNK:  return XT_FILE_LNK;
	case S_IFREG:  return XT_FILE_REG;
	case S_IFSOCK: return XT_FILE_SOCK;
	default:       return XT_FILE_UNKNOWN;
	}
}

int xtFileGetInfo(struct xtFileInfo *restrict fileInfo, const char *restrict path)
{
	struct stat stats;
	if (lstat(path, &stats) == -1)
		return _xtTranslateSysError(errno);
	fileInfo->type = ftype_to_xt(stats.st_mode);
	fileInfo->creationTime = 0; // Unsupported unfortunately
	fileInfo->accessTime = stats.st_atime;
	fileInfo->modificationTime = stats.st_mtime;
	fileInfo->size = stats.st_size;
	return 0;
}

int xtFileGetRealPath(char *restrict buf, size_t buflen, const char *restrict path)
{
	(void)buflen; // Future use hopefully
	return realpath(path, buf) ? 0 : _xtTranslateSysError(errno);
}

int xtFileGetTempDir(char *buf, size_t buflen)
{
#if defined(P_tmpdir)
	snprintf(buf, buflen, "%s", P_tmpdir);
#else
	snprintf(buf, buflen, "%s", "/tmp");
#endif
	return 0;
}

int xtFileMove(const char *restrict src, const char *restrict dst)
{
	return rename(src, dst) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileRemove(const char *path)
{
	return remove(path) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileRemoveDir(const char *path)
{
	return rmdir(path) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileSetCWD(const char *path)
{
	return chdir(path) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileSetSize(const char *path, unsigned long long size)
{
	return truncate(path, size) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileTempFile(char *restrict buf, size_t buflen, FILE **restrict f)
{
	char path[32];
	snprintf(path, sizeof path, "/tmp/tmpfile.XXXXXX");
	int fd = mkstemp(path);
	if (fd == -1)
		return _xtTranslateSysError(errno);
	*f = fdopen(fd, "wb+");
	if (!*f) {
		close(fd);
		unlink(path);
		return _xtTranslateSysError(errno);
	}
	if (buf)
		snprintf(buf, buflen, "%s", path);
	return 0;
}

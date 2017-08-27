// XT headers
#include <xt/file.h>
#include <xt/error.h>
#include <xt/sort.h>
#include <xt/string.h>

// System headers
#include <dirent.h>
#include <errno.h>
#include <fcntl.h> // for open() flags
#include <strings.h>
#include <sys/dir.h> // scandir
#include <sys/stat.h> // for the "stat" call, to obtain a file's size
#include <unistd.h> // necessary for the stat struct

// STD headers
#include <stdlib.h>
#include <string.h>

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
	bool fileExists;
	int ret = xtFileExists(path, &fileExists);
	if (ret != 0)
		return ret;
	if (!fileExists)
		return XT_ENOENT;
	char buf[FILENAME_MAX];
	snprintf(buf, FILENAME_MAX, "/usr/bin/xdg-open \"%s\"", path);
	return system(buf) == 0 ? 0 : XT_EUNKNOWN; // We can't know what the retval means
}

int xtFileExists(const char *restrict path, bool *restrict exists)
{
	if (!path)
		return XT_EINVAL;
	struct stat statbuf;
	if (stat(path, &statbuf) == -1) {
		int err = errno;
		if (err == ENOENT) {
			*exists = false;
			return 0;
		}
		return _xtTranslateSysError(errno);
	}
	*exists = true;
	return 0;
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

int xtFileFindFirstFile(
	struct xtFileFind *restrict handle,
	const char *restrict path,
	char *restrict buf, size_t buflen)
{
	int ret;
	if (!(handle->dir = opendir(path))) {
		ret = _xtTranslateSysError(errno);
		goto cleanup;
	}
	return xtFileFindNextFile(handle, buf, buflen);
cleanup:
	closedir(handle->dir);
	return ret;
}

int xtFileFindNextFile(struct xtFileFind *restrict handle, char *restrict buf, size_t buflen)
{
	int ret;
	if ((ret = readdir_r(handle->dir, &handle->entry, &handle->result)) != 0 || !handle->result)
		return ret == 0 && !handle->result ? XT_ENOENT : _xtTranslateSysError(ret);
	xtstrncpy(buf, handle->entry.d_name, buflen);
	return 0;
}

int xtFileFindClose(struct xtFileFind *handle)
{
	return closedir(handle->dir) == 0 ? 0 : _xtTranslateSysError(errno);
}

char *xtFileGetHomeDir(char *buf, size_t buflen)
{
	char *dir = getenv("HOME");
	if (!dir)
		return NULL;
	snprintf(buf, buflen, "%s", dir);
	return buf;
}

int xtFileGetRealPath(char *restrict buf, size_t buflen, const char *restrict path)
{
	(void) buflen; // Future use hopefully
	return realpath(path, buf) ? 0 : _xtTranslateSysError(errno);
}

int xtFileGetSizeByHandle(FILE *restrict f, unsigned long long *restrict size)
{
	if (!f)
		return XT_EINVAL;
	struct stat statbuf;
	if (fstat(fileno(f), &statbuf) == -1)
		return _xtTranslateSysError(errno);
	*size = statbuf.st_size;
	return 0;
}

int xtFileGetSizeByName(const char *restrict path, unsigned long long *restrict size)
{
	if (!path)
		return XT_EINVAL;
	struct stat statbuf;
	if (stat(path, &statbuf) == -1)
		return _xtTranslateSysError(errno);
	*size = statbuf.st_size;
	return 0;
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

int xtFileIsDir(const char *restrict path, bool *restrict isDirectory)
{
	struct stat buf;
	if (stat(path, &buf) == -1)
		return _xtTranslateSysError(errno);
	*isDirectory = S_ISDIR(buf.st_mode);
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

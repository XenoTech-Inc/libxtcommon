// XT headers
#include <xt/file.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <errno.h>
#include <sys/stat.h> // for the "stat" call, to obtain a file's size
#include <unistd.h> // necessary for the stat struct

// STD headers
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int xtFileCreateDir(const char *path)
{
	return mkdir(path, S_IRWXU) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileExists(const char *path, bool *exists)
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
	snprintf(cmdbuf, sizeof(cmdbuf) / sizeof(cmdbuf[0]), "readlink /proc/%zu/exe 2> /dev/null", (size_t) getpid());
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
	return dotPlus != '\0' ? dotPlus : NULL;
}

char *xtFileGetHomeDir(char *buf, size_t buflen)
{
	char *dir = getenv("HOME");
	if (!dir)
		return NULL;
	snprintf(buf, buflen, "%s", dir);
	return buf;
}

int xtFileGetRealPath(const char *path, char *buf, size_t buflen)
{
	(void) buflen; // Future use hopefully
	return realpath(path, buf) ? 0 : _xtTranslateSysError(errno);
}

int xtFileGetSizeByHandle(FILE *f, unsigned long long *size)
{
	if (!f)
		return XT_EINVAL;
	struct stat statbuf;
	if (fstat(fileno(f), &statbuf) == -1)
		return _xtTranslateSysError(errno);
	*size = statbuf.st_size;
	return 0;
}

int xtFileGetSizeByName(const char *path, unsigned long long *size)
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

int xtFileIsDir(const char *path, bool *isDirectory)
{
	struct stat buf;
	if (stat(path, &buf) == -1)
		return _xtTranslateSysError(errno);
	*isDirectory = S_ISDIR(buf.st_mode);
	return 0;
}

int xtFileRemoveDir(const char *path)
{
	return rmdir(path) == 0 ? 0 : _xtTranslateSysError(errno);
}

int xtFileSetCWD(const char *path)
{
	return chdir(path) == 0 ? 0 : _xtTranslateSysError(errno);
}

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
		len = fread(buf, 1, sizeof(buf) / sizeof(buf[0]), src);
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

void xtFileExecute(const char *path)
{
	char buf[FILENAME_MAX];
	snprintf(buf, FILENAME_MAX, "xdg-open \"%s\"", path);
	system(buf);
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
/**
 * This function belongs to xtFileGetFiles().
 */
static int caseCompare(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

int xtFileGetFiles(const char *restrict path, struct xtListP *restrict files)
{
	int ret;
	struct dirent **namelist;
	size_t fileNameLen;
	int cnt = scandir(path, &namelist, NULL, caseCompare);
	if (cnt < 0)
		return _xtTranslateSysError(errno);
	for (int i = 0; i < cnt; ++i) {
		// Length of the file name including the null terminator
		fileNameLen = strlen(namelist[i]->d_name) + 1;
		struct xtFile *file = malloc(sizeof(struct xtFile));
		if (!file) {
			ret = XT_ENOMEM;
			goto error;
		}
		file->path = malloc(fileNameLen);
		if (!file->path) {
			free(file);
			ret = XT_ENOMEM;
			goto error;
		}
		memcpy(file->path, namelist[i]->d_name, fileNameLen);
		switch (namelist[i]->d_type) {
		case DT_REG: file->type = XT_FILE_REG; break;
		case DT_DIR: file->type = XT_FILE_DIR; break;
		case DT_LNK: file->type = XT_FILE_LNK; break;
		default: file->type = XT_FILE_UNKNOWN; break;
		}
		if ((ret = xtListPAdd(files, file)) != 0)
			goto error;
	}
	ret = 0;
error:
	for (int i = 0; i < cnt; ++i)
		free(namelist[i]);
	free(namelist);
	if (ret != 0) {
		for (int i = cnt - 1; i >= 0; --i) {
			struct xtFile *file;
			xtListPGet(files, i, (void**) &file);
			free(file->path);
			free(file);
		}
		// Just empty the whole list
		xtListPClear(files);
	}
	return ret;
}

char *xtFileGetHomeDir(char *buf, size_t buflen)
{
	char *dir = getenv("HOME");
	if (!dir)
		return NULL;
	snprintf(buf, buflen, "%s", dir);
	return buf;
}

#if 0
int xtFileGetPathFromFilePointer(char *restrict buf, size_t buflen, FILE *restrict f)
{
	if (!f)
		return XT_EINVAL;
	char cmd[256];
	int fd = fileno(f);
	sprintf(cmd, "/proc/self/fd/%d", fd);
	ssize_t len = readlink(cmd, buf, buflen);
	if (len == -1)
		return _xtTranslateSysError(errno);
	buf[len] = '\0';
	return 0;
}
#endif

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
	snprintf(path, sizeof(path) / sizeof(path[0]), "/tmp/tmpfile.XXXXXX");
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

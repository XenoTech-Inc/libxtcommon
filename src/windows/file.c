// XT headers
#include <xt/file.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <windows.h>

// STD headers
#include <dirent.h>
#include <string.h>

int xtFileCopy(const char *src, const char *dst)
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

int xtFileCopyByHandle(FILE *src, FILE *dst)
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
	return CreateDirectory((LPCTSTR) path, NULL) != 0 ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileExists(const char *path, bool *exists)
{
	if (!path)
		return XT_EINVAL;
	DWORD ret = GetFileAttributes(path);
	*exists = ret != INVALID_FILE_ATTRIBUTES;
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
	if (GetCurrentDirectoryA(buflen, buf) == 0)
		return _xtTranslateSysError(GetLastError());
	xtStringReplaceAll(buf, '\\', '/');
	return 0;
}

int xtFileGetExecutablePath(char *buf, size_t buflen)
{
	if (GetModuleFileNameA(NULL, buf, buflen) == 0)
		return _xtTranslateSysError(GetLastError());
	xtStringReplaceAll(buf, '\\', '/');
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
	char *homeDrive = getenv("HOMEDRIVE"), *homePath = getenv("HOMEPATH");
	if (!homePath || !homeDrive)
		return NULL;
	snprintf(buf, buflen, "%s%s", homeDrive, homePath);
	xtStringReplaceAll(buf, '\\', '/');
	return buf;
}

int xtFileGetRealPath(const char *path, char *buf, size_t buflen)
{
	if (!_fullpath(buf, path, buflen))
		return XT_EINVAL;
	xtStringReplaceAll(buf, '\\', '/');
	return 0;
}

int xtFileGetSizeByHandle(FILE *f, unsigned long long *size)
{
	if (!f)
		return XT_EINVAL;
	unsigned long long endPos, currPos = _ftelli64(f);
	_fseeki64(f, 0, SEEK_END);
	endPos = _ftelli64(f);
	_fseeki64(f, currPos, SEEK_SET);
	*size = endPos;
	return 0;
}

int xtFileGetSizeByName(const char *path, unsigned long long *size)
{
	if (!path)
		return XT_EINVAL;
	FILE *f = fopen(path, "r");
	if (!f)
		return XT_ENOENT;
	int ret = xtFileGetSizeByHandle(f, size);
	fclose(f);
	return ret;
}

int xtFileGetTempDir(char *buf, size_t buflen)
{
	if (buflen < 2)
		return XT_EINVAL;
	DWORD ret = GetTempPath(buflen, buf);
	if (ret == 0)
		return _xtTranslateSysError(GetLastError());
	buf[strlen(buf) - 1] = '\0'; // Replace trailing backslash with null-terminator
	xtStringReplaceAll(buf, '\\', '/');
	return 0;
}

int xtFileIsDir(const char *path, bool *isDirectory)
{
	WIN32_FILE_ATTRIBUTE_DATA f;
	f.dwFileAttributes = 0;
	// Cannot request the max info standard, then this won't work
	if (GetFileAttributesEx(path, GetFileExInfoStandard, &f) == 0)
		return _xtTranslateSysError(GetLastError());
	*isDirectory = f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	return 0;
}

int xtFileMove(const char *src, const char *dst)
{
	return MoveFileEx(src, dst, MOVEFILE_COPY_ALLOWED) == TRUE ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileRemove(const char *path)
{
	return DeleteFile(path) == TRUE ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileRemoveDir(const char *path)
{
	return RemoveDirectory(path) == TRUE ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileSetCWD(const char *path)
{
	return SetCurrentDirectory((LPCSTR) path) != 0 ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileTempFile(FILE **f, char *buf, size_t buflen)
{
	int ret;
	char tmpdir[MAX_PATH];
	char path[MAX_PATH];
	ret = xtFileGetTempDir(tmpdir, sizeof(tmpdir) / sizeof(tmpdir[0]));
	if (ret != 0)
		return ret;
	unsigned numb = GetTempFileName(tmpdir, "tmp", 0, path);
	if (numb == 0)
		return _xtTranslateSysError(errno);
	xtStringReplaceAll(path, '\\', '/');
	*f = fopen(path, "wb+");
	if (!*f)
		return _xtTranslateSysError(errno);
	if (buf)
		snprintf(buf, buflen, "%s", path);
	return 0;
}
// XT headers
#include <xt/file.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <windows.h>

// STD headers
#include <dirent.h>
#include <string.h>

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
	char sbuf[FILENAME_MAX];
	if (GetCurrentDirectoryA(FILENAME_MAX, sbuf) == 0)
		return _xtTranslateSysError(GetLastError());
	snprintf(buf, buflen, "%s", sbuf);
	xtStringReplaceAll(buf, '\\', '/');
	return 0;
}

int xtFileGetExecutablePath(char *buf, size_t buflen)
{
	TCHAR sbuf[FILENAME_MAX];
	if (GetModuleFileName(NULL, buf, FILENAME_MAX) == 0)
		return _xtTranslateSysError(GetLastError());
	snprintf(buf, buflen, "%s", sbuf);
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
	char *dir = getenv("TEMP");
	if (!dir)
		return XT_ENOENT;
	snprintf(buf, buflen, "%s", dir);
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

int xtFileRemoveDir(const char *path)
{
	return RemoveDirectory(path) != 0 ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileSetCWD(const char *path)
{
	return SetCurrentDirectory((LPCSTR) path) != 0 ? 0 : _xtTranslateSysError(GetLastError());
}

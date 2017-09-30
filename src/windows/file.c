// XT headers
#include <xt/file.h>
#include <_xt/error.h>
#include <xt/error.h>
#include <xt/string.h>
#include <_xt/time.h>

// System headers
#include <dirent.h>
#include <errno.h>
#include <io.h>
#include <windows.h>

// STD headers
#include <stdint.h>
#include <string.h>

int xtFileFindFirstFile(
	struct xtFileFind *restrict handle,
	const char *restrict path,
	char *restrict buf, size_t buflen)
{
	char sbuf[PATH_MAX];
	snprintf(sbuf, sizeof sbuf, "%s\\*.*", path);
	WIN32_FIND_DATA wfd;
	if ((handle->handle = FindFirstFile(sbuf, &wfd)) == INVALID_HANDLE_VALUE)
		return _xtTranslateSysError(GetLastError());
	xtstrncpy(buf, wfd.cFileName, buflen);
	return 0;
}

int xtFileFindNextFile(struct xtFileFind *restrict handle, char *restrict buf, size_t buflen)
{
	WIN32_FIND_DATA wfd;
	if (!FindNextFile(handle->handle, &wfd))
		return _xtTranslateSysError(GetLastError());
	xtstrncpy(buf, wfd.cFileName, buflen);
	return 0;
}

int xtFileFindClose(struct xtFileFind *handle)
{
	return FindClose(handle->handle) ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileAccess(const char *path, enum xtFileAccessMode mode)
{
	int flags = 0;
	if (mode & XT_FILE_F_OK) flags |= 0x00;
	if (mode & XT_FILE_R_OK) flags |= 0x04;
	if (mode & XT_FILE_W_OK) flags |= 0x02;
	// Executable mode is not supported on Windows so ignore it...
	errno_t ret = _access_s(path, flags);
	switch (ret) { // We must translate these right here
	case 0:      return 0;
	case EACCES: return XT_EACCES;
	case EINVAL: return XT_EINVAL;
	case ENOENT: return XT_ENOENT;
	default:     return XT_EUNKNOWN;
	}
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
	return CreateDirectory((LPCTSTR) path, NULL) != 0 ? 0 : _xtTranslateSysError(GetLastError());
}

int xtFileExecute(const char *path)
{
	intptr_t ret = (intptr_t)ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOW);
	return ret > 32 ? 0 : _xtTranslateSysError(ret);
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
	return *dotPlus != '\0' ? dotPlus : NULL;
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

static enum xtFileType ftype_to_xt(DWORD attributes)
{
	enum xtFileType type = XT_FILE_UNKNOWN;

	if (attributes & FILE_ATTRIBUTE_ARCHIVE)       type |= XT_FILE_REG;
	if (attributes & FILE_ATTRIBUTE_DIRECTORY)     type |= XT_FILE_DIR;
	if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) type |= XT_FILE_LNK;

	if (type != XT_FILE_UNKNOWN)                   type &= ~XT_FILE_UNKNOWN;
	return type;
}

int xtFileGetInfo(struct xtFileInfo *restrict fileInfo, const char *restrict path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	// Cannot request the max info standard, then this won't work
	if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
		return _xtTranslateSysError(GetLastError());
	fileInfo->type = ftype_to_xt(fad.dwFileAttributes);
	fileInfo->creationTime = _xtFileTimeToLLU(&fad.ftCreationTime) / 10000000 - 11644473600LLU;
	fileInfo->accessTime = _xtFileTimeToLLU(&fad.ftLastAccessTime) / 10000000 - 11644473600LLU;
	fileInfo->modificationTime = _xtFileTimeToLLU(&fad.ftLastWriteTime) / 10000000 - 11644473600LLU;
	fileInfo->size = ((ULONGLONG)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
	return 0;
}

int xtFileGetRealPath(char *restrict buf, size_t buflen, const char *restrict path)
{
	if (!_fullpath(buf, path, buflen))
		return XT_EINVAL;
	xtStringReplaceAll(buf, '\\', '/');
	return 0;
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

int xtFileMove(const char *restrict src, const char *restrict dst)
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

int xtFileTempFile(char *restrict buf, size_t buflen, FILE **restrict f)
{
	int ret;
	char tmpdir[MAX_PATH];
	char path[MAX_PATH];
	ret = xtFileGetTempDir(tmpdir, sizeof tmpdir);
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

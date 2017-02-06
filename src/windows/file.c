// XT headers
#include <xt/file.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <dirent.h>
#include <windows.h>

// STD headers
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
	return CreateDirectory((LPCTSTR) path, NULL) != 0 ? 0 : _xtTranslateSysError(GetLastError());
}

void xtFileExecute(const char *path)
{
	ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOW);
}

int xtFileExists(const char *restrict path, bool *restrict exists)
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

int xtFileGetFiles(const char *restrict path, struct xtListP *restrict files)
{
	int ret;
	WIN32_FIND_DATA fdFile;
	HANDLE handle;
	char sbuf[4096];
	snprintf(sbuf, sizeof(sbuf) / sizeof(sbuf[0]), "%s\\*.*", path);
	if ((handle = FindFirstFile(sbuf, &fdFile)) == INVALID_HANDLE_VALUE)
		return _xtTranslateSysError(GetLastError());
	size_t fileNameLen;
	unsigned cnt = 0;
	for (; FindNextFile(handle, &fdFile); ++cnt) {
		// Length of the file name including the null terminator
		fileNameLen = strlen(fdFile.cFileName) + 1;
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
		memcpy(file->path, fdFile.cFileName, fileNameLen);
		// DO NOT change the order of these statements! Files can have multiple types.
		// A link can have link as attribute and directory for example
		if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			file->type = XT_FILE_LNK;
		else if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			file->type = XT_FILE_DIR;
		else if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
			file->type = XT_FILE_REG;
		else
			file->type = XT_FILE_UNKNOWN;
		if ((ret = xtListPAdd(files, file)) != 0)
			goto error;
	}
	ret = 0;
error:
	FindClose(handle);
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
	char *homeDrive = getenv("HOMEDRIVE"), *homePath = getenv("HOMEPATH");
	if (!homePath || !homeDrive)
		return NULL;
	snprintf(buf, buflen, "%s%s", homeDrive, homePath);
	xtStringReplaceAll(buf, '\\', '/');
	return buf;
}

int xtFileGetRealPath(char *restrict buf, size_t buflen, const char *restrict path)
{
	if (!_fullpath(buf, path, buflen))
		return XT_EINVAL;
	xtStringReplaceAll(buf, '\\', '/');
	return 0;
}

int xtFileGetSizeByHandle(FILE *restrict f, unsigned long long *restrict size)
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

int xtFileGetSizeByName(const char *restrict path, unsigned long long *restrict size)
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

int xtFileIsDir(const char *restrict path, bool *restrict isDirectory)
{
	WIN32_FILE_ATTRIBUTE_DATA f;
	f.dwFileAttributes = 0;
	// Cannot request the max info standard, then this won't work
	if (GetFileAttributesEx(path, GetFileExInfoStandard, &f) == 0)
		return _xtTranslateSysError(GetLastError());
	*isDirectory = f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
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

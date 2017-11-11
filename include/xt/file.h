/**
 * @brief Various functions to handle files with ease.
 *
 * On error, the specified buffer's content for all functions is undefined, unless otherwise noted.
 * @file file.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_FILE_H
#define _XT_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/list.h>
#include <xt/os_macros.h>

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if XT_IS_LINUX
	#include <dirent.h>
#endif

/**
 * @brief Used to scan files in a directory.
 */
struct xtFileFind {
#if XT_IS_LINUX
	DIR *dir;
	struct dirent *entry;
#else
	void *handle;
#endif
};
/**
 * Opens the directory and fetches the first file name.
 * The order in which the files are scanned is undefined.
 * @param buf - Receives the name of the file.
 * @return Zero if the directory was opened and the first file name was fetched,
 * otherwise an error code.
 * @remarks All resources are cleaned up automatically if the function call may
 * fail.
 */
int xtFileFindFirstFile(
	struct xtFileFind *restrict handle,
	const char *restrict path,
	char *restrict buf, size_t buflen
);
/**
 * Advances to the next file in the list.
 * @param buf - Receives the name of the file.
 * @return Zero if the file name was fetched, otherwise an error code.
 */
int xtFileFindNextFile(
	struct xtFileFind *restrict handle,
	char *restrict buf, size_t buflen);
/**
 * Cleans up all resources belonging to \a handle.
 */
int xtFileFindClose(struct xtFileFind *handle);
/**
 * All supported access modes for xtFileAccess().
 */
enum xtFileAccessMode {
	/** File exists. */
	XT_FILE_F_OK = 0x01,
	/** File is readable. */
	XT_FILE_R_OK = 0x02,
	/** File is writeable. */
	XT_FILE_W_OK = 0x04,
	/** File is executable. */
	XT_FILE_X_OK = 0x08
};
/**
 * Checks if all accessibility checks in \a mode are granted by the file.
 * @return Zero if all rights in \a mode are granted, otherwise an error code.
 * @remarks If a platform does not support one of the modes, they are ignored.
 */
int xtFileAccess(const char *path, enum xtFileAccessMode mode);
/**
 * Copies the file from \a src to \a dst.
 * @return Zero if the file has been copied, otherwise an error code.
 */
int xtFileCopy(const char *restrict src, const char *restrict dst);
/**
 * Copies the file from \a src to \dst by their handle.
 * @return Zero if the file has been copied, otherwise an error code.
 * @remarks On success the stream position indicator is set to the beginning of
 * both files. On error, the position of the indicator is undefined. The file
 * handles are NEVER closed by this function!
 */
int xtFileCopyByHandle(FILE *restrict src, FILE *restrict dst);
/**
 * Creates a new directory.
 * @return Zero if the directory has been created, otherwise an error code.
 */
int xtFileCreateDir(const char *path);
/**
 * Opens the file with the default associated program.
 * This program is executed as new process, so keep in mind that the
 * caller thread should return immidiately. The file has to be readable for
 * this function to work.
 * @return Zero if the file has been executed, otherwise an error code.
 * XT_EUNKNOWN is returned if the file was executed but returned an error
 * code afterwards.
 */
int xtFileExecute(const char *path);
/**
 * Returns the filename from the complete path, including the extension.
 * @param path - A buffer which contains the complete path
 * @return NULL if \a path is an invalid path, otherwise a pointer to a
 * substring of \a path.
 * @remark The file does not have to exist in order for this to work. This
 * function basically just strips the preceding files / folders in the path.
 */
const char *xtFileGetBaseName(const char *path);
/**
 * Retrieves the current working directory, encoded as ANSI.
 */
int xtFileGetCWD(char *buf, size_t buflen);
/**
 * Tells you the path of where the executable resides that is running this code.
 * @return Zero if the path has been retrieved successfully, otherwise an error code.
 */
int xtFileGetExecutablePath(char *buf, size_t buflen);
/**
 * Returns the extension of the specified file.
 * @param path - Pointer to a string which contains the file path.
 * @return A pointer containing only the extension of the file, excluding the dot. NULL is returned on error.
 * @remark This pointer will point to the extension in the \a path string, meaning that NO bytes are copied over.
 */
const char *xtFileGetExtension(const char *path);
/**
 * Tells you the home directory for the user that is currently running the
 * program.
 * @return A pointer to \a buf on success, otherwise NULL.
 */
char *xtFileGetHomeDir(char *buf, size_t buflen);
/**
 * @brief All supported file types for this API.
 */
enum xtFileType {
	/** Unknown or unsupported file type. */
	XT_FILE_UNKNOWN = 0x01,
	/* Block device. */
	XT_FILE_BLK =     0x02,
	/* Character device. */
	XT_FILE_CHR =     0x04,
	/** Directory. */
	XT_FILE_DIR =     0x08,
	/* Named pipe. (FIFO) */
	XT_FILE_FIFO =    0x10,
	/** Symbolic link or Hard link. */
	XT_FILE_LNK =     0x20,
	/** Regular file. */
	XT_FILE_REG =     0x40,
	/* Unix domain socket. */
	XT_FILE_SOCK =    0x80
};
/**
 * @brief Holds some information about a certain file.
 *
 * All timestamps are in milliseconds since the UNIX epoch time in UTC.
 */
struct xtFileInfo {
	enum xtFileType type;
	unsigned long long creationTime;
	unsigned long long accessTime;
	unsigned long long modificationTime;
	unsigned long long size;
};
/**
 * Retrieves information for \a path.
 * @return Zero if the information has been fetched, otherwise an error code.
 */
int xtFileGetInfo(struct xtFileInfo *fileInfo, const char *path);
/*
 * Tells you the absolute path of \a path. The absolute path shall be stored
 * in \a buf.
 * @return Zero if the path has been fetched, otherwise an error code.
 */
int xtFileGetRealPath(char *restrict buf, size_t buflen, const char *restrict path);
/**
 * Tells you the path to the system's temporary directory.
 * @return Zero if the path has been fetched, otherwise an error code.
 */
int xtFileGetTempDir(char *buf, size_t buflen);
/**
 * Moves \a src to \a dst.
 * @return Zero if the file has been moved, otherwise an error code.
 */
int xtFileMove(const char *restrict src, const char *restrict dst);
/**
 * Removes a file.
 * @return Zero if the file has been deleted, otherwise an error code.
 */
int xtFileRemove(const char *path);
/**
 * Removes a directory. It must be empty in order for this to work!
 * @return Zero if the folder has been deleted, otherwise an error code.
 */
int xtFileRemoveDir(const char *path);
/**
 * Changes the working directory to \a path.
 * @param path - The absolute path to the new working directory.
 * @return Zero if the working directory has changed successfully, otherwise an
 * error code.
 */
int xtFileSetCWD(const char *path);
/**
 * Truncates the size of file that resides at \a path to \a size.
 * Depending on the filesystem in use, this operation may complete
 * instantaneously. If you're unlucky, the file has to be rewritten physically.
 * @return Zero if the file has been truncated to the new size, otherwise an
 * error code.
 */
int xtFileSetSize(const char *path, unsigned long long size);
/**
 * Creates a temporary binary file that is automatically removed when it is
 * closed or at program termination. The file is opened as in fopen() for update
 * in binary mode (that is, "wb+").
 * @param buf - Receives the path of the new temp file on success. Set this to a
 * NULL pointer and it shall not be filled.
 * @param f - A pointer to a file pointer. It shall receive a valid file pointer
 * on success. On failure it shall be set to NULL.
 * @return Zero if the temporary file has been created, otherwise an error code.
 * @remarks Depending on the platform, you might only be able to copy this file,
 * not moving it.
 */
int xtFileTempFile(char *restrict buf, size_t buflen, FILE **restrict f);

#ifdef __cplusplus
}
#endif

#endif

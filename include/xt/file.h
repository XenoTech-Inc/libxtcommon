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

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief All supported file types for this API.
 */
enum xtFileType {
	/** Unknown or unsupported file type. */
	XT_FILE_UNKNOWN,
	/* Block device. */
	XT_FILE_BLK,
	/* Character device. */
	XT_FILE_CHR,
	/** Directory. */
	XT_FILE_DIR,
	/* Named pipe. (FIFO) */
	XT_FILE_FIFO,
	/** Symbolic link or Hard link. */
	XT_FILE_LNK,
	/** Regular file. */
	XT_FILE_REG,
	/* Unix domain socket. */
	XT_FILE_SOCK
};
/**
 * @brief Holds some information about a certain file.
 */
struct xtFile {
	char *path;
	enum xtFileType type;
};
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
 * caller thread should return immidiately.
 * @return Zero if the file has been executed, otherwise an error code.
 * XT_EUNKNOWN is returned if the file was executed but returned an error
 * code afterwards.
 */
int xtFileExecute(const char *path);
/**
 * Tells you whether the specified file or directory exists.
 * It should be noted that it is possible that because of permissions
 * the function cannot check the path. This means that you should check the
 * return value of this function to be safe.
 * @return Zero if there was a successful check for the file, otherwise an error code.
 */
int xtFileExists(const char *restrict path, bool *restrict exists);
/**
 * Returns the filename from the complete path, including the extension.
 * @param path - A buffer which contains the complete path
 * @return NULL if \a path is an invalid path, otherwise a pointer to a substring of \a path.
 * @remark The file does not have to exist in order for this to work. This function
 * basically just strips the preceding files / folders in the path.
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
 * Tells you what files reside under \a path.
 * @param files - An initialized xtList, it should be empty. The list will be filled with xtFile's. They are always sorted alphabetically.
 * @return Zero if the file list has been retrieved, otherwise an error code.
 * @remarks You must free each element and clear the list. E.g.:
 * <pre>
 * struct xtListP list;
 * // Initialize list and get files
 * size_t n = xtListPGetCount(&list);
 * for (size_t i = 0; i < n; ++i) {
 * 	struct xtFile *file;
 * 	xtListPGet(&list, i, (void**) &file);
 * 	free(file->path);
 * 	free(file);
 * }
 * xtListPClear(&list);
 * </pre>
 */
int xtFileGetFiles(const char *restrict path, struct xtListP *restrict files);
/**
 * Tells you the home directory for the user that is currently running the program.
 * @return A pointer to \a buf on success, otherwise NULL.
 */
char *xtFileGetHomeDir(char *buf, size_t buflen);
/*
 * Tells you the absolute path of \a path. The absolute path shall be stored in \a buf.
 * @return Zero if the path has been fetched, otherwise an error code.
 */
int xtFileGetRealPath(char *restrict buf, size_t buflen, const char *restrict path);
/**
 * Tells you the size of the specified file. Zero is returned on error.
 * @remark Suited for large files (2GB+)
 */
int xtFileGetSizeByHandle(FILE *restrict f, unsigned long long *restrict size);
/**
 * Tells you the size of the specified file. Zero is returned on error.
 * @remark Suited for large files (2GB+)
 */
int xtFileGetSizeByName(const char *restrict path, unsigned long long *restrict size);
/**
 * Tells you the path to the system's temporary directory.
 * @return Zero if the path has been fetched, otherwise an error code.
 */
int xtFileGetTempDir(char *buf, size_t buflen);
/**
 * Returns whether the specified path is a directory or not.
 */
int xtFileIsDir(const char *restrict path, bool *restrict isDirectory);
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
 * @return Zero if the working directory has changed successfully, otherwise an error code.
 */
int xtFileSetCWD(const char *path);
/**
 * Creates a temporary binary file that is automatically removed when it is
 * closed or at program termination. The file is opened as in fopen() for update, in
 * binary mode (that is, "wb+").
 * @param buf - Receives the path of the new temp file on success. Set this to a null pointer
 * and it shall not be filled.
 * @param f - A pointer to a file pointer. It shall receive a valid file pointer on success.
 * On failure it shall be set to NULL.
 * @return Zero if the temporary file has been created, otherwise an error code.
 * @remarks Depending on the platform, you might only be able to copy this file, not moving it.
 * This file is NOT deleted automatically! You must do this manually.
 */
int xtFileTempFile(char *restrict buf, size_t buflen, FILE **restrict f);

#ifdef __cplusplus
}
#endif

#endif

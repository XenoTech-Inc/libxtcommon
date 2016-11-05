/**
 * @brief Various functions to handle files with ease.
 * 
 * @file file.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_FILE_H
#define _XT_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Creates a new directory.
 * @return Zero if the directory has been created, otherwise an error code.
 */
int xtFileCreateDir(const char *path);
/**
 * Tells you whether the specified file or directory exists. 
 * It should be noted that it is possible that because of permissions 
 * the function cannot check the path. This means that you should check the 
 * return value of this function to be safe.
 * @return Zero if there was a successful check for the file, otherwise an error code.
 */
int xtFileExists(const char *path, bool *exists);
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
 * Tells you the home directory for the user that is currently running the program. 
 * @return A pointer to \a buf on success, otherwise NULL.
 */
char *xtFileGetHomeDir(char *buf, size_t buflen);
/*
 * Tells you the absolute path of \a path. The absolute path shall be stored in \a buf.
 * @return Zero if the path has been fetched, otherwise an error code.
 */
int xtFileGetRealPath(const char *path, char *buf, size_t buflen);
/**
 * Tells you the size of the specified file. Zero is returned on error.
 * @remark Suited for large files (2GB+)
 */
int xtFileGetSizeByHandle(FILE *f, unsigned long long *size);
/**
 * Tells you the size of the specified file. Zero is returned on error.
 * @remark Suited for large files (2GB+)
 */
int xtFileGetSizeByName(const char *path, unsigned long long *size);
/**
 * Tells you the path to the system's temporary directory.
 * @return Zero if the path has been fetched, otherwise an error code.
 */
int xtFileGetTempDir(char *buf, size_t buflen);
/**
 * Returns whether the specified path is a directory or not. False is always returned on error.
 */
int xtFileIsDir(const char *path, bool *isDirectory);
/**
 * Removes directory. Do note that the directory should be empty.
 * @return Zero if the folder has been deleted, otherwise an error code.
 */
int xtFileRemoveDir(const char *path);
/**
 * Changes the working directory to \a path.
 * @param path - The absolute path to the new working directory.
 * @return Zero if the working directory has changed successfully, otherwise an error code.
 */
int xtFileSetCWD(const char *path);

#ifdef __cplusplus
}
#endif

#endif

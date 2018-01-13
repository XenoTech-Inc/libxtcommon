#include <xt/file.h>
#include <xt/error.h>
#include <xt/os.h>
#include <xt/string.h>

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static const char *get_file_type(enum xtFileType type, char *buf, size_t buflen)
{
	char *currBuf = buf;
	*currBuf = '\0';
	#define APPEND(str) currBuf += snprintf(currBuf, buflen - (currBuf - buf), str)
	#define STR(x) #x

	if (type & XT_FILE_BLK)  APPEND(" "STR(XT_FILE_BLK));
	if (type & XT_FILE_CHR)  APPEND(" "STR(XT_FILE_CHR));
	if (type & XT_FILE_DIR)  APPEND(" "STR(XT_FILE_DIR));
	if (type & XT_FILE_FIFO) APPEND(" "STR(XT_FILE_FIFO));
	if (type & XT_FILE_LNK)  APPEND(" "STR(XT_FILE_LNK));
	if (type & XT_FILE_REG)  APPEND(" "STR(XT_FILE_REG));
	if (type & XT_FILE_SOCK) APPEND(" "STR(XT_FILE_SOCK));

	return buf;
}

static void dump_file(const char *path)
{
	int ret;
	char sbuf[256];
	struct xtFileInfo fileInfo;
	if ((ret = xtFileGetInfo(&fileInfo, path) != 0)) {
		xtprintf("Failure to retrieve info for file %s, error: %s\n", path, xtGetErrorStr(ret));
		return;
	}
	puts("------------------------------");
	xtprintf("Name: %s\n", path);
	xtprintf("Type:%s\n", get_file_type(fileInfo.type, sbuf, sizeof sbuf));
	xtprintf("Creation time: %s\n", xtFormatTime(sbuf, sizeof sbuf, fileInfo.creationTime));
	xtprintf("Access time: %s\n", xtFormatTime(sbuf, sizeof sbuf, fileInfo.accessTime));
	xtprintf("Modification time: %s\n", xtFormatTime(sbuf, sizeof sbuf, fileInfo.modificationTime));
	xtprintf("Size: %llu\n", fileInfo.size);
}

int main(void)
{
	puts("-- DIRECTORY TEST");
	char path[256];
	struct xtFileIterator find;
	unsigned i = 0;

	if (xtFileIteratorStart(&find, "."))
		return 1;
	for (; !xtFileIteratorNext(&find, path, sizeof path); ++i)
		dump_file(path);

	printf("%d files found\n", i);
	return 0;
}

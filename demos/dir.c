#include <xt/file.h>
#include <xt/error.h>
#include <xt/os.h>
#include <xt/string.h>

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static struct stats stats;

const char *getFileType(unsigned type)
{
	switch (type) {
	case XT_FILE_DIR: return "DIR";
	case XT_FILE_REG: return "REG";
	case XT_FILE_LNK: return "LINK";
	default: return "Unknown";
	}
}

int main(void)
{
	int ret;
	struct xtListP list;
	stats_init(&stats, "directory");
	puts("-- DIRECTORY TEST");
	if ((ret = xtListPCreate(&list, 256))) {
		FAIL("xtListPCreate()");
		xtfprintf(stderr, "Unable to create the list: %s\n", xtGetErrorStr(ret));
		return EXIT_FAILURE;
	} else
		PASS("xtListPCreate()");
	ret = xtFileGetFiles(".", &list);
	if (ret)
		FAIL("xtFileGetFiles()");
	else
		PASS("xtFileGetFiles()");
	xtprintf("File list retrieval: %s\n", xtGetErrorStr(ret));
	xtprintf("Amount of files: %zu\n", xtListPGetCount(&list));
	struct xtFile *file;
	size_t n = xtListPGetCount(&list);
	for (size_t i = 0; i < n; ++i) {
		xtListPGet(&list, i, (void**) &file);
		xtprintf("%s - %s\n", getFileType(file->type), file->path);
		free(file->path);
		free(file);
	}
	xtListPDestroy(&list);
	stats_info(&stats);
	return stats_status(&stats);
}

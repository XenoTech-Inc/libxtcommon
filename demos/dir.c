#include <xt/file.h>
#include <xt/error.h>

#include <stdio.h>
#include <stdlib.h>

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
	xtList list;
	if ((ret = xtListCreate(&list, 256))) {
		printf("Unable to create the list: %s\n", xtGetErrorStr(ret));
		return EXIT_FAILURE;
	}
	ret = xtFileGetFiles(".", &list);
	printf("File list retrieval: %s\n", xtGetErrorStr(ret));
	printf("Amount of files: %zu\n", xtListGetCount(&list));
	struct xtFile *file;
	for (unsigned i = 0; i < xtListGetCount(&list); ++i) {
		xtListGet(&list, i, (void**) &file);
		printf("%s - %s\n", getFileType(file->type), file->path);
	}
	xtListDestroy(&list);
	return EXIT_SUCCESS;
}

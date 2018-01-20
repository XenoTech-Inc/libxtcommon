/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

#include <xt/file.h>
#include <xt/mman.h>
#include <xt/string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "utils.h"

static struct stats stats;

static void map_self(const char *prog)
{
	void *map = XT_MMAN_MAP_FAILED;
	int fd = -1;
	struct xtFileInfo info;

	fd = open(prog, O_RDONLY);
	if (fd == -1) {
		FAIL("map_self() - Can't read myself?");
		goto fail;
	}
	if (xtFileGetInfo(&info, prog)) {
		FAIL("map_self() - Can't get file info");
		goto fail;
	}
	map = xtmmap(NULL, info.size, XT_MMAN_PROT_READ, XT_MMAN_MAP_FILE | XT_MMAN_MAP_PRIVATE, fd, 0);
	if (map == XT_MMAN_MAP_FAILED) {
		FAIL("map_self() - xtmmap()");
		goto fail;
	}

	PASS("map_self");
fail:
	if (map != XT_MMAN_MAP_FAILED)
		xtmunmap(map, info.size);
	if (fd != -1)
		close(fd);
}

#define PAGE_SIZE 4096
#define PAGE_ALLOC_SIZE (256 * PAGE_SIZE)

static void simulate_page_alloc(void)
{
	void *map = XT_MMAN_MAP_FAILED;
	size_t size = PAGE_ALLOC_SIZE;
	int ret = 0;

	map = xtmmap(NULL, size, XT_MMAN_PROT_READ | XT_MMAN_PROT_WRITE, XT_MMAN_MAP_ANONYMOUS | XT_MMAN_MAP_PRIVATE, -1, 0);
	if (map == XT_MMAN_MAP_FAILED) {
		FAIL("simulate_page_alloc() - xtmmap()");
		ret = 1;
		goto fail;
	}
	/* Fill page with some data */
	memset(map, 0xff, size);
	/* Read back data */
	for (unsigned char *data = map, *end = data + size; data < end; ++data)
		if (*data != 0xff) {
			FAIL("simulate_page_alloc() - fill data");
			ret = 1;
			goto fail;
		}
fail:
	if (map != XT_MMAN_MAP_FAILED)
		if (xtmunmap(map, size) && ret)
			FAIL("simulate_page_alloc() - xtmunmap()");
	if (!ret)
		PASS("simulate_page_alloc()");
}

static void simulate_page_resize(void)
{
	void *map = XT_MMAN_MAP_FAILED, *map2;
	size_t size = PAGE_ALLOC_SIZE;

	map = xtmmap(NULL, size, XT_MMAN_PROT_READ | XT_MMAN_PROT_WRITE, XT_MMAN_MAP_ANONYMOUS | XT_MMAN_MAP_PRIVATE, -1, 0);
	if (map == XT_MMAN_MAP_FAILED) {
		FAIL("simulate_page_resize() - xtmmap()");
		goto fail;
	}
	/* Fill page with some data */
	memset(map, 0x55, size);
	/* Resize mapping */
	map2 = xtmremap(map, size, 2 * size, XT_MMAN_MREMAP_MAYMOVE);
	if (map2 == XT_MMAN_MAP_FAILED) {
		FAIL("simulate_page_resize() - xtmremap()");
		goto fail;
	}
	map = map2;
	PASS("simulate_page_resize()");
fail:
	if (map != XT_MMAN_MAP_FAILED)
		xtmunmap(map, size);
}

int main(int argc, char **argv)
{
	stats_init(&stats, "mman");
	puts("-- MMAN TEST");

	if (argc < 1 || !argv[0] || !*argv[0])
		SKIP("map_self()");
	else
		map_self(argv[0]);
	simulate_page_alloc();
	simulate_page_resize();

	stats_info(&stats);
	return stats_status(&stats);
}

#define _GNU_SOURCE
// XT headers
#include <xt/mman.h>

// System headers
#include <sys/mman.h>

// STD headers
#include <stdarg.h> // va_list

static int mmap_translate_prot(int prot)
{
	int mmap_prot = 0;

	if (prot & XT_MMAN_PROT_EXEC)
		mmap_prot |= PROT_EXEC;
	if (prot & XT_MMAN_PROT_WRITE)
		mmap_prot |= PROT_WRITE;
	if (prot & XT_MMAN_PROT_READ)
		mmap_prot |= PROT_READ;

	return mmap_prot;
}

static int mmap_translate_flags(int flags)
{
	int mmap_flags = 0;

	if (flags & XT_MMAN_MAP_ANONYMOUS)
		mmap_flags |= MAP_ANONYMOUS;
	if (flags & XT_MMAN_MAP_FIXED)
		mmap_flags |= MAP_FIXED;
	if (flags & XT_MMAN_MAP_PRIVATE)
		mmap_flags |= MAP_PRIVATE;
	if (flags & XT_MMAN_MAP_SHARED)
		mmap_flags |= MAP_SHARED;
	if (flags & XT_MMAN_MAP_FILE)
		mmap_flags |= MAP_FILE;

	return mmap_flags;
}

static int mmap_translate_ms_flags(int flags)
{
	int mmap_flags = 0;

	if (flags & XT_MMAN_MS_ASYNC)
		mmap_flags |= MS_ASYNC;
	if (flags & XT_MMAN_MS_SYNC)
		mmap_flags |= MS_SYNC;
	if (flags & XT_MMAN_MS_INVALIDATE)
		mmap_flags |= MS_INVALIDATE;

	return mmap_flags;
}

void *xtmmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	return mmap(
		addr, len,
		mmap_translate_prot(prot),
		mmap_translate_flags(flags),
		fildes, off
	);
}

int xtmunmap(void *addr, size_t len)
{
	return munmap(addr, len);
}

int xtmprotect(void *addr, size_t len, int prot)
{
	return mprotect(addr, len, mmap_translate_prot(prot));
}

int xtmsync(void *addr, size_t len, int flags)
{
	return msync(addr, len, mmap_translate_ms_flags(flags));
}

int xtmlock(const void *addr, size_t len)
{
	return mlock(addr, len);
}

int xtmunlock(const void *addr, size_t len)
{
	return munlock(addr, len);
}

void *xtmremap(void *old_address, size_t old_size, size_t new_size, int flags, ... /* void *new_address */)
{
	va_list args;
	void *new_address, *retval;
	int new = 0;

	va_start(args, flags);

	if (flags & XT_MMAN_MREMAP_FIXED) {
		new = 1;
		new_address = va_arg(args, void*);
	}
	flags = mmap_translate_flags(flags);
	if (new)
		retval = mremap(old_address, old_size, new_size, flags, new_address);
	else
		retval = mremap(old_address, old_size, new_size, flags);

	va_end(args);

	return retval;
}

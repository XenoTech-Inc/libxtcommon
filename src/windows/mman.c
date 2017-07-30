#define _GNU_SOURCE

// XT headers
#include <xt/mman.h>
#include <xt/error.h>
#include <xt/thread.h>
#include <xt/string.h>
#include <_xt/error.h>

// System headers
#include <windows.h>
#include <io.h>

// STD headers
#include <stdio.h>
#include <stdarg.h> // va_list

#ifndef FILE_MAP_EXECUTE
#define FILE_MAP_EXECUTE    0x0020
#endif

#ifdef XT_MMAN_DEBUG
#define dbgf(f,...) xtprintf(f, ## __VA_ARGS__)
#define dbgs(s) puts(s)
#else
#define dbgf(f,...) ((void)0)
#define dbgs(s) ((void)0)
#endif

/*
 * Sigh... windoze sucks ass yet again...
 * Because xtmremap does not work on windoze out-of-the-box, we have
 * to implement a heap that keeps track of all memory allocations...
 */
static xtMutex _xtMmanLock = XT_MUTEX_INIT;

#define XT_MMAN_HEAP_CAP_DEFAULT 32

struct _xtMmap {
	void *addr;
	size_t size;
	int prot, flags;
	int fd;
	off_t off;
};

static struct _xtMmapHeap {
	struct _xtMmap *data;
	unsigned count, cap;
	int init;
} _xtMmapDefaultHeap;

static void _xtMmapInit(struct _xtMmap *this, void *addr, size_t size, int prot, int flags, int fd, off_t off)
{
	this->addr = addr;
	this->size = size;
	this->prot = prot;
	this->flags = flags;
	this->fd = fd;
	this->off = off;
}

/* Not used at the moment, but may be enabled to prevent valgrind from complaining about non-freed memory */
#if 0
static void _xtMmapFree(struct _xtMmap *this)
{
	if (this->addr != XT_MMAN_MAP_FAILED)
		xtmunmap(this->addr, this->size);
	_xtMmapInit(this, XT_MMAN_MAP_FAILED, 0, 0, 0, -1, 0);
}
#endif

static int _xtMmapHeapInit(struct _xtMmapHeap *this, unsigned cap)
{
	int retval = 1;
	struct _xtMmap *data = malloc(cap * sizeof *data);
	if (!data)
		goto fail;
	this->data = data;
	this->count = 0;
	this->cap = cap;
	retval = 0;
fail:
	if (retval) {
		if (data)
			free(data);
	}
	return retval;
}

static int _xtMmapHeapLazyInit(struct _xtMmapHeap *this)
{
	return this->init ? 0 : _xtMmapHeapInit(this, XT_MMAN_HEAP_CAP_DEFAULT);
}

static void _xtMmapHeapSwap(struct _xtMmapHeap *this, unsigned a, unsigned b)
{
	struct _xtMmap tmp;
	tmp = this->data[a];
	this->data[a] = this->data[b];
	this->data[b] = tmp;
}

#define heap_parent(x) (((x)-1)/2)
#define heap_right(x) (2*((x)+1))
#define heap_left(x) (heap_right(x)-1)

static inline void *mmap_key(const struct _xtMmapHeap *h, unsigned i)
{
	return (void*)h->data[i].addr;
}

static void _xtMmapHeapSift(struct _xtMmapHeap *h, unsigned i)
{
	unsigned l, r, m, size;

	r = heap_right(i);
	l = heap_left(i);
	m = i;
	size = h->count;

	if (l < size && mmap_key(h, l) < mmap_key(h, m))
		m = l;
	if (r < size && mmap_key(h, r) < mmap_key(h, m))
		m = r;

	if (m != i) {
		_xtMmapHeapSwap(h, i, m);
		_xtMmapHeapSift(h, m);
	}
}

static void _xtMmapHeapSiftup(struct _xtMmapHeap *h, unsigned i)
{
	for (unsigned j; i > 0; i = j) {
		j = heap_parent(i);

		if (mmap_key(h, j) > mmap_key(h, i))
			_xtMmapHeapSwap(h, j, i);
		else
			break;
	}
}

static struct _xtMmap *_xtMmapHeapGet(const struct _xtMmapHeap *h, void *key)
{
	struct _xtMmap *map = NULL;
	xtMutexLock(&_xtMmanLock);
	for (unsigned i = 0, n = h->count; i < n;) {
		const void *hkey = mmap_key(h, i);

		if (hkey == key) {
			map = (struct _xtMmap*)&h->data[i];
			goto end;
		}
		if (hkey > key)
			i = heap_right(i) >= n ? heap_left(i) : heap_right(i);
		else
			i = heap_left(i);
	}
end:
	xtMutexUnlock(&_xtMmanLock);
	return map;
}

static unsigned _xtMmapHeapGetIndex(const struct _xtMmapHeap *h, void *key)
{
	for (unsigned i = 0, n = h->count; i < n;) {
		const void *hkey = mmap_key(h, i);

		if (hkey == key)
			return i;
		if (hkey > key)
			i = heap_right(i) >= n ? heap_left(i) : heap_right(i);
		else
			i = heap_left(i);
	}

	return h->cap;
}

#ifdef XT_MMAN_DEBUG
static void _xtMmapDump(const struct _xtMmap *this)
{
	xtprintf(
		"addr = %8p, size: %8zu, prot: %8X, flags: %8X\n",
		this->addr, this->size, this->prot, this->flags
	);
}
#else
static inline void _xtMmapDump(const struct _xtMmap *this)
{
	(void)this;
}
#endif

static void _xtMmapHeapDump(const struct _xtMmapHeap *this)
{
	for (unsigned i = 0, n = this->count; i < n; ++i)
		_xtMmapDump(&this->data[i]);
}

static int _xtMmapHeapRemove(struct _xtMmapHeap *h, void *key, size_t size, bool checklen)
{
	int retval = XT_EUNKNOWN;
	unsigned i;

	xtMutexLock(&_xtMmanLock);
	i = _xtMmapHeapGetIndex(h, key);
	/* Cannot remove mapping */
	if (i >= h->cap) {
		retval = XT_EINVAL;
		goto fail;
	}
	/*
	 * There is no need to check for this strictly speaking,
	 * but this ensures that it behaves the same on windoze and linux.
	 */
	if (checklen && h->data[i].size != size) {
		retval = XT_EINVAL;
		goto fail;
	}
	/*
	 * XXX assumes NULL really is 0 in numerical value.
	 * Not zero according to the standard, because someptr = 0; in the standard
	 * is equivalent to: someptr = NULL;
	 * But we assume here that 0 really is 0 if you would hexdump the value.
	 */
	h->data[i].addr = 0;
	/* Remove the mapping */
	_xtMmapHeapSiftup(h, i);
	h->data[0] = h->data[--h->count];
	_xtMmapHeapSift(h, 0);

	_xtMmapHeapDump(h);
	retval = 0;
fail:
	xtMutexUnlock(&_xtMmanLock);
	return retval;
}

static int _xtMmapHeapAdd(struct _xtMmapHeap *h, void *addr, size_t size, int prot, int flags, int fd, off_t off)
{
	int retval = XT_EUNKNOWN;
	xtMutexLock(&_xtMmanLock);
	retval = _xtMmapHeapLazyInit(h);
	if (retval)
		goto fail;
	if (h->count >= h->cap) {
		unsigned new_cap = h->cap << 1;
		struct _xtMmap *new = realloc(h->data, new_cap * sizeof *new);
		if (!new) {
			retval = XT_ENOMEM;
			goto fail;
		}
		h->data = new;
		h->cap = new_cap;
	}
	_xtMmapInit(&h->data[h->count], addr, size, prot, flags, fd, off);
	_xtMmapHeapSift(h, h->count++);

	_xtMmapHeapDump(h);
	retval = 0;
fail:
	xtMutexUnlock(&_xtMmanLock);
	return retval;
}

static DWORD mmap_translate_prot_page(int prot)
{
	DWORD mmap_prot = 0;

	if (prot == XT_MMAN_PROT_NONE)
		return mmap_prot;

	if (prot & XT_MMAN_PROT_EXEC)
		mmap_prot = prot & XT_MMAN_PROT_WRITE ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
	else
		mmap_prot = prot & XT_MMAN_PROT_WRITE ? PAGE_READWRITE : PAGE_READONLY;

	return mmap_prot;
}

static DWORD mmap_translate_prot_file(int prot)
{
	DWORD access = 0;

	if (prot == XT_MMAN_PROT_NONE)
		return access;

	if (prot & XT_MMAN_PROT_READ)
		access |= FILE_MAP_READ;
	if (prot & XT_MMAN_PROT_WRITE)
		access |= FILE_MAP_WRITE;
	if (prot & XT_MMAN_PROT_EXEC)
		access |= FILE_MAP_EXECUTE;

	return access;
}

void *xtmmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
#ifdef XT_MMAN_DEBUG
	size_t file_offset;
#endif
	HANDLE fm, h;
	DWORD dwFileOffsetLow, dwFileOffsetHigh, protect, desiredAccess;
	DWORD dwMaxSizeLow, dwMaxSizeHigh;
	off_t maxSize;
	int retval = XT_EUNKNOWN;
	void *map = XT_MMAN_MAP_FAILED;

	/* We cannot ask a desired address, so we have to ignore this */
	(void)addr;
	dwFileOffsetLow = (sizeof(off_t) <= sizeof(DWORD)) ? (DWORD)off : (DWORD)(off & 0xFFFFFFFFL);
	dwFileOffsetHigh = (sizeof(off_t) <= sizeof(DWORD)) ? (DWORD)0 : (DWORD)((off >> 32) & 0xFFFFFFFFL);
	protect = mmap_translate_prot_page(prot);
	desiredAccess = mmap_translate_prot_file(prot);
	maxSize = off + (off_t)len;
	dwMaxSizeLow = (sizeof(off_t) <= sizeof(DWORD)) ? (DWORD)maxSize : (DWORD)(maxSize & 0xFFFFFFFFL);
	dwMaxSizeHigh = (sizeof(off_t) <= sizeof(DWORD)) ? (DWORD)0 : (DWORD)((maxSize >> 32) & 0xFFFFFFFFL);

	if (!len || (flags & XT_MMAN_MAP_FIXED) || prot == XT_MMAN_PROT_EXEC) {
		if (!len)
			puts("bad length");
		if (flags & XT_MMAN_MAP_FIXED)
			puts("map fixed not supported");
		if (prot == XT_MMAN_PROT_EXEC)
			puts("exec not supported");
		retval = XT_EINVAL;
		goto fail;
	}
	h = !(flags & XT_MMAN_MAP_ANONYMOUS) ? (HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;
	dbgf("file handle: %p\n", h);
	if (!(flags & XT_MMAN_MAP_ANONYMOUS) && h == INVALID_HANDLE_VALUE) {
		retval = XT_EBADF;
		goto fail;
	}
	fm = CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);
	dbgf("file mapping: %p\n", fm);
	if (!fm)
		goto fail;
	map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);
	dbgf("virtual mapping: %p\n", map);
	CloseHandle(fm);
	if (!map)
		goto fail;
	retval = _xtMmapHeapAdd(&_xtMmapDefaultHeap, map, len, prot, flags, fildes, off);
fail:
	if (retval) {
		dbgs("fail");
		dbgf("xtmmap: %s\n", xtGetErrorStr(_xtTranslateSysError(retval)));
		if (map)
			(void)xtmunmap(map, len);
	}
	return retval ? XT_MMAN_MAP_FAILED : map;
}

static int _xtmunmap(void *addr, size_t len, bool checklen)
{
	int retval = _xtMmapHeapRemove(&_xtMmapDefaultHeap, addr, len, checklen);
	if (retval)
		goto fail;
	if (!UnmapViewOfFile(addr)) {
		retval = XT_EPERM;
#ifdef XT_MMAN_DEBUG
		/* Log this serious error */
		xtfprintf(stderr, "xtmunmap: heap corrupt: %s\n", _xtTranslateSysError(retval));
#endif
		goto fail;
	}
fail:
	return retval;
}

int xtmunmap(void *addr, size_t len)
{
	return _xtmunmap(addr, len, true);
}

int xtmprotect(void *addr, size_t len, int prot)
{
	DWORD newProtect = mmap_translate_prot_page(prot);
	DWORD oldProtect = 0;
	if (VirtualProtect(addr, len, newProtect, &oldProtect))
		return 0;
	return XT_EPERM;
}

int xtmsync(void *addr, size_t len, int flags)
{
	(void)flags;
	if (FlushViewOfFile(addr, len))
		return 0;
	return XT_EPERM;
}

int xtmlock(const void *addr, size_t len)
{
	if (VirtualLock((LPVOID)addr, len))
		return 0;
	return XT_EPERM;
}

int xtmunlock(const void *addr, size_t len)
{
	if (VirtualUnlock((LPVOID)addr, len))
		return 0;
	return XT_EPERM;
}

void *xtmremap(void *old_address, size_t old_size, size_t new_size, int flags, ... /* void *new_address */)
{
	va_list args;
	int error = XT_EUNKNOWN;
	void *map = XT_MMAN_MAP_FAILED;
	void *new_address;
	/* Ignore old size */
	(void)old_size;

	va_start(args, flags);
	/* Grab new_address if specified and ignore it */
	if (flags & XT_MMAN_MREMAP_FIXED) {
		new_address = va_arg(args, void*);
		(void)new_address;
		error = XT_EOPNOTSUPP;
		goto fail;
	}
	if (!(flags & XT_MMAN_MREMAP_MAYMOVE)) {
		error = XT_EOPNOTSUPP;
		goto fail;
	}
	/* Try to create new mapping first and then free old one */
	const struct _xtMmap *m = _xtMmapHeapGet(&_xtMmapDefaultHeap, old_address);
	if (!m || old_size != m->size) {
		error = XT_EINVAL;
		goto fail;
	}
	error = _xtmunmap(old_address, 0, false);
	if (error) {
		/* Log this serious error */
		xtfprintf(stderr, "xtmremap: leaking old mapping at %p: %s\n", old_address, _xtTranslateSysError(error));
		abort();
	}
	map = xtmmap(NULL, new_size, m->prot, m->flags, m->fd, m->off);
	if (map == XT_MMAN_MAP_FAILED) {
		error = XT_EINVAL;
		goto fail;
	}
	error = 0;
fail:
	va_end(args);
	if (error) {
#ifdef XT_MMAN_DEBUG
		xtPerror("xtmremap", error);
#endif
		if (map && map != XT_MMAN_MAP_FAILED)
			(void)_xtmunmap(map, 0, false);
		map = XT_MMAN_MAP_FAILED;
	}
	return map;
}

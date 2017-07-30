/**
 * @brief Functions for virtual file mapping
 *
 * All provided functions are similar to the ones defined in sys/mman.h on POSIX.
 * See the documentation in sys/mman.h for more details about these functions.
 *
 * NOTE: xtmmap and xtmremap behave quite differently on Windows!
 *
 * @file mman.h
 * @author Folkert van Verseveld
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_MMAN_H
#define _XT_MMAN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XT_MMAN_PROT_NONE       0
#define XT_MMAN_PROT_READ       1
#define XT_MMAN_PROT_WRITE      2
#define XT_MMAN_PROT_EXEC       4

#define XT_MMAN_MAP_FILE        0
#define XT_MMAN_MAP_SHARED      1
#define XT_MMAN_MAP_PRIVATE     2
#define XT_MMAN_MAP_TYPE        0xf
#define XT_MMAN_MAP_FIXED       0x10
#define XT_MMAN_MAP_ANONYMOUS   0x20
#define XT_MMAN_MAP_ANON        XT_MMAN_MAP_ANONYMOUS

#define XT_MMAN_MAP_FAILED      ((void*)-1)

/* Flags for msync. */
#define XT_MMAN_MS_ASYNC        1
#define XT_MMAN_MS_SYNC         2
#define XT_MMAN_MS_INVALIDATE   4

#define XT_MMAN_MREMAP_MAYMOVE 1
#define XT_MMAN_MREMAP_FIXED   2

/**
 * @brief map files or devices into memory
 * Create a new mapping in the virtual address space of the calling process.
 * The starting address for the new mapping is specified in addr.
 * The length argument specifies the length of the mapping.
 * NOTE: \a addr is ignored on Windows and XT_MMAN_PROT_EXEC does not work on Windows!
 */
void *xtmmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int xtmunmap(void *addr, size_t len);
int xtmprotect(void *addr, size_t len, int prot);
int xtmsync(void *addr, size_t len, int flags);
int xtmlock(const void *addr, size_t len);
int xtmunlock(const void *addr, size_t len);
/**
 * @brief remap a virtual memory address
 * Expand (or shrink) an existing memory mapping, potentially moving it at the same time (controlled by the flags argument and the available virtual address space).
 * NOTE: Only XT_MMAN_MREMAP_MAYMOVE is accepted on Windows and any other value fails with XT_EINVAL!
 */
void *xtmremap(void *old_address, size_t old_size, size_t new_size, int flags, ... /* void *new_address */);

#ifdef __cplusplus
}
#endif

#endif

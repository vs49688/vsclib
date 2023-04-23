/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
/*
  vsci_aligned_malloc and friends, implemented using Microsoft's public
  interfaces and with the help of the algorithm description provided
  by Wu Yongwei: http://sourceforge.net/mailarchive/message.php?msg_id=3847075

  I hereby place this implementation in the public domain.
               -- Steven G. Johnson (stevenj@alum.mit.edu)
*/

#include <errno.h>
#include <string.h>
#include <vsclib/mem.h>

#define NOT_POWER_OF_TWO(n) (((n) & ((n)-1)))
#define UI(p)               ((uintptr_t)(p))
#define CP(p)               ((char *)p)

#define PTR_ALIGN(p0, alignment, offset) \
    ((void *)(((UI(p0) + (alignment + sizeof(void *)) + offset) & (~UI(alignment - 1))) - offset))

/* Pointer must sometimes be aligned; assume sizeof(void*) is a power of two. */
#define ORIG_PTR(p) (*(((void **)(UI(p) & (~UI(sizeof(void *) - 1)))) - 1))

void *vsc_xaligned_offset_malloc(const VscAllocator *a, size_t size, size_t alignment, size_t offset)
{
    void *p0, *p;

    if(NOT_POWER_OF_TWO(alignment)) {
        errno = EINVAL;
        return ((void *)0);
    }
    if(size == 0)
        return ((void *)0);
    if(alignment < sizeof(void *))
        alignment = sizeof(void *);

    /* Including the extra sizeof(void*) is overkill on a 32-bit
       machine, since malloc is already 8-byte aligned, as long
       as we enforce alignment >= 8 ...but oh well.  */

    p0 = vsc_xalloc(a, size + (alignment + sizeof(void *)));
    if(!p0)
        return ((void *)0);
    p           = PTR_ALIGN(p0, alignment, offset);
    ORIG_PTR(p) = p0;
    return p;
}

void *vsc_xaligned_malloc(const VscAllocator *a, size_t size, size_t alignment)
{
    return vsc_xaligned_offset_malloc(a, size, alignment, 0);
}

void vsc_xaligned_free(const VscAllocator *a, void *memblock)
{
    vsc_xfree(a, ORIG_PTR(memblock));
}

void *vsc_xaligned_offset_realloc(const VscAllocator *a, void *memblock, size_t size, size_t alignment, size_t offset)
{
    void     *p0, *p;
    ptrdiff_t shift;

    if(!memblock)
        return vsc_xaligned_offset_malloc(a, size, alignment, offset);
    if(NOT_POWER_OF_TWO(alignment))
        goto bad;
    if(size == 0) {
        vsc_xaligned_free(a, memblock);
        return ((void *)0);
    }
    if(alignment < sizeof(void *))
        alignment = sizeof(void *);

    p0 = ORIG_PTR(memblock);
    /* It is an error for the alignment to change. */
    if(memblock != PTR_ALIGN(p0, alignment, offset))
        goto bad;
    shift = CP(memblock) - CP(p0);

    p0 = vsc_xrealloc(a, p0, size + (alignment + sizeof(void *)));
    if(!p0)
        return ((void *)0);
    p = PTR_ALIGN(p0, alignment, offset);

    /* Relative shift of actual data may be different from before, ugh.  */
    if(shift != CP(p) - CP(p0))
        /* ugh, moves more than necessary if size is increased.  */
        memmove(CP(p), CP(p0) + shift, size);

    ORIG_PTR(p) = p0;
    return p;

bad:
    errno = EINVAL;
    return ((void *)0);
}

void *vsc_xaligned_realloc(const VscAllocator *a, void *memblock, size_t size, size_t alignment)
{
    return vsc_xaligned_offset_realloc(a, memblock, size, alignment, 0);
}

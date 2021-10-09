/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2021 Zane van Iperen (zane@zanevaniperen.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Basic aligned allocator implementation, using a footer to store
 * size and alignment information.
 *
 * A footer is used instead of a header to minimise excess usage.
 * If a header were used with a large alignment, there could be an excessive
 * amount of padding between the header and the block itself.
 * Using a footer prevents this, as long as the footer is smaller than the
 * block itself, which it should be in most cases. malloc_usable_size() works on
 * all allocated blocks, so finding the footer is trivial.
 *
 * As POSIX/glibc/musl don't provide "aligned_realloc" capability, we
 * technically can't support it, but if the new alignment is smaller than
 * the default alignment, we can fake it (as it'd be aligned anyway because
 * everything's POT). If the alignment is larger, bail with VSC_ERROR(ENOTSUP) and
 * vsc_alloc_ex() will emulate it with a malloc-copy-free. Sorry.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <vsclib/assert.h>
#include <vsclib/error.h>
#include <vsclib/mem.h>
#include <vsclib/types.h>
#include <malloc.h>

#define MEMHDR_SIG 0xFEED5EED /* Formerly Chuck's */

typedef struct MemFooter {
    size_t size;
    size_t alignment;
    uint32_t sig;
} MemFooter;

static inline void *align_up(void *p, size_t alignment)
{
    vsc_assert(VSC_IS_POT(alignment));
    return (void*)((size_t)((uintptr_t)p + (alignment - 1)) & -alignment);
}

static inline void *align_down(void *p, size_t alignment)
{
    uint8_t *a = align_up(p, alignment);
    while(a >= (uint8_t*)p)
        a -= alignment;

    return a;
}

/* Scan backwards at each aligned address until we find it. */
static MemFooter *locate_footer(uint8_t *p)
{
    /* NB: Valgrind cracks the shits at this, even though the memory is usable. */
    MemFooter *ftr = align_down(p + malloc_usable_size(p), VSC_ALIGNOF(MemFooter));

    while(ftr->sig != MEMHDR_SIG)
        --ftr;

    return ftr;
}


static int _malloc(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user)
{
    MemFooter *ftr = NULL, *nftr;
    uint8_t *p, *end;
    size_t reqsize, oldsize = 0;

    (void)user;
    vsc_assert(VSC_IS_POT(alignment));

    /* Make everything easier for ourselves. */
    if(*ptr == NULL)
        flags &= ~VSC_ALLOC_REALLOC;

    /* Size of the block + footer + alignment padding. */
    reqsize = (size_t)align_up((void*)size, VSC_ALIGNOF(MemFooter)) + VSC_ALIGNOF(MemFooter) + sizeof(MemFooter);

    if(flags & VSC_ALLOC_REALLOC) {
        vsc_assert(*ptr != NULL);
        ftr = locate_footer(*ptr);
        oldsize = ftr->size;

        /*
         * There's no "aligned realloc" functionality, but if the alignment
         * is <= VSC_ALIGNOF(vsc_max_align_t), then the standard realloc()
         * is suitable. Otherwise, bail and let vsc_xalloc_ex() emulate it.
         */
        if(ftr->alignment > VSC_ALIGNOF(vsc_max_align_t))
            return VSC_ERROR(ENOTSUP);

        p = vsc_sys_realloc(*ptr, reqsize);
    } else {
        p = vsc_sys_aligned_malloc(reqsize, alignment);
    }

    if(p == NULL)
        return VSC_ERROR(ENOMEM);

    vsc_assert(VSC_IS_ALIGNED(p, alignment));

    end = p + malloc_usable_size(p); /* This might be bigger than size. */

    /* Jump back until we find the closest aligned position to the end .*/
    nftr = align_down(end, VSC_ALIGNOF(MemFooter));
    while((uint8_t*)(nftr + 1) > end)
        --nftr;

    /* Ensure our footer doesn't overlap, this is a bug. */
    vsc_assert(p + size <= (uint8_t*)nftr);

    /* Increase this in case there's any padding. */
    nftr->size      = (uintptr_t)nftr - (uintptr_t)p;
    nftr->alignment = alignment;
    nftr->sig       = MEMHDR_SIG;

    vsc_assert(nftr->size >= size);

    if(flags & VSC_ALLOC_ZERO && nftr->size > oldsize)
        memset(p + oldsize, 0, nftr->size - oldsize);

    *ptr = p;
    return 0;
}

static void _free(void *p, void *user)
{
    int errno_;

    (void)user;
    if(p == NULL)
        return;

    errno_ = errno;
    vsc_sys_aligned_free(p);
    errno = errno_;
}

static size_t _size(void *p, void *user)
{
    (void)user;
    if(p == NULL)
        return 0;

    return locate_footer(p)->size;
}

const VscAllocator vsclib_system_allocator = {
    .alloc     = _malloc,
    .free      = _free,
    .size      = _size,
    .alignment = VSC_ALIGNOF(vsc_max_align_t),
    .user      = NULL,
};
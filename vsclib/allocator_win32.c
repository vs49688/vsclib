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
 * Basic aligned allocator implementation using a header to
 * store size information. This doesn't require extra padding
 * because Windows nicely provides _aligned_offset_realloc().
 *
 * The size information is required in order to implement
 * VscAllocator::size. Without it, the only way to get the size of
 * an _aligned_offset_realloc()'d block is to use _aligned_msize(),
 * which requires saving the alignment, which would use the same
 * amount of storage anyway.
 */
#include <stdint.h>
#include <malloc.h>
#include <vsclib/assert.h>
#include <vsclib/mem.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h> /* For MEMORY_ALLOCATION_ALIGNMENT */

#define MEMHDR_SIG 0xFEED5EED /* Formerly Chuck's */

typedef struct MemHeader {
    size_t   size;
    uint32_t sig;
} MemHeader;

static MemHeader *mem2hdr(void *p)
{
    MemHeader *hdr = (MemHeader*)p - 1;
    vsc_assert(hdr->sig == MEMHDR_SIG);
    return hdr;
}

static int _malloc(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user)
{
    MemHeader *hdr = NULL, *nhdr;
    uint8_t *p;
    size_t reqsize, oldsize = 0;

    (void)user;
    vsc_assert(VSC_IS_POT(alignment));

    /* Make everything easier for ourselves. */
    if(*ptr == NULL)
        flags &= ~VSC_ALLOC_REALLOC;

    /* Size of the block + footer + alignment padding. */
    reqsize = sizeof(MemHeader) + size;

    if(flags & VSC_ALLOC_REALLOC) {
        vsc_assert(*ptr != NULL);
        hdr     = mem2hdr(*ptr);
        oldsize = hdr->size;
    }

    p = _aligned_offset_realloc(hdr, reqsize, alignment, sizeof(MemHeader));
    if(p == NULL)
        return -ENOMEM;

    nhdr = (MemHeader*)p;
    p   += sizeof(MemHeader);

    vsc_assert(VSC_IS_ALIGNED(p, alignment));
    vsc_assert(VSC_IS_ALIGNED(nhdr, VSC_ALIGNOF(MemHeader)));

    //nhdr->size      = _aligned_msize(nhdr, alignment, sizeof(MemHeader)) - sizeof(MemHeader);
    nhdr->size      = size; /* NB: I've never seen this differ from the above. */
    nhdr->sig       = MEMHDR_SIG;

    if(flags & VSC_ALLOC_ZERO && nhdr->size > oldsize)
        memset(p + oldsize, 0, nhdr->size - oldsize);

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
    _aligned_free(mem2hdr(p));
    errno = errno_;
}

static size_t _size(void *p, void *user)
{
    (void)user;
    if(p == NULL)
        return 0;

    return mem2hdr(p)->size;
}

const VscAllocator vsclib_system_allocator = {
    .alloc     = _malloc,
    .free      = _free,
    .size      = _size,
    .alignment = MEMORY_ALLOCATION_ALIGNMENT,
    .user      = NULL,
};
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
 * Basic aligned allocator implementation using a header to store extra information.
 *
 * The header should be at the start of the block.
 *
 * In most cases, i.e. the no special alignment requirements, the data starts
 * immediately after the header. The header location can be verified by looking
 * for a signature at the preceding `sizeof(void*)` bytes.
 *
 * In the case where the alignment requires excessive padding, this method
 * won't work. The naive method is to scan backwards for the signature (slow).
 * Instead, store a pointer to the header in the `sizeof(void*)` bytes, in the
 * same place the signature would be and use that to locate the header.
 *
 * If these bytes match the signature, we know the header is immediately preceding
 * the data. If they don't, assume it's a pointer to the header.
 *
 * TODO:
 * - Add an optimisation to store the header as a footer when the alignment padding
 *   would take up too much space. This still keeps the header address preceding the
 *   data for easy lookup, but a flag would need to be stored in the header so we
 *   don't try to free the wrong pointer.
 */
#include <string.h>
#include <vsclib/assert.h>
#include <vsclib/types.h>
#include <vsclib/error.h>
#include <vsclib/mem.h>

#define MEMHDR_SIG ((uintptr_t)0xFEED5EEDFEED5EEDu) /* Formerly Chuck's */

typedef struct MemHeader {
    size_t size;
    union {
        struct {
            size_t align_power : 8;
            size_t reserved : VSC_SIZE_T_BITSIZE - 8;
        };
        size_t _pad;
    };
    uintptr_t sig;
} MemHeader;

static MemHeader *mem2hdr(void *p)
{
    MemHeader *hdr  = NULL;
    uintptr_t *pend = (uintptr_t *)VSC_ALIGN_DOWN(p, VSC_ALIGNOF(void *));

    if(pend[-1] == MEMHDR_SIG)
        hdr = (MemHeader *)pend - 1;
    else
        hdr = (MemHeader *)*(pend - 1);

    vsc_assert(VSC_IS_ALIGNED(hdr, VSC_ALIGNOF(MemHeader)));
    vsc_assert(hdr->sig == MEMHDR_SIG);
    return hdr;
}

static void *hdr2mem(MemHeader *hdr, size_t alignment)
{
    return vsc_align_up(hdr + 1, alignment);
}

static int malloc_(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user)
{
    MemHeader                     *hdr = NULL, *nhdr;
    uint8_t                       *p;
    size_t                         reqsize, oldsize = 0;
    uintptr_t                      shift;
    const VscSystemAllocatorProcs *procs = user;

    (void)user;
    vsc_assert(VSC_IS_POT(alignment));

    /* Enforce the minimum alignment, this will break headers otherwise. */
    if(alignment < VSC_ALIGNOF(void *))
        alignment = VSC_ALIGNOF(void *);

    /* Make everything easier for ourselves. */
    if(*ptr == NULL)
        flags &= ~VSC_ALLOC_REALLOC;

    vsc_assert(((flags & VSC_ALLOC_REALLOC) && *ptr != NULL) || (flags & VSC_ALLOC_REALLOC) == 0);

    /* Size of the header + block. */
    reqsize = sizeof(MemHeader) + size + alignment;

    if(flags & VSC_ALLOC_REALLOC) {
        vsc_assert(*ptr != NULL);
        hdr     = mem2hdr(*ptr);
        oldsize = hdr->size;

        /* It is an error for the alignment to change. */
        if(!VSC_IS_ALIGNED(*ptr, alignment))
            return -VSC_ERROR(EINVAL);

        shift = (uintptr_t)(*ptr) - (uintptr_t)hdr;
    }

    if((p = procs->realloc(hdr, reqsize)) == NULL)
        return VSC_ERROR(ENOMEM);

    nhdr = (MemHeader *)p;
    p    = hdr2mem(nhdr, alignment);

    vsc_assert(VSC_IS_ALIGNED(p, alignment));
    vsc_assert(VSC_IS_ALIGNED(nhdr, VSC_ALIGNOF(MemHeader)));

    /*
     * For large alignments, store a pointer to the header immediately
     * before our data, so we don't have to scan backwards for the header.
     */
    vsc_assert(VSC_IS_ALIGNED(p, VSC_ALIGNOF(void *)));
    if((uintptr_t)(nhdr + 1) != (uintptr_t)p) {
        vsc_assert((uintptr_t)p - (uintptr_t)(nhdr + 1) >= sizeof(void *));
        *((void **)p - 1) = nhdr;
    }

    nhdr->align_power = vsc_ctz(alignment);
    nhdr->size        = size;
    nhdr->sig         = MEMHDR_SIG;

    if(flags & VSC_ALLOC_REALLOC) {
        /*
         * If our alignment has increased, but we're still aligned our what we
         * were before, the padding between the header/data may have changed. Account for this.
         */
        if(shift != ((uintptr_t)p - (uintptr_t)nhdr))
            memmove(p, (void *)((uintptr_t)nhdr + shift), oldsize);
    }

    if(flags & VSC_ALLOC_ZERO && nhdr->size > oldsize)
        memset(p + oldsize, 0, nhdr->size - oldsize);

    *ptr = p;
    return 0;
}

static void free_(void *p, void *user)
{
    const VscSystemAllocatorProcs *procs = user;

    if(p == NULL)
        return;

    procs->free(mem2hdr(p));
}

static size_t size_(void *p, void *user)
{
    (void)user;
    if(p == NULL)
        return 0;

    return mem2hdr(p)->size;
}

/**
 * \brief Create a new allocator using the provided system-like allocation functions.
 *
 * \param procs A pointer to the VscSystemAllocatorProcs. May not be NULL;
 *
 * \return A new allocator that uses the provided system-like allocation functions.
 */
VscAllocator vsc_allocator_new(const VscSystemAllocatorProcs *procs)
{
    vsc_assert(procs != NULL);

    return (VscAllocator){
        .alloc     = malloc_,
        .free      = free_,
        .size      = size_,
        .alignment = VSC_ALIGNOF(vsc_max_align_t),
        .user      = (void *)procs,
    };
}

const VscSystemAllocatorProcs sysprocs = {
    .realloc = vsc_sys_realloc,
    .free    = vsc_sys_free,
};

const VscAllocator vsclib_system_allocator = {
    .alloc = malloc_,
    .free  = free_,
    .size  = size_,
    /* FIXME: See if I need to use MEMORY_ALLOCATION_ALIGNMENT on Windows */
    .alignment = VSC_ALIGNOF(vsc_max_align_t),
    .user      = (void *)&sysprocs,
};

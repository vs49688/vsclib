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
#include <stdlib.h>
#include <string.h>
#include <vsclib/error.h>
#include <vsclib/assert.h>
#include <vsclib/mem.h>
#include <vsclib/types.h>

uint8_t vsc_ctz(unsigned int x)
{
    vsc_assert(VSC_IS_POT(x));
    vsc_assert(x != 0);

#if defined(__GNUC__)
    return __builtin_ctz(x);
#else
    uint8_t pwr = 0;
    while(x > 1) {
        ++pwr;
        x >>= 1;
    }
    return pwr;
#endif
}

void *vsc_xalloc(const VscAllocator *a, size_t size)
{
    void *p = NULL;
    vsc_assert(a != NULL);
    if(vsc_xalloc_ex(a, &p, size, 0, a->alignment) < 0)
        return NULL;

    return p;
}

int vsc_xalloc_ex(const VscAllocator *a, void **ptr, size_t size, uint32_t flags, size_t alignment)
{
    int ret;

    vsc_assert(a != NULL && ptr != NULL);

    if(alignment == 0)
        alignment = a->alignment;

    vsc_assert(VSC_IS_POT(alignment));

    /*
     * If a new allocation, remove the VSC_ALLOC_REALLOC flag.
     * Simplifies implementing allocators.
     */
    if(*ptr == NULL)
        flags &= ~VSC_ALLOC_REALLOC;

    ret = a->alloc(ptr, size, alignment, flags, a->user);

    /* If our allocator can't handle realloc'ing, fake it. */
    if(ret == VSC_ERROR(ENOTSUP) && (flags & VSC_ALLOC_REALLOC)) {
        void  *p     = *ptr;
        size_t msize = a->size(*ptr, a->user);

        /* Lucky! */
        if(size <= msize)
            return 0;

        if((ret = a->alloc(ptr, size, alignment, flags & ~VSC_ALLOC_REALLOC, a->user)) < 0)
            return ret;

        if(p != NULL) {
            memcpy(*ptr, p, msize);
            a->free(p, a->user);
        }
    } else if(ret < 0) {
        if(flags & VSC_ALLOC_NOFAIL)
            abort();

        return ret;
    }

    vsc_assert(VSC_IS_ALIGNED(*ptr, alignment));
    return 0;
}

void vsc_xfree(const VscAllocator *a, void *p)
{
    vsc_assert(a != NULL);

    if(p == NULL)
        return;

    a->free(p, a->user);
}

void *vsc_xrealloc(const VscAllocator *a, void *ptr, size_t size)
{
    if(size == 0) {
        vsc_xfree(a, ptr);
        return NULL;
    }

    if(vsc_xalloc_ex(a, &ptr, size, VSC_ALLOC_REALLOC, 0) < 0)
        return NULL;

    return ptr;
}

void *vsc_malloc(size_t size)
{
    return vsc_xalloc(vsclib_system_allocator, size);
}

void *vsc_xcalloc(const VscAllocator *a, size_t nmemb, size_t size)
{
    void *p = NULL;
    if(size > 0 && nmemb > SIZE_MAX / size)
        return NULL;

    if(vsc_xalloc_ex(a, &p, size * nmemb, VSC_ALLOC_ZERO, 0) < 0)
        return NULL;

    return p;
}

void *vsc_calloc(size_t nmemb, size_t size)
{
    return vsc_xcalloc(vsclib_system_allocator, nmemb, size);
}

void vsc_free(void *p)
{
    vsc_xfree(vsclib_system_allocator, p);
}

void *vsc_realloc(void *ptr, size_t size)
{
    return vsc_xrealloc(vsclib_system_allocator, ptr, size);
}

void *vsc_aligned_malloc(size_t size, size_t alignment)
{
    void *p = NULL;
    if(vsc_xalloc_ex(vsclib_system_allocator, &p, size, 0, alignment) < 0)
        return NULL;

    return p;
}

void vsc_aligned_free(void *ptr)
{
    vsc_xfree(vsclib_system_allocator, ptr);
}

/*
 * Ported from https://github.com/llvm-mirror/libcxx/blob/6952d1478ddd5a1870079d01f1a0e1eea5b09a1a/src/memory.cpp#L217
 */
void *vsc_align(size_t alignment, size_t size, void **ptr, size_t *space)
{
    void *r = NULL;
    if(size <= *space) {
        char  *p1 = *ptr;
        char  *p2 = (char *)((size_t)(p1 + (alignment - 1)) & -alignment);
        size_t d  = (size_t)(p2 - p1);

        if(d <= *space - size) {
            r    = p2;
            *ptr = r;
            *space -= d;
        }
    }

    return r;
}

int vsc_block_xalloc(const VscAllocator *a, void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks, uint32_t flags)
{
    int                      r;
    size_t                   reqsize, initial_align;
    void                    *block = NULL, *lastptr = NULL;
    const VscBlockAllocInfo *lastblock = NULL;

    if(blockinfo == NULL || nblocks < 1 || ptr == NULL || a == NULL)
        return VSC_ERROR(EINVAL);

    /* Doesn't make sense for us... */
    flags &= ~VSC_ALLOC_REALLOC;

    /* Pass 1: calculate the buffer size */
    reqsize       = blockinfo[0].element_size * blockinfo[0].count;
    initial_align = blockinfo[0].alignment == 0 ? a->alignment : blockinfo[0].alignment;

    /* If we're starting aligned to both X and X^2, make us only aligned to X. */
    if(VSC_IS_ALIGNED(reqsize, initial_align) && VSC_IS_ALIGNED(reqsize, initial_align << 1)) {
        reqsize += initial_align;
        vsc_assert(!VSC_IS_ALIGNED(reqsize, initial_align << 1));
    }

    for(size_t i = 1; i < nblocks; ++i) {
        const VscBlockAllocInfo *bai   = blockinfo + i;
        void                    *p     = (void *)reqsize;
        size_t                   size  = bai->element_size * bai->count;
        size_t                   space = ~(size_t)0;
        size_t                   align = bai->alignment == 0 ? a->alignment : bai->alignment;
        size_t                   pad;

        vsc_align(align, size, &p, &space);

        pad = ~(size_t)0 - space;
        size += pad;
        reqsize += size;
    }

    if((r = vsc_xalloc_ex(a, &block, reqsize, flags, blockinfo[0].alignment)) < 0)
        return r;

    /* Pass 2: Calculate the aligned pointers */
    ptr[0] = lastptr = block;
    lastblock        = blockinfo;
    if(blockinfo[0].out != NULL)
        *blockinfo[0].out = block;

    for(size_t i = 1; i < nblocks; ++i) {
        const VscBlockAllocInfo *curr = blockinfo + i;
        size_t                   prev_size, curr_size, space, align;
        void                    *p;

        curr_size = curr->element_size * curr->count;

        /*
         * Special case for empty blocks, set them NULL.
         * Do NOT update last{ptr,block}.
         */
        if(curr_size == 0) {
            if(curr->out != NULL)
                *curr->out = NULL;

            ptr[i] = NULL;
            continue;
        }

        prev_size = lastblock->element_size * lastblock->count;
        p         = (void *)((uintptr_t)lastptr + prev_size);
        space     = reqsize - ((uintptr_t)p - (uintptr_t)ptr[0]);
        align     = curr->alignment == 0 ? a->alignment : curr->alignment;

        ptr[i] = vsc_align(align, curr_size, &p, &space);

        if(ptr[i] == NULL) {
            /*
             * This is a bug and should never happen.
             * If it does, check pass 1.
             */
            vsc_xfree(a, block);
            memset(ptr, 0, sizeof(ptr[0]) * nblocks);
            return VSC_ERROR(ENOSPC);
        }

        if(curr->out != NULL)
            *curr->out = ptr[i];

        lastptr   = ptr[i];
        lastblock = curr;
    }

    return 0;
}

int vsc_block_alloc(void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks, uint32_t flags)
{
    return vsc_block_xalloc(vsclib_system_allocator, ptr, blockinfo, nblocks, flags);
}

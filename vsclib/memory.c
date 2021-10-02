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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <vsclib/assert.h>
#include <vsclib/mem.h>
#include <vsclib/types.h>

void *vsc_xalloc(const VscAllocator *a, size_t size)
{
    void *p = NULL;
    vsc_assert(a != NULL);
    if(vsc_xalloc_ex(a, &p, size, 0, a->alignment) < 0) {
        errno = ENOMEM;
        return NULL;
    }

    return p;
}

int vsc_xalloc_ex(const VscAllocator *a, void **ptr, size_t size, uint32_t flags, size_t alignment)
{
    int ret;

    vsc_assert(a != NULL && ptr != NULL);

    if(alignment == 0)
        alignment = a->alignment;

    vsc_assert(VSC_IS_POT(alignment));

    ret = a->alloc(ptr, size, alignment, flags, a->user);

    /* If our allocator can't handle realloc'ing, fake it. */
    if(ret == -EOPNOTSUPP && (flags & VSC_ALLOC_REALLOC)) {
        void *p = *ptr;
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
    return vsc_xalloc(&vsclib_system_allocator, size);
}

void *vsc_calloc(size_t nmemb, size_t size)
{
    void *p = NULL;
    if(size > 0 && nmemb > SIZE_MAX / size) {
        errno = ENOMEM; /* FIXME: Is this the right code? */
        return NULL;
    }

    if(vsc_xalloc_ex(&vsclib_system_allocator, &p, size * nmemb, VSC_ALLOC_ZERO, 0) < 0) {
        errno = ENOMEM;
        return NULL;
    }

    return p;
}

void vsc_free(void *p)
{
    vsc_xfree(&vsclib_system_allocator, p);
}

void *vsc_realloc(void *ptr, size_t size)
{
    void *p = vsc_xrealloc(&vsclib_system_allocator, ptr, size);
    if(p == NULL)
        errno = ENOMEM;

    return p;
}

void *vsc_aligned_malloc(size_t size, size_t alignment)
{
    void *p = NULL;
    if(vsc_xalloc_ex(&vsclib_system_allocator, &p, size, 0, alignment) < 0) {
        errno = ENOMEM;
        return NULL;
    }

    return p;
}

void vsc_aligned_free(void *ptr)
{
    vsc_xfree(&vsclib_system_allocator, ptr);
}


/*
 * Ported from https://github.com/llvm-mirror/libcxx/blob/6952d1478ddd5a1870079d01f1a0e1eea5b09a1a/src/memory.cpp#L217
 */
void *vsc_align(size_t alignment, size_t size, void **ptr, size_t *space)
{
    void *r = NULL;
    if(size <= *space) {
        char *p1 = *ptr;
        char *p2 = (char*)((size_t)(p1 + (alignment - 1)) & -alignment);
        size_t d = (size_t)(p2 - p1);

        if(d <= *space - size) {
            r       = p2;
            *ptr    = r;
            *space -= d;
        }
    }

    return r;
}

void *vsc_block_xalloc(const VscAllocator *a, void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks)
{
    size_t reqsize;
    void *block = NULL;

    if(blockinfo == NULL || nblocks < 1 || ptr == NULL || a == NULL) {
        errno = EINVAL;
        return NULL;
    }

    /* Pass 1: calculate the buffer size */
    reqsize = blockinfo[0].element_size * blockinfo[0].count;
    for(size_t i = 1; i < nblocks; ++i) {
        const VscBlockAllocInfo *bai = blockinfo + i;
        void *p      = (void*)reqsize;
        size_t size  = bai->element_size * bai->count;
        size_t space = ~(size_t)0;
        size_t pad;

        vsc_align(bai->alignment, size, &p, &space);

        pad      = ~(size_t)0 - space;
        size    += pad;
        reqsize += size;
    }

    if(vsc_xalloc_ex(a, &block, reqsize, 0, blockinfo[0].alignment) < 0) {
        errno = ENOMEM;
        return NULL;
    }

    /* Pass 2: Calculate the aligned pointers */
    ptr[0] = block;
    for(size_t i = 1; i < nblocks; ++i) {
        const VscBlockAllocInfo *bai = blockinfo + i;

        size_t size  = bai->element_size * bai->count;
        void *p      = (void*)((uintptr_t)ptr[i-1] + size);
        size_t space = reqsize - (uintptr_t)ptr[i-1];

        ptr[i] = vsc_align(bai->alignment, size, &p, &space);
        if(ptr[i] == NULL) {
            /*
             * This is a bug and should never happen.
             * If it does, check pass 1.
             */
            vsc_xfree(a, block);
            memset(ptr, 0, sizeof(ptr[0]) * nblocks);
            errno = ENOSPC;
            return NULL;
        }
    }

    return block;
}

void *vsc_block_alloc(void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks)
{
    return vsc_block_xalloc(&vsclib_system_allocator, ptr, blockinfo, nblocks);
}

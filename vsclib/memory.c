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

static int _malloc(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user)
{
    int errno_;
    void *p;

    /*
     * NB: Realloc'ing not supported with the system allocator due
     *     there being no way to realloc with alignment.
     *     vsc_xalloc_ex can handle this.
     */
    if(flags & VSC_ALLOC_REALLOC)
        return -EOPNOTSUPP;

    /* We're not allowed to change errno. */
    errno_ = errno;
    p      = vsc_sys_aligned_malloc(size, alignment);
    errno  = errno_;

    if(p == NULL)
        return -ENOMEM;

    if(flags & VSC_ALLOC_ZERO)
        memset(p, 0, size);

    *ptr = p;
    return 0;
}

static void _free(void *p, void *user)
{
    int errno_;

    if(p == NULL)
        return;

    errno_ = errno;
    vsc_sys_aligned_free(p);
    errno = errno_;
}

static size_t _size(void *p, void *user)
{
    return vsc_sys_malloc_usable_size(p);
}

const VscAllocator vsclib_system_allocator = {
    .alloc     = _malloc,
    .free      = _free,
    .size      = _size,
    .alignment = 16, /* FIXME: Find a way to get this per-platform. */
    .user      = NULL,
};

void *vsc_xalloc(const VscAllocator *a, size_t size)
{
    vsc_assert(a != NULL);
    return vsc_xalloc_ex(a, NULL, size, 0, a->alignment);
}

void *vsc_xalloc_ex(const VscAllocator *a, void *ptr, size_t size, VscAllocFlags flags, size_t alignment)
{
    int errno_, ret;
    void *p;

    vsc_assert(a != NULL);

    if(alignment == 0)
        alignment = a->alignment;

    vsc_assert(VSC_IS_POT(alignment));

    errno_ = errno;
    p      = ptr;
    ret    = a->alloc(&p, size, alignment, flags, a->user);

    /* If our allocator can't handle realloc'ing, fake it. */
    if(ret == -EOPNOTSUPP && (flags & VSC_ALLOC_REALLOC)) {
        size_t msize = a->size(ptr, a->user);

        /* Lucky! */
        if(size <= msize)
            return ptr;

        p   = ptr;
        ret = a->alloc(&p, size, alignment, flags & ~VSC_ALLOC_REALLOC, a->user);

        if(ret < 0) {
            errno = -ret;
            return NULL;
        }

        if(ptr != NULL) {
            memcpy(p, ptr, msize);
            a->free(ptr, a->user);
        }
    } else if(ret < 0) {
        errno = -ret;
        return NULL;
    }

    errno = errno_;
    return p;
}


void vsc_xfree(const VscAllocator *a, void *p)
{
    int errno_;

    vsc_assert(a != NULL);

    if(p == NULL)
        return;

    errno_ = errno;
    a->free(p, a->user);
    errno  = errno_;
}

void *vsc_xrealloc(const VscAllocator *a, void *ptr, size_t size)
{
    if(size == 0) {
        vsc_free(ptr);
        return NULL;
    }

    return vsc_xalloc_ex(a, ptr, size, VSC_ALLOC_REALLOC, 0);
}

void *vsc_malloc(size_t size)
{
    return vsc_xalloc(&vsclib_system_allocator, size);
}

void *vsc_calloc(size_t nmemb, size_t size)
{
    return vsc_xalloc_ex(&vsclib_system_allocator, NULL, size, VSC_ALLOC_ZERO, 0);
}

void vsc_free(void *p)
{
    vsc_xfree(&vsclib_system_allocator, p);
}

void *vsc_realloc(void *ptr, size_t size)
{
    return vsc_xrealloc(&vsclib_system_allocator, ptr, size);
}

void *vsc_aligned_malloc(size_t size, size_t alignment)
{
    return vsc_xalloc_ex(&vsclib_system_allocator, NULL, size, 0, alignment);
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
    void *block;

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

    if((block = vsc_xalloc_ex(a, NULL, reqsize, 0, blockinfo[0].alignment)) == NULL)
        return NULL;

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

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
#include <vsclib/assert.h>
#include <vsclib/mem.h>

static void *_malloc(size_t size, void *user)
{
    int errno_ = errno;
    void *p = vsc_malloc(size);
    errno = errno_;
    return p;
}

static void _free(void *p, void *user)
{
    int errno_ = errno;
    vsc_free(p);
    errno = errno_;
}

static void *_realloc(void *ptr, size_t size, void *user)
{
    int errno_ = errno;
    void *p = vsc_realloc(ptr, size);
    errno = errno_;
    return p;
}

const VscAllocator vsclib_system_allocator = {
    .alloc = _malloc,
    .free = _free,
    .realloc = _realloc,
    .user = NULL
};

void *vsc_xalloc(const VscAllocator *a, size_t size)
{
    vsc_assert(a != NULL);

    int errno_ = errno;
    void *p = a->alloc(size, a->user);
    errno = errno_;
    return p;
}

void vsc_xfree(const VscAllocator *a, void *p)
{
    vsc_assert(a != NULL);

    if(p == NULL)
        return;

    int errno_ = errno;
    a->free(p, a->user);
    errno = errno_;
}

void *vsc_xrealloc(const VscAllocator *a, void *ptr, size_t size)
{
    vsc_assert(a != NULL);

    int errno_ = errno;
    void *p = a->realloc(ptr, size, a->user);
    errno = errno_;
    return p;
}

void *vsc_malloc(size_t size)
{
    return malloc(size);
}

void vsc_free(void *p)
{
    free(p);
}

void *vsc_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

#if defined(_MSC_VER)
void *vsc_aligned_malloc(size_t size, size_t alignment)
{
    vsc_assert(VSC_IS_POT(alignment)); /* Or error with EINVAL? */
    return _aligned_malloc(size, alignment);
}

void vsc_aligned_free(void *ptr)
{
    _aligned_free(ptr);
}
#else
void *vsc_aligned_malloc(size_t size, size_t alignment)
{
    return aligned_alloc(alignment, size);
}

void vsc_aligned_free(void *ptr)
{
    free(ptr);
}
#endif

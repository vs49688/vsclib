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
#ifndef _VSCLIB_MEM_H
#define _VSCLIB_MEM_H

#include <stddef.h>
#include "memdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

static inline int vsc_is_pot(unsigned int alignment)
{
    return VSC_IS_POT(alignment);
}

static inline int vsc_is_aligned(void *p, size_t alignment)
{
    return VSC_IS_ALIGNED(p, alignment);
}

void *vsc_align(size_t alignment, size_t size, void **ptr, size_t *space);

void *vsc_malloc(size_t size);
void *vsc_calloc(size_t nmemb, size_t size);
void  vsc_free(void *p);
void *vsc_realloc(void *ptr, size_t size);

void *vsc_aligned_malloc(size_t size, size_t alignment);
void  vsc_aligned_free(void *ptr);

void *vsc_xalloc(const VscAllocator *a, size_t size);
void *vsc_xalloc_ex(const VscAllocator *a, void *ptr, size_t size, VscAllocFlags flags, size_t alignment);
void  vsc_xfree(const VscAllocator *a, void *p);
void *vsc_xrealloc(const VscAllocator *a, void *ptr, size_t size);

void *vsc_block_xalloc(const VscAllocator *a, void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks);
void *vsc_block_alloc(void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks);

void *vsc_sys_malloc(size_t size);
void *vsc_sys_calloc(size_t nmemb, size_t size);
void  vsc_sys_free(void *p);
void *vsc_sys_realloc(void *ptr, size_t size);
void *vsc_sys_aligned_malloc(size_t size, size_t alignment);
void  vsc_sys_aligned_free(void *ptr);
size_t vsc_sys_malloc_usable_size(void *ptr);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_MEM_H */

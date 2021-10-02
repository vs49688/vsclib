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
/** \file vsclib/mem.h */
#ifndef _VSCLIB_MEM_H
#define _VSCLIB_MEM_H

#include <stddef.h>
#include "platform.h"
#include "memdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern const VscAllocator vsclib_system_allocator;

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

/**
 * \brief Allocate a block of memory using the given allocator.
 *
 * \param a    A pointer to the allocator to use. May not be NULL.
 * \param size The requested size of the allocation.
 *
 * \return On success, returns a pointer to the allocated block.
 *         On failure, returns NULL and sets `errno`.
 *
 * \remark This is same as calling `vsc_xalloc_ex(a, NULL, size, 0, a->alignment);`
 */
void *vsc_xalloc(const VscAllocator *a, size_t size);

/**
 * \brief Extended memory allocation routine.
 *
 * \param a         A pointer to the allocator to use. May not be NULL.
 * \param ptr[in]   The address of an already existing block. May be NULL. Ignored if
 *                  the #VSC_ALLOC_REALLOC flag is not set.
 * \param size      The requested size of the allocation.
 * \param flags     The allocation flags. See #VscAllocFlags documentation.
 * \param alignment The alignment of the allocated block. If zero, use the allocator's
 *                  default alignment. If nonzero, must be power-of-two.
 *
 * \remark If the supplied allocator does not support #VSC_ALLOC_REALLOC,
 *         vsc_xalloc_ex() will emulate it with a alloc-copy-free operation.
 *
 * \returns On success, this function returns a pointer to the allocated
 *          block. On failure, returns NULL and sets `errno`.
 */
void *vsc_xalloc_ex(const VscAllocator *a, void *ptr, size_t size, uint32_t flags, size_t alignment);

/**
 * \brief Free memory allocated by vsc_xalloc(), vsc_xrealloc(), or vsc_xalloc_ex().
 *
 * \remark This is a thin wrapper around #VscAllocator::free.
 *
 * \param a A pointer to the allocator to use. May not be NULL.
 * \param p A pointer to the memory to free.
 *
 * \remark vsc_xfree() is a no-op if \p p is NULL.
 */
void vsc_xfree(const VscAllocator *a, void *p);

/**
 * \brief Reallocate memory allocated by vsc_xalloc(), vsc_xrealloc(), or vsc_xalloc_ex().
 *
 * \param a    A pointer to the allocator to use. May not be NULL.
 * \param ptr  A pointer to the memory to reallocate/free. May be NULL.
 * \param size The requested size of the allocation. If zero, this is equivalent to vsc_xfree().
 *
 * \return On success, a pointer to the allocated memory. If \p size was 0, or on error, returns NULL.
 *         `errno` will be set appropriately.
 *
 * \remark This may not be supported by all allocators.
 * \remark This is a thin wrapper around vsc_xalloc_ex() and thus inherits
 *         its behaviour.
 * \remark The semantics of this are the same as realloc().
 * \remark This is the same as calling `vsc_xalloc_ex(a, ptr, size, VSC_ALLOC_REALLOC, 0);`
 *
 * \sa realloc()
 */
void *vsc_xrealloc(const VscAllocator *a, void *ptr, size_t size);

void *vsc_block_xalloc(const VscAllocator *a, void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks);
void *vsc_block_alloc(void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks);

/**
 * \brief Invoke the system's memory allocation procedure.
 *
 * \remark Directly calls malloc().
 */
void *vsc_sys_malloc(size_t size);

/**
 * \brief Invoke the system's memory allocation+zero procedure.
 *
 * \remark Directly calls calloc().
 */
void *vsc_sys_calloc(size_t nmemb, size_t size);

/**
 * \brief Invoke the system's memory release procedure.
 *
 * \remark Directly calls free().
 */
void  vsc_sys_free(void *p);

/**
 * \brief Invoke the system's reallocation procedure.
 *
 * \remark Directly calls realloc().
 */
void *vsc_sys_realloc(void *ptr, size_t size);

/**
 * \brief Invoke the system's aligned allocation procedure.
 *
 * \remark On Windows, calls _aligned_malloc().
 * \remark On C11 systems, calls aligned_alloc().
 */
void *vsc_sys_aligned_malloc(size_t size, size_t alignment);

/**
 * \brief Invoke the systems's aligned free prodecure.
 *
 * This is capable of freeing memory allocated by vsc_sys_aligned_malloc().
 *
 * \remark On Windows, calls _aligned_free(). Otherwise, calls free().
 */
void  vsc_sys_aligned_free(void *ptr);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_MEM_H */

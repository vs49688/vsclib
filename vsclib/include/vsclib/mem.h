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

/**
 * \brief Check if the given alignment is a valid
 *  power-of-two.
 *
 * \param alignment The alignment.
 *
 * \return If \p alignment is a valid power-of-two,
 *  returns 1. Otherwise, returns 0.
 *
 * \remark Note that 0 is NOT a valid power of two.
 * \sa https://stackoverflow.com/a/600306
 */
static inline int vsc_is_pot(unsigned int alignment)
{
    return VSC_IS_POT(alignment);
}

/**
 * \brief Check if \p p is aligned to \p alignment.
 *
 * \param p         The pointer whose alignment to check.
 * \param alignment The expected alignment. Must be power-of-two.
 *
 * \return If \p is aligned to \p alignment, returns 1. Otherwise,
 *  returns 0.
 */
static inline int vsc_is_aligned(void *p, size_t alignment)
{
    return VSC_IS_ALIGNED(p, alignment);
}

/**
 * \brief Given a pointer ptr to a buffer of size space, returns a
 * pointer aligned by the specified alignment for size number of bytes
 * and decreases space argument by the number of bytes used for alignment.
 * The first aligned address is returned.
 *
 * The function modifies the pointer only if it would be possible to fit
 * the wanted number of bytes aligned by the given alignment into the buffer.
 * If the buffer is too small, the function does nothing and returns `NULL`.
 * The behavior is undefined if alignment is not a power of two.
 *
 * \param alignment the desired alignment
 * \param size      the size of the storage to be aligned
 * \param ptr       pointer to contiguous storage of at least space bytes
 * \param space     the size of the buffer in which to operate
 *
 * \returns The adjusted value of \p ptr, or `NULL` if the space provided is too small.
 *
 * \sa https://en.cppreference.com/w/cpp/memory/align
 */
void *vsc_align(size_t alignment, size_t size, void **ptr, size_t *space);

/**
 * \brief Invoke vsc_xalloc() with the system's default allocator.
 *
 * \remark The semantics of this are the same as malloc().
 *
 * \sa malloc()
 * \sa vsc_xalloc()
 */
void *vsc_malloc(size_t size);

/**
 * \brief Invoke vsc_xcalloc() with the system's default allocator.
 *
 * \remark The semantics of this are the same as calloc().
 *
 * \sa calloc()
 * \sa vsc_xcalloc()
 */
void *vsc_calloc(size_t nmemb, size_t size);

/**
 * \brief Invoke vsc_xfree() with the system's default allocator.
 *
 * \remark The semantics of this are the same as free().
 *
 * \sa free()
 * \sa vsc_xfree()
 */
void vsc_free(void *p);

/**
 * \brief Invoke vsc_xrealloc() with the system's default allocator.
 *
 * \remark The semantics of this are the same as realloc().
 *
 * \sa vsc_xrealloc
 * \sa realloc()
 */
void *vsc_realloc(void *ptr, size_t size);

/**
 * \brief Allocate an aligned block of memory using the system's
 *  default allocator.
 *
 * The returned block may be passed to either vsc_free() or
 * vsc_aligned_free().
 *
 * \param size The size of the block to allocate.
 * \param alignment The alignment of the block. Must be a power-of-two.
 *
 * \returns On success, returns a pointer to the allocated block.
 *          On failure, returns NULL.
 *
 */
void *vsc_aligned_malloc(size_t size, size_t alignment);

/**
 * \brief Same as invoking vsc_free().
 *
 * \param ptr A pointer to the memory to free. May NULL.
 *
 * \remark This is only provided for consistency purposes - it
 *  is no different from vsc_free().
 */
void vsc_aligned_free(void *ptr);

/**
 * \brief Allocate a block of memory using the given allocator.
 *
 * \param a    A pointer to the allocator to use. May not be NULL.
 * \param size The requested size of the allocation.
 *
 * \return On success, returns a pointer to the allocated block.
 *         On failure, returns NULL.
 *
 * \remark This is same as calling `vsc_xalloc_ex(a, NULL, size, 0, a->alignment);`
 */
void *vsc_xalloc(const VscAllocator *a, size_t size);

/**
 * \brief Extended memory allocation routine.
 *
 * \param a           A pointer to the allocator to use. May not be NULL.
 * \param ptr[in,out] A pointer to receive the address of the allocated block.
 *                    If the #VSC_ALLOC_REALLOC flag is set, this may contain a pointer to
 *                    an existing block to be reallocated.
 * \param size        The requested size of the allocation.
 * \param flags       The allocation flags. See #VscAllocFlags documentation.
 * \param alignment   The alignment of the allocated block. If zero, use the allocator's
 *                    default alignment. If nonzero, must be power-of-two.
 *
 * \remark If the supplied allocator does not support #VSC_ALLOC_REALLOC,
 *         vsc_xalloc_ex() will emulate it with a alloc-copy-free operation.
 *
 * \returns On success, returns 0. On failure, returns a negative error value.
 */
int vsc_xalloc_ex(const VscAllocator *a, void **ptr, size_t size, uint32_t flags, size_t alignment);

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

/**
 * \brief Allocate a block of memory capable of holding \p nmemb elements of
 * \p size bytes using the system's default allocator.
 *
 * The returned pointer may be passed to vsc_free.
 *
 * \param a      A pointer to the allocator to use. May not be NULL.
 * \param nmemb The number of elements.
 * \param size  The size of each element.
 *
 * \returns On success, a pointer to the allocated block, or NULL if allocation failed.
 *
 * \remark The semantics of this are the same as calloc().
 * \remark This is the same as invoking vsc_xalloc_ex() with the #VSC_ALLOC_ZERO
 *  flag.
 * \sa calloc()
 *
 */
void *vsc_xcalloc(const VscAllocator *a, size_t nmemb, size_t size);

/**
 * \brief Allocate a contiguous block of memory described by the
 * provided \p blockinfo argument.
 *
 * \param a         A pointer to the allocator to use. May NOT be NULL.
 * \param ptr       An array of void* to receive the starting addresses of each block.
 * \param blockinfo An array of VscBlockAllocInfo structures describing the blocks.
 * \param nblocks   The number of blocks, i.e. the number of elements in \p ptr and \p blockinfo.
 * \param flags     The allocation flags. See #VscAllocFlags documentation.
 *
 * \return On success, returns 0. On failure, returns a negative error value.
 *
 * \remark If the total size (VscBlockAllocInfo::count * VscBlockAllocInfo::element_size) of a block is
 *   zero, it's address will be set to NULL. This is to prevent subtle bugs by causing crashes in
 *   their place.
 */
int vsc_block_xalloc(const VscAllocator *a, void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks, uint32_t flags);

/**
 * \brief Invoke vsc_block_xalloc() with the default allocator.
 * \sa vsc_block_xalloc
 */
int vsc_block_alloc(void **ptr, const VscBlockAllocInfo *blockinfo, size_t nblocks, uint32_t flags);

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
void vsc_sys_free(void *p);

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

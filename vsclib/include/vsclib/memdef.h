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
#ifndef _VSCLIB_MEMDEF_H
#define _VSCLIB_MEMDEF_H

#include <stddef.h>
#include <stdint.h>

typedef enum VscAllocFlags {
    /**
     * @brief Memory should be zero'd after allocation.
     */
    VSC_ALLOC_ZERO    = 1 << 0,
    VSC_ALLOC_NOFAIL  = 1 << 1,
    /**
     * @brief Attempt to reallocate an existing buffer.
     * This may not be supported by all allocators.
     */
    VSC_ALLOC_REALLOC = 1 << 2,
} VscAllocFlags;

/**
 * @brief Memory allocation callback procedure.
 *
 * Invoked by `vsc_xalloc_ex()` to allocate memory.
 *
 * @param[inout] ptr        A pointer to receive the address of the allocated buffer.
 *                          If the `VSC_ALLOC_REALLOC` flag is set, this may contain a pointer
 *                          to an already-existing buffer to be reallocated.
 * @param[in]    size       The requested size of the allocation.
 * @param[in]    alignment  The required alignment of the buffer. Must be power-of-two.
 * @param[in]    flags      The memory allocation flags.
 * @param[in]    user       A user-provided pointer.
 *
 * @remark  If reallocation is not supported, and the `VSC_ALLOC_REALLOC` flag is set, then
 *          this function should fail immediately and return `-EOPNOTSUPP`. There is special
 *          handling in `vsc_xalloc_ex()` for this situation, which will emulate the behaviour
 *          if possible.
 * @remark  This function MUST NOY modify errno.
 *
 * @returns On success, this function returns 0 and writes the address of the newly-allocated
 *          buffer to @par ptr. On error, this functions a negative errno value.
 */
typedef int (*VscAllocatorAllocProc)(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user);

/**
 * @brief Memory release callback procedure.
 *
 * @param[in] p     A pointer to the memory to free. NULL pointers are ignored.
 * @param[in] user  A user-provided pointer.
 */
typedef void (*VscAllocatorFreeProc)(void *p, void *user);

/**
 * @brief Memory size callback procedure.
 *
 * @param[in] p     A pointer to target block.
 * @param[in] user  A user-provided pointer.
 *
 * @returns Returns the effective usable size of a block of memory. If @p is NULL,
 *          returns 0.
 */
typedef size_t (*VscAllocatorSizeProc)(void *p, void *user);

typedef struct
{
    VscAllocatorAllocProc alloc;
    VscAllocatorFreeProc  free;
    VscAllocatorSizeProc  size;

    size_t alignment;
    void  *user;
} VscAllocator;

typedef struct VscBlockAllocInfo {
    size_t count;
    size_t element_size;
    size_t alignment;
} VscBlockAllocInfo;

#endif /* _VSCLIB_MEMDEF_H */

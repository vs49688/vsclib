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
// ReSharper disable CppJoinDeclarationAndAssignment
#include <cstring>
#include <type_traits>
#include <vsclib/assert.h>
#include <vsclib/types.h>
#include <vsclib/error.h>
#include <vsclib/mem.h>
#include "allocator_internal.h"

template <typename H>
H *vsc__allocator_mem2hdr(void *p)
{
    H         *hdr  = nullptr;
    uintptr_t *pend = static_cast<uintptr_t *>(VSC_ALIGN_DOWN(p, VSC_ALIGNOF(void *)));

    if(pend[-1] == VSC__MEMHDR_SIG)
        hdr = reinterpret_cast<H *>(pend) - 1;
    else
        hdr = reinterpret_cast<H *>(*(pend - 1));

    vsc_assert(VSC_IS_ALIGNED(hdr, VSC_ALIGNOF(H)));
    vsc_assert(hdr->sig == VSC__MEMHDR_SIG);
    return hdr;
}

template <typename H>
void *vsc__allocator_hdr2mem(H *hdr, size_t alignment)
{
    return vsc_align_up(hdr + 1, alignment);
}

static VscAllocFlags operator&(VscAllocFlags a, int b)
{
    /* Because C++. */
    return static_cast<VscAllocFlags>(static_cast<std::underlying_type_t<VscAllocFlags>>(a) &
                                      static_cast<std::underlying_type_t<VscAllocFlags>>(b));
}

template <typename H>
static int malloc_(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user)
{
    H        *hdr = nullptr, *nhdr;
    uint8_t  *p;
    size_t    reqsize, oldsize = 0;
    uintptr_t shift = 0;

    (void)user;
    vsc_assert(VSC_IS_POT(alignment));

    /* Enforce the minimum alignment, this will break headers otherwise. */
    if(alignment < VSC_ALIGNOF(void *))
        alignment = VSC_ALIGNOF(void *);

    /* Make everything easier for ourselves. */
    if(*ptr == nullptr)
        flags = flags & ~VSC_ALLOC_REALLOC;

    vsc_assert(((flags & VSC_ALLOC_REALLOC) && *ptr != nullptr) || (flags & VSC_ALLOC_REALLOC) == 0);

    /* Size of the header + block. */
    reqsize = sizeof(H) + size + alignment;

    if(flags & VSC_ALLOC_REALLOC) {
        vsc_assert(*ptr != nullptr);
        hdr     = vsc__allocator_mem2hdr<H>(*ptr);
        oldsize = hdr->size;

        /* If is an error for the alignment to decrease. */
        if(alignment < (1u << hdr->align_power))
            return VSC_ERROR(EINVAL);

        shift = reinterpret_cast<uintptr_t>(*ptr) - reinterpret_cast<uintptr_t>(hdr);
    }

    if((p = static_cast<uint8_t *>(vsc_sys_realloc(hdr, reqsize))) == nullptr)
        return VSC_ERROR(ENOMEM);

    nhdr = reinterpret_cast<H *>(p);
    p    = static_cast<uint8_t *>(vsc__allocator_hdr2mem(nhdr, alignment));

    vsc_assert(VSC_IS_ALIGNED(p, alignment));
    vsc_assert(VSC_IS_ALIGNED(nhdr, VSC_ALIGNOF(H)));

    /*
     * For large alignments, store a pointer to the header immediately
     * before our data, so we don't have to scan backwards for the header.
     */
    vsc_assert(VSC_IS_ALIGNED(p, VSC_ALIGNOF(void *)));
    if(reinterpret_cast<uintptr_t>(nhdr + 1) != reinterpret_cast<uintptr_t>(p)) {
        vsc_assert(reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(nhdr + 1) >= sizeof(void *));
        *(reinterpret_cast<void **>(p) - 1) = nhdr;
    }

    nhdr->size        = size;
    nhdr->align_power = vsc_ctz(alignment);
    nhdr->reserved    = 0;
    nhdr->sig         = VSC__MEMHDR_SIG;

    if(flags & VSC_ALLOC_REALLOC) {
        /*
         * If our alignment has increased, but we're still aligned our what we
         * were before, the padding between the header/data may have changed. Account for this.
         */
        if(shift != (reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(nhdr)))
            memmove(p, reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(nhdr) + shift), oldsize);
    }

    if(flags & VSC_ALLOC_ZERO && nhdr->size > oldsize)
        memset(p + oldsize, 0, nhdr->size - oldsize);

    *ptr = p;
    return 0;
}

template <typename H>
static void free_(void *p, void *user)
{
    (void)user;
    if(p == nullptr)
        return;

    vsc_sys_free(vsc__allocator_mem2hdr<H>(p));
}

template <typename H>
static size_t size_(void *p, void *user)
{
    (void)user;
    if(p == nullptr)
        return 0;

    return vsc__allocator_mem2hdr<H>(p)->size;
}

static constexpr VscAllocator default_allocator = {
    /* .alloc     = */ malloc_<MemHeader>,
    /* .free      = */ free_<MemHeader>,
    /* .size      = */ size_<MemHeader>,
    /* .alignment = */ VSC_ALIGNOF(vsc_max_align_t),
    /* .user      = */ nullptr,
};

extern "C" const VscAllocator *const vsclib_system_allocator = &default_allocator;

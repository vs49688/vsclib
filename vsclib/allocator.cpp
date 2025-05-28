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
#include "allocator_internal.hpp"
#include "vsclib/resource.h"

template <typename H>
static H *vsc_allocator_mem2hdr(void *p)
{
    H         *hdr  = nullptr;
    uintptr_t *pend = static_cast<uintptr_t *>(VSC_ALIGN_DOWN(p, VSC_ALIGNOF(void *)));

    if(pend[-1] == H::kSignature)
        hdr = reinterpret_cast<H *>(pend) - 1;
    else
        hdr = reinterpret_cast<H *>(*(pend - 1));

    vsc_assert(VSC_IS_ALIGNED(hdr, VSC_ALIGNOF(H)));
    vsc_assert(hdr->base.sig == H::kSignature);
    return hdr;
}

template <typename H>
static void *vsc__allocator_hdr2mem(H *hdr, size_t alignment)
{
    return vsc_align_up(hdr + 1, alignment);
}

template <typename H>
static void *vsc__allocator_hdr2mem(H *hdr)
{
    return vsc_align_up(hdr + 1, 1 << hdr->base.align_power);
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
        hdr     = vsc_allocator_mem2hdr<H>(*ptr);
        oldsize = hdr->base.size;

        /* If is an error for the alignment to decrease. */
        if(alignment < (1u << hdr->base.align_power))
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

    memset(nhdr, 0, sizeof(H));

    nhdr->base.size        = size;
    nhdr->base.align_power = vsc_ctz(alignment);
    nhdr->base.reserved    = 0;
    nhdr->base.sig         = H::kSignature;

    if(flags & VSC_ALLOC_REALLOC) {
        /*
         * If our alignment has increased, but we're still aligned our what we
         * were before, the padding between the header/data may have changed. Account for this.
         */
        if(shift != (reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(nhdr)))
            memmove(p, reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(nhdr) + shift), oldsize);
    }

    if(flags & VSC_ALLOC_ZERO && nhdr->base.size > oldsize)
        memset(p + oldsize, 0, nhdr->base.size - oldsize);

    *ptr = p;
    return 0;
}

template <typename H>
static void free_(void *p, void *user)
{
    (void)user;
    if(p == nullptr)
        return;

    vsc_sys_free(vsc_allocator_mem2hdr<H>(p));
}

template <typename H>
static size_t size_(void *p, void *user)
{
    (void)user;
    if(p == nullptr)
        return 0;

    return vsc_allocator_mem2hdr<H>(p)->base.size;
}

static constexpr VscAllocator default_allocator = {
    /* .alloc     = */ malloc_<MemHeader>,
    /* .free      = */ free_<MemHeader>,
    /* .size      = */ size_<MemHeader>,
    /* .alignment = */ VSC_ALIGNOF(vsc_max_align_t),
    /* .user      = */ nullptr,
};

extern "C" const VscAllocator *const vsclib_system_allocator = &default_allocator;

/*------------------------------------------------------------------*
 * Beneath here lies the meat and potatoes of the resource system.  *
 *                                                                  *
 * This cannot be in resource.c because it needs access to the same *
 * allocation-logic that I _really_ don't want to expose elsewhere. *
 *------------------------------------------------------------------*/

static ResourceHeader *vsc_allocator_mem2rhdr(void *p)
{
    ResourceHeader *hdr = vsc_allocator_mem2hdr<ResourceHeader>(p);
    vsc_assert(hdr->base.is_resource);
    return hdr;
}

static void free_internal(ResourceHeader *hdr)
{
    vsc_assert(hdr->base.is_resource);

    /*
     * Prevent a cycle.
     */
    if(hdr->base.in_free)
        return;

    hdr->base.in_free = 1;

    /*
     * Invoke the destructor, if any.
     */
    if(hdr->destructor != nullptr) {
        hdr->destructor(vsc__allocator_hdr2mem(hdr), hdr->base.size, 1u << hdr->base.align_power);
    }

    /*
     * Murder our children.
     */
    while(hdr->children.head != nullptr) {
        free_internal(resnode_remove(resnode_head(&hdr->children)));
    }

    /*
     * Emancipate ourselves.
     */
    if(resnode_inserted(hdr))
        resnode_remove(hdr);

    /*
     * Fill the block with something noticable just in case.
     * Specifically also wipes the signature.
     */
    memset(hdr, 0xDD, sizeof(ResourceHeader));

    /*
     * Finally, we're done.
     */
    vsc_sys_free(hdr);
}

static void resource_free(void *p, void *user)
{
    (void)user;
    if(p == nullptr)
        return;

    free_internal(vsc_allocator_mem2hdr<ResourceHeader>(p));
}

static int resource_alloc(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user)
{
    int             r;
    ResourceHeader *hdr;
    ResourceHeader *parent = static_cast<ResourceHeader *>(user);

    if((r = malloc_<ResourceHeader>(ptr, size, alignment, flags, user)) < 0)
        return r;

    hdr                   = vsc_allocator_mem2hdr<ResourceHeader>(*ptr);
    hdr->base.is_resource = 1;

    if(parent != nullptr) {
        resnode_add_head(&parent->children, hdr);
    }

    return 0;
}

extern "C" VscAllocator vsc_res_allocator(void *res)
{
    ResourceHeader *hdr = res ? vsc_allocator_mem2rhdr(res) : nullptr;
    return {
        /* .alloc     = */ resource_alloc,
        /* .free      = */ resource_free,
        /* .size      = */ size_<ResourceHeader>,
        /* .alignment = */ VSC_ALIGNOF(vsc_max_align_t),
        /* .user      = */ hdr,
    };
}

extern "C" void *vsc_res_allocate(void *parent, size_t size, size_t alignment)
{
    void              *p = nullptr;
    const VscAllocator a = vsc_res_allocator(parent);

    if(vsc_xalloc_ex(&a, &p, size, VSC_ALLOC_ZERO, alignment) < 0)
        return nullptr;

    return p;
}

extern "C" void vsc_res_free(void *res)
{
    resource_free(res, nullptr);
}

extern "C" void *vsc_res_add(void *parent, void *res)
{
    ResourceHeader *hdr  = vsc_allocator_mem2rhdr(res);
    ResourceHeader *phdr = vsc_allocator_mem2rhdr(parent);

    /*
     * Remove from any parent list
     */
    if(resnode_inserted(hdr)) {
        resnode_remove(hdr);
    }

    resnode_add_head(&phdr->children, hdr);

    return res;
}

extern "C" void *vsc_res_remove(void *res)
{
    resnode_remove(vsc_allocator_mem2rhdr(res));
    return res;
}

extern "C" size_t vsc_res_size(void *res)
{
    return vsc_allocator_mem2rhdr(res)->base.size;
}

extern "C" int vsc_res_enum_children(void *res, VscResourceEnumProc proc, void *user)
{
    int             r;
    ResourceHeader *hdr;

    vsc_assert(proc != nullptr);

    hdr = vsc_allocator_mem2rhdr(res);

    for(ResourceHeader *ptr = hdr->children.head; ptr; ptr = ptr->next) {
        if((r = proc(vsc__allocator_hdr2mem<ResourceHeader>(ptr), user)) != 0)
            return r;
    }

    return 0;
}

extern "C" void vsc_res_set_destructor(void *res, VscResourceDestructorProc proc)
{
    vsc_allocator_mem2rhdr(res)->destructor = proc;
}

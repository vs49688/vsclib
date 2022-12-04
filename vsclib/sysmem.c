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
#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#elif !defined(__MACH__)
#   include <malloc.h>
#endif
#include <vsclib/assert.h>
#include <vsclib/mem.h>

void *vsc_sys_malloc(size_t size)
{
#if defined(_WIN32)
    HANDLE hHeap;

    if((hHeap = GetProcessHeap()) == NULL)
        return NULL;

    return HeapAlloc(hHeap, 0, size);
#else
    return malloc(size);
#endif
}

void *vsc_sys_calloc(size_t nmemb, size_t size)
{
#if defined(_WIN32)
    HANDLE hHeap;

    if(size > 0 && nmemb > SIZE_MAX / size)
        return NULL;

    if((hHeap = GetProcessHeap()) == NULL)
        return NULL;

    return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size * nmemb);
#else
    return calloc(nmemb, size);
#endif
}

void vsc_sys_free(void *p)
{
    if(p == NULL)
        return;

#if defined(_WIN32)
    HeapFree(GetProcessHeap(), 0, p);
#else
    free(p);
#endif
}

void *vsc_sys_realloc(void *ptr, size_t size)
{
#if defined(_WIN32)
    HANDLE hHeap;

    if((hHeap = GetProcessHeap()) == NULL)
        return NULL;

    if(ptr == NULL)
        return HeapAlloc(hHeap, 0, size);

    return HeapReAlloc(hHeap, 0, ptr, size);
#else
    return realloc(ptr, size);
#endif
}

void *vsc_sys_aligned_malloc(size_t size, size_t alignment)
{
#if defined(_WIN32)
    vsc_assert(VSC_IS_POT(alignment)); /* Or error with EINVAL? */
    return _aligned_malloc(size, alignment);
#else
    void *p;
    /*
     * Prefer aligned_alloc(), but this fails on macOS if the
     * alignment is "not supported by the implementation".
     * posix_memalign() will work provided alignment is a POT
     * and a multiple of sizeof(void*).
     */
    if((p = aligned_alloc(alignment, size)) != NULL)
        return p;

    vsc_assert(VSC_IS_POT(alignment)); /* Or error with EINVAL? */
    if(alignment < sizeof(void*))
        alignment = sizeof(void*);

    if(posix_memalign(&p, alignment, size) != 0)
        return NULL;

    return p;
#endif
}

void vsc_sys_aligned_free(void *ptr)
{
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

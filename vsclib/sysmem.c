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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif !defined(__MACH__)
#include <malloc.h>
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

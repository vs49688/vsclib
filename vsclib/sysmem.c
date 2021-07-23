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
#include <malloc.h>
#include <vsclib/assert.h>
#include <vsclib/mem.h>

void *vsc_sys_malloc(size_t size)
{
    return malloc(size);
}

void *vsc_sys_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void vsc_sys_free(void *p)
{
    free(p);
}

void *vsc_sys_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void *vsc_sys_aligned_malloc(size_t size, size_t alignment)
{
#if defined(_WIN32)
    vsc_assert(VSC_IS_POT(alignment)); /* Or error with EINVAL? */
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
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

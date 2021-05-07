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

static inline int vsc_is_pot(unsigned int alignment)
{
    return VSC_IS_POT(alignment);
}

static inline int vsc_is_aligned(void *p, size_t alignment)
{
    return VSC_IS_ALIGNED(p, alignment);
}

void *vsc_malloc(size_t size);
void  vsc_free(void *p);
void *vsc_realloc(void *ptr, size_t size);

void *vsc_aligned_malloc(size_t size, size_t alignment);
void  vsc_aligned_free(void *ptr);

void *vsc_xalloc(const VscAllocator *a, size_t size);
void  vsc_xfree(const VscAllocator *a, void *p);
void *vsc_xrealloc(const VscAllocator *a, void *ptr, size_t size);

#endif /* _VSCLIB_MEM_H */
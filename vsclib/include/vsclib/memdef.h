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

#include <stdint.h>

/* FIXME: Find a better place for these. */
#define VSC_IS_POT(a) ((a) != 0 && ((a) & ((a) - 1)) == 0)
#define VSC_IS_ALIGNED(p, a) (((uintptr_t)(p) & ((a) - 1)) == 0)

typedef enum VscAllocFlags {
    VSC_ALLOC_ZERO   = 1 << 0,
    /* VSC_ALLOC_NOFAIL = 1 << 1, */
} VscAllocFlags;

typedef struct
{
    void *(*alloc)(size_t size, VscAllocFlags flags, void *user);
    void  (*free)(void *p, void *user);
    void *(*realloc)(void *ptr, size_t size, void *user);

    void *user;
} VscAllocator;

#endif /* _VSCLIB_MEMDEF_H */
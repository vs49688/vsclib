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
#ifndef _VSCLIB_ALLOCATOR_INTERNAL_H
#define _VSCLIB_ALLOCATOR_INTERNAL_H

#include <stdint.h>
#include <stddef.h>

#define VSC__MEMHDR_SIG ((uintptr_t)0xFEED5EEDFEED5EEDu) /* Formerly Chuck's */

typedef struct MemHeader {
    size_t size;
    union {
        struct {
            size_t align_power : 8;
            size_t reserved : VSC_SIZE_T_BITSIZE - 8;
        };
        size_t _pad;
    };
    uintptr_t sig;
} MemHeader;

MemHeader *vsc__allocator_mem2hdr(void *p);
void      *vsc__allocator_hdr2mem(MemHeader *hdr, size_t alignment);

#endif /* _VSCLIB_ALLOCATOR_INTERNAL_H */

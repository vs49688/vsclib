/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2022 Zane van Iperen (zane@zanevaniperen.com)
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
#ifndef _VSCLIB_MINGW_ALIGNED_MALLOC_H
#define _VSCLIB_MINGW_ALIGNED_MALLOC_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

void *vsci_aligned_offset_malloc(size_t size, size_t alignment, size_t offset);

void *vsci_aligned_malloc(size_t size, size_t alignment);

void vsci_aligned_free(void *memblock);

void *vsci_aligned_offset_realloc(void *memblock, size_t size, size_t alignment, size_t offset);

void *vsci_aligned_realloc(void *memblock, size_t size, size_t alignment);


#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_MINGW_ALIGNED_MALLOC_H */

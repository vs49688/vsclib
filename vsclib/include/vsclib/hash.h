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
#ifndef _VSCLIB_HASH_H
#define _VSCLIB_HASH_H

#include <stddef.h>
#include <stdint.h>
#include "hashdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

vsc_hash_t vsc_hash(const void *data, size_t size);
vsc_hash_t vsc_hash_string(const char *s);

uint32_t vsc_crc32(const void *data, size_t size);
uint32_t vsc_crc32c(const void *data, size_t size);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_HASH_H */

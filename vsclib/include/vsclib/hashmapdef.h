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
#ifndef _VSCLIB_HASHMAPDEF_H
#define _VSCLIB_HASHMAPDEF_H

#include <stdint.h>
#include "mem.h"
#include "hashdef.h"


typedef enum VscHashMapResizePolicy
{
    VSC_HASHMAP_RESIZE_LOAD_FACTOR  = 0,
    VSC_HASHMAP_RESIZE_NONE         = 1,
} VscHashMapResizePolicy;

typedef vsc_hash_t(*VscHashmapHashProc)(const void *key);
typedef int       (*VscHashMapCompareProc)(const void *a, const void *b);

typedef struct VscHashMapBucket
{
    /**
     * @brief Item hash. Is VSC_INVALID_HASH if empty.
     */
    vsc_hash_t  hash;
    const void  *key;
    void        *value;
} VscHashMapBucket;

typedef struct VscHashMap
{
    size_t                          size;
    size_t                          num_buckets;
    VscHashMapBucket                *buckets;
    VscHashMapResizePolicy          resize_policy;
    struct { uint16_t num, den; }   load_min;
    struct { uint16_t num, den; }   load_max;
    VscHashmapHashProc              hash_proc;
    VscHashMapCompareProc           compare_proc;
    const VscAllocator              *allocator;
} VscHashMap;

#endif /* _VSCLIB_HASHMAPDEF_H */

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
#ifndef _VSCLIB_HASHMAP_H
#define _VSCLIB_HASHMAP_H

#include <stddef.h>
#include "hashmapdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Manually hash a key.
 *
 * @param   hm The hash map instance. May not be NULL.
 * @param   key A pointer to the key.
 * @return  The hash of the provided key. This will never be #VSC_INVALID_HASH.
 */
vsc_hash_t vsc_hashmap_hash(const VscHashMap *hm, const void *key);

/**
 * @brief Compare two keys for equality.
 *
 * @param   hm The hash map instance. May not be NULL.
 * @param   a A pointer to the first key.
 * @param   b A pointer to the second key.
 * @return  If the keys are equal, returns 1, or 0 if they're not.
 */
int vsc_hashmap_compare(const VscHashMap *hm, const void *a, const void *b);

VscHashMap *vsc_hashmap_alloca(VscHashMapHashProc hash, VscHashMapCompareProc compare, const VscAllocator *a);
VscHashMap *vsc_hashmap_alloc(VscHashMapHashProc hash, VscHashMapCompareProc compare);
void        vsc_hashmap_free(VscHashMap *hm);

int vsc_hashmap_clear(VscHashMap *hm);

/**
 * @brief Configure the minimum and maximum load factors of the hash map.
 *
 * @param   hm      The hash map instance.
 * @param   min_num The numerator of the minimum load factor.
 * @param   min_den The denominator of the minimum load factor.
 * @param   max_num The numerator of the maximum load factor.
 * @param   max_den The denominator of the maximum load factor.
 * @return  On success, returns 0. If the function fails, it returns a negative error value.
 *          The function can fail under the following conditions:
 *          - `hm` is NULL.
 *          - One or both of the numerators or denominators are 0.
 *          - One of both of the numerators are greater than or equal to its
 *            respective denominator.
 *          - The minimum load factor is greater than the maximum load factor.
 */
int vsc_hashmap_configure(VscHashMap *hm, uint16_t min_num, uint16_t min_den, uint16_t max_num, uint16_t max_den);

/**
 * @brief Reset a hash map to its default state, releasing all memory.
 *
 * It may be used as if `vsc_hashmap_alloc()` has just been called.
 *
 * @param hm The hash map instance. Must not be NULL.
 */
void vsc_hashmap_reset(VscHashMap *hm);

/**
 * @brief Resize a hash map so it can hold `nelem` elements.
 *
 * @param   hm The hash map instance. Must not be NULL.
 * @param   nelem The number of elements this map must be able to hold.
 *          This cannot be less than the current number of elements in the map.
 * @return
 *
 * @remark  This is *not* affected by the current resize policy.
 */
int    vsc_hashmap_resize(VscHashMap *hm, size_t nelem);
int    vsc_hashmap_insert(VscHashMap *hm, const void *key, void *value);
void  *vsc_hashmap_find_by_hash(const VscHashMap *hm, vsc_hash_t hash);
void  *vsc_hashmap_find(const VscHashMap *hm, const void *key);

/**
 * @brief Update an existing value in the hash map.
 *
 * Unlike vsc_hashmao_inesrt(), this is guaranteed to _only_ update an existing value,
 * i.e. it won't attempt a rebalance.
 *
 * @remark This is safe to use during the vsc_hashmap_enumerate() callback.
 *
 * @param hm    The hash map instance. Must not be NULL.
 * @param key   The key to update.
 * @param value The new value.
 *
 * @return If the key exists, the value is set to \p value, returns 1.
 *         If the key doesn't exist, returns 0.
 */
int    vsc_hashmap_update(const VscHashMap *hm, const void *key, void *value);
void  *vsc_hashmap_remove(VscHashMap *hm, const void *key);
size_t vsc_hashmap_size(const VscHashMap *hm);
size_t vsc_hashmap_capacity(const VscHashMap *hm);

VscHashMapResizePolicy vsc_hashmap_resize_policy(const VscHashMap *hm);
VscHashMapResizePolicy vsc_hashmap_set_resize_policy(VscHashMap *hm, VscHashMapResizePolicy policy);

/**
 * @brief Get a pointer to the first bucket. This may be empty.
 *
 * @param hm The hash map instance. Must not be NULL.
 *
 * @return If map is empty, i.e. if vsc_hashmap_capacity() returns 0, this will return NULL.
 *         Otherwise, returns a pointer to the first (possibly empty), bucket.
 */
const VscHashMapBucket *vsc_hashmap_first(const VscHashMap *hm);

int vsc_hashmap_enumerate(const VscHashMap *hm, VscHashMapEnumProc proc, void *user);

vsc_hash_t vsc_hashmap_default_hash(const void *k);
int        vsc_hashmap_default_compare(const void *a, const void *b);

vsc_hash_t vsc_hashmap_string_hash(const void *s);
int        vsc_hashmap_string_compare(const void *a, const void *b);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_HASHMAP_H */

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
#include <string.h>
#include <vsclib/assert.h>
#include <vsclib/error.h>
#include <vsclib/hash.h>
#include <vsclib/hashmap.h>

#define VSC_HASHMAP_MIN_BUCKET_AUTO_ALLOCATION 16

struct VscHashMap {
    size_t                 size;
    size_t                 num_buckets;
    VscHashMapBucket      *buckets;
    VscHashMapResizePolicy resize_policy;
    struct {
        uint16_t num, den;
    } load_min;
    struct {
        uint16_t num, den;
    } load_max;
    VscHashMapHashProc    hash_proc;
    VscHashMapCompareProc compare_proc;
    const VscAllocator   *allocator;
};

/* This should be optimised out in Release builds. */
static inline void validate(const VscHashMap *hm)
{
    vsc_assert(hm != NULL);
    vsc_assert(hm->size <= hm->num_buckets);
    vsc_assert(hm->hash_proc != NULL);
    vsc_assert(hm->compare_proc != NULL);
    vsc_assert(hm->allocator != NULL);
    vsc_assert(hm->load_min.den > 0);
    vsc_assert(hm->load_min.num < hm->load_min.den);
    vsc_assert(hm->load_max.den > 0);
    vsc_assert(hm->load_max.num < hm->load_max.den);
    vsc_assert(hm->load_min.num * hm->load_max.den <= hm->load_max.num * hm->load_max.den);
}

static inline VscHashMapBucket *reset_bucket(VscHashMapBucket *bkt)
{
    bkt->hash  = VSC_INVALID_HASH;
    bkt->key   = NULL;
    bkt->value = NULL;
    return bkt;
}

vsc_hash_t vsc_hashmap_hash(const VscHashMap *hm, const void *key)
{
    vsc_hash_t hash = hm->hash_proc(key);
    vsc_assert(hash != VSC_INVALID_HASH);
    return hash;
}

int vsc_hashmap_compare(const VscHashMap *hm, const void *a, const void *b)
{
    return hm->compare_proc(a, b) != 0;
}

VscHashMap *vsc_hashmap_alloca(VscHashMapHashProc hash, VscHashMapCompareProc compare, const VscAllocator *a)
{
    VscHashMap *hm;

    vsc_assert(hash != NULL);
    vsc_assert(compare != NULL);
    vsc_assert(a != NULL);

    if((hm = vsc_xalloc(a, sizeof(VscHashMap))) == NULL)
        return NULL;

    *hm = (VscHashMap){
        .size          = 0,
        .num_buckets   = 0,
        .buckets       = NULL,
        .resize_policy = VSC_HASHMAP_RESIZE_LOAD_FACTOR,
        .load_min.num  = 1,
        .load_min.den  = 2,
        .load_max.num  = 3,
        .load_max.den  = 4,
        .hash_proc     = hash,
        .compare_proc  = compare,
        .allocator     = a,
    };

    return hm;
}

VscHashMap *vsc_hashmap_alloc(VscHashMapHashProc hash, VscHashMapCompareProc compare)
{
    return vsc_hashmap_alloca(hash, compare, vsclib_system_allocator);
}

void vsc_hashmap_free(VscHashMap *hm)
{
    validate(hm);

    vsc_hashmap_reset(hm);
    vsc_xfree(hm->allocator, hm);
}

int vsc_hashmap_clear(VscHashMap *hm)
{
    validate(hm);
    hm->size = 0;
    for(size_t i = 0; i < hm->num_buckets; ++i)
        reset_bucket(hm->buckets + i);

    return 0;
}

int vsc_hashmap_configure(VscHashMap *hm, uint16_t min_num, uint16_t min_den, uint16_t max_num, uint16_t max_den)
{
    uint32_t numa, numb;

    if(min_num == 0 || min_den == 0 || min_num >= min_den)
        return VSC_ERROR(EINVAL);

    if(max_num == 0 || max_den == 0 || max_num >= max_den)
        return VSC_ERROR(EINVAL);

    validate(hm);

    /*
     * Convert the fractions to the same base so we can
     * compare their numerators.
     *
     * a     c   a * d     c * b
     * - and - = ----- and -----
     * b     d   b * d     d * b
     */
    numa = min_num * max_den;
    numb = max_num * min_den;

    if(numa > numb)
        return VSC_ERROR(EINVAL);

    hm->load_min.num = min_num;
    hm->load_min.den = min_den;
    hm->load_max.num = max_num;
    hm->load_max.den = max_den;
    return 0;
}

void vsc_hashmap_reset(VscHashMap *hm)
{
    validate(hm);

    vsc_xfree(hm->allocator, hm->buckets);
    hm->num_buckets = 0;
    hm->buckets     = NULL;
}

/*
 * Circularly loop over each of the buckets, starting at index `start`.
 */
#define LOOP_BUCKETS(hm, idxname, start)                                                \
    for(size_t _i = 0, (idxname) = (start) % (hm)->num_buckets; _i < (hm)->num_buckets; \
        ++_i, (idxname)          = ((idxname) + 1) % (hm)->num_buckets)

static VscHashMapBucket *add_or_replace_bucket(const VscHashMap *hm, VscHashMapBucket *buckets, size_t n,
                                               const VscHashMapBucket *bkt, int *added)
{
    if(n == 0)
        return NULL;

    LOOP_BUCKETS(hm, index, bkt->hash % n)
    {
        VscHashMapBucket *b = buckets + index;

        /* Shortcut: use first empty bucket. */
        if(b->hash == VSC_INVALID_HASH) {
            *added = 1;
            *b     = *bkt;
            return b;
        }

        if(b->hash != bkt->hash)
            continue;

        /* Duplicate key, replace it. */
        if(vsc_hashmap_compare(hm, bkt->key, b->key)) {
            *added = 0;
            *b     = *bkt;
            return b;
        }
    }

    *added = 0;
    return NULL;
}

int vsc_hashmap_resize(VscHashMap *hm, size_t nelem)
{
    VscHashMapBucket *bkts, *tmpbkts;

    validate(hm);

    if(nelem == 0 || nelem < hm->size)
        return VSC_ERROR(EINVAL);

    /* Nothing to do. */
    if(nelem == hm->size)
        return 0;

    /* Make sure the realloc() size won't overflow. */
    if(nelem >= (SIZE_MAX / sizeof(VscHashMapBucket)))
        return VSC_ERROR(ERANGE);

    /* Reallocate the buckets. */
    bkts = vsc_xrealloc(hm->allocator, hm->buckets, sizeof(VscHashMapBucket) * nelem);
    if(bkts == NULL)
        return VSC_ERROR(ENOMEM);

    hm->buckets = bkts;

    for(size_t i = hm->num_buckets; i < nelem; ++i)
        reset_bucket(hm->buckets + i);

    /* Shortcut - no items. no problem! */
    if(hm->size == 0) {
        hm->num_buckets = nelem;
        return 0;
    }

    /*
     * Allocate a temp bucket list (hue) to work with. Note that this
     * needs to be done after the hm->buckets alloc to play nice to linear
     * allocators.
     */
    tmpbkts = vsc_xcalloc(hm->allocator, nelem, sizeof(VscHashMapBucket));
    if(tmpbkts == NULL) {

        /*
         * This is a bit of a sticky situation - allocation has failed,
         * and now we have more buckets that we can't redistribute in to.
         * This also means it's not safe to rely on another realloc to shrink
         * it again.
         *
         * Our only safe option is to waste the new memory :(
         */
        return VSC_ERROR(ENOMEM);
    }

    hm->num_buckets = nelem;

    for(size_t i = 0; i < nelem; ++i)
        reset_bucket(tmpbkts + i);

    /* Now redistribute everything. */
    for(size_t i = 0; i < nelem; ++i) {
        int               added;
        VscHashMapBucket *bkt = hm->buckets + i;
        if(bkt->hash == VSC_INVALID_HASH)
            continue;

        added = 0;
        bkt   = add_or_replace_bucket(hm, tmpbkts, nelem, hm->buckets + i, &added);
        vsc_assert(bkt != NULL);
    }

    memcpy(hm->buckets, tmpbkts, sizeof(VscHashMapBucket) * nelem);
    vsc_xfree(hm->allocator, tmpbkts);

    // fprintf(stderr, "Resizing to %zu\n", hm->num_buckets);
    return 0;
}

static inline int intceil(size_t *result, size_t num, size_t den)
{
#if 1
    *result = (num / den) + (size_t)((num % den) != 0);
    return 0;
#else
    if(num >= SIZE_MAX - (den - 1))
        return VSC_ERROR(ERANGE);

    *result = (num + den - 1) / den;
    return 0;
#endif
}

static int maybe_resize(VscHashMap *hm)
{
    size_t thresh, minreq, tmp;
    int    r;

    /*
     * Make sure the threshold calculation doesn't overflow.
     *
     * If this is actively limiting your required size then either:
     * - resize it manually using vsc_hashmap_resize(), or
     * - use a load factor where the numerator is 1.
     */
    if((r = intceil(&tmp, SIZE_MAX, hm->load_max.num)) < 0)
        return r;

    if(hm->num_buckets >= tmp)
        return VSC_ERROR(ERANGE); /* or EOVERFLOW? */

    /* Same as "n * load_factor" */
    if((r = intceil(&thresh, hm->num_buckets * hm->load_max.num, hm->load_max.den)) < 0)
        return r;

    /* Nothing to do */
    if(hm->size < thresh)
        return 0;

    /* Don't allow resize if explicitly disabled. */
    if(hm->resize_policy == VSC_HASHMAP_RESIZE_NONE)
        return VSC_ERROR(EPERM);

    if((r = intceil(&tmp, SIZE_MAX, hm->load_max.den)) < 0)
        return r;

    if(hm->size + 1 >= tmp)
        return VSC_ERROR(ERANGE); /* or EOVERFLOW? */

    /*
     * Determine the minimum number of buckets required to match
     * the target load factor, then double it.
     */
    if((r = intceil(&minreq, (hm->size + 1) * hm->load_min.den, hm->load_min.num)) < 0)
        return r;

    return vsc_hashmap_resize(hm, VSC_MAX(minreq, VSC_HASHMAP_MIN_BUCKET_AUTO_ALLOCATION));
}

int vsc_hashmap_insert(VscHashMap *hm, const void *key, void *value)
{
    VscHashMapBucket tmpbkt;
    int              r;
    int              added;

    validate(hm);

    tmpbkt.hash  = vsc_hashmap_hash(hm, key);
    tmpbkt.key   = key;
    tmpbkt.value = value;

    if(tmpbkt.hash == VSC_INVALID_HASH)
        return VSC_ERROR(ERANGE);

    /* See if we need to resize. */
    if((r = maybe_resize(hm)) < 0) {
        /*
         * Resize required, but not allowed.
         * Not an error, assume the user knows what they're doing.
         */
        if(r != VSC_ERROR(EPERM))
            return r;
    }

    added = 0;
    if(add_or_replace_bucket(hm, hm->buckets, hm->num_buckets, &tmpbkt, &added) == NULL)
        return VSC_ERROR(ENOSPC);

    if(added) {
        ++hm->size;
    }

    return 0;
}

void *vsc_hashmap_find_by_hash(const VscHashMap *hm, vsc_hash_t hash)
{
    if(hash == VSC_INVALID_HASH)
        return NULL;

    validate(hm);

    if(hm->num_buckets == 0)
        return NULL;

    LOOP_BUCKETS(hm, index, hash % hm->num_buckets)
    {
        const VscHashMapBucket *bkt = hm->buckets + index;

        if(bkt->hash == VSC_INVALID_HASH)
            break;

        if(bkt->hash == hash)
            return bkt->value;
    }

    return NULL;
}

static const VscHashMapBucket *find_bucket(const VscHashMap *hm, const void *key, size_t *outindex)
{
    vsc_hash_t hash;

    validate(hm);

    hash = vsc_hashmap_hash(hm, key);

    if(hm->num_buckets == 0)
        return NULL;

    /*
     * Search for the key starting at index until we:
     * 1. find it,
     * 2. hit an empty bucket (not found),
     * 3. do a complete loop  (not found)
     *    - This is O(n), but should almost never happen if your
     *      hash function is good enough and there's enough buckets.
     */
    LOOP_BUCKETS(hm, index, hash % hm->num_buckets)
    {
        const VscHashMapBucket *bkt = hm->buckets + index;

        /* Stop at first empty bucket, item isn't here. */
        if(bkt->hash == VSC_INVALID_HASH)
            break;

        if(bkt->hash != hash)
            continue;

        /* In case of hash collision. */
        if(!vsc_hashmap_compare(hm, key, bkt->key))
            continue;

        if(outindex != NULL)
            *outindex = index;
        return bkt;
    }

    return NULL;
}

void *vsc_hashmap_find(const VscHashMap *hm, const void *key)
{
    const VscHashMapBucket *bkt;

    if((bkt = find_bucket(hm, key, NULL)) == NULL)
        return NULL;

    return bkt->value;
}

int vsc_hashmap_update(const VscHashMap *hm, const void *key, void *value)
{
    VscHashMapBucket *bkt;

    /* NB: Safe const-away cast. We're not changing the map, we're changing the bucket. */
    if((bkt = (VscHashMapBucket *)find_bucket(hm, key, NULL)) == NULL)
        return 1;

    bkt->value = value;
    return 0;
}

void *vsc_hashmap_remove(VscHashMap *hm, const void *key)
{
    VscHashMapBucket *bkt;
    size_t            index;
    void             *val;

    bkt = (VscHashMapBucket *)find_bucket(hm, key, &index);
    if(bkt == NULL)
        return NULL;

    val = bkt->value;
    reset_bucket(bkt);

    /* Search through the remaining buckets, to see if we can fill the gap. */
    LOOP_BUCKETS(hm, cidx, index + 1)
    {
        size_t            nidx;
        VscHashMapBucket *bkt2 = hm->buckets + cidx;

        /* Have an empty bucket, we're done here! */
        if(bkt2->hash == VSC_INVALID_HASH)
            break;

        /* If our hash can be moved back, do it. */
        nidx = bkt2->hash % hm->num_buckets;
        if(nidx <= index) {
            *bkt = *bkt2;
            reset_bucket(bkt2);
            index = cidx;
            bkt   = bkt2;
        }
    }

    --hm->size;
    return val;
}

size_t vsc_hashmap_size(const VscHashMap *hm)
{
    validate(hm);
    return hm->size;
}

size_t vsc_hashmap_capacity(const VscHashMap *hm)
{
    validate(hm);
    return hm->num_buckets;
}

VscHashMapResizePolicy vsc_hashmap_resize_policy(const VscHashMap *hm)
{
    validate(hm);
    return hm->resize_policy;
}

VscHashMapResizePolicy vsc_hashmap_set_resize_policy(VscHashMap *hm, VscHashMapResizePolicy policy)
{
    VscHashMapResizePolicy old;
    validate(hm);

    old               = hm->resize_policy;
    hm->resize_policy = policy;
    return old;
}

const VscHashMapBucket *vsc_hashmap_first(const VscHashMap *hm)
{
    validate(hm);

    /* Shortcut out. */
    if(hm->size == 0)
        return NULL;

    return hm->buckets;
}

int vsc_hashmap_enumerate(const VscHashMap *hm, VscHashMapEnumProc proc, void *user)
{
    int r;

    validate(hm);

    for(size_t i = 0; i < hm->num_buckets; ++i) {
        const VscHashMapBucket *bkt = hm->buckets + i;

        if(bkt->hash == VSC_INVALID_HASH)
            continue;

        if((r = proc(bkt->key, bkt->value, bkt->hash, user)) != 0)
            return r;
    }

    return 0;
}

vsc_hash_t vsc_hashmap_default_hash(const void *k)
{
    return (vsc_hash_t)k;
}

int vsc_hashmap_default_compare(const void *a, const void *b)
{
    return a == b;
}

vsc_hash_t vsc_hashmap_string_hash(const void *s)
{
    return vsc_hash_string(s);
}

int vsc_hashmap_string_compare(const void *a, const void *b)
{
    if(a == b)
        return 0;

    if(a == NULL || b == NULL)
        return 0;

    return strcmp(a, b) == 0;
}

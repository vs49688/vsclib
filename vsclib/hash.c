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
#include <limits.h>
#include <errno.h>
#include <vsclib/hash.h>

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash.h"

vsc_hash_t vsc_hash(const void *data, size_t size)
{
    if(data == NULL && size != 0) {
        errno = EINVAL;
        return VSC_INVALID_HASH;
    }

    /*
     * Use whichever one matches the machine's native word type.
     * See https://stackoverflow.com/a/67223865
     */
#if defined(_WIN64) || LONG_MAX == 9223372036854775807L
    return XXH3_64bits_withSeed(data, size, 0);
#else /* LONG_MAX == 2147483647L */
    return XXH32(data, size, 0);
#endif
}

/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Zane van Iperen (zane@zanevaniperen.com)
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
#include <vsclib/mem.h>

#if VSC_HAVE_INTRIN_H
#include <intrin.h>
#endif

#if VSC_HAVE_BITSCANFORWARD
#pragma intrinsic(_BitScanForward)
#endif

#if VSC_HAVE_BITSCANFORWARD64
#pragma intrinsic(_BitScanForward64)
#endif

uint8_t vsc_ctz(size_t x)
{
    vsc_assert(VSC_IS_POT(x));
    vsc_assert(x != 0);

#if defined(__GNUC__) && VSC_SIZEOF_SIZE_T <= VSC_SIZEOF_INT
    return __builtin_ctz((unsigned int)x);
#elif defined(__GNUC__) && VSC_SIZEOF_SIZE_T <= VSC_SIZEOF_LONG
    return __builtin_ctzl((unsigned long)x);
#elif defined(__GNUC__) && VSC_SIZEOF_SIZE_T <= VSC_SIZEOF_LONG_LONG
    return __builtin_ctzll((unsigned long long)x);
#elif defined(_MSC_VER) && VSC_HAVE_BITSCANFORWARD && VSC_SIZEOF_SIZE_T <= VSC_SIZEOF_LONG
    unsigned long l;
    (void)_BitScanForward(&l, (unsigned long)x);
    return (uint8_t)l;
#elif defined(_MSC_VER) && VSC_HAVE_BITSCANFORWARD64 && VSC_SIZEOF_SIZE_T <= VSC_SIZEOF_INT64_T
    unsigned long l;
    (void)_BitScanForward64(&l, (unsigned __int64)x);
    return (uint8_t)l;
#else
    uint8_t pwr = 0;
    while(x > 1) {
        ++pwr;
        x >>= 1;
    }
    return pwr;
#endif
}

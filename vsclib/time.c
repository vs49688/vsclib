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

#include <time.h>
#include <stdlib.h>
#include <vsclib/time.h>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#elif defined(_POSIX_C_SOURCE)
#   include <unistd.h>
#   include <sys/time.h>
#endif

vsc_counter_t vsc_counter_ns(void)
{
#if defined(_WIN32)
    static LARGE_INTEGER frequency = {0};

    LARGE_INTEGER ticks;

    /*
     * There's a *minor* race in which this may be executed multiple
     * times if called from multiple threads. But it doesn't matter as
     * the values will be the same.
     */
    if(frequency.QuadPart == 0) {
        /* Will never fail on XP+ */
        if(QueryPerformanceFrequency(&frequency) == 0)
            abort();
    }

    if(QueryPerformanceCounter(&ticks) == 0)
        abort();

    return ((vsc_counter_t)ticks.QuadPart * 1000000000) / frequency.QuadPart;
#elif _POSIX_C_SOURCE >= 199309L && defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0
    struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };

#if defined(__linux__) && defined(CLOCK_MONOTONIC_RAW)
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0)
        goto done;
#endif
    if(clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        goto done;

    /* Please fix your broken system... */
    abort();
done:
    return (vsc_counter_t)ts.tv_sec * 1000000000 + (vsc_counter_t)ts.tv_nsec;
#else
#error "Don't know how to get monotonic system counter."
#endif
}

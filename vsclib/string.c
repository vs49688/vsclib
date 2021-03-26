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
#include <assert.h>
#include "vsclib_i.h"

char *vsc_strdupra(const char *start, const char *end, const VscAllocator *a)
{
    char *buf;
    size_t len;
    assert(start != NULL && end != NULL && end >= start && a != NULL);

    len = (size_t)(end - start);
    if((buf = vsci_xalloc(a, (len + 1) * sizeof(char))) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    memcpy(buf, start, len);
    buf[len] = '\0';
    return buf;
}


char *vsc_strdupr(const char *start, const char *end)
{
    return vsc_strdupra(start, end, &vsclib_system_allocator);
}

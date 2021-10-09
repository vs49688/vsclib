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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <limits.h>

#include <vsclib/assert.h>
#include <vsclib/mem.h>

char *vsc_wstrtocstra(const wchar_t *ws, size_t *len, unsigned int cp, const VscAllocator *a)
{
    int x;
    char *s;
    size_t _len = wcslen(ws) + 1;

    if(_len >= INT_MAX)
        return errno = EOVERFLOW, NULL;

    if((x = WideCharToMultiByte(cp, 0, ws, (int)_len, NULL, 0, NULL, NULL)) == 0)
        return errno = EINVAL, NULL;
    vsc_assert(x > 0);

    if((s = vsc_xalloc(a, x * sizeof(char))) == NULL)
        return errno = ENOMEM, NULL;

    if((x = WideCharToMultiByte(cp, 0, ws, (int)_len, s, x, NULL, NULL)) == 0) {
        vsc_xfree(a, s);
        return errno = EINVAL, NULL;
    }
    vsc_assert(x > 0);

    if(len)
        *len = (size_t)x;

    return s;
}

char *vsc_wstrtocstr(const wchar_t *ws, size_t *len, unsigned int cp)
{
    return vsc_wstrtocstra(ws, len, cp, &vsclib_system_allocator);
}
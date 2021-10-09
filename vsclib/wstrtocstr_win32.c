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

#include <errno.h>
#include <limits.h>
#include <vsclib/assert.h>
#include <vsclib/error.h>
#include <vsclib/mem.h>

#include "util_win32.h"

int vsc_wstrtocstra(const wchar_t *ws, unsigned int cp, char **s, size_t *len, const VscAllocator *a)
{
    int x;
    char *_s;
    size_t _len = wcslen(ws) + 1;

    if(_len >= INT_MAX)
        return VSC_ERROR(EOVERFLOW);

    if((x = WideCharToMultiByte(cp, 0, ws, (int)_len, NULL, 0, NULL, NULL)) == 0)
        return vsci_map_win32err(GetLastError());
    vsc_assert(x > 0);

    if((_s = vsc_xalloc(a, x * sizeof(char))) == NULL)
        return VSC_ERROR(ENOMEM);

    if((x = WideCharToMultiByte(cp, 0, ws, (int)_len, _s, x, NULL, NULL)) == 0) {
        vsc_xfree(a, s);
        return vsci_map_win32err(GetLastError());
    }
    vsc_assert(x > 0);

    if(len)
        *len = (size_t)x;

    *s = _s;
    return 0;
}

int vsc_wstrtocstr(const wchar_t *ws, unsigned int cp, char **s, size_t *len)
{
    return vsc_wstrtocstra(ws, cp, s, len, &vsclib_system_allocator);
}
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

int vsc_cstrtowstra(const char *s, unsigned int cp, wchar_t **ws, size_t *len, const VscAllocator *a)
{
    int x;
    size_t _len;
    wchar_t *_ws;

    _len = strlen(s) + 1;
    if(_len >= INT_MAX)
        return VSC_ERROR(EOVERFLOW);

    /* Get the required size. */
    if((x = MultiByteToWideChar(cp, 0, s, (int)_len, NULL, 0)) == 0)
        return vsci_map_win32err(GetLastError());
    vsc_assert(x > 0);

    if((_ws = vsc_xalloc(a, x * sizeof(wchar_t))) == NULL)
        return VSC_ERROR(ENOMEM);

    /* Now convert. */
    if((x = MultiByteToWideChar(cp, 0, s, (int)_len, _ws, x)) == 0) {
        vsc_xfree(a, _ws);
        return vsci_map_win32err(GetLastError());
    }
    vsc_assert(x > 0);

    if(len)
        *len = (size_t)x;

    *ws = _ws;
    return 0;
}

int vsc_cstrtowstr(const char *s, unsigned int cp, wchar_t **ws, size_t *len)
{
    return vsc_cstrtowstra(s, cp, ws, len, &vsclib_system_allocator);
}
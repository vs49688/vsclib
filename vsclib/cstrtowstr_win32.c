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
#include <vsclib/mem.h>

wchar_t *vsc_cstrtowstra(const char *s, size_t *len, unsigned int cp, const VscAllocator *a)
{
    int x;
    size_t _len;
    wchar_t *ws;

    _len = strlen(s) + 1;
    if(_len >= INT_MAX)
        return errno = EOVERFLOW, NULL;

    /* Get the required size. */
    if((x = MultiByteToWideChar(cp, 0, s, (int)_len, NULL, 0)) == 0)
        return errno = EINVAL, NULL;
    vsc_assert(x > 0);

    if((ws = vsc_xalloc(a, x * sizeof(wchar_t))) == NULL)
        return errno = ENOMEM, NULL;

    /* Now convert. */
    if((x = MultiByteToWideChar(cp, 0, s, (int)_len, ws, x)) == 0) {
        vsc_xfree(a, ws);
        return errno = EINVAL, NULL;
    }
    vsc_assert(x > 0);

    if(len)
        *len = (size_t)x;

    return ws;
}

wchar_t *vsc_cstrtowstr(const char *s, size_t *len, unsigned int cp)
{
    return vsc_cstrtowstra(s, len, cp, &vsclib_system_allocator);
}

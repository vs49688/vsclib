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
#include <assert.h>
#include <limits.h>

#include "vsclib_i.h"

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
    assert(x > 0);

    if((ws = vsci_xalloc(a, x * sizeof(wchar_t))) == NULL)
        return errno = ENOMEM, NULL;

    /* Now convert. */
    if((x = MultiByteToWideChar(cp, 0, s, (int)_len, ws, x)) == 0) {
        vsci_xfree(a, ws);
        return errno = EINVAL, NULL;
    }
    assert(x > 0);

    if(len)
        *len = (size_t)x;

    return ws;
}

wchar_t *vsc_cstrtowstr(const char *s, size_t *len, unsigned int cp)
{
    return vsc_cstrtowstra(s, len, cp, &vsclib_system_allocator);
}

char *vsc_wstrtocstra(const wchar_t *ws, size_t *len, unsigned int cp, const VscAllocator *a)
{
    int x;
    char *s;
    size_t _len = wcslen(ws) + 1;

    if(_len >= INT_MAX)
        return errno = EOVERFLOW, NULL;

    if((x = WideCharToMultiByte(cp, 0, ws, (int)_len, NULL, 0, NULL, NULL)) == 0)
        return errno = EINVAL, NULL;
    assert(x > 0);

    if((s = vsci_xalloc(a, x * sizeof(char))) == NULL)
        return errno = ENOMEM, NULL;

    if((x = WideCharToMultiByte(cp, 0, ws, (int)_len, s, x, NULL, NULL)) == 0) {
        vsci_xfree(a, s);
        return errno = EINVAL, NULL;
    }
    assert(x > 0);

    if(len)
        *len = (size_t)x;

    return s;
}

char *vsc_wstrtocstr(const wchar_t *ws, size_t *len, unsigned int cp)
{
    return vsc_wstrtocstra(ws, len, cp, &vsclib_system_allocator);
}

char *vsc_searchpatha(const char *f, size_t *len, const VscAllocator *a)
{
    DWORD x;
    size_t _len;
    wchar_t *ws = NULL, *wf = NULL;
    char *of = NULL;

    if((wf = vsc_cstrtowstra(f, &_len, CP_UTF8, a)) == NULL)
        return NULL; /* vsc_cstrtowstra() sets errno. */

    if((x = SearchPathW(NULL, wf, L".exe", 0, NULL, NULL)) == 0)
        goto done;

    if((ws = vsci_xalloc(a, (size_t)x * sizeof(wchar_t))) == NULL)
        goto done;

    if((x = SearchPathW(NULL, wf, L".exe", x, ws, NULL)) == 0)
        goto done;

    if((of = vsc_wstrtocstra(ws, len, CP_UTF8, a)) == NULL)
        goto done;

done:
    if(x == 0)
        errno = ENOENT;

    if(ws != NULL)
        vsci_xfree(a, ws);

    if(wf != NULL)
        vsci_xfree(a, wf);

    return of;
}

char *vsc_searchpath(const char *f, size_t *len)
{
    return vsc_searchpatha(f, len, &vsclib_system_allocator);
}

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

#include <vsclib/mem.h>
#include <vsclib/string.h>

#include "util_win32.h"

char *vsc_searchpatha(const char *f, size_t *len, const VscAllocator *a)
{
    DWORD x;
    size_t _len;
    wchar_t *ws = NULL, *wf = NULL;
    char *of = NULL;

    if((wf = vsci_cstrtowstra_compat(f, &_len, CP_UTF8, a)) == NULL)
        return NULL; /* vsc_cstrtowstra() sets errno. */

    if((x = SearchPathW(NULL, wf, L".exe", 0, NULL, NULL)) == 0)
        goto done;

    if((ws = vsc_xalloc(a, (size_t)x * sizeof(wchar_t))) == NULL) {
        errno = ENOMEM;
        goto done;
    }

    if((x = SearchPathW(NULL, wf, L".exe", x, ws, NULL)) == 0)
        goto done;

    if((of = vsci_wstrtocstra_compat(ws, len, CP_UTF8, a)) == NULL)
        goto done;

done:
    if(x == 0)
        errno = ENOENT;

    if(ws != NULL)
        vsc_xfree(a, ws);

    if(wf != NULL)
        vsc_xfree(a, wf);

    return of;
}

char *vsc_searchpath(const char *f, size_t *len)
{
    return vsc_searchpatha(f, len, &vsclib_system_allocator);
}

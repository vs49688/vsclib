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
#if defined(WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#   include "util_win32.h"
#endif

#include <vsclib/error.h>
#include <vsclib/io.h>
#include <vsclib/string.h>
#include <vsclib/mem.h>

int vsc_fopena(const char *pathname, const char *mode, FILE **fp, const VscAllocator *a)
{
#if defined(_WIN32)
    wchar_t *wpath = NULL, *wmode = NULL;
    FILE *f = NULL;
    int r;

    if((r = vsc_cstrtowstra(pathname, CP_UTF8, &wpath, NULL, a)) < 0)
        return r;

    if((r = vsc_cstrtowstra(mode, CP_UTF8, &wmode, NULL, a)) < 0)
        goto done;

    if((f = _wfopen(wpath, wmode)) == NULL) {
        r = VSC_ERROR(errno);
        goto done;
    }

done:
    if(wmode != NULL)
        vsc_xfree(a, wmode);

    if(wpath != NULL)
        vsc_xfree(a, wpath);

    if(r < 0)
        return r;

    *fp = f;
    return 0;
#else
    FILE *f;
    if((f = fopen(pathname, mode)) == NULL)
        return VSC_ERROR(errno);
    *fp = f;
    return 0;
#endif
}

int vsc_fopen(const char *pathname, const char *mode, FILE **fp)
{
    return vsc_fopena(pathname, mode, fp, &vsclib_system_allocator);
}

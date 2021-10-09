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
#else
#   include <unistd.h>
#endif

#include <vsclib/error.h>
#include <vsclib/io.h>
#include <vsclib/string.h>
#include <vsclib/mem.h>

#if defined(_WIN32)
#   include "util_win32.h"
#endif

int vsc_chdir(const char *path)
{
    return vsc_chdira(path, &vsclib_system_allocator);
}

int vsc_chdira(const char *path, const VscAllocator *a)
{
#if defined(_WIN32)
    wchar_t *wpath = NULL;
    int r;

    if((wpath = vsci_cstrtowstra_compat(path, NULL, CP_UTF8, a)) == NULL)
        return VSC_ERROR(errno);

    if(!SetCurrentDirectoryW(wpath)) {
        r = vsci_map_win32err(GetLastError());
        goto done;
    }

    r = 0;
done:
    vsc_xfree(a, wpath);
    return r;
#else
    int r = chdir(path);
    if(r < 0)
        return VSC_ERROR(errno);
    return 0;
#endif
}

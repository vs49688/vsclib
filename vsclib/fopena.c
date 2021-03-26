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
#endif

#include "vsclib_i.h"

FILE *vsc_fopena(const char *pathname, const char *mode, const VscAllocator *a)
{
#if defined(_WIN32)
    wchar_t *wpath = NULL, *wmode = NULL;
    FILE *f = NULL;
    int errno_;

    if((wpath = vsc_cstrtowstra(pathname, NULL, CP_UTF8, a)) == NULL)
        goto done;

    if((wmode = vsc_cstrtowstra(mode, NULL, CP_UTF8, a)) == NULL)
        goto done;

    if((f = _wfopen(wpath, wmode)) == NULL)
        goto done;

done:
    errno_ = errno;

    if(wmode != NULL)
        vsci_xfree(a, wmode);

    if(wpath != NULL)
        vsci_xfree(a, wpath);

    errno = errno_;
    return f;
#else
    return fopen(pathname, mode);
#endif
}

FILE *vsc_fopen(const char *pathname, const char *mode)
{
    return vsc_fopena(pathname, mode, &vsclib_system_allocator);
}

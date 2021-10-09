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
#ifdef _WIN32

#include <vsclib/error.h>
#include <vsclib/string.h>
#include "util_win32.h"

int vsci_map_win32err(DWORD dwErr)
{
    switch(dwErr) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return VSC_ERROR(ENOENT);
        case ERROR_PATH_BUSY:
            return VSC_ERROR(EBUSY);
        case ERROR_ACCESS_DENIED:
            return VSC_ERROR(EACCES);
        case ERROR_INSUFFICIENT_BUFFER:
        case ERROR_INVALID_FLAGS:
        case ERROR_INVALID_PARAMETER:
        case ERROR_NO_UNICODE_TRANSLATION:
        default:
            return VSC_ERROR(EINVAL);
    }
}

wchar_t *vsci_cstrtowstra_compat(const char *s, size_t *len, unsigned int cp, const VscAllocator *a)
{
    return vsc_cstrtowstra(s, len, cp, a);
}

char *vsci_wstrtocstra_compat(const wchar_t *ws, size_t *len, unsigned int cp, const VscAllocator *a)
{
    return vsc_wstrtocstra(ws, len, cp, a);
}

#endif

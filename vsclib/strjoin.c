/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Zane van Iperen (zane@zanevaniperen.com)
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
#include <stdarg.h>
#include <string.h>
#include <vsclib/mem.h>
#include <vsclib/string.h>

char *vsc_strjoin(const char *delim, ...)
{
    char   *r;
    va_list ap;
    va_start(ap, delim);
    r = vsc_vstrjoina(vsclib_system_allocator, delim, ap);
    va_end(ap);
    return r;
}

char *vsc_strjoina(const VscAllocator *a, const char *delim, ...)
{
    char   *r;
    va_list ap;
    va_start(ap, delim);
    r = vsc_vstrjoina(a, delim, ap);
    va_end(ap);
    return r;
}

char *vsc_vstrjoin(const char *delim, va_list ap)
{
    return vsc_vstrjoina(vsclib_system_allocator, delim, ap);
}

char *vsc_vstrjoina(const VscAllocator *a, const char *delim, va_list ap)
{
    va_list ap2;
    size_t  n, nreq, dlen;
    char   *val, *next;

    if(delim == NULL)
        return NULL;

    dlen = strlen(delim);

    va_copy(ap2, ap);

    for(n = 0, nreq = 0;; ++n) {
        const char *elem;

        if((elem = va_arg(ap2, const char *)) == NULL)
            break;

        nreq += strlen(elem);
    }

    if(n > 0)
        nreq += ((n - 1) * dlen);

    nreq += 1;

    va_end(ap2);

    if((val = vsc_xcalloc(a, nreq, sizeof(char))) == NULL)
        return NULL;

    next = val;
    for(size_t i = 0; i < n; ++i) {
        const char *seg = va_arg(ap, const char *);
        next            = vsc_stpcpy(next, seg);

        if(i != (n - 1)) {
            next = vsc_stpcpy(next, delim);
        }
    }

    return val;
}

char *vsc_strnjoin(const char *delim, const char **elems, size_t nelem)
{
    return vsc_strnjoina(vsclib_system_allocator, delim, elems, nelem);
}

char *vsc_strnjoina(const VscAllocator *a, const char *delim, const char **elems, size_t nelem)
{
    size_t nreq, dlen;
    char  *val, *next;

    if(delim == NULL)
        return NULL;

    dlen = strlen(delim);

    nreq = 0;
    for(size_t i = 0; i < nelem; ++i) {
        if(elems[i] == NULL)
            return NULL;

        nreq += strlen(elems[i]);
    }

    if(nelem > 0)
        nreq += ((nelem - 1) * dlen);

    nreq += 1;

    if((val = vsc_xcalloc(a, nreq, sizeof(char))) == NULL)
        return NULL;

    next = val;
    for(size_t i = 0; i < nelem; ++i) {
        next = vsc_stpcpy(next, elems[i]);

        if(i != (nelem - 1)) {
            next = vsc_stpcpy(next, delim);
        }
    }

    return val;
}

/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2025 Zane van Iperen (zane@zanevaniperen.com)
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
#include <vsclib/string.h>
#include <vsclib/resource.h>

static int accumulate_size(void *res, void *user)
{
    *(size_t *)user += vsc_res_size(res);
    vsc_res_enum_children(res, accumulate_size, user);
    return 0;
}

size_t vsc_res_size_total(void *res)
{
    size_t size = vsc_res_size(res);
    vsc_res_enum_children(res, accumulate_size, &size);
    return size;
}

char *vsc_res_strdup(void *res, const char *s)
{
    const VscAllocator a = vsc_res_allocator(res);
    return vsc_strdupa(s, &a);
}

char *vsc_res_strdupr(void *res, const char *start, const char *end)
{
    const VscAllocator a = vsc_res_allocator(res);
    return vsc_strdupra(start, end, &a);
}

char *vsc_res_vasprintf(void *res, const char *fmt, va_list ap)
{
    const VscAllocator a = vsc_res_allocator(res);
    return vsc_vasprintfa(&a, fmt, ap);
}

char *vsc_res_asprintf(void *res, const char *fmt, ...)
{
    char   *s;
    va_list ap;

    va_start(ap, fmt);
    s = vsc_res_vasprintf(res, fmt, ap);
    va_end(ap);
    return s;
}

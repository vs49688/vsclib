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
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <vsclib.h>

int vsc_for_each_delim(const char *begin, const char *end, char delim, VscForEachDelimProc proc, void *user)
{
    int r;
    const char *start = begin;

    if(begin > end)
        return VSC_ERROR(EINVAL);

    for(const char *next; start != end; start = next) {
        next = memchr(start, delim, end - start);
        if(next == NULL)
            next = end;

        if((r = proc(start, next, user)))
            return r;

        if(next != end)
            ++next;

    }

    assert(start == end);

    /* Handle the special case of a trailing delimiter. */
    if(begin < end && *(end - 1) == delim)
        return proc(start, end, user);

    return 0;
}

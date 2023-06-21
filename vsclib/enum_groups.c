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

#define _GNU_SOURCE
#include <vsclib/platform.h>
#include <errno.h>

#if !defined(_WIN32)
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#endif

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <vsclib/assert.h>
#include <vsclib/error.h>
#include <vsclib/mem.h>
#include <vsclib/enum_groups.h>

int vsc_enum_groupsa(struct passwd *passwd, VscEnumGroupsProc proc, void *user, const VscAllocator *a)
{
    /* Absolutely disgusting. */
#if defined(_WIN32) || !defined(VSC_HAVE_GETGRENT_R)
    (void)passwd;
    (void)proc;
    (void)user;
    (void)a;
    return VSC_ERROR(ENOTSUP);
#else
    size_t buflen = 2048; /* 2048 is enough for the HPC. */
    char  *buf    = NULL;
    int    ret = 0, rc = 0;

    vsc_assert(passwd != NULL);
    vsc_assert(proc != NULL);

    errno = 0;
    setgrent();
    if(errno != 0)
        return VSC_ERROR(errno);

    for(struct group *g = NULL;;) {
        struct group grp;
        void        *buf2;

        if(buf == NULL || rc == ERANGE) {
            if((buf2 = vsc_xrealloc(a, buf, buflen)) == NULL) {
                ret = VSC_ERROR(ENOMEM);
                break;
            }
            buf = buf2;
        }

        if((rc = getgrent_r(&grp, buf, buflen, &g)) == ENOENT)
            break;

        if(rc == ERANGE) {
            buflen *= 2;
            continue;
        }

        if(rc != 0) {
            ret = VSC_ERROR(rc);
            break;
        }

        if(g->gr_gid == passwd->pw_gid) {
            if((ret = proc(g, user)) != 0)
                goto done;
            continue;
        }

        for(char *const *u = g->gr_mem; *u != NULL; ++u) {
            if(strcmp(passwd->pw_name, *u) != 0)
                continue;

            if((ret = proc(g, user)) != 0)
                goto done;
            break;
        }
    }

done:
    if(buf != NULL)
        vsc_xfree(a, buf);

    endgrent();

    return ret;
#endif
}

int vsc_enum_groups(struct passwd *passwd, VscEnumGroupsProc proc, void *user)
{
    return vsc_enum_groupsa(passwd, proc, user, vsclib_system_allocator);
}

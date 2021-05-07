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
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "vsclib_i.h"

static int statfile(const char *p, uid_t uid, gid_t gid)
{
    struct stat statbuf;
    memset(&statbuf, 0, sizeof(statbuf));

    if(stat(p, &statbuf) < 0)
        return -1;

    /* EACCES The file or a script interpreter is not a regular file. */
    if(!S_ISREG(statbuf.st_mode))
        return errno = EACCES, -1;

    /* Check permissions, least-specific to most-specific.  */
    if(statbuf.st_mode & S_IXOTH)
        return 0;

    if((statbuf.st_mode & S_IXGRP) && (statbuf.st_gid == gid))
        return 0;

    if((statbuf.st_mode & S_IXUSR) && (statbuf.st_uid == uid))
        return 0;

    /* EACCES Execute permission is denied for the file or a script or ELF interpreter. */
    return errno = EACCES, -1;
}

char *vsc_searchpatha(const char *f, size_t *len, const VscAllocator *a)
{
    /* Get PATH. If empty, fall back to _CS_PATH. */
    const char *path = getenv("PATH");
    const char *pend;
    char *buf;
    size_t dlen = 0, flen;
    uid_t uid;
    gid_t gid;

    if(path == NULL || path[0] == '\0') {
        char *_path;
        size_t n;

        if((n = confstr(_CS_PATH, NULL, 0)) == 0)
            return errno = EINVAL, NULL;

        /* If _CS_PATH overflows the stack then something's wrong. */
        _path = (char*)alloca(n * sizeof(char));
        if(confstr(_CS_PATH, _path, n))
            return errno = EINVAL, NULL;

        path = _path;
    }

    pend = path + strlen(path);

    /* Get the max buffer size. */
    dlen = 0;
    for(const char *o = path, *c = strchr(o, ':'); o < pend; o = c + 1, c = strchr(o, ':')) {
        size_t dist;
        if(c == NULL)
            c = pend;

        dist = c - o;
        if(dist > dlen)
            dlen = dist;
    }

    flen  = strlen(f) + 1; /* /%s */
    dlen += flen + 1;

    if((buf = vsc_xalloc(a, dlen * sizeof(char))) == NULL)
        return errno = ENOMEM, NULL;

    uid = getuid();
    gid = getgid();

    for(const char *o = path, *c = strchr(o, ':'); o < pend; o = c + 1, c = strchr(o, ':'))
    {
        if(c == NULL)
            c = pend;

        size_t dist = c - o;
        strncpy(buf, o, dist);
        buf[dist] = '/';
        strcpy(buf + dist + 1, f);

        if((statfile(buf, uid, gid)) < 0)
            continue;

        if(len)
            *len = dist + flen;
        return buf;
    }

    vsc_xfree(a, buf);
    return errno = ENOENT, NULL;
}

char *vsc_searchpath(const char *f, size_t *len)
{
    return vsc_searchpatha(f, len, &vsclib_system_allocator);
}

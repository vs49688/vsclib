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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <vsclib/types.h>
#include <vsclib/error.h>
#include <vsclib/iodef.h>

int vsc_fileno(FILE *stream)
{
    int r;
#if defined(_WIN32)
    r = _fileno(stream);
#else
    r = fileno(stream);
#endif
    if(r < 0)
        return VSC_ERROR(errno);
    return r;
}

int vsc_open(const char *pathname, int flags)
{
    int r;
#if defined(_WIN32)
    r = _open(pathname, flags);
#else
    r = open(pathname, flags);
#endif
    if(r < 0)
        return VSC_ERROR(errno);
    return r;
}

int vsc_dup2(int oldfd, int newfd)
{
    int r;
#if defined(_WIN32)
    r = _dup2(oldfd, newfd);
#else
    r = dup2(oldfd, newfd);
#endif
    if(r < 0)
        return VSC_ERROR(errno);
    return r;
}

int vsc_close(int fd)
{
    int r;
#if defined(_WIN32)
    r = _close(fd);
#else
    r = close(fd);
#endif
    if(r < 0)
        return VSC_ERROR(errno);
    return r;
}

vsc_off_t vsc_ftello(FILE *stream)
{
    vsc_off_t r;
#if defined(WIN32)
    r = _ftelli64(stream);
#else
    r = ftello(stream);
#endif
    if(r < 0)
        return VSC_ERROR(errno);
    return r;
}

int vsc_fseeko(FILE *stream, vsc_off_t offset, int whence)
{
    int r;
#if defined(WIN32)
    r = _fseeki64(stream, offset, whence);
#else
    r = fseeko(stream, offset, whence);
#endif
    if(r < 0)
        return VSC_ERROR(errno);
    return r;
}

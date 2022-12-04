/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2022 Zane van Iperen (zane@zanevaniperen.com)
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
#include <vsclib/error.h>
#include <vsclib/mem.h>
#include <vsclib/io.h>
#include <vsclib/file_stdio.h>
#include <vsclib/file_internal.h>

typedef struct VscStdioFile {

    const VscFileProcs *vtable;
    FILE               *fp;
    int                own;
    const VscAllocator *allocator;

} VscStdioFile;

static int stdio_read(VscFile *_self, void *restrict ptr, size_t size, size_t nmemb, size_t *nread)
{
    size_t n;
    FILE   *fp = ((VscStdioFile *)_self)->fp;

    n = fread(ptr, size, nmemb, fp);
    if(nread != NULL)
        *nread = n;

    if(ferror(fp))
        return VSC_ERROR(EIO);

    if(feof(fp))
        return VSC_ERROR_EOF;

    return 0;
}

static int stdio_write(VscFile *_self, const void *restrict ptr, size_t size, size_t nmemb, size_t *nwritten)
{
    size_t n;
    FILE   *fp = ((VscStdioFile *)_self)->fp;

    n = fwrite(ptr, size, nmemb, fp);
    if(nwritten != NULL)
        *nwritten = n;

    if(ferror(fp))
        return VSC_ERROR(EIO);

    if(feof(fp))
        return VSC_ERROR_EOF;

    return 0;
}

static int stdio_seek(VscFile *self, vsc_off_t offset, VscFileWhence whence)
{
    int w;

    switch(whence) {
        case VSC_FILE_SEEK_SET: w = SEEK_SET; break;
        case VSC_FILE_SEEK_CUR: w = SEEK_CUR; break;
        case VSC_FILE_SEEK_END: w = SEEK_END; break;
        default: return VSC_ERROR(EINVAL);
    }

    return vsc_fseeko(((VscStdioFile *)self)->fp, offset, w);
}

static vsc_off_t stdio_tell(VscFile *self)
{
    return vsc_ftello(((VscStdioFile *)self)->fp);
}

static int stdio_flush(VscFile *_self)
{
    VscStdioFile *self = (VscStdioFile *)_self;
    int          r;

    r = fflush(self->fp);
    if(r == EOF)
        return VSC_ERROR(errno);

    if(r != 0)
        return VSC_ERROR(EIO);

    return 0;
}

static int stdio_close(VscFile *_self)
{
    VscStdioFile *self = (VscStdioFile *)_self;
    int          r;

    if(self->own)
        r = fclose(self->fp);
    else
        r = 0;

    /*
     * If fclose() fails:
     * > any further access (including another call to fclose()) to
     * > the stream results in undefined behavior.)
     */
    vsc_xfree(self->allocator, self);

    if(r == EOF)
        return VSC_ERROR(errno);

    if(r != 0)
        return VSC_ERROR(EIO);

    return 0;
}

const static VscFileProcs stdio_file_vtable = {
    .read  = stdio_read,
    .write = stdio_write,
    .seek  = stdio_seek,
    .tell  = stdio_tell,
    .flush = stdio_flush,
    .close = stdio_close,
};

int vscf_stdio_is(VscFile *fp)
{
    return (const void*)fp->_reserved == &stdio_file_vtable;
}

FILE *vscf_stdio_get(VscFile *fp)
{
    VscStdioFile *self = (VscStdioFile *)fp;

    vsc_assert(vscf_stdio_is(fp));

    return self->fp;
}

FILE *vscf_stdio_release(VscFile *fp)
{
    VscStdioFile *self = (VscStdioFile *)fp;

    vsc_assert(vscf_stdio_is(fp));

    self->own = 0;
    return self->fp;
}

int vscf_stdio_opena(const char *pathname, const char *mode, VscFile **fp, const VscAllocator *allocator)
{
    VscStdioFile *file;
    FILE         *f;
    int          r;

    if(pathname == NULL || mode == NULL || fp == NULL || allocator == NULL)
        return VSC_ERROR(EINVAL);

    if((file = vsc_xcalloc(allocator, 1, sizeof(VscStdioFile))) == NULL)
        return VSC_ERROR(ENOMEM);

    if((r = vsc_fopena(pathname, mode, &f, allocator)) < 0) {
        vsc_xfree(allocator, file);
        return r;
    }

    *file = (VscStdioFile){
        .vtable    = &stdio_file_vtable,
        .fp        = f,
        .own       = 1,
        .allocator = allocator,
    };

    *fp = (VscFile *)file;

    return 0;
}

int vscf_stdio_open(const char *pathname, const char *mode, VscFile **fp)
{
    return vscf_stdio_opena(pathname, mode, fp, &vsclib_system_allocator);
}

int vscf_stdio_wrapa(FILE *f, int takeown, VscFile **fp, const VscAllocator *allocator)
{
    VscStdioFile *file;

    if(f == NULL || fp == NULL || allocator == NULL)
        return VSC_ERROR(EINVAL);

    if((file = vsc_xcalloc(allocator, 1, sizeof(VscStdioFile))) == NULL)
        return VSC_ERROR(ENOMEM);

    *file = (VscStdioFile){
        .vtable    = &stdio_file_vtable,
        .fp        = f,
        .own       = takeown,
        .allocator = allocator,
    };

    *fp = (VscFile *)file;

    return 0;
}

int vscf_stdio_wrap(FILE *f, int takeown, VscFile **fp)
{
    return vscf_stdio_wrapa(f, takeown, fp, &vsclib_system_allocator);
}

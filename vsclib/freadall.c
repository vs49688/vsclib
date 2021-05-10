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
#include <errno.h>

/* From https://stackoverflow.com/a/62371749 */
#if defined(_MSC_VER)
#	undef _CRT_INTERNAL_NONSTDC_NAMES
#	define _CRT_INTERNAL_NONSTDC_NAMES 1
#	include <sys/stat.h>
#	if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#		define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#	endif
#	if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#		define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#	endif
#else
#	include <sys/stat.h>
#endif

#if defined(__linux__)
#	include <linux/fs.h>
#	include <sys/ioctl.h>
#	include <sys/types.h>
#endif

#include "vsclib_i.h"

const static VscFreadallState default_state = {
	.file_size  = 0,
	.blk_size   = 4096,
	.bytes_read = 0,
	.statbuf    = { 0 },
};

/* Use ioctl() to get the size of a block device. */
static int get_size_ioctl(int fd, size_t *size)
{
#if defined(__linux__)
    if(ioctl(fd, BLKGETSIZE64, size) < 0) {
        unsigned long s;
        if(ioctl(fd, BLKGETSIZE, &s) < 0)
            return -1;

        /* NB: This isn't sector size, it's always 512. */
        *size = s * 512;
    }
    return 0;
#else
    errno = ENOTSUP;
    return -1;
#endif
}

static int get_blksize_ioctl(int fd, vsc_blksize_t *blksize)
{
#if defined(__linux__)
    unsigned int _blksize;
    if(ioctl(fd, BLKIOOPT, &_blksize) < 0)
        return -1;

    /* Most physical disks don't supply this. */
    if(_blksize == 0) {
        errno = ENOTSUP;
        return -1;
    }

    *blksize = (vsc_blksize_t)_blksize;
    return 0;
#else
    errno = ENOTSUP;
    return -1;
#endif
}

/* NB: This doesn't restore the stream to its original position. */
static int get_size_seektell(FILE *f, size_t *size)
{
    vsc_off_t off;
    if(vsc_fseeko(f, 0, SEEK_END) < 0)
        return -1;

    if((off = vsc_ftello(f)) < 0)
        return -1;

    *size = (size_t)off;
    return 0;
}

static int get_sizes(FILE *f, VscFreadallState *state)
{
    int fd;

    *state = default_state;

    if((fd = vsc_fileno(f)) < 0) {
        if(errno != EBADF)
            return -1;

        /*
         * Not necessarily an error, f could be from fmemopen().
         * Try seek/tell, then fall back to streaming.
         */
        errno = 0;
        (void)get_size_seektell(f, &state->file_size);
        return 0;
    }

    if(fstat(fd, &state->statbuf) < 0)
        return -1;

    /* Fail-fast if we're a directory. */
    if(S_ISDIR(state->statbuf.st_mode))
        return errno = EISDIR, -1;

#if !defined(_WIN32)
    state->blk_size = state->statbuf.st_blksize;
#endif

    if(S_ISREG(state->statbuf.st_mode)) {
        state->file_size = (size_t)state->statbuf.st_size;
    }
#if !defined(_WIN32)
    else if(S_ISBLK(state->statbuf.st_mode)) {
        /*
         * For block devices, try to use ioctl, then fall back to seek/tell.
         * If seek/tell fails, fall back to streaming.
         */
        if(get_size_ioctl(fd, &state->file_size) < 0)
            (void)get_size_seektell(f, &state->file_size);

        (void)get_blksize_ioctl(fd, &state->blk_size);

        errno = 0;
    }
#endif
    else if(!S_ISREG(state->statbuf.st_mode)) {
        /* Sockets, pipes, and character devices have to be streamed. */
        state->file_size = 0;
    }

    return 0;
}

/* Default do-nothing procedures. Always allows continue. */
static int default_init_proc(VscFreadallState *state, void *user)
{
    (void)state;
    (void)user;
    return 0;
}

static int default_chunk_proc(const VscFreadallState *state, void *user)
{
    (void)state;
    (void)user;
    return 0;
}

int vsc_freadalla(void **ptr, size_t *size, FILE *f, const VscAllocator *a)
{
    return vsc_freadalla_ex(ptr, size, f, default_init_proc, default_chunk_proc, NULL, a);
}

int vsc_freadall(void **ptr, size_t *size, FILE *f)
{
    return vsc_freadalla(ptr, size, f, &vsclib_system_allocator);
}

int vsc_freadall_ex(void **ptr, size_t *size, FILE *f, VscFreadallInitProc init_proc, VscFreadallChunkProc chunk_proc, void *user)
{
    return vsc_freadalla_ex(ptr, size, f, init_proc, chunk_proc, user, &vsclib_system_allocator);
}

int vsc_freadalla_ex(void **ptr, size_t *size, FILE *f, VscFreadallInitProc init_proc, VscFreadallChunkProc chunk_proc, void *user, const VscAllocator *a)
{
    VscFreadallState state, user_state;
    vsc_off_t save;
    char *p;

    if(ptr == NULL || size == NULL || f == NULL || init_proc == NULL || chunk_proc == NULL || a == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* Save our current position if possible. */
    if((save = vsc_ftello(f)) < 0) {
        /*
         * EBADF: Not a seekable stream.
         * ESPIPE: Can't seek on a pipe.
         * Anything else: something screwy
         */
        if(errno != EBADF && errno != ESPIPE)
            return -1;

        errno = 0;
    }

    if(get_sizes(f, &state) < 0)
        return -1;

    if(save >= 0) {
        /* Restore our position, if any. */
        if(vsc_fseeko(f, save, SEEK_SET) < 0)
            return -1;

        /* If we're already not at the start, don't allocate more than we need to. */
        if(state.file_size > 0) {
            vsc_assert((size_t)save <= state.file_size);
            state.file_size -= save;
        }
    }

    if(state.file_size > 0)
        ++state.file_size; /* Read past EOF, to avoid an additional malloc() + fread(). */
    else
        state.file_size = (size_t)state.blk_size;

    p = NULL;

    state.bytes_read = 0;

    /* Invoke the "init" callback. The caller may want to check/tweak our discovered properties. */
    user_state = state;
    if(init_proc(&user_state, user) < 0) {
        errno = ECANCELED;
        return -1;
    }

    /* Only accept changes on these fields. */
    state.file_size = user_state.file_size;
    state.blk_size  = user_state.blk_size;

    for(;!feof(f) && !ferror(f);) {
        if(p == NULL || state.bytes_read >= state.file_size) {
            void *_p;

            while(state.bytes_read >= state.file_size)
                state.file_size += state.blk_size;

            if((_p = vsc_xrealloc(a, p, state.file_size)) == NULL) {
                vsc_xfree(a, p);
                return errno = ENOMEM, -1;
            }
            p = _p;
        }

        state.bytes_read += fread(p + state.bytes_read, 1, state.file_size - state.bytes_read, f);

        if(chunk_proc(&state, user) < 0) {
            vsc_xfree(a, p);
            errno = ECANCELED;
            return -1;
        }
    }

    if(ferror(f))
        errno = EIO;

    if(errno != 0) {
        vsc_xfree(a, p);
        return -1;
    }

    *ptr  = p;
    *size = state.bytes_read;
    return 0;
}
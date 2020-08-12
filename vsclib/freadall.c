/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2020 Zane van Iperen (zane@zanevaniperen.com)
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
#include <assert.h>
#include <sys/stat.h>

#if defined(__linux__)
#	include <linux/fs.h>
#	include <sys/ioctl.h>
#	include <sys/types.h>
#endif

#include "vsclib_i.h"

#if defined(_WIN32)
typedef long blksize_t; /* Isn't used anyway. */
#endif

/* Use ioctl() to get the size of a block device. */
static int get_size_ioctl(int fd, size_t *size)
{
#if defined(__linux__)
	if(ioctl(fd, BLKGETSIZE64, size) < 0)
	{
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

static int get_blksize_ioctl(int fd, blksize_t *blksize)
{
#if defined(__linux__)
	unsigned int _blksize;
	if(ioctl(fd, BLKIOOPT, &_blksize) < 0)
		return -1;

	/* Most physical disks don't supply this. */
	if(_blksize == 0)
	{
		errno = ENOTSUP;
		return -1;
	}

	*blksize = (blksize_t)_blksize;
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

static int get_sizes(FILE *f, size_t *_size, blksize_t *_blksize)
{
	*_size = 0;
	*_blksize = 4096;

	int fd = vsc_fileno(f);
	if(fd < 0)
	{
		if(errno != EBADF)
			return -1;

		/*
		 * Not necessarily an error, f could be from fmemopen().
		 * Try seek/tell, then fall back to streaming.
		 */
		errno = 0;
		(void)get_size_seektell(f, _size);
		return 0;
	}

	struct stat statbuf;
	if(fstat(fd, &statbuf) < 0)
		return -1;

	/* Fail-fast if we're a directory. */
	if(S_ISDIR(statbuf.st_mode))
		return errno = EISDIR, -1;

#if !defined(_WIN32)
	*_blksize = statbuf.st_blksize;
#endif

	if(S_ISREG(statbuf.st_mode))
	{
		*_size = (size_t)statbuf.st_size;
	}
	else if(S_ISBLK(statbuf.st_mode))
	{
		/*
		 * For block devices, try to use ioctl, then fall back to seek/tell.
		 * If seek/tell fails, fall back to streaming.
		 */
		if(get_size_ioctl(fd, _size) < 0)
			(void)get_size_seektell(f, _size);

		(void)get_blksize_ioctl(fd, _blksize);

		errno = 0;
	}
	else if(!S_ISREG(statbuf.st_mode))
	{
		/* Sockets, pipes, and character devices have to be streamed. */
		*_size = 0;
	}

	return 0;
}

int vsc_freadalla(void **ptr, size_t *size, FILE *f, const vsc_allocator_t *a)
{
	vsc_off_t save;
	size_t fsize;
	blksize_t blksize;
	size_t currpos;

	if(ptr == NULL || size == NULL || f == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	/* Save our current position if possible. */
	if((save = vsc_ftello(f)) < 0)
	{
		/*
		 * EBADF: Not a seekable stream.
		 * ESPIPE: Can't seek on a pipe.
		 * Anything else: something screwy
		 */
		if(errno != EBADF && errno != ESPIPE)
			return -1;

		errno = 0;
	}

	/*
	 * Get the sizes.
	 * If fsize is 0, then it's either an actual 0-byte file, or the size can't be determined.
	 * blksize will be the "Block size for filesystem I/O".
	 * Mapped directly to the st_blksize field of `struct stat`. If not
	 */
	if(get_sizes(f, &fsize, &blksize) < 0)
		return -1;

	if(save >= 0)
	{
		/* Restore our position, if any. */
		if(vsc_fseeko(f, save, SEEK_SET) < 0)
			return -1;

		/* If we're already not at the start, don't allocate more than we need to. */
		if(fsize > 0)
		{
			assert(save <= fsize);
			fsize -= save;
		}
	}

	if(fsize > 0)
		++fsize; /* Read past EOF, to avoid an additional malloc() + fread(). */
	else
		fsize = (size_t)blksize;

	char *p = NULL;

	currpos = 0;
	for(;!feof(f) && !ferror(f);)
	{
		if(p == NULL || currpos >= fsize)
		{
			while(currpos >= fsize)
				fsize += blksize;

			void *_p;
			if((_p = vsci_xrealloc(a, p, fsize)) == NULL)
			{
				vsci_xfree(a, p);
				return errno = ENOMEM, -1;
			}
			p = _p;
		}


		size_t nread = fread(p + currpos, 1, fsize - currpos, f);
		currpos += nread;
	}

	if(ferror(f))
		errno = EIO;

	if(errno != 0)
	{
		vsci_xfree(a, p);
		return -1;
	}

	*ptr = p;
	*size = currpos;
	return 0;
}

int vsc_freadall(void **ptr, size_t *size, FILE *f)
{
	return vsc_freadalla(ptr, size, f, &vsclib_system_allocator);
}

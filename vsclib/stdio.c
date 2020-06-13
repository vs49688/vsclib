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
#if defined(WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <io.h>
#else
#	include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include "vsclib_i.h"

int vsci_fileno(FILE *stream)
{
#if defined(_WIN32)
	return _fileno(stream);
#else
	return fileno(stream);
#endif
}

int vsci_open(const char *pathname, int flags)
{
#if defined(_WIN32)
	return _open(pathname, flags);
#else
	return open(pathname, flags);
#endif
}

int vsci_dup2(int oldfd, int newfd)
{
#if defined(_WIN32)
	return _dup2(oldfd, newfd);
#else
	return dup2(oldfd, newfd);
#endif
}

int vsci_close(int fd)
{
#if defined(_WIN32)
	return _close(fd);
#else
	return close(fd);
#endif
}

vsc_off_t vsci_ftello(FILE *stream)
{
#if defined(WIN32)
	return _ftelli64(stream);
#else
	return ftello(stream);
#endif
}

int vsci_fseeko(FILE *stream, vsc_off_t offset, int whence)
{
#if defined(WIN32)
	return _fseeki64(stream, offset, whence);
#else
	return fseeko(stream, offset, whence);
#endif
}

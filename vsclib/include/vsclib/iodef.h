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
#ifndef _VSCLIB_IODEF_H
#define _VSCLIB_IODEF_H

#include <stdio.h>
#include "types.h"
#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32)
typedef __int64   vsc_off_t;
typedef long int  vsc_blksize_t;
#else
typedef off_t     vsc_off_t;
typedef blksize_t vsc_blksize_t;
#endif

typedef struct VscFreadallState {
	/**
	* @brief The size of the file, in bytes.
	* @remark If this is 0, then the file is either an actual 0-byte
	*                 file, or the size cannot be determined.
	*/
	size_t          file_size;

	/**
	* @brief Optimal block size for filesystem I/O.
	*
	* @remark This is maps directly to the `st_blocksize` field of `struct stat`.
	* @return This may be different to the `statbuf::st_blocksize` field. If this
	*         is the case, then alternative methods were used in order to find the
	*         block size. The value in `statbuf` is the original.
	*/
	vsc_blksize_t       blk_size;

	/**
	* @brief The number of bytes read so far.
	*/
	size_t          bytes_read;

	/**
	* @brief A `struct stat` of the file.
	*/
	struct stat statbuf;
} VscFreadallState;

typedef int (*VscFreadallInitProc)(VscFreadallState *state, void *user);
typedef int (*VscFreadallChunkProc)(const VscFreadallState *state, void *user);

#endif /* _VSCLIB_IODEF_H */
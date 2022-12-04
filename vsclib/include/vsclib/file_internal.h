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
/**
 * \file vsclib/file_internal.h
 *
 * \brief This file contains types and structures for implementing VscFile.
 *   These are NOT stable and may change at any time.
 */
#ifndef _VSCLIB_FILE_INTERNAL_H
#define _VSCLIB_FILE_INTERNAL_H

#include "file.h"

/** \sa vscf_read() */
typedef int (*VscFileReadProc)(VscFile *fp, void *ptr, size_t size, size_t nmemb, size_t *nread);

/** \sa vscf_write() */
typedef int (*VscFileWriteProc)(VscFile *fp, const void *ptr, size_t size, size_t nmemb, size_t *nwritten);

/** \sa vscf_seek() */
typedef int (*VscFileSeekProc)(VscFile *fp, vsc_off_t offset, VscFileWhence whence);

/** \sa vscf_tell() */
typedef vsc_off_t (*VscFileTellProc)(VscFile *fp);

/** \sa vscf_flush() */
typedef int (*VscFileFlushProc)(VscFile *fp);

/** \sa vscf_close() */
typedef int (*VscFileCloseProc)(VscFile *fp);

typedef struct VscFileProcs {

    VscFileReadProc  read;
    VscFileWriteProc write;
    VscFileSeekProc  seek;
    VscFileTellProc  tell;
    VscFileFlushProc flush;
    VscFileCloseProc close;

} VscFileProcs;

struct VscFile { uintptr_t _reserved; };

#endif /* _VSCLIB_FILE_INTERNAL_H */

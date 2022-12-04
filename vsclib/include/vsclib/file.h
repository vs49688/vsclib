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
/** \file vsclib/file.h */
#ifndef _VSCLIB_FILE_H
#define _VSCLIB_FILE_H

#include <stdio.h>
#include "memdef.h"
#include "iodef.h"
#include "filedef.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Read \p nmemb items of data from the file, each \p size bytes long.
 *
 * \param[in]  fp     A pointer to the VscFile object. May not be NULL.
 * \param[in]  ptr    A pointer to the buffer to receive the data.
 * \param[in]  size   The size of each item, in bytes.
 * \param[in]  nmemb  The number of items of \p size bytes to read.
 * \param[out] nread  The number of items read. May be NULL if this value is not required.
 *                     This will only equal the total number of bytes if \p size is 1.
 *
 * \return On success, returns 0. Otherwise, a negative error value is returned.
 *  If the end-of-file is reached, this will error with #VSC_ERROR_EOF.
 */
int vscf_read(VscFile *fp, void *ptr, size_t size, size_t nmemb, size_t *nread);

/**
 * \brief Write \p nmemb items of data to the file, each \p size bytes long.
 *
 * \param[in]  fp       A pointer to the VscFile object. May not be NULL.
 * \param[in]  ptr      A pointer to the data to be written.
 * \param[in]  size     The size of each item, in bytes.
 * \param[in]  nmemb    The number of items of \p size to write.
 * \param[out] nwritten The number of items written. May be NULL if this value is not required.
 *                       This will only equal the total number of bytes if \p size is 1.
 *
 * \return On success, returns 0. Otherwise, a negative error value is returned.
 *  If the end-of-file is reached, this will error with #VSC_ERROR_EOF.
 */
int vscf_write(VscFile *fp, const void *ptr, size_t size, size_t nmemb, size_t *nwritten);

/**
 * \brief
 *
 * \param[in] fp A pointer to the VscFile object. May not be NULL.
 * \param[in] offset
 * \param[in] whence
 * \return
 */
int vscf_seek(VscFile *fp, vsc_off_t offset, VscFileWhence whence);

vsc_off_t vscf_tell(VscFile *fp);

int vscf_flush(VscFile *fp);

/**
 * \brief Close the underlying file and release all associated resources.
 *
 * \param[in] fp A pointer to the #VscFile object. May not be NULL.
 *
 * \return If successful, returns 0. Otherwise, a negative error value is
 *   returned. In either case, any further access (including another call
 *   to vscf_close()) to the file results in undefined behaviour.
 */
int vscf_close(VscFile *fp);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_FILE_H */

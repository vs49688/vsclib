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
/** \file vsclib/file_stdio.h */
#ifndef _VSCLIB_FILE_STDIO_H
#define _VSCLIB_FILE_STDIO_H

#include "file.h"

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * \brief Check if the given file is a wrapped stdio file
 *
 * \param[in] fp A pointer to the VscFile object. May not be NULL.
 *
 * \return If \p fp is a wrapped stdio file, returns nonzero.
 *   Otherwise, returns 0.
 */
int vscf_stdio_is(VscFile *fp);

/**
 * \brief Get the underlying stdio FILE pointer.
 *
 * \param[in] fp A pointer to the VscFile object. May not be NULL.
 *
 * \return A pointer to the underlying stdio FILE object.
 */
FILE *vscf_stdio_get(VscFile *fp);

/**
 * \brief Release ownership of the underling stdio FILE object.
 *
 * This wrapper may still be used, however the FILE must now be
 * fclose()'d manually. It is now effectively the as a wrapper
 * created with vscf_stdio_wrapa().
 *
 * \param[in] fp A pointer to the VscFile object. May not be NULL.
 *
 * \return A pointer to the underlying stdio FILE object.
 *
 * \remark vscf_stdio_release() is idempotent.
 */
FILE *vscf_stdio_release(VscFile *fp);

/**
 * \brief Create a VscFile handle backed by a stdio stream.
 *
 * The internal stdio stream is owned by the VscFile object, and will
 * closed when vscf_close() is called. vscf_stdio_get() may be used
 * to retrieve the stream, and vscf_stdio_release() may be used to
 * revoke ownership.
 *
 * This is effectively a thin wrapper around fopen(3). Refer to the
 * stdio documentation for semantics.
 *
 * \param[in]  pathname  The name of the file to open.
 * \param[in]  mode      The fopen(3) mode argument.
 * \param[out] fp        A pointer to receive the VscFile object.
 * \param[in]  allocator The allocator to use to allocate the VscFile object.
 *
 * \return On success, returns 0 and the resulting VscFile instance is written into \p fp.
 *  On failure, a negative error value is returned.
 *
 * \sa vscf_stdio_get
 * \sa vscf_stdio_release
 */
int vscf_stdio_opena(const char *pathname, const char *mode, VscFile **fp, const VscAllocator *allocator);

/**
 * \brief Invoke vscf_stdio_opena() with the system's default allocator.
 *
 * \sa vscf_stdio_opena()
 */
int vscf_stdio_open(const char *pathname, const char *mode, VscFile **fp);

/**
 * \brief Wraps an existing stdio stream in a VscFile.
 *
 * Enables using VscFile-accepting functions with a regular stdio stream.
 *
 * \param[in]  fp        A pointer to the stdio stream.
 * \param[in]  takeown   Take ownership of the stream? If nonzero, then the stream
 *                       will be "owned" by the VscFile, i.e. closed upon vscf_close().
 *                       Ownership may be released via the usual methods.
 * \param[out] fp        A pointer to receive the VscFile object.
 * \param[in]  allocator The allocator to use t allocate the VscFile object.
 *
 * \return
 *
 * \sa vscf_stdio_get
 * \sa vscf_stdio_release
 */
int vscf_stdio_wrapa(FILE *f, int takeown, VscFile **fp, const VscAllocator *allocator);

/**
 * \brief Invoke vscf_stdio_wrapa() with the system's default allocator.
 *
 * \sa vscf_stdio_wrapa()
 */
int vscf_stdio_wrap(FILE *f, int takeown, VscFile **fp);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_FILE_STDIO_H */

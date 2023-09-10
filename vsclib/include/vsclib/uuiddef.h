/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Zane van Iperen (zane@zanevaniperen.com)
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
/** \file vsclib/uuiddef.h */
#ifndef _VSCLIB_UUIDDEF_H
#define _VSCLIB_UUIDDEF_H

#include <stdint.h>

/** \brief The size of a raw UUID in bytes. */
#define VSC_UUID_SIZE        16

/** \brief The maximum possible size for a formatted UUID. */
#define VSC_UUID_STRING_SIZE 39 /* (VSC_UUID_SIZE * 2) + 4 + 2 + 1 */

typedef enum VscUUIDFormatFlags {
    /** \brief The UUID will be formatted as defined in RFC4122 using lower-case hex digits. */
    VSC_UUID_FORMAT_NORMAL  = 0 << 0,
    /** \brief The UUID will be formatted as defined in RFC4122 using upper-case hex digits. */
    VSC_UUID_FORMAT_UPPER   = 1 << 0,
    /** \brief The UUID will start with a '{' and end with a '}', as per Microsoft convention. */
    VSC_UUID_FORMAT_MS      = 1 << 2,
    /** \brief The UUID will be formatted without separating dashes. */
    VSC_UUID_FORMAT_COMPACT = 1 << 3,
} VscUUIDFormatFlags;

typedef struct VscUUID {
    uint8_t v[VSC_UUID_SIZE];
} VscUUID;

#endif /* _VSCLIB_UUIDDEF_H */

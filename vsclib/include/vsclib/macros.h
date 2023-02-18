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
/** \file vsclib/macros.h */
#ifndef _VSCLIB_MACROS_H
#define _VSCLIB_MACROS_H

#define VSC_ASIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define VSC_U8(v) ((unsigned)(v) & 0xFFu)

#define VSC_PACKU32(a, b, c, d) \
    ((VSC_U8(a) << 24u) | (VSC_U8(b) << 16u) | \
     (VSC_U8(c) <<  8u) | (VSC_U8(d) <<  0u))

#define VSC_FOURCC(a, b, c, d) VSC_PACKU32(d, c, b, a)

#define VSC_FOURCCBE(a, b, c, d) VSC_PACKU32(a, b, c, d)

#endif /* _VSCLIB_MACROS_H */

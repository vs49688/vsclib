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

#define VSC_FOURCC(a, b, c, d) \
    ((((unsigned)(a) & 0xFF) <<  0) | (((unsigned)(b) & 0xFF) <<  8) | \
     (((unsigned)(c) & 0xFF) << 16) | (((unsigned)(d) & 0xFF) << 24) )

#define VSC_FOURCCBE(a, b, c, d) VSC_FOURCC(d, c, b, a)

#endif /* _VSCLIB_MACROS_H */

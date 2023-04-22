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
#ifndef _VSCLIB_COLOURDEF_H
#define _VSCLIB_COLOURDEF_H

#include <stdint.h>
#include "macros.h"

/* https://www.mail-archive.com/pixman@lists.freedesktop.org/msg01866.html */

#define VSC_COLOUR_C5_TO_C8(c) ((((c)&0x1Fu) * 527u + 23u) >> 6u)
#define VSC_COLOUR_C6_TO_C8(c) ((((c)&0x3Fu) * 259u + 33u) >> 6u)
#define VSC_COLOUR_C1_TO_C8(c) ((((c)&0x01u) * 255u + 0u) >> 0u)

#define VSC_COLOUR_ARGB1555(a, r, g, b) \
    (((VSC_U8(a) >> 7u) << 15u) | ((VSC_U8(r) >> 3u) << 10u) | ((VSC_U8(g) >> 3u) << 5u) | ((VSC_U8(b) >> 3u) << 0u))

#define VSC_COLOUR_XRGB1555(r, g, b)    VSC_COLOUR_ARGB1555(0xFFu, r, g, b)

#define VSC_COLOUR_ARGB8888(a, r, g, b) VSC_PACKU32(a, r, g, b)

#define VSC_COLOUR_XRGB8888(r, g, b)    VSC_PACKU32(0xFFu, r, g, b)

#define VSC_COLOUR_RGBA8888(r, g, b, a) VSC_PACKU32(r, g, b, a)

#define VSC_COLOUR_RGB565(r, g, b)      (((VSC_U8(r) >> 3u) << 11u) | ((VSC_U8(g) >> 2u) << 5u) | ((VSC_U8(b) >> 3u) << 0u))

typedef struct VscColour32 {
    uint8_t r, g, b, a;
} VscColour32;

#endif /* _VSCLIB_COLOURDEF_H */

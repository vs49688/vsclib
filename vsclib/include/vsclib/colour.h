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
#ifndef _VSCLIB_COLOUR_H
#define _VSCLIB_COLOUR_H

#include "colourdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

VscColour32 vsc_colour_unpack_rgb565(uint16_t pixel);
VscColour32 vsc_colour_unpack_argb1555(uint16_t pixel);
VscColour32 vsc_colour_unpack_xrgb1555(uint16_t pixel);
VscColour32 vsc_colour_unpack_argb8888(uint32_t pixel);
VscColour32 vsc_colour_unpack_xrgb8888(uint32_t pixel);

uint16_t vsc_colour_pack_rgb565(VscColour32 c);
uint16_t vsc_colour_pack_argb1555(VscColour32 c);
uint16_t vsc_colour_pack_xrgb1555(VscColour32 c);
uint32_t vsc_colour_pack_argb8888(VscColour32 c);
uint32_t vsc_colour_pack_xrgb8888(VscColour32 c);


#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_COLOUR_H */

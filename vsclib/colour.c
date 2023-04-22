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
#include <vsclib/colour.h>

VscColour32 vsc_colour_unpack_rgb565(uint16_t pixel)
{
    return (VscColour32){
        .r = VSC_COLOUR_C5_TO_C8((pixel & 0xF800u) >> 11u),
        .g = VSC_COLOUR_C6_TO_C8((pixel & 0x07E0u) >> 5u),
        .b = VSC_COLOUR_C5_TO_C8((pixel & 0x001Fu) >> 0u),
        .a = 0xFFu,
    };
}

VscColour32 vsc_colour_unpack_argb1555(uint16_t pixel)
{
    return (VscColour32){
        .r = VSC_COLOUR_C5_TO_C8((pixel & 0x7C00u) >> 10u),
        .g = VSC_COLOUR_C5_TO_C8((pixel & 0x03E0u) >> 5u),
        .b = VSC_COLOUR_C5_TO_C8((pixel & 0x001Fu) >> 0u),
        .a = VSC_COLOUR_C1_TO_C8(pixel >> 15u),
    };
}

VscColour32 vsc_colour_unpack_xrgb1555(uint16_t pixel)
{
    return vsc_colour_unpack_argb1555(pixel | 0x8000u);
}

VscColour32 vsc_colour_unpack_argb8888(uint32_t pixel)
{
    return (VscColour32){
        .r = (uint8_t)((pixel & 0x00FF0000u) >> 16u),
        .g = (uint8_t)((pixel & 0x0000FF00u) >> 8u),
        .b = (uint8_t)((pixel & 0x000000FFu) >> 0u),
        .a = (uint8_t)((pixel & 0xFF000000u) >> 24u),
    };
}

VscColour32 vsc_colour_unpack_xrgb8888(uint32_t pixel)
{
    return vsc_colour_unpack_argb8888(pixel | 0xFF000000u);
}

uint16_t vsc_colour_pack_rgb565(VscColour32 c)
{
    return VSC_COLOUR_RGB565(c.r, c.g, c.b);
}

uint16_t vsc_colour_pack_argb1555(VscColour32 c)
{
    return VSC_COLOUR_ARGB1555(c.a, c.r, c.g, c.b);
}

uint16_t vsc_colour_pack_xrgb1555(VscColour32 c)
{
    return VSC_COLOUR_XRGB1555(c.r, c.g, c.b);
}

uint32_t vsc_colour_pack_argb8888(VscColour32 c)
{
    return VSC_COLOUR_ARGB8888(c.a, c.r, c.g, c.b);
}

uint32_t vsc_colour_pack_xrgb8888(VscColour32 c)
{
    return VSC_COLOUR_XRGB8888(c.r, c.g, c.b);
}

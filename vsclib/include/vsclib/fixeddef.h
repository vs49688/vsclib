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
#ifndef _VSCLIB_FIXEDDEF_H
#define _VSCLIB_FIXEDDEF_H

#include <stdint.h>

const static float VSC_X1616_ONE = 65536.0f; /* 1 << 16 */
const static float VSC_X2012_ONE =  4096.0f; /* 1 << 12 */
const static float VSC_X0412_ONE =  4096.0f; /* 1 << 12 */

/* Fixed-point types. Are structs to prevent simple assignment. */
typedef struct { int32_t v; } VscX1616;
typedef struct { int32_t v; } VscX2012;
typedef struct { int16_t v; } VscX0412;

#endif /* _VSCLIB_FIXEDDEF_H */

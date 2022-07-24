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
#ifndef _VSCLIB_TYPES_H
#define _VSCLIB_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include "vsclib_config.h"

#if defined(WIN32)
#   include <basetsd.h>
typedef SSIZE_T   vsc_ssize_t;
#else
#   include <sys/types.h>
typedef ssize_t   vsc_ssize_t;
#endif

#if defined(VSC_HAVE_MAX_ALIGN_T)
typedef max_align_t vsc_max_align_t;
#else
typedef union {
    char       *p;
    double      d;
    long double ld;
    long int    i;
} vsc_max_align_t;
#endif

#endif /* _VSCLIB_TYPES_H */
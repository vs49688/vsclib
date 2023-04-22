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
/** \file vsclib/error.h */
#ifndef _VSCLIB_ERROR_H
#define _VSCLIB_ERROR_H

#include <assert.h>
#include <errno.h>
#include "macros.h"

#if EINVAL < 0
#define VSC_ERROR(x) (-(-(x & 0xFFFF)))
#else
#define VSC_ERROR(x) (-(x & 0xFFFF))
#endif

#define VSC_UNERROR(err)         (-(err)&0xFFFF)

#define VSC_MKERR(b, c, d)       (-(int)VSC_FOURCCBE(' ', b, c, d))

#define VSC_ERROR_IS_SYSTEM(err) (!(-(err)&VSC_MKERR(0, 0, 0)))

/** \brief Success. */
#define VSC_ERROR_SUCCESS 0

/** \brief End of file. */
#define VSC_ERROR_EOF VSC_MKERR('E', 'O', 'F')

/** \brief Stack overflow. */
#define VSC_ERROR_STACKOFLOW VSC_MKERR('S', 'O', 'F')

/** \brief Stack underflow. */
#define VSC_ERROR_STACKUFLOW VSC_MKERR('S', 'U', 'F')

/** \brief Unknown error. */
#define VSC_ERROR_UNKNOWN VSC_MKERR('U', 'N', 'K')

/** \brief This is a bug. */
#define VSC_ERROR_BUG VSC_MKERR('B', 'U', 'G')

static_assert(VSC_ERROR(0xFFFF) < 0, "VSC_ERROR() >= 0");
static_assert(VSC_MKERR(0xFF, 0xFF, 0xFF) < 0, "VSC_MKERR() >= 0");
static_assert(VSC_ERROR(0x1FFFF) == VSC_ERROR(0xFFFF), "VSC_ERROR(0x1FFFF) != VSC_ERROR(0xFFFF)");
static_assert(VSC_ERROR_IS_SYSTEM(VSC_ERROR(0xFFFF)), "!VSC_ERROR_IS_SYSTEM(VSC_ERROR()");
static_assert(!VSC_ERROR_IS_SYSTEM(VSC_MKERR(0xFF, 0xFF, 0xFF)), "VSC_ERROR_IS_SYSTEM(VSC_MKERR()");

#endif /* _VSCLIB_ERROR_H */

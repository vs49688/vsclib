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
#ifndef _VSCLIB_PLATFORM_H
#define _VSCLIB_PLATFORM_H

/*
 * This file is mainly so consumers don't need to
 * include vsclib_config.h
 */
#include "vsclib_config.h"

#if defined(_WIN32)
#	define VSC_DEVNULL "NUL"
#else
#	define VSC_DEVNULL "/dev/null"
#endif

/* CMake sets NDEBUG this in all non-debug builds, so just use that. */
#if !defined(VSC_DEBUG) && !defined(NDEBUG)
#   define VSC_DEBUG
#endif

#endif /* _VSCLIB_PLATFORM_H */
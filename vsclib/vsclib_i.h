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
#ifndef _VSCLIB_I_H
#define _VSCLIB_I_H

#include <errno.h>

#include "vsclib.h"

extern const VscAllocator vsclib_system_allocator;

#define vsci_xalloc vsc_xalloc
#define vsci_xfree vsc_xfree
#define vsci_xrealloc vsc_xrealloc

#ifndef EOPNOTSUPP
#   error EOPNOTSUPP not defined, please fix your system.
#endif


#endif /* _VSCLIB_I_H */

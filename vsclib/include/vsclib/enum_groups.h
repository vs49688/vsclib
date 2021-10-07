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
#ifndef _VSCLIB_POSIXUTIL_H
#define _VSCLIB_POSIXUTIL_H

#include "memdef.h"

struct passwd;
struct group;

typedef int(*VscEnumGroupsProc)(const struct group *grp, void *user);

#if defined(__cplusplus)
extern "C" {
#endif

int vsc_enum_groups(struct passwd *passwd, VscEnumGroupsProc proc, void *user);
int vsc_enum_groupsa(struct passwd *passwd, VscEnumGroupsProc proc, void *user, const VscAllocator *a);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_POSIXUTIL_H */
/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Zane van Iperen (zane@zanevaniperen.com)
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
/** \file vsclib/uuid.h */
#ifndef _VSCLIB_UUID_H
#define _VSCLIB_UUID_H

#include "uuiddef.h"

#if defined(__cplusplus)
extern "C" {
#endif

VscUUID vsc_uuid_nil(void);
VscUUID vsc_uuid_max(void);
int     vsc_uuid_compare(const VscUUID *a, const VscUUID *b);
char   *vsc_uuid_format(char *dst, const VscUUID *uuid, uint32_t flags);
int     vsc_uuid_parse(VscUUID *uuid, const char *s);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_UUID_H */

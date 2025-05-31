/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2025 Zane van Iperen (zane@zanevaniperen.com)
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
#ifndef _VSCLIB_RESOURCE_H
#define _VSCLIB_RESOURCE_H

#include <stdarg.h>
#include "platform.h"
#include "memdef.h"
#include "resourcedef.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*VscResourceDestructorProc)(void *p, size_t size, size_t alignment);
typedef int VscResourceEnumProc(void *res, void *user);

/**
 * \brief Get a VscAllocator instance that parents anything allocated to \p res.
 *
 * \param res[in] A pointer to the resource. Must not be NULL.
 *
 * \returns A VscAllocator instance that parents anything allocated to \p res.
 *
 * \remark This function is designed to act as a bridge between the resource system
 *  and everything else allocator-aware.
 */
VscAllocator vsc_res_allocator(void *res);

void  *vsc_res_alloc(void *parent, size_t size);
void   vsc_res_free(void *res);
void  *vsc_res_add(void *parent, void *res);
void  *vsc_res_remove(void *res);
size_t vsc_res_size(void *res);
size_t vsc_res_size_total(void *res);
void   vsc_res_set_destructor(void *res, VscResourceDestructorProc proc);
int    vsc_res_enum_children(void *res, VscResourceEnumProc proc, void *user);

void *vsc_res_calloc(void *parent, size_t nmemb, size_t size);
char *vsc_res_strdup(void *parent, const char *s);
char *vsc_res_strdupr(void *parent, const char *start, const char *end);
char *vsc_res_vasprintf(void *parent, const char *fmt, va_list ap);
char *vsc_res_asprintf(void *parent, const char *fmt, ...) VSC_PRINTF_ATTRIB(2, 3);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_RESOURCE_H */

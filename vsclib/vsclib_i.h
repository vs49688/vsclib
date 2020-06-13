/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2020 Zane van Iperen (zane@zanevaniperen.com)
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

#include "vsclib.h"

extern vsc_allocator_t vsclib_system_allocator;

/* stdio.c */
#if defined(WIN32)
typedef __int64 vsc_off_t;
#else
typedef off_t vsc_off_t;
#endif

int			vsci_fileno(FILE *stream);
int			vsci_open(const char *pathname, int flags);
int			vsci_dup2(int oldfd, int newfd);
int			vsci_close(int fd);
vsc_off_t	vsci_ftello(FILE *stream);
int			vsci_fseeko(FILE *stream, vsc_off_t offset, int whence);

void		*vsci_xalloc(const vsc_allocator_t *a, size_t size);
void		vsci_xfree(const vsc_allocator_t *a, void *p);
void		*vsci_xrealloc(const vsc_allocator_t *a, void *ptr, size_t size);

#endif /* _VSCLIB_I_H */

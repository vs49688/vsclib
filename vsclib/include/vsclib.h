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
 *
 * vsclib public header.
 *
 * Try to limit types to C/POSIX types only.
 * Obvious exceptions are:
 * - VscAllocator
 * - VscEnumGroupsProc
 * - VscForEachDelimProc
 */
#ifndef _VSCLIB_H
#define _VSCLIB_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "vsclib_config.h"
#include "vsclib/mem.h"
#include "vsclib/io.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* getline.c */
vsc_ssize_t vsc_getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
vsc_ssize_t vsc_getdelima(char **lineptr, size_t *n, int delim, FILE *stream, const VscAllocator *a);

vsc_ssize_t vsc_getline(char **lineptr, size_t *n, FILE *stream);
vsc_ssize_t vsc_getlinea(char **lineptr, size_t *n, FILE *stream, const VscAllocator *a);

/* for_each.c */
typedef int (*VscForEachDelimProc)(const char *s, const char *e, void *user);

/**
 * @brief Invoke a procedure for each element between successive @par delim characters.
 *
 * If @par proc returns a non-zero value, iteration is immediately aborted and the value
 * returned.
 *
 * @param begin The begin pointer.
 * @param end The after-the-end pointer.
 * @param delim The delimiter.
 * @param proc A procedure to be invoked for each element between @par delim.
 * @param user A user pointer that is passed to @par proc.
 * @return If @par proc returns a non-zero value, that value is returned. Otherwise 0.
 */
int vsc_for_each_delim(const char *begin, const char *end, char delim, VscForEachDelimProc proc, void *user);

/* searchpath.c */
char *vsc_searchpath(const char *f, size_t *len);
char *vsc_searchpatha(const char *f, size_t *len, const VscAllocator *a);

/* vsprintfa.c */
char *vsc_asprintf(const char *fmt, ...);
char *vsc_asprintfa(const VscAllocator *a, const char *fmt, ...);

char *vsc_vasprintf(const char *fmt, va_list ap);
char *vsc_vasprintfa(const VscAllocator *a, const char *fmt, va_list ap);

/* string.c */
char *vsc_strdupr(const char *start, const char *end);
char *vsc_strdupra(const char *start, const char *end, const VscAllocator *a);

/* chdir.c */
int vsc_chdir(const char *path);
int vsc_chdira(const char *path, const VscAllocator *a);

/* enum_groups.c */
struct passwd;
struct group;

typedef int(*VscEnumGroupsProc)(const struct group *grp, void *user);

int vsc_enum_groups(struct passwd *passwd, VscEnumGroupsProc proc, void *user);
int vsc_enum_groupsa(struct passwd *passwd, VscEnumGroupsProc proc, void *user, const VscAllocator *a);

#if defined(_WIN32)
wchar_t *vsc_cstrtowstr(const char *s, size_t *len, unsigned int cp);
wchar_t *vsc_cstrtowstra(const char *s, size_t *len, unsigned int cp, const VscAllocator *a);

char *vsc_wstrtocstr(const wchar_t *ws, size_t *len, unsigned int cp);
char *vsc_wstrtocstra(const wchar_t *ws, size_t *len, unsigned int cp, const VscAllocator *a);
#endif

#if defined(__cplusplus)
}
#endif
#endif /* _VSCLIB_H */

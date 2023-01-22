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
#ifndef _VSCLIB_STRING_H
#define _VSCLIB_STRING_H

#include <stdio.h>
#include "platform.h"
#include "types.h"
#include "memdef.h"
#include "stringdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

vsc_ssize_t vsc_getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
vsc_ssize_t vsc_getdelima(char **lineptr, size_t *n, int delim, FILE *stream, const VscAllocator *a);

vsc_ssize_t vsc_getline(char **lineptr, size_t *n, FILE *stream);
vsc_ssize_t vsc_getlinea(char **lineptr, size_t *n, FILE *stream, const VscAllocator *a);

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

char *vsc_asprintf(const char *fmt, ...) VSC_PRINTF_ATTRIB(1, 2);
char *vsc_asprintfa(const VscAllocator *a, const char *fmt, ...) VSC_PRINTF_ATTRIB(2, 3);

char *vsc_vasprintf(const char *fmt, va_list ap);
char *vsc_vasprintfa(const VscAllocator *a, const char *fmt, va_list ap);

char *vsc_strdup(const char *s);
char *vsc_strdupa(const char *s, const VscAllocator *a);

char *vsc_strdupr(const char *start, const char *end);
char *vsc_strdupra(const char *start, const char *end, const VscAllocator *a);

#if defined(_WIN32)
int vsc_cstrtowstr(const char *s, unsigned int cp, wchar_t **ws, size_t *len);
int vsc_cstrtowstra(const char *s, unsigned int cp, wchar_t **ws, size_t *len, const VscAllocator *a);

int vsc_wstrtocstr(const wchar_t *ws, unsigned int cp, char **s, size_t *len);
int vsc_wstrtocstra(const wchar_t *ws, unsigned int cp, char **s, size_t *len, const VscAllocator *a);
#endif

char *vsc_stpcpy(char *dst, const char *src);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_STRING_H */
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
#include <vsclib/platform.h>
#if VSC_HAVE_STPCPY || VSC_HAVE_STRCPY
#include <string.h>
#endif
#include <vsclib/string.h>

char *vsc_stpcpy(char *dst, const char *src)
{
#if VSC_HAVE_STPCPY
    return stpcpy(dst, src);
#elif VSC_HAVE_STRCPY
    return strcpy(dst, src) + strlen(src);
#else
    for(; (*dst = *src); src++, dst++)
        ;
    return dst;
#endif
}

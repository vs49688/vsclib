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

#define _XOPEN_SOURCE 600
#undef _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <vsclib/error.h>

static int vsc_isprint(int c)
{
    return (unsigned)c - 0x20 < 0x5f;
}

int vsc_vfperror(FILE *fp, int err, const char *fmt, va_list ap)
{
    char        errbuf[128];
    const char *errmsg;
    int         r;

    if(vfprintf(fp, fmt, ap) < 0)
        return VSC_ERROR(errno);

    errmsg = NULL;
    if(VSC_ERROR_IS_SYSTEM(err)) {
#if defined(_WIN32)
        if((errno = strerror_s(errbuf, sizeof(errbuf), errno)) != 0)
            return VSC_ERROR(errno);
#else
        if(strerror_r(VSC_UNERROR(err), errbuf, sizeof(errbuf)) < 0)
            return VSC_ERROR(errno);
#endif
        errmsg = errbuf;
    } else
        switch(err) {
            case VSC_ERROR_SUCCESS:
                errmsg = "Success";
                break;
            case VSC_ERROR_EOF:
                errmsg = "End of file";
                break;
            case VSC_ERROR_STACKOFLOW:
                errmsg = "Stack overflow";
                break;
            case VSC_ERROR_STACKUFLOW:
                errmsg = "Stack underflow";
                break;
            default:
                break;
        }

    if(errmsg != NULL) {
        r = fprintf(fp, ": %s\n", errmsg);
    } else {
        unsigned char c[4] = {
            [0] = ((-err) >> 24) & 0xFF,
            [1] = ((-err) >> 16) & 0xFF,
            [2] = ((-err) >> 8) & 0xFF,
            [3] = ((-err) >> 0) & 0xFF,
        };

        if(!vsc_isprint(c[0]) || !vsc_isprint(c[1]) || !vsc_isprint(c[2]) || !vsc_isprint(c[3])) {
            r = fprintf(fp, ": Unknown error 0x%08x\n", -err);
        } else {
            r = fprintf(fp, ": Unknown error '%c%c%c%c'\n", c[0], c[1], c[2], c[3]);
        }
    }

    if(r < 0)
        return VSC_ERROR(EIO);

    return 0;
}

int vsc_fperror(FILE *fp, int err, const char *fmt, ...)
{
    int     r;
    va_list ap;
    va_start(ap, fmt);
    r = vsc_vfperror(fp, err, fmt, ap);
    va_end(ap);
    return r;
}

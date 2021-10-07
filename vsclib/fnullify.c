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
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <vsclib/error.h>
#include <vsclib/io.h>

int vsc_fnullify(FILE *f)
{
    int fd, devnull, r;

    if(f == NULL)
        return VSC_ERROR(EINVAL);

    /* If fmemopen()'d */
    if((fd = vsc_fileno(f)) < 0)
        return fd;

    if((devnull = vsc_open(VSC_DEVNULL, O_RDWR)) < 0)
        return devnull;

    if((r = vsc_dup2(devnull, fd)) < 0) {
        /* What do I actually do if this fails? */
        (void)vsc_close(devnull);
        return r;
    }

    return fd;
}

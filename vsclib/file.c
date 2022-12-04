/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2022 Zane van Iperen (zane@zanevaniperen.com)
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

#include <stddef.h>
#include <vsclib/file_internal.h>

int vscf_read(VscFile *fp, void *restrict ptr, size_t size, size_t nmemb, size_t *nread)
{
    return ((VscFileProcs *)fp->_reserved)->read(fp, ptr, size, nmemb, nread);
}

int vscf_write(VscFile *fp, const void *restrict ptr, size_t size, size_t nmemb, size_t *nwritten)
{
    return ((VscFileProcs *)fp->_reserved)->write(fp, ptr, size, nmemb, nwritten);
}

int vscf_seek(VscFile *fp, vsc_off_t offset, VscFileWhence whence)
{
    return ((VscFileProcs *)fp->_reserved)->seek(fp, offset, whence);
}

vsc_off_t vscf_tell(VscFile *fp)
{
    return ((VscFileProcs *)fp->_reserved)->tell(fp);
}

int vscf_flush(VscFile *fp)
{
    return ((VscFileProcs *)fp->_reserved)->flush(fp);
}

int vscf_close(VscFile *fp)
{
    return ((VscFileProcs *)fp->_reserved)->close(fp);
}

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
#include <string.h>
#include <vsclib/error.h>
#include <vsclib/assert.h>
#include <vsclib/string.h>
#include <vsclib/uuid.h>

static const char *alphabet_lower = "0123456789abcdef";
static const char *alphabet_upper = "0123456789ABCDEF";

VscUUID vsc_uuid_nil(void)
{
    VscUUID uuid;
    memset(uuid.v, 0, VSC_UUID_SIZE);
    return uuid;
}

VscUUID vsc_uuid_max(void)
{
    VscUUID uuid;
    memset(uuid.v, 0xFF, VSC_UUID_SIZE);
    return uuid;
}

int vsc_uuid_compare(const VscUUID *a, const VscUUID *b)
{
    return memcmp(a->v, b->v, VSC_UUID_SIZE);
}

char *vsc_uuid_format(char *dst, const VscUUID *uuid, uint32_t flags)
{
    const char *alphabet = alphabet_lower;
    char       *s        = dst;

    if(flags & VSC_UUID_FORMAT_UPPER)
        alphabet = alphabet_upper;

    if(flags & VSC_UUID_FORMAT_MS)
        *s++ = '{';

    for(int i = 0; i < VSC_UUID_SIZE; ++i) {
        uint8_t ch = uuid->v[i];

        if(((flags & VSC_UUID_FORMAT_COMPACT) == 0) && (i == 4 || i == 6 || i == 8 || i == 10))
            *s++ = '-';

        *s++ = alphabet[ch >> 4];
        *s++ = alphabet[ch & 0xF];
    }

    if(flags & VSC_UUID_FORMAT_MS)
        *s++ = '}';

    *s++ = '\0';
    return dst;
}

static uint8_t denibble(char c)
{
    vsc_assert(vsc_isxdigit(c));

    if(vsc_isdigit(c))
        return c - '0';

    if(c >= 'a')
        return 0xA + c - 'a';

    return 0xA + c - 'A';
}

int vsc_uuid_parse(VscUUID *uuid, const char *s)
{
    size_t   len;
    int      has_braces, has_dashes;
    uint8_t *u;

    if(uuid == NULL || s == NULL)
        return VSC_ERROR(EINVAL);

    len        = strlen(s);
    has_braces = len == 34 || len == 38;

    /*
     * Validate and ignore leading/trailing braces.
     */
    if(has_braces) {
        if(s[0] != '{' || s[len - 1] != '}')
            return VSC_ERROR(EINVAL);

        ++s;
        len -= 2;
    }

    if(len != 32 && len != 36)
        return VSC_ERROR(EINVAL);

    has_dashes = len == 36;

    /*
     * Parse - read the nibbles two-at-a-time.
     */
    u = uuid->v;
    for(size_t i = 0; i < len; ++i) {
        char c1, c2;
        if(has_dashes && (i == 8 || i == 13 || i == 18 || i == 23)) {
            if(s[i] != '-')
                return VSC_ERROR(EINVAL);

            continue;
        }

        c1 = s[i++];
        c2 = s[i];

        if(!vsc_isxdigit(c1) || !vsc_isxdigit(c2))
            return VSC_ERROR(EINVAL);

        *u++ = denibble(c1) << 4 | denibble(c2);
    }

    return 0;
}

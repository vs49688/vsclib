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
#include <system_error>
#include "vscpplib.hpp"

static void *_malloc(size_t size, void *user) noexcept
{
    std::wstring *t = reinterpret_cast<std::wstring*>(user);
    try {
        t->resize(size);
    } catch(...) {
        errno = ENOMEM;
        return nullptr;
    }

    return t->data();
}

static void _free(void *p, void *user) noexcept
{
    std::wstring *t = reinterpret_cast<std::wstring*>(user);
    int errno_ = errno;
    t->clear();
    errno = errno_;
}

static void *_realloc(void *ptr, size_t size, void *user) noexcept
{
    if(ptr != nullptr && size == 0) {
        _free(ptr, user);
        return nullptr;
    }

    return _malloc(size, user);
}

static vsc_allocator_t vscpplib_wstring_allocator = {
	.alloc = _malloc,
	.free = _free,
	.realloc = _realloc,
	.user = nullptr,
};

wchar_t *vsc_cstrtowstra(const char *s, size_t *len, unsigned int cp, const vsc_allocator_t *a);


template<typename T = std::wstring>
T cstrtowstra(const char *s, unsigned int cp)
{
    T ws;
    vsc_allocator_t a = vscpplib_wstring_allocator;
    a.user = &ws;

    wchar_t *ss = vsc_cstrtowstra(s, nullptr, cp, &a);
    if(ss == nullptr) {
        if(errno == ENOMEM)
            throw std::bad_alloc();
        else
            throw std::system_error(std::error_code(errno, std::system_category()));
    }

    return ws;
}

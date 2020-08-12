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
#ifndef _VSCPPLIB_HPP
#define _VSCPPLIB_HPP

#include <type_traits>
#include <algorithm>
#include <string_view>

#include <vsclib.h>

namespace vsc {

/* NB: Relying on the compiler to inline these. */

using allocator_t        = vsc_allocator_t;

const auto fopen         = ::vsc_fopen;
const auto fopena        = ::vsc_fopena;

const auto freadall      = ::vsc_freadall;
const auto freadalla     = ::vsc_freadalla;

const auto fnullify      = ::vsc_fnullify;

const auto searchpath    = ::vsc_searchpath;
const auto searchpatha   = ::vsc_searchpatha;

const auto asprintf      = ::vsc_asprintf;
const auto asprintfa     = ::vsc_asprintfa;

const auto vasprintf     = ::vsc_vasprintf;
const auto vasprintfa    = ::vsc_vasprintfa;

#if _XOPEN_SOURCE > 500
using enum_groups_proc_t = ::vsc_enum_groups_proc_t;

const auto enum_groups   = ::vsc_enum_groups;
const auto enum_groupsa  = ::vsc_enum_groupsa;
#endif

#if defined(_WIN32)
const auto cstrtowstr    = ::vsc_cstrtowstr;
const auto cstrtowstra   = ::vsc_cstrtowstra;

const auto wstrtocstr    = ::vsc_wstrtocstr;
const auto wstrtocstra   = ::vsc_wstrtocstra;
#endif

const auto swap_uint16   = ::vsc_swap_uint16;
const auto swap_uint32   = ::vsc_swap_uint32;
const auto swap_uint64   = ::vsc_swap_uint64;

const auto swap_int16    = ::vsc_swap_int16;
const auto swap_int32    = ::vsc_swap_int32;
const auto swap_int64    = ::vsc_swap_int64;

#define VSCPPLIB_DECLARE_IO_X2Y2X(type, c1, c2) \
	const auto c1##_to_##c2  = vsc_##c1##_to_##c2; \
	const auto c2##_to_##c1  = vsc_##c2##_to_##c1;

#define VSCPPLIB_DECLARE_XXX(bits) \
	VSCPPLIB_DECLARE_IO_X2Y2X( int##bits##_t, le ##bits, native) \
    VSCPPLIB_DECLARE_IO_X2Y2X(uint##bits##_t, leu##bits, native) \
    VSCPPLIB_DECLARE_IO_X2Y2X( int##bits##_t, be ##bits, native) \
    VSCPPLIB_DECLARE_IO_X2Y2X(uint##bits##_t, beu##bits, native)

VSCPPLIB_DECLARE_XXX(16)
VSCPPLIB_DECLARE_XXX(32)
VSCPPLIB_DECLARE_XXX(64)

#undef VSCPPLIB_DECLARE_XXX
#undef VSCPPLIB_DECLARE_IO_X2Y2X

#define VSCPPLIB_DECLARE_READWRITE(bits) \
	const auto write_uint##bits = ::vsc_write_uint##bits; \
	const auto write_int##bits  = ::vsc_write_int##bits; \
	const auto read_uint##bits  = ::vsc_read_uint##bits; \
	const auto read_int##bits   = ::vsc_read_int##bits;

VSCPPLIB_DECLARE_READWRITE(8);
VSCPPLIB_DECLARE_READWRITE(16);
VSCPPLIB_DECLARE_READWRITE(32);
VSCPPLIB_DECLARE_READWRITE(64);

#undef VSCPPLIB_DECLARE_READWRITE

/* TODO: {F}READWRITE_E */

template<
	typename V,
	typename CharT = char,
	typename InputIt = const CharT*,
	typename Traits = std::char_traits<CharT>,
	typename ViewT = std::basic_string_view<CharT, Traits>
>
bool for_each_delim(InputIt begin, InputIt end, CharT delim, V&& proc)
{
	for(InputIt start = begin, next; start != end; start = next)
	{
		if((next = std::find(start, end, delim)))
		{
			if constexpr(std::is_same_v<std::decay_t<std::invoke_result_t<V, ViewT, size_t>>, void>)
				proc(ViewT(start, std::distance(start, next)));
			else if(!proc(ViewT(start, std::distance(start, next))))
				return false;

			if(next != end)
				++next;
		}
	}

	return true;
}

}

#endif /* _VSCPPLIB_HPP */
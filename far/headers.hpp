#ifndef HEADERS_HPP_9A02D08B_02BB_4240_845F_36ED60ED2647
#define HEADERS_HPP_9A02D08B_02BB_4240_845F_36ED60ED2647
#pragma once

/*
headers.hpp

Стандартные заголовки
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "common/compiler.hpp"

#if !CHECK_COMPILER(CL, 19, 29, 30148)
#error Visual C++ 2019 Update 16.11.24 (or higher) required
#elif !CHECK_COMPILER(GCC, 12, 0, 0)
#error GCC 12.0.0 (or higher) required
#elif !CHECK_COMPILER(CLANG, 16, 0, 0)
#error Clang 16.0.0 (or higher) required
#endif

#ifdef __GNUC__
// Current implementation of wcschr etc. in gcc removes const from returned pointer. Issue has been opened since 2007.
// These semi-magical defines and appropriate overloads in cpp.hpp are intended to fix this madness.

// Force C version to return const
#undef _CONST_RETURN
#define _CONST_RETURN const
// Disable broken inline overloads
#define __CORRECT_ISO_CPP_WCHAR_H_PROTO
// Enable inline overloads in common/cpp.hpp
#define FAR_ENABLE_CORRECT_ISO_CPP_WCHAR_H_OVERLOADS
#endif

#include "disable_warnings_in_std_begin.hpp"
//----------------------------------------------------------------------------

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <bit>
#include <bitset>
#include <chrono>
#include <compare>
#include <concepts>
#include <exception>
#include <forward_list>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <queue>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <regex>
#include <set>
#include <span>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <cassert>
#include <cctype>
#include <cfloat>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>

//----------------------------------------------------------------------------
#include "disable_warnings_in_std_end.hpp"

#include "common/cpp.hpp"

using string = std::wstring;
using string_view = std::wstring_view;
using size_t = std::size_t;

inline namespace literals
{
	using namespace std::literals;
}

// Basic sanity checks to avoid surprizes

// char must be unsigned: we use it for indexing in a few places
static_assert(std::unsigned_integral<char>);

// wchar_t must be 16-bit: we have lookup tables
static_assert(sizeof(wchar_t) == 2);

// C and by extension C++ standard define it as uint_least16_t, which is insane.
static_assert(sizeof(char16_t) == 2);

// C and by extension C++ standard define it as uint_least32_t, which is insane.
static_assert(sizeof(char32_t) == 4);

// source encoding must be UTF-8, we use various special characters directly
static_assert("𠜎"sv == "\xF0\xA0\x9C\x8E"sv);


namespace features
{
	constexpr auto
		mantis_698 = false,
		mantis_2562 = false,
		win10_curdir = false,

		reserved = false;

}

// BUGBUG remove
#define PRECOMPILE_PLATFORM_HEADERS
#ifdef PRECOMPILE_PLATFORM_HEADERS
#include "platform.headers.hpp"
#endif

#endif // HEADERS_HPP_9A02D08B_02BB_4240_845F_36ED60ED2647

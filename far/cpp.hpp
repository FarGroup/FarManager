#ifndef CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
#define CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
#pragma once

/*
cpp.hpp

Some workarounds & emulations for C++ features, missed in currently used compilers & libraries.

Here be dragons
*/
/*
Copyright © 2013 Far Group
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

// Internal:
#include "common/compiler.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

#if COMPILER(GCC)
// These inline implementations in gcc/cwchar are wrong and non-compilable if _CONST_RETURN is defined.
namespace std
{
	inline wchar_t* wcschr(wchar_t* p, wchar_t c)
	{
		return const_cast<wchar_t*>(wcschr(const_cast<const wchar_t*>(p), c));
	}

	inline wchar_t* wcspbrk(wchar_t* s1, const wchar_t* s2)
	{
		return const_cast<wchar_t*>(wcspbrk(const_cast<const wchar_t*>(s1), s2));
	}

	inline wchar_t* wcsrchr(wchar_t* p, wchar_t c)
	{
		return const_cast<wchar_t*>(wcsrchr(const_cast<const wchar_t*>(p), c));
	}

	inline wchar_t* wcsstr(wchar_t* s1, const wchar_t* s2)
	{
		return const_cast<wchar_t*>(wcsstr(const_cast<const wchar_t*>(s1), s2));
	}

	inline wchar_t* wmemchr(wchar_t* p, wchar_t c, size_t n)
	{
		return const_cast<wchar_t*>(wmemchr(const_cast<const wchar_t*>(p), c, n));
	}
}

using std::wcschr;
using std::wcspbrk;
using std::wcsrchr;
using std::wcsstr;
using std::wmemchr;

#endif

#if COMPILER(GCC) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace std::this_thread
{
	inline void yield() noexcept
	{
		Sleep(0);
	}
}
#endif


#ifndef __cpp_lib_erase_if
namespace std
{
	namespace detail
	{
		template<typename container, typename predicate>
		void associative_erase_if(container& Container, const predicate& Predicate)
		{
			for (auto i = Container.begin(), End = Container.end(); i != End; )
			{
				if (Predicate(*i))
				{
					i = Container.erase(i);
				}
				else
				{
					++i;
				}
			}
		}
	}

	template <typename predicate, typename... traits>
	void erase_if(std::set<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::multiset<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::map<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::multimap<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::unordered_set<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::unordered_multiset<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::unordered_map<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

	template <typename predicate, typename... traits>
	void erase_if(std::unordered_multimap<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }
}
#endif

#if COMPILER(CLANG) && IS_MICROSOFT_SDK() && defined __cpp_char8_t && !defined __cpp_lib_char8_t
namespace std
{
	inline namespace literals
	{
		inline namespace string_view_literals
		{
WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wuser-defined-literals")
			[[nodiscard]]
			constexpr basic_string_view<char8_t> operator"" sv(const char8_t* const Str, size_t const Size) noexcept
			{
				return { Str, Size };
			}
WARNING_POP()
		}
	}
}
#endif

#endif // CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD

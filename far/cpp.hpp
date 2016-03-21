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

#include "common/compiler.hpp"

#if COMPILER == C_GCC
namespace std
{
	template <class C>
	constexpr auto size(const C& c)
	{
		return c.size();
	}

	template <class T, size_t N>
	constexpr auto size(const T (&array)[N]) noexcept
	{
		return N;
	}


	template <class C>
	constexpr auto empty(const C& c)
	{
		return c.empty();
	}

	template <class T, size_t N>
	constexpr auto empty(const T (&array)[N])
	{
		return false;
	}

	template <class E>
	constexpr auto empty(initializer_list<E> il) noexcept
	{
		return !il.size();
	}


	template <class C>
	constexpr auto data(C& c)
	{
		return c.data();
	}

	template <class C>
	constexpr auto data(const C& c)
	{
		return c.data();
	}

	template <class T, size_t N>
	constexpr T* data(T (&array)[N]) noexcept
	{
		return array;
	}

	template <class E>
	constexpr const E* data(initializer_list<E> il) noexcept
	{
		return il.begin();
	}
}
#endif

#endif // CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD

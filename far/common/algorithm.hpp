﻿#ifndef ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
#define ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
#pragma once

/*
algorithm.hpp
*/
/*
Copyright © 2014 Far Group
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

#include "preprocessor.hpp"
#include "type_traits.hpp"

#include <stdexcept>

//----------------------------------------------------------------------------

template<class T>
void repeat(size_t count, const T& f)
{
	for(size_t i = 0; i != count; ++i)
	{
		f();
	}
}

template<typename Iter1, typename Iter2>
void apply_permutation(Iter1 first, Iter1 last, Iter2 indices)
{
	using difference_type = std::iter_value_t<Iter2>;
	const difference_type length = std::distance(first, last);
	for (difference_type i = 0; i < length; ++i)
	{
		auto current = i;
		while (i != indices[current])
		{
			const auto next = indices[current];
			if (next < 0 || next >= length)
			{
				indices[i] = next;
				throw std::range_error("Invalid index in permutation");
			}
			if (next == current)
			{
				indices[i] = next;
				throw std::range_error("Not a permutation");
			}
			using std::swap;
			swap(first[current], first[next]);
			indices[current] = current;
			current = next;
		}
		indices[current] = current;
	}
}

// Unified container emplace
template<typename container, typename... args>
void emplace(container& Container, args&&... Args)
{
	if constexpr (requires { Container.emplace_hint(Container.end(), *Container.begin()); })
		Container.emplace_hint(Container.end(), FWD(Args)...);
	else
		Container.emplace(Container.end(), FWD(Args)...);
}

// uniform "contains"
template<typename element>
[[nodiscard]]
constexpr bool contains(range_like auto const& Range, const element& Element)
{
	if constexpr (requires { Range.contains(*Range.begin()); })
	{
		return Range.contains(Element);
	}
	else
	{
		// everything else
		const auto End = std::cend(Range);
		return std::find(std::cbegin(Range), End, Element) != End;
	}
}

template<typename min_type, typename value_type, typename max_type>
constexpr bool in_closed_range(min_type const& Min, value_type const& Value, max_type const& Max)
{
	return Min <= Value && Value <= Max;
}

template<typename arg, typename... args>
constexpr bool any_of(arg const& Arg, args const... Args)
{
	static_assert(sizeof...(Args) > 0);

	return (... || (Arg == Args));
}

template<typename... args>
constexpr bool none_of(args const... Args)
{
	return !any_of(Args...);
}

#endif // ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0

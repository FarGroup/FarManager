#ifndef ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
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

#include <ranges>
#include <stdexcept>

//----------------------------------------------------------------------------

void repeat(size_t const count, const auto& f)
{
	for(size_t i = 0; i != count; ++i)
	{
		f();
	}
}

void apply_permutation(std::ranges::random_access_range auto& Range, std::random_access_iterator auto const indices)
{
	auto first = std::ranges::begin(Range);
	auto last = std::ranges::end(Range);
	using index_type = std::iter_value_t<decltype(indices)>;

	for (index_type i = 0, length = static_cast<index_type>(last - first); i != length; ++i)
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

			std::ranges::swap(first[current], first[next]);
			indices[current] = current;
			current = next;
		}
		indices[current] = current;
	}
}

// Unified container emplace
void emplace(auto& Container, auto&&... Args)
{
	if constexpr (requires { Container.emplace_hint(Container.end(), *Container.begin()); })
		Container.emplace_hint(Container.end(), FWD(Args)...);
	else
		Container.emplace(Container.end(), FWD(Args)...);
}

// uniform "contains"
[[nodiscard]]
constexpr bool contains(std::ranges::range auto const& Range, const auto& Element)
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

constexpr bool in_closed_range(auto const& Min, auto const& Value, auto const& Max)
{
	return Min <= Value && Value <= Max;
}

constexpr bool any_of(auto const& Arg, auto const&... Args)
{
	static_assert(sizeof...(Args) > 0);

	return (... || (Arg == Args));
}

constexpr bool none_of(auto const&... Args)
{
	return !any_of(Args...);
}

#endif // ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0

#ifndef UTILITY_HPP_D8E934C7_BF30_4CEB_B80C_6E508DF7A1BC
#define UTILITY_HPP_D8E934C7_BF30_4CEB_B80C_6E508DF7A1BC
#pragma once

/*
utility.hpp
*/
/*
Copyright © 2017 Far Group
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

template<typename container>
void reserve_exp_noshrink(container& Container, size_t Capacity)
{
	// Unlike vector, string is allowed to shrink (another splendid design decision from the committee):
	// "Calling reserve() with a res_arg argument less than capacity() is in effect a non-binding shrink request." (21.4.4 basic_string capacity)
	// gcc decided to go mental and made that a _binding_ shrink request.
	const auto CurrentCapacity = Container.capacity();
	if (Capacity <= CurrentCapacity)
		return;

	// For vector reserve typically allocates exactly the requested amount instead of exponential growth.
	// This can be really bad if called in a loop.
	Capacity = std::max(static_cast<size_t>(CurrentCapacity * 1.5), Capacity);

	Container.reserve(Capacity);
}


template<class T>
void clear_and_shrink(T& container)
{
	T Tmp;
	using std::swap;
	swap(container, Tmp);
}


template<class T>
void node_swap(T& Container, const typename T::const_iterator& a, const typename T::const_iterator& b)
{
	const auto NextA = std::next(a), NextB = std::next(b);
	Container.splice(NextA, Container, b);
	Container.splice(NextB, Container, a);
}


template<class T>
auto make_hash(const T& value)
{
	return std::hash<T>{}(value);
}


template<typename T>
constexpr auto as_unsigned(T Value)
{
	return static_cast<std::make_unsigned_t<T>>(Value);
}

template<typename T>
constexpr auto as_underlying_type(T Value)
{
	return static_cast<std::underlying_type_t<T>>(Value);
}


constexpr auto bit(size_t Number)
{
	return 1 << Number;
}


constexpr size_t aligned_size(size_t Size, size_t Alignment = MEMORY_ALLOCATION_ALIGNMENT)
{
	return (Size + (Alignment - 1)) & ~(Alignment - 1);
}

template<typename T, int Alignment = MEMORY_ALLOCATION_ALIGNMENT>
constexpr auto aligned_sizeof()
{
	return aligned_size(sizeof(T), Alignment);
}

namespace enum_helpers
{
	template<class O, class R = void, class T>
	constexpr auto operation(T a, T b)
	{
		return static_cast<std::conditional_t<std::is_same_v<R, void>, T, R>>(O()(as_underlying_type(a), as_underlying_type(b)));
	}
}

namespace detail
{
	template<class... args>
	struct overload_t;

	template<typename arg>
	struct overload_t<arg>: arg
	{
		using type = arg;
		using arg::operator();
	};

	template<typename arg, typename... args>
	struct overload_t<arg, args...>: arg, overload_t<args...>::type
	{
		using type = overload_t;

		explicit overload_t(arg&& Arg, args&&... Args):
			arg(FWD(Arg)),
			args(FWD(Args))...
		{
		}

		using arg::operator();
		using overload_t<args...>::type::operator();
	};
}

template<typename... args>
auto overload(args&&... Args)
{
	return detail::overload_t<args...>(FWD(Args)...);
}

#endif // UTILITY_HPP_D8E934C7_BF30_4CEB_B80C_6E508DF7A1BC

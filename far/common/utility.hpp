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

#include "function_traits.hpp"
#include "preprocessor.hpp"

//----------------------------------------------------------------------------

template<typename T>
class base: public T
{
protected:
	using T::T;
	using base_ctor = base;
	using base_type = T;
};

inline size_t grow_exp_noshrink(size_t const Current, size_t const Desired)
{
	// Unlike vector, string is allowed to shrink (another splendid design decision from the committee):
	// "Calling reserve() with a res_arg argument less than capacity() is in effect a non-binding shrink request." (21.4.4 basic_string capacity)
	// gcc decided to go mental and made that a _binding_ shrink request.
	if (Desired < Current)
		return Current;

	// For vector reserve typically allocates exactly the requested amount instead of exponential growth.
	// This can be really bad if called in a loop.
	return std::max(Current + Current / 2, Desired);
}

template<typename container>
void reserve_exp_noshrink(container& Container, size_t const DesiredCapacity)
{
	Container.reserve(grow_exp_noshrink(Container.capacity(), DesiredCapacity));
}

template<typename container>
void resize_exp_noshrink(container& Container, size_t const DesiredSize)
{
	Container.resize(grow_exp_noshrink(Container.size(), DesiredSize), {});
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
[[nodiscard]]
auto make_hash(const T& value)
{
	return std::hash<T>{}(value);
}

template<class type>
void hash_combine(size_t& Seed, const type& Value)
{
	const auto MagicValue =
#ifdef _WIN64
		// (sqrt(5) - 1) * 2^63
		11400714819323198485ull
#else
		// (sqrt(5) - 1) * 2^31
		2654435769ul
#endif
	;

	Seed ^= make_hash(Value) + MagicValue + (Seed << 6) + (Seed >> 2);
}

template<typename... args>
size_t hash_combine_all(const args&... Args)
{
	size_t Seed = 0;
	(..., hash_combine(Seed, Args));
	return Seed;
}


template<typename iterator>
[[nodiscard]]
size_t hash_range(iterator First, iterator Last)
{
	size_t Seed = 0;

	for (; First != Last; ++First)
	{
		hash_combine(Seed, *First);
	}

	return Seed;
}

template<typename iterator>
void hash_range(size_t& Seed, iterator First, iterator Last)
{
	for (; First != Last; ++First)
	{
		hash_combine(Seed, *First);
	}
}

template<typename T>
[[nodiscard]]
constexpr auto as_signed(T Value)
{
	return static_cast<std::make_signed_t<T>>(Value);
}

template<typename T>
[[nodiscard]]
constexpr auto as_unsigned(T Value)
{
	return static_cast<std::make_unsigned_t<T>>(Value);
}

template<typename T>
[[nodiscard]]
constexpr auto as_underlying_type(T Value)
{
	return static_cast<std::underlying_type_t<T>>(Value);
}

[[nodiscard]]
constexpr auto bit(size_t const Number)
{
	return 1ull << Number;
}

[[nodiscard]]
constexpr auto operator ""_bit(unsigned long long const Number)
{
	return bit(Number);
}

namespace flags
{
	template<typename value_type, typename flags_type>
	constexpr bool check_any(const value_type& Value, flags_type Bits)
	{
		return (Value & Bits) != 0;
	}

	template<typename value_type, typename flags_type>
	constexpr bool check_all(const value_type& Value, flags_type Bits)
	{
		return static_cast<flags_type>(Value & Bits) == Bits;
	}

	template<typename value_type, typename flags_type>
	constexpr void set(value_type& Value, flags_type Bits)
	{
		Value |= Bits;
	}

	template<typename value_type, typename flags_type>
	constexpr void clear(value_type& Value, flags_type Bits)
	{
		Value &= ~static_cast<value_type>(Bits);
	}

	template<typename value_type, typename flags_type>
	constexpr void change(value_type& Value, flags_type Bits, bool Set)
	{
		Set? bit_set(Value, Bits) : bit_clear(Value, Bits);
	}
}

[[nodiscard]]
constexpr size_t aligned_size(size_t Size, size_t Alignment = alignof(std::max_align_t))
{
	return (Size + (Alignment - 1)) & ~(Alignment - 1);
}

template<typename T, size_t Alignment = alignof(std::max_align_t)>
[[nodiscard]]
constexpr auto aligned_sizeof()
{
	return aligned_size(sizeof(T), Alignment);
}

namespace enum_helpers
{
	template<class O, class R = void, class T>
	[[nodiscard]]
	constexpr auto operation(T a, T b)
	{
		return static_cast<std::conditional_t<std::is_same_v<R, void>, T, R>>(O()(as_underlying_type(a), as_underlying_type(b)));
	}
}


template<typename... args>
struct [[nodiscard]] overload: args...
{
	explicit overload(args&&... Args):
		args(FWD(Args))...
	{
	}

	using args::operator()...;
};

template<typename... args> overload(args&&...) -> overload<args...>;


namespace detail
{
	template<typename T>
	using is_void_or_trivially_copyable = std::disjunction<std::is_void<T>, std::is_trivially_copyable<T>>;
}

template<typename src_type, typename dst_type>
void copy_memory(const src_type* Source, dst_type* Destination, size_t const Size) noexcept
{
	static_assert(std::conjunction_v<
		detail::is_void_or_trivially_copyable<src_type>,
		detail::is_void_or_trivially_copyable<dst_type>
	>);

	if (Size) // paranoid gcc null checks are paranoid
		std::memmove(Destination, Source, Size);
}

template<typename T>
decltype(auto) view_as(void const* const BaseAddress, size_t const Offset = 0)
{
	static_assert(std::is_trivially_copyable_v<T>);

	const auto Ptr = static_cast<void const*>(static_cast<char const*>(BaseAddress) + Offset);

	if constexpr (std::is_pointer_v<T>)
	{
		return static_cast<T>(Ptr);
	}
	else
	{
		return *static_cast<T const*>(Ptr);
	}
}

template<typename T>
decltype(auto) view_as(unsigned long long const Address)
{
	return view_as<T>(reinterpret_cast<void const*>(Address));
}

#endif // UTILITY_HPP_D8E934C7_BF30_4CEB_B80C_6E508DF7A1BC

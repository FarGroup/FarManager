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

#include "preprocessor.hpp"
#include "type_traits.hpp"

#include <bit>
#include <functional>
#include <optional>
#include <utility>

#include <concepts>
#include <cstddef>
#include <cstring>

#include "cpp.hpp"

//----------------------------------------------------------------------------

template<typename T>
class base: public T
{
protected:
	using T::T;
	using base_ctor = base;
	using base_type = T;
};

inline size_t grow_exp(size_t const Current, std::optional<size_t> const Desired)
{
	if (Desired && *Desired <= Current)
		return Current;

	// reserve typically allocates exactly the requested amount instead of exponential growth.
	// This can be really bad if called in a loop.
	const auto LowerBound = Current + std::max(1uz, Current / 2);
	return Desired? std::max(LowerBound, *Desired) : LowerBound;
}

void reserve_exp(auto& Container, size_t const DesiredCapacity)
{
	Container.reserve(grow_exp(Container.capacity(), DesiredCapacity));
}

void resize_exp(auto& Container)
{
	Container.resize(grow_exp(Container.size(), {}), {});
}

void resize_exp(auto& Container, size_t const DesiredSize)
{
	Container.resize(grow_exp(Container.size(), DesiredSize), {});
}


template<class T>
void clear_and_shrink(T& container)
{
	T Tmp;
	std::ranges::swap(container, Tmp);
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

void hash_combine(size_t& Seed, const auto& Value)
{
	// https://en.wikipedia.org/wiki/Hash_function#Fibonacci_hashing
	const auto MagicValue =
#ifdef _WIN64
		// 2^64 / phi
		11400714819323198485ull
#else
		// 2^32 / phi
		2654435769ul
#endif
	;

	Seed ^= make_hash(Value) + MagicValue + (Seed << 6) + (Seed >> 2);
}

size_t hash_combine_all(const auto&... Args)
{
	size_t Seed = 0;
	(..., hash_combine(Seed, Args));
	return Seed;
}

void hash_range(size_t& Seed, auto const& Range)
{
	for (const auto& i: Range)
	{
		hash_combine(Seed, i);
	}
}

size_t hash_range(auto const& Range)
{
	size_t Seed = 0;
	hash_range(Seed, Range);
	return Seed;
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

[[nodiscard]]
constexpr auto bit(size_t const Number)
{
	return 1ull << Number;
}

[[nodiscard]]
consteval auto operator""_bit(unsigned long long const Number)
{
	return bit(Number);
}

namespace flags
{
	namespace detail
	{
		template<typename T> requires std::integral<T> || std::is_enum_v<T>
		constexpr auto reveal(T const Value)
		{
			return as_unsigned(sane_to_underlying(Value));
		}

		template<typename T> requires (!std::integral<T> && !std::is_enum_v<T>)
		constexpr auto& reveal(T& Value)
		{
			return Value;
		}
	}

	constexpr bool check_any(const auto& Value, auto const Bits)
	{
		return (detail::reveal(Value) & detail::reveal(Bits)) != 0;
	}

	constexpr bool check_one(const auto& Value, auto const Bit)
	{
		assert(std::has_single_bit(detail::reveal(Bit)));
		return check_any(Value, Bit);
	}

	constexpr bool check_all(const auto& Value, auto const Bits)
	{
		return (detail::reveal(Value) & detail::reveal(Bits)) == detail::reveal(Bits);
	}

	template<typename value_type>
	constexpr void set(value_type& Value, auto const Bits)
	{
		if constexpr (requires { Value |= detail::reveal(Bits); })
			Value |= detail::reveal(Bits);
		else
			Value = static_cast<value_type>(detail::reveal(Value) | detail::reveal(Bits));
	}

	template<typename value_type>
	constexpr void clear(value_type& Value, auto const Bits)
	{
		const auto Mask = static_cast<std::remove_reference_t<decltype(detail::reveal(Value))>>(detail::reveal(Bits));
		if constexpr (requires { Value &= ~Mask; })
			Value &= ~Mask;
		else
			Value = static_cast<value_type>(detail::reveal(Value) & ~Mask);
	}

	template<typename value_type>
	constexpr void invert(value_type& Value, auto const Bits)
	{
		if constexpr (requires { Value ^= detail::reveal(Bits); })
			Value ^= detail::reveal(Bits);
		else
			Value = static_cast<value_type>(detail::reveal(Value) ^ detail::reveal(Bits));
	}

	constexpr void change(auto& Value, auto const Bits, bool const Set)
	{
		Set? set(Value, Bits) : clear(Value, Bits);
	}

	constexpr void copy(auto& Value, auto const Mask, auto const Bits)
	{
		clear(Value, Mask);
		set(Value, detail::reveal(Bits) & detail::reveal(Mask));
	}
}

[[nodiscard]]
constexpr size_t aligned_size(size_t Size, size_t Alignment = alignof(std::max_align_t))
{
	return (Size + (Alignment - 1)) & ~(Alignment - 1);
}

template<typename T, size_t Alignment = alignof(std::max_align_t)>
constexpr inline auto aligned_sizeof = aligned_size(sizeof(T), Alignment);

[[nodiscard]]
inline bool is_aligned(const void* Address, const size_t Alignment)
{
	return !(std::bit_cast<uintptr_t>(Address) % Alignment);
}

template<typename T>
[[nodiscard]]
bool is_aligned(const T& Object)
{
	return is_aligned(&Object, alignof(T));
}

namespace enum_helpers
{
	template<typename T>
	concept bit_flags_enum = std::is_enum_v<T> && requires { T::is_bit_flags; };

	namespace detail
	{
		template<class O, class T>
		[[nodiscard]]
		constexpr auto operation(T const a, T const b)
		{
			return static_cast<T>(O()(std::to_underlying(a), std::to_underlying(b)));
		}
	}

	[[nodiscard]] constexpr auto operator|(bit_flags_enum auto const a, bit_flags_enum auto const b)
	{
		return detail::operation<std::bit_or<>>(a, b);
	}

	[[nodiscard]] constexpr auto& operator|=(bit_flags_enum auto& a, bit_flags_enum auto const b)
	{
		return a = a | b;
	}

	[[nodiscard]] constexpr auto operator&(bit_flags_enum auto const a, bit_flags_enum auto const b)
	{
		return detail::operation<std::bit_and<>>(a, b);
	}

	[[nodiscard]] constexpr auto& operator&=(bit_flags_enum auto& a, bit_flags_enum auto const b)
	{
		return a = a & b;
	}
}

using enum_helpers::operator|;
using enum_helpers::operator|=;
using enum_helpers::operator&;
using enum_helpers::operator&=;

template<typename... args>
struct [[nodiscard]] overload: args...
{
	using args::operator()...;

	consteval void operator()(auto a) const
	{
		static_assert(!sizeof(a), "Unsupported type");
	}
};

// For Clang
template<typename... args> overload(args...) -> overload<args...>;

namespace detail
{
	template<typename T>
	concept void_or_trivially_copyable = std::disjunction_v<std::is_void<T>, std::is_trivially_copyable<T>>;
}

void copy_memory(detail::void_or_trivially_copyable auto const* const Source, detail::void_or_trivially_copyable auto* const Destination, size_t const Size) noexcept
{
	if (Size) // paranoid gcc null checks are paranoid
		std::memmove(Destination, Source, Size);
}

namespace detail
{
	template<typename T, typename void_type> requires std::same_as<std::remove_const_t<void_type>, void>
	constexpr decltype(auto) cast_as(void_type* const BaseAddress, intptr_t const Offset)
	{
		constexpr auto IsConst = std::is_const_v<void_type>;

		const auto Ptr = static_cast<void_type*>(static_cast<std::conditional_t<IsConst, const std::byte, std::byte>*>(BaseAddress) + Offset);

		if constexpr (std::is_pointer_v<T>)
		{
			if constexpr (!std::is_void_v<std::remove_pointer_t<T>>)
				assert(is_aligned(Ptr, alignof(std::remove_pointer_t<T>)));

			return static_cast<T>(Ptr);
		}
		else
		{
			assert(Ptr);
			assert(is_aligned(Ptr, alignof(T)));

			return *static_cast<std::conditional_t<IsConst, const T, T>*>(Ptr);
		}
	}

	template<typename T>
	concept buffer_type = std::same_as<std::remove_const_t<T>, void> || std::is_trivially_copyable_v<T>;

	template<typename T>
	concept writable_buffer_type = !std::is_const_v<T> && (std::same_as<T, void> || std::is_trivially_copyable_v<T>);
}

template<typename T>
constexpr decltype(auto) view_as(detail::buffer_type auto const* const BaseAddress, intptr_t const Offset = 0)
{
	return detail::cast_as<T>(static_cast<void const*>(BaseAddress), Offset);
}

template<typename T>
constexpr decltype(auto) view_as(unsigned long long const Address)
{
	return view_as<T>(static_cast<void const*>(nullptr), Address);
}

template<typename T>
constexpr decltype(auto) edit_as(detail::writable_buffer_type auto* const BaseAddress, intptr_t const Offset = 0)
{
	return detail::cast_as<T>(static_cast<void*>(BaseAddress), Offset);
}

template<typename T>
constexpr decltype(auto) edit_as(unsigned long long const Address)
{
	return edit_as<T>(static_cast<void*>(nullptr), Address);
}

template<typename T> requires std::is_trivially_copyable_v<T>
constexpr auto view_as_opt(detail::buffer_type auto const* const Begin, detail::buffer_type auto const* const End, size_t const Offset = 0)
{
	return static_cast<char const*>(static_cast<void const*>(Begin)) + Offset + sizeof(T) <= static_cast<char const*>(static_cast<void const*>(End))?
		view_as<T const*>(Begin, Offset) :
		nullptr;
}

template<typename T> requires std::is_trivially_copyable_v<T>
constexpr auto view_as_opt(detail::buffer_type auto const* const Buffer, size_t const Size, size_t const Offset = 0)
{
	return view_as_opt<T>(Buffer, static_cast<char const*>(static_cast<void const*>(Buffer)) + Size, Offset);
}

template<typename T> requires std::is_trivially_copyable_v<T>
constexpr auto view_as_opt(std::ranges::contiguous_range auto const& Buffer, size_t const Offset = 0)
{
	static_assert(detail::buffer_type<std::ranges::range_value_t<decltype(Buffer)>>);
	return view_as_opt<T>(std::ranges::cdata(Buffer), std::ranges::size(Buffer), Offset);
}

template<typename large_type, typename small_type>
constexpr large_type make_integer(small_type const LowPart, small_type const HighPart)
{
	static_assert(sizeof(large_type) == sizeof(small_type) * 2);

	return static_cast<large_type>(HighPart) << (sizeof(large_type) / 2 * CHAR_BIT) | static_cast<large_type>(LowPart);
}

template<typename large_type, size_t LowPart, size_t HighPart>
constexpr large_type make_integer()
{
	constexpr auto Shift = (sizeof(large_type) / 2 * CHAR_BIT);
	constexpr auto Max = std::numeric_limits<large_type>::max() >> Shift;
	static_assert(LowPart <= Max);
	static_assert(HighPart <= Max);

	return static_cast<large_type>(HighPart) << Shift | static_cast<large_type>(LowPart);
}

template<typename small_type, size_t Index, typename large_type>
constexpr small_type extract_integer(large_type const Value)
{
	static_assert(sizeof(small_type) < sizeof(large_type));
	static_assert(sizeof(small_type) * Index < sizeof(large_type));

	return static_cast<small_type>(Value >> sizeof(small_type) * Index * CHAR_BIT);
}

#endif // UTILITY_HPP_D8E934C7_BF30_4CEB_B80C_6E508DF7A1BC

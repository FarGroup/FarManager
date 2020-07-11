#ifndef BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
#define BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
#pragma once

/*
bytes_view.hpp
*/
/*
Copyright © 2016 Far Group
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

#include "range.hpp"

//----------------------------------------------------------------------------

namespace detail
{
	template<typename T, bool Add>
	using add_const_if_t = std::conditional_t<Add, T const, T>;

	template<typename return_type, typename T, REQUIRES((std::disjunction_v<std::is_void<T>, std::is_trivially_copyable<T>>))>
	return_type bytes_impl(T* const Data, size_t const Size)
	{
		constexpr auto IsConst = std::is_const_v<T>;
		using void_type = add_const_if_t<void, IsConst>;
		using byte_type = add_const_if_t<std::byte, IsConst>;
		return { static_cast<byte_type*>(static_cast<void_type*>(Data)), Size };
	}

	template<typename return_type, typename T>
	auto bytes_impl(T& Object)
	{
		if constexpr (is_span_v<T>)
		{
			using value_type = std::remove_reference_t<decltype(*std::data(std::declval<T&>()))>;
			static_assert(std::is_trivially_copyable_v<value_type>);

			return bytes_impl<return_type>(std::data(Object), std::size(Object) * sizeof(value_type));
		}
		else if constexpr (std::is_trivially_copyable_v<T>)
		{
			return bytes_impl<return_type>(&Object, sizeof(Object));
		}
		else
		{
			static_assert(!sizeof(T), "The type is not serialisable");
		}
	}
}


using bytes_view = std::basic_string_view<std::byte>;

[[nodiscard]]
constexpr inline bytes_view operator "" _bv(const char* Str, std::size_t Size) noexcept
{
	return { static_cast<std::byte const*>(static_cast<void const*>(Str)), Size };
}


using bytes = std::basic_string<std::byte>;

[[nodiscard]]
/*constexpr*/ inline bytes operator "" _b(const char* Str, std::size_t Size) noexcept
{
	return bytes{ operator ""_bv(Str, Size) };
}


template<typename T>
[[nodiscard]]
auto view_bytes(T const* const Data, size_t const Size)
{
	return detail::bytes_impl<bytes_view>(Data, Size);
}

template<typename T>
[[nodiscard]]
auto view_bytes(T const& Object)
{
	return detail::bytes_impl<bytes_view>(Object);
}

template<typename T>
[[nodiscard]]
auto edit_bytes(T* const Data, size_t const Size)
{
	return detail::bytes_impl<span<std::byte>>(Data, Size);
}

template<typename T>
[[nodiscard]]
auto edit_bytes(T& Object)
{
	return detail::bytes_impl<span<std::byte>>(Object);
}

[[nodiscard]]
inline std::string_view to_string_view(bytes_view const Bytes)
{
	return { static_cast<char const*>(static_cast<void const*>(Bytes.data())), Bytes.size() };
}

template<typename T>
[[nodiscard]]
bool deserialise(bytes_view const Bytes, T& Value) noexcept
{
	const auto ValueBytes = edit_bytes(Value);

	if (ValueBytes.size() != Bytes.size())
		return false;

	std::copy(ALL_CONST_RANGE(Bytes), ValueBytes.begin());
	return true;
}

#endif // BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3

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

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>

//----------------------------------------------------------------------------

namespace detail
{
	template<typename type, typename base_type>
	struct same_traits: std::char_traits<base_type>
	{
	private:
		using base = std::char_traits<base_type>;

		static constexpr auto* to(type* Char)       { return std::bit_cast<base_type*>(Char); }
		static constexpr auto* to(const type* Char) { return std::bit_cast<const base_type*>(Char); }
		static constexpr auto& to(type& Char)       { return *to(&Char); }
		static constexpr auto& to(const type& Char) { return *to(&Char); }

		static constexpr auto* from(base_type* Char)       { return std::bit_cast<type*>(Char); }
		static constexpr auto* from(const base_type* Char) { return std::bit_cast<const type*>(Char); }
		static constexpr auto& from(base_type& Char)       { return *from(&Char); }
		static constexpr auto& from(const base_type& Char) { return *from(&Char); }

	public:
		using char_type  = type;
		using int_type   = typename base::int_type;
		using off_type   = typename base::off_type;
		using pos_type   = typename base::pos_type;
		using state_type = typename base::state_type;

		static void assign(char_type& Left, const char_type& Right) noexcept
		{
			return base::assign(to(Left), to(Right));
		}

		static char_type* assign(char_type* const Data, size_t const Size, char_type const Value) noexcept
		{
			return from(base::assign(to(Data), Size, to(Value)));
		}

		static bool eq(char_type const Left, char_type const Right)
		{
			return base::eq(to(Left), to(Right));
		}

		static bool lt(char_type const Left, char_type const Right)
		{
			return base::lt(to(Left), to(Right));
		}

		static int compare(const char_type* const Str1, const char_type* const Str2, size_t const Size)
		{
			return base::compare(to(Str1), to(Str2), Size);
		}

		static size_t length(const char_type* const Data)
		{
			return base::length(to(Data));
		}

		static const char_type* find(const char_type* const Data, size_t const Size, const char_type& Value)
		{
			return from(base::find(to(Data), Size, to(Value)));
		}

		static char_type* move(char_type* const Dst, const char_type* const Src, std::size_t const Size)
		{
			return from(base::move(to(Dst), to(Src), Size));
		}

		static char_type* copy(char_type* const Dst, const char_type* const Src, std::size_t const Size)
		{
			return from(base::copy(to(Dst), to(Src), Size));
		}

		static int_type not_eof(int_type const Value)
		{
			return base::not_eof(Value);
		}

		static char_type to_char_type(int_type const Value)
		{
			return from(base::to_char_type(Value));
		}

		static int_type to_int_type(char_type const Value)
		{
			return base::to_int_type(to(Value));
		}

		static bool eq_int_type(int_type a, int_type b)
		{
			return base::eq_int_type(a, b);
		}

		static int_type eof()
		{
			return base::eof();
		}

	};

	template<typename T, bool Add>
	using add_const_if_t = std::conditional_t<Add, T const, T>;

	template<typename return_type, typename T> requires std::is_void_v<T> || std::is_trivially_copyable_v<T>
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
		if constexpr (std::ranges::contiguous_range<T>)
		{
			static_assert(std::is_trivially_copyable_v<std::ranges::range_value_t<T>>);

			return bytes_impl<return_type>(std::ranges::data(Object), std::ranges::size(Object) * sizeof(std::ranges::range_value_t<T>));
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

using byte_traits = detail::same_traits<std::byte, char>;

using bytes_view = std::basic_string_view<std::byte, byte_traits>;

[[nodiscard]]
constexpr bytes_view operator""_bv(const char* Str, std::size_t Size) noexcept
{
	return { static_cast<std::byte const*>(static_cast<void const*>(Str)), Size };
}


using bytes = std::basic_string<std::byte, byte_traits>;

[[nodiscard]]
/*constexpr*/ inline bytes operator""_b(const char* Str, std::size_t Size) noexcept
{
	return bytes{ operator""_bv(Str, Size) };
}


[[nodiscard]]
auto view_bytes(auto const* const Data, size_t const Size)
{
	return detail::bytes_impl<bytes_view>(Data, Size);
}

[[nodiscard]]
auto view_bytes(auto const& Object)
{
	return detail::bytes_impl<bytes_view>(Object);
}

[[nodiscard]]
auto edit_bytes(auto* const Data, size_t const Size)
{
	return detail::bytes_impl<std::span<std::byte>>(Data, Size);
}

[[nodiscard]]
auto edit_bytes(auto& Object)
{
	return detail::bytes_impl<std::span<std::byte>>(Object);
}

[[nodiscard]]
inline std::string_view to_string_view(bytes_view const Bytes)
{
	return { static_cast<char const*>(static_cast<void const*>(Bytes.data())), Bytes.size() };
}

[[nodiscard]]
bool deserialise(bytes_view const Bytes, auto& Value) noexcept
{
	const auto ValueBytes = edit_bytes(Value);

	if (ValueBytes.size() != Bytes.size())
		return false;

	std::ranges::copy(Bytes, ValueBytes.begin());
	return true;
}

#endif // BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3

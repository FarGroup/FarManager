#ifndef UUID_HPP_260060CF_1464_4A3F_82E0_94803A6A5EC6
#define UUID_HPP_260060CF_1464_4A3F_82E0_94803A6A5EC6
#pragma once

/*
uuid.hpp
*/
/*
Copyright © 2020 Far Group
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

//----------------------------------------------------------------------------

namespace uuid
{
	namespace detail
	{
		constexpr auto uuid_length = "00000000-0000-0000-0000-000000000000"sv.size();

		[[noreturn]]
		inline void fail(const char* Message)
		{
			throw std::runtime_error(Message);
		}

		[[nodiscard]]
		constexpr inline auto hex_to_int(wchar_t const c)
		{
			if (L'0' <= c && c <= L'9')
				return c - '0';

			if (L'a' <= c && c <= L'f')
				return c - 'a' + 10;

			if (L'A' <= c && c <= L'F')
				return c - L'A' + 10;

			else // GCC 8.1 don't understand this without an explicit 'else'
			fail("Invalid character");
		}

		template<typename char_type, size_t... I>
		[[nodiscard]]
		constexpr auto parse_impl(char_type const* const Str, std::index_sequence<I...> const Sequence)
		{
			constexpr auto N = Sequence.size();
			static_assert(N <= 4);
			using int_type = std::conditional_t<N == 1, uint8_t, std::conditional_t<N == 2, uint16_t, uint32_t>>;

			return int_type((... | (
				hex_to_int(Str[(N - 1 - I) * 2 + 0]) << (I * 8 + 4) |
				hex_to_int(Str[(N - 1 - I) * 2 + 1]) << (I * 8 + 0)
				)));
		}

		template<size_t Bytes, typename char_type>
		[[nodiscard]]
		constexpr auto parse(char_type const*& Iterator)
		{
			auto Value = parse_impl(Iterator, std::make_index_sequence<Bytes>{});
			Iterator += Bytes * 2;
			return Value;
		}

		constexpr class [[nodiscard]] separator_t{} Separator;

		template<typename char_type>
		[[nodiscard]]
		constexpr auto& operator+=(char_type const*& Iterator, separator_t)
		{
			if (*Iterator != L'-')
				fail("Invalid character");

			return ++Iterator;
		}

		template<typename char_type>
		[[nodiscard]]
		constexpr UUID parse(char_type const* Iterator)
		{
			return
			{
				parse<4>(Iterator),
				parse<2>(Iterator += Separator),
				parse<2>(Iterator += Separator),
				{
					parse<1>(Iterator += Separator),
					parse<1>(Iterator),
					parse<1>(Iterator += Separator),
					parse<1>(Iterator),
					parse<1>(Iterator),
					parse<1>(Iterator),
					parse<1>(Iterator),
					parse<1>(Iterator)
				}
			};
		}

		template<size_t Digit>
		[[nodiscard]]
		constexpr char int_to_hex(unsigned Value)
		{
			const auto i = (Value & (0xFu << 4 * Digit)) >> 4 * Digit;
			return i < 10? i + '0' : i - 10 + 'A';
		}

		template<typename int_type, typename handler, size_t... I>
		constexpr void serialise_impl(int_type Value, handler const& Handler, std::index_sequence<I...> const Sequence)
		{
			constexpr auto N = Sequence.size() * 2;

			(..., (
				Handler(int_to_hex<N - I * 2 - 1>(Value)),
				Handler(int_to_hex<N - I * 2 - 2>(Value))
			));
		}

		template<typename int_type, typename handler>
		constexpr void serialise(int_type Value, handler const& Handler)
		{
			serialise_impl(Value, Handler, std::make_index_sequence<sizeof(Value)>{});
		}
	}

	template<typename char_type>
	[[nodiscard]]
	constexpr auto parse(char_type const* const Str, size_t const Size)
	{
		if (!(Size == detail::uuid_length || (Size == detail::uuid_length + 2 && Str[0] == L'{' && Str[Size - 1] == L'}')))
			detail::fail("Incorrect format");

		return detail::parse(Str + (Size != detail::uuid_length));
	}

	template<typename string_type>
	[[nodiscard]]
	auto parse(string_type const& Str)
	{
		return parse(Str.data(), Str.size());
	}

	template<typename string_type>
	[[nodiscard]]
	std::optional<UUID> try_parse(string_type const& Str)
	{
		if (Str.empty())
			return {};

		try
		{
			return parse(Str.data(), Str.size());
		}
		catch (...)
		{
			return {};
		}
	}

	[[nodiscard]]
	inline auto str(UUID const& Uuid)
	{
		std::wstring Result;
		Result.reserve(detail::uuid_length);

		const auto put = [&](char const c)
		{
			Result.push_back(c);
		};

		const auto serialise = [&](auto const i)
		{
			detail::serialise(i, put);
		};

		constexpr auto Separator = '-';

		serialise(Uuid.Data1);
		put(Separator);
		serialise(Uuid.Data2);
		put(Separator);
		serialise(Uuid.Data3);
		put(Separator);
		serialise(Uuid.Data4[0]);
		serialise(Uuid.Data4[1]);
		put(Separator);
		serialise(Uuid.Data4[2]);
		serialise(Uuid.Data4[3]);
		serialise(Uuid.Data4[4]);
		serialise(Uuid.Data4[5]);
		serialise(Uuid.Data4[6]);
		serialise(Uuid.Data4[7]);

		return Result;
	}

	namespace literals
	{
		[[nodiscard]]
		constexpr inline auto operator ""_uuid(const char* const Str, size_t const Size)
		{
			return parse(Str, Size);
		}
	}
}

inline namespace literals
{
	using namespace uuid::literals;
}

#endif // UUID_HPP_260060CF_1464_4A3F_82E0_94803A6A5EC6

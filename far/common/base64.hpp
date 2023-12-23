#ifndef BASE64_HPP_C0835DDC_DB5A_4C40_A84C_146D6CFE6BFD
#define BASE64_HPP_C0835DDC_DB5A_4C40_A84C_146D6CFE6BFD
#pragma once

/*
base64.hpp
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

#include "algorithm.hpp"
#include "bytes_view.hpp"

//----------------------------------------------------------------------------

namespace base64
{
	namespace detail
	{
		using bits_set = uint_fast32_t;

		inline auto byte_to_bits(std::byte const Byte, size_t const i) noexcept
		{
			return std::to_integer<bits_set>(Byte) << (8 * (2 - i));
		}

		template<size_t... I>
		auto bytes_to_bits(std::forward_iterator auto& Iterator, std::index_sequence<I...>) noexcept
		{
			bits_set Result{};
			(..., (Result |= byte_to_bits(*Iterator++, I)));
			return Result;
		}

		inline auto bits_to_char(bits_set const Bits, size_t const i)
		{
			constexpr char EncodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			return EncodingTable[(Bits & (0b111111 << (6 * (3 - i)))) >> (6 * (3 - i))];
		}

		template<size_t... I>
		void bits_to_chars(bits_set const Bits, std::string& Str, std::index_sequence<I...>) noexcept
		{
			(..., Str.push_back(bits_to_char(Bits, I)));
		}

		template<size_t Bytes, typename iterator_t>
		void encode(iterator_t& Iterator, std::string& Str) noexcept
		{
			bits_to_chars(bytes_to_bits(Iterator, std::make_index_sequence<Bytes>{}), Str, std::make_index_sequence<Bytes + 1>{});
			Str.append(3 - Bytes, '=');
		}


		inline auto char_to_bits(char const Char, size_t const i)
		{
			static uint8_t constexpr no = -1, DecodingTable[]
			{
				no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
				no, no, no, no, no, no, no, no, no, no, no, 62, no, no, no, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, no, no, no, no, no, no,
				no,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, no, no, no, no, no,
				no, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, no, no, no, no, no,
			};

			const auto error = [&]
			{
				using namespace std::string_literals;
				throw std::runtime_error("Invalid base64 character "s + Char);
			};

			if (static_cast<size_t>(Char) >= std::size(DecodingTable))
				error();

			const auto Bits = DecodingTable[static_cast<size_t>(Char)];
			if (Bits == no)
				error();

			return static_cast<bits_set>((Bits & 0b111111) << (6 * (3 - i)));
		}

		template<size_t... I>
		auto chars_to_bits(std::forward_iterator auto& Iterator, std::index_sequence<I...>)
		{
			bits_set Result{};
			(..., (Result |= char_to_bits(*Iterator++, I)));
			return Result;
		}

		template<size_t... I>
		void bits_to_bytes(bits_set const Bits, bytes& Str, std::index_sequence<I...>)
		{
			(..., Str.push_back(static_cast<std::byte>((Bits >> 8 * (2 - I)) & 0xFF)));
		}

		template<size_t Chars, typename iterator_t>
		void decode(iterator_t& Iterator, bytes& Str)
		{
			bits_to_bytes(chars_to_bits(Iterator, std::make_index_sequence<Chars>{}), Str, std::make_index_sequence<Chars - 1>{});
		}
	}

	inline auto encode(bytes_view const Data)
	{
		std::string Result;
		Result.reserve((Data.size() + 2) / 3 * 4);

		auto Iterator = Data.cbegin();
		auto const Chunks = Data.size() / 3;

		repeat(Chunks, [&]
		{
			detail::encode<3>(Iterator, Result);
		});

		switch (Data.size() - Chunks * 3)
		{
		case 1: detail::encode<1>(Iterator, Result); break;
		case 2: detail::encode<2>(Iterator, Result); break;
		}

		return Result;
	}

	inline auto decode(std::string_view Data)
	{
		Data.remove_suffix(Data.size() - (Data.find_last_not_of('=') + 1));

		bytes Result;
		Result.reserve((Data.size() + 3) / 4 * 3);

		auto Iterator = Data.cbegin();
		auto const Chunks = Data.size() / 4;

		repeat(Chunks, [&]
		{
			detail::decode<4>(Iterator, Result);
		});

		switch (Data.size() - Chunks * 4)
		{
		case 1: detail::decode<1>(Iterator, Result); break;
		case 2: detail::decode<2>(Iterator, Result); break;
		case 3: detail::decode<3>(Iterator, Result); break;
		}

		return Result;
	}
}

#endif // BASE64_HPP_C0835DDC_DB5A_4C40_A84C_146D6CFE6BFD

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

#include "bytes_view.hpp"

//----------------------------------------------------------------------------

namespace base64
{
	namespace detail
	{
		template<typename iterator_t, size_t... I>
		auto bin_take(iterator_t& Iterator, std::index_sequence<I...>) noexcept
		{
			const auto take_one = [&](size_t const i) { return (std::to_integer<int>(*Iterator++)) << (16 - 8 * i); };
			return (... | take_one(I));
		}

		template<size_t... I>
		void bin_store(int const Bits, bytes& Str, std::index_sequence<I...>) noexcept
		{
			(..., Str.push_back(static_cast<std::byte>((Bits >> 8 * (2 - I)) & 0xFF)));
		}

		template<size_t I>
		void bin_store(int const Bits, bytes& Str) noexcept
		{
			bin_store(Bits, Str, std::make_index_sequence<I>{});
		}

		inline auto text_take(std::string_view& Data) noexcept
		{
			size_t Count = 0;

			auto const get_alphabet_char = [&](char& To)
			{
				static int constexpr no = -1, Table[]
				{
					no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
					no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
					no, no, no, no, no, no, no, no, no, no, no, 62, no, no, no, 63,
					52, 53, 54, 55, 56, 57, 58, 59, 60, 61, no, no, no, no, no, no,
					no,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
					15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, no, no, no, no, no,
					no, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
					41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, no, no, no, no, no,
				};

				if (const auto Skip = std::find_if(ALL_CONST_RANGE(Data), [](size_t const i) { return i < std::size(Table) && Table[i] != no; }) - Data.cbegin())
					Data.remove_prefix(Skip);

				if (Data.empty())
					return false;

				To = Table[static_cast<size_t>(Data.front())];
				Data.remove_prefix(1);
				++Count;
				return true;
			};

			char Sextets[4]{};

			get_alphabet_char(Sextets[0]) &&
			get_alphabet_char(Sextets[1]) &&
			get_alphabet_char(Sextets[2]) &&
			get_alphabet_char(Sextets[3]);

			auto const Bits =
				(Sextets[0] << 3 * 6) +
				(Sextets[1] << 2 * 6) +
				(Sextets[2] << 1 * 6) +
				(Sextets[3] << 0 * 6);

			return std::pair(Bits, Count);
		}

		template<size_t... I>
		void text_store(int const Bits, std::string& Str, std::index_sequence<I...>) noexcept
		{
			constexpr char EncodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			(..., Str.push_back(EncodingTable[(Bits & (0b111111 << 6 * (3 - I))) >> 6 * (3 - I)]));
		}

		template<size_t Bytes, typename iterator_t>
		void encode(iterator_t& Iterator, std::string& Str) noexcept
		{
			auto const Bits = bin_take(Iterator, std::make_index_sequence<Bytes>{});
			text_store(Bits, Str, std::make_index_sequence<Bytes + 1>{});
		}

		template<size_t Bytes, typename iterator_t>
		void decode(iterator_t& Iterator, bytes& Str) noexcept
		{
			auto const Bits = text_take(Iterator, std::make_index_sequence<Bytes>{});
			bin_store(Bits, Str, std::make_index_sequence<Bytes - 1>{});
		}
	}

	inline std::string encode(bytes_view const Data)
	{
		std::string Str;
		Str.reserve((Data.size() + 2) / 3 * 4);

		auto Iterator = Data.cbegin();
		auto const Chunks = Data.size() / 3;

		for (size_t i = 0; i != Chunks; ++i)
		{
			detail::encode<3>(Iterator, Str);
		}

		switch (Data.size() - Chunks * 3)
		{
		case 1:
			detail::encode<1>(Iterator, Str);
			Str.push_back('=');
			Str.push_back('=');
			break;

		case 2:
			detail::encode<2>(Iterator, Str);
			Str.push_back('=');
			break;
		}

		return Str;
	}

	inline bytes decode(std::string_view Data)
	{
		if (const auto PaddingSize = std::find_if(ALL_CONST_REVERSE_RANGE(Data), [](char const i) { return i != '='; }) - Data.crbegin())
			Data.remove_suffix(PaddingSize);

		bytes Str;
		Str.reserve((Data.size() + 3) / 4 * 3);

		for (;;)
		{
			const auto [Bits, Count] = detail::text_take(Data);

			switch (Count)
			{
			default: return Str;
			case 2: detail::bin_store<1>(Bits, Str); break;
			case 3: detail::bin_store<2>(Bits, Str); break;
			case 4: detail::bin_store<3>(Bits, Str); break;
			}
		}
	}
}

#endif // BASE64_HPP_C0835DDC_DB5A_4C40_A84C_146D6CFE6BFD

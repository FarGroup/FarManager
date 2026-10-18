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
#include "exception.hpp"

//----------------------------------------------------------------------------

namespace base64
{
	inline auto encode(bytes_view const Data)
	{
		constexpr char EncodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		std::string Result;
		Result.reserve((Data.size() + 2) / 3 * 4);

		uint_fast32_t Accumulator{};
		size_t Bits{};

		for (const auto Byte: Data)
		{
			Accumulator = (Accumulator << CHAR_BIT) | std::to_integer<uint_fast32_t>(Byte);
			Bits += CHAR_BIT;

			while (Bits >= 6)
			{
				Bits -= 6;
				Result.push_back(EncodingTable[(Accumulator >> Bits) & 0b111111]);
			}
		}

		if (Bits)
			Result.push_back(EncodingTable[(Accumulator << (6 - Bits)) & 0b111111]);

		Result.resize((Result.size() + 3) / 4 * 4, '=');

		return Result;
	}

	inline auto decode(std::string_view Data)
	{
		Data.remove_suffix(Data.size() - (Data.find_last_not_of('=') + 1));

		bytes Result;
		Result.reserve((Data.size() + 3) / 4 * 3);

		uint_fast32_t Accumulator{};
		size_t Bits{};

		constexpr uint8_t no = -1, DecodingTable[]
		{
			no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
			no, no, no, no, no, no, no, no, no, no, no, 62, no, no, no, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, no, no, no, no, no, no,
			no,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, no, no, no, no, no,
			no, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, no, no, no, no, no,
		};

		for (const auto Char: Data)
		{
			size_t const Index = Char;
			if (Index >= std::size(DecodingTable) || DecodingTable[Index] == no)
			{
				using namespace std::string_literals;
				throw_exception("Invalid base64 character "s + Char);
			}

			Accumulator = (Accumulator << 6) | DecodingTable[Index];
			Bits += 6;

			if (Bits >= CHAR_BIT)
			{
				Bits -= CHAR_BIT;
				Result.push_back(static_cast<std::byte>((Accumulator >> Bits) & 0xFF));
			}
		}

		return Result;
	}
}
#endif // BASE64_HPP_C0835DDC_DB5A_4C40_A84C_146D6CFE6BFD

/*
char_width.cpp

Fullwidth support
*/
/*
Copyright © 2021 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "char_width.hpp"

// Internal:
#include "console.hpp"
#include "locale.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	enum class full_width
	{
		off,
		on,
		automatic
	};

	static auto s_FullWidthState = full_width::off;

	enum class codepoint_width: char
	{
		ambiguous,
		narrow,
		wide,
	};

	[[nodiscard]]
	static auto quick_width(char_width::codepoint const Codepoint) noexcept
	{
		return in_closed_range(0x20u, Codepoint, 0x7Eu)?
			codepoint_width::narrow :
			codepoint_width::ambiguous;
	}

	struct unicode_range
	{
		char_width::codepoint
			LowerBound,
			UpperBound;
		codepoint_width Width;

		[[nodiscard]]
		bool operator<(char_width::codepoint const Codepoint) const noexcept
		{
			return UpperBound < Codepoint;
		}
	};

	// These mappings are based on src\types\CodepointWidthDetector.cpp from Windows Terminal.
	// Generated on 10/25/2020 7:32:04 AM (UTC) from Unicode 13.0.0.

	// Note: unlike Terminal, we don't use any overrides since we don't control any drawing code
	// and have no other choice but to do as the Romans do.
	using cpw = codepoint_width;
	static constexpr unicode_range s_WideAndAmbiguousTable[]
	{
		{ 0x0000A1, 0x0000A1, cpw::ambiguous },
		{ 0x0000A4, 0x0000A4, cpw::ambiguous },
		{ 0x0000A7, 0x0000A8, cpw::ambiguous },
		{ 0x0000AA, 0x0000AA, cpw::ambiguous },
		{ 0x0000AD, 0x0000AE, cpw::ambiguous },
		{ 0x0000B0, 0x0000B4, cpw::ambiguous },
		{ 0x0000B6, 0x0000BA, cpw::ambiguous },
		{ 0x0000BC, 0x0000BF, cpw::ambiguous },
		{ 0x0000C6, 0x0000C6, cpw::ambiguous },
		{ 0x0000D0, 0x0000D0, cpw::ambiguous },
		{ 0x0000D7, 0x0000D8, cpw::ambiguous },
		{ 0x0000DE, 0x0000E1, cpw::ambiguous },
		{ 0x0000E6, 0x0000E6, cpw::ambiguous },
		{ 0x0000E8, 0x0000EA, cpw::ambiguous },
		{ 0x0000EC, 0x0000ED, cpw::ambiguous },
		{ 0x0000F0, 0x0000F0, cpw::ambiguous },
		{ 0x0000F2, 0x0000F3, cpw::ambiguous },
		{ 0x0000F7, 0x0000FA, cpw::ambiguous },
		{ 0x0000FC, 0x0000FC, cpw::ambiguous },
		{ 0x0000FE, 0x0000FE, cpw::ambiguous },
		{ 0x000101, 0x000101, cpw::ambiguous },
		{ 0x000111, 0x000111, cpw::ambiguous },
		{ 0x000113, 0x000113, cpw::ambiguous },
		{ 0x00011B, 0x00011B, cpw::ambiguous },
		{ 0x000126, 0x000127, cpw::ambiguous },
		{ 0x00012B, 0x00012B, cpw::ambiguous },
		{ 0x000131, 0x000133, cpw::ambiguous },
		{ 0x000138, 0x000138, cpw::ambiguous },
		{ 0x00013F, 0x000142, cpw::ambiguous },
		{ 0x000144, 0x000144, cpw::ambiguous },
		{ 0x000148, 0x00014B, cpw::ambiguous },
		{ 0x00014D, 0x00014D, cpw::ambiguous },
		{ 0x000152, 0x000153, cpw::ambiguous },
		{ 0x000166, 0x000167, cpw::ambiguous },
		{ 0x00016B, 0x00016B, cpw::ambiguous },
		{ 0x0001CE, 0x0001CE, cpw::ambiguous },
		{ 0x0001D0, 0x0001D0, cpw::ambiguous },
		{ 0x0001D2, 0x0001D2, cpw::ambiguous },
		{ 0x0001D4, 0x0001D4, cpw::ambiguous },
		{ 0x0001D6, 0x0001D6, cpw::ambiguous },
		{ 0x0001D8, 0x0001D8, cpw::ambiguous },
		{ 0x0001DA, 0x0001DA, cpw::ambiguous },
		{ 0x0001DC, 0x0001DC, cpw::ambiguous },
		{ 0x000251, 0x000251, cpw::ambiguous },
		{ 0x000261, 0x000261, cpw::ambiguous },
		{ 0x0002C4, 0x0002C4, cpw::ambiguous },
		{ 0x0002C7, 0x0002C7, cpw::ambiguous },
		{ 0x0002C9, 0x0002CB, cpw::ambiguous },
		{ 0x0002CD, 0x0002CD, cpw::ambiguous },
		{ 0x0002D0, 0x0002D0, cpw::ambiguous },
		{ 0x0002D8, 0x0002DB, cpw::ambiguous },
		{ 0x0002DD, 0x0002DD, cpw::ambiguous },
		{ 0x0002DF, 0x0002DF, cpw::ambiguous },
		{ 0x000300, 0x00036F, cpw::ambiguous },
		{ 0x000391, 0x0003A1, cpw::ambiguous },
		{ 0x0003A3, 0x0003A9, cpw::ambiguous },
		{ 0x0003B1, 0x0003C1, cpw::ambiguous },
		{ 0x0003C3, 0x0003C9, cpw::ambiguous },
		{ 0x000401, 0x000401, cpw::ambiguous },
		{ 0x000410, 0x00044F, cpw::ambiguous },
		{ 0x000451, 0x000451, cpw::ambiguous },
		{ 0x001100, 0x00115F, cpw::wide      },
		{ 0x002010, 0x002010, cpw::ambiguous },
		{ 0x002013, 0x002016, cpw::ambiguous },
		{ 0x002018, 0x002019, cpw::ambiguous },
		{ 0x00201C, 0x00201D, cpw::ambiguous },
		{ 0x002020, 0x002022, cpw::ambiguous },
		{ 0x002024, 0x002027, cpw::ambiguous },
		{ 0x002030, 0x002030, cpw::ambiguous },
		{ 0x002032, 0x002033, cpw::ambiguous },
		{ 0x002035, 0x002035, cpw::ambiguous },
		{ 0x00203B, 0x00203B, cpw::ambiguous },
		{ 0x00203E, 0x00203E, cpw::ambiguous },
		{ 0x002074, 0x002074, cpw::ambiguous },
		{ 0x00207F, 0x00207F, cpw::ambiguous },
		{ 0x002081, 0x002084, cpw::ambiguous },
		{ 0x0020AC, 0x0020AC, cpw::ambiguous },
		{ 0x002103, 0x002103, cpw::ambiguous },
		{ 0x002105, 0x002105, cpw::ambiguous },
		{ 0x002109, 0x002109, cpw::ambiguous },
		{ 0x002113, 0x002113, cpw::ambiguous },
		{ 0x002116, 0x002116, cpw::ambiguous },
		{ 0x002121, 0x002122, cpw::ambiguous },
		{ 0x002126, 0x002126, cpw::ambiguous },
		{ 0x00212B, 0x00212B, cpw::ambiguous },
		{ 0x002153, 0x002154, cpw::ambiguous },
		{ 0x00215B, 0x00215E, cpw::ambiguous },
		{ 0x002160, 0x00216B, cpw::ambiguous },
		{ 0x002170, 0x002179, cpw::ambiguous },
		{ 0x002189, 0x002189, cpw::ambiguous },
		{ 0x002190, 0x002199, cpw::ambiguous },
		{ 0x0021B8, 0x0021B9, cpw::ambiguous },
		{ 0x0021D2, 0x0021D2, cpw::ambiguous },
		{ 0x0021D4, 0x0021D4, cpw::ambiguous },
		{ 0x0021E7, 0x0021E7, cpw::ambiguous },
		{ 0x002200, 0x002200, cpw::ambiguous },
		{ 0x002202, 0x002203, cpw::ambiguous },
		{ 0x002207, 0x002208, cpw::ambiguous },
		{ 0x00220B, 0x00220B, cpw::ambiguous },
		{ 0x00220F, 0x00220F, cpw::ambiguous },
		{ 0x002211, 0x002211, cpw::ambiguous },
		{ 0x002215, 0x002215, cpw::ambiguous },
		{ 0x00221A, 0x00221A, cpw::ambiguous },
		{ 0x00221D, 0x002220, cpw::ambiguous },
		{ 0x002223, 0x002223, cpw::ambiguous },
		{ 0x002225, 0x002225, cpw::ambiguous },
		{ 0x002227, 0x00222C, cpw::ambiguous },
		{ 0x00222E, 0x00222E, cpw::ambiguous },
		{ 0x002234, 0x002237, cpw::ambiguous },
		{ 0x00223C, 0x00223D, cpw::ambiguous },
		{ 0x002248, 0x002248, cpw::ambiguous },
		{ 0x00224C, 0x00224C, cpw::ambiguous },
		{ 0x002252, 0x002252, cpw::ambiguous },
		{ 0x002260, 0x002261, cpw::ambiguous },
		{ 0x002264, 0x002267, cpw::ambiguous },
		{ 0x00226A, 0x00226B, cpw::ambiguous },
		{ 0x00226E, 0x00226F, cpw::ambiguous },
		{ 0x002282, 0x002283, cpw::ambiguous },
		{ 0x002286, 0x002287, cpw::ambiguous },
		{ 0x002295, 0x002295, cpw::ambiguous },
		{ 0x002299, 0x002299, cpw::ambiguous },
		{ 0x0022A5, 0x0022A5, cpw::ambiguous },
		{ 0x0022BF, 0x0022BF, cpw::ambiguous },
		{ 0x002312, 0x002312, cpw::ambiguous },
		{ 0x00231A, 0x00231B, cpw::wide      },
		{ 0x002329, 0x00232A, cpw::wide      },
		{ 0x0023E9, 0x0023EC, cpw::wide      },
		{ 0x0023F0, 0x0023F0, cpw::wide      },
		{ 0x0023F3, 0x0023F3, cpw::wide      },
		{ 0x002460, 0x0024E9, cpw::ambiguous },
		{ 0x0024EB, 0x0024FF, cpw::ambiguous },
		{ 0x002500, 0x00259F, cpw::ambiguous },
		{ 0x0025A0, 0x0025A1, cpw::ambiguous },
		{ 0x0025A3, 0x0025A9, cpw::ambiguous },
		{ 0x0025B2, 0x0025B3, cpw::ambiguous },
		{ 0x0025B6, 0x0025B7, cpw::ambiguous },
		{ 0x0025BC, 0x0025BD, cpw::ambiguous },
		{ 0x0025C0, 0x0025C1, cpw::ambiguous },
		{ 0x0025C6, 0x0025C8, cpw::ambiguous },
		{ 0x0025CB, 0x0025CB, cpw::ambiguous },
		{ 0x0025CE, 0x0025D1, cpw::ambiguous },
		{ 0x0025E2, 0x0025E5, cpw::ambiguous },
		{ 0x0025EF, 0x0025EF, cpw::ambiguous },
		{ 0x0025FD, 0x0025FE, cpw::wide      },
		{ 0x002605, 0x002606, cpw::ambiguous },
		{ 0x002609, 0x002609, cpw::ambiguous },
		{ 0x00260E, 0x00260F, cpw::ambiguous },
		{ 0x002614, 0x002615, cpw::wide      },
		{ 0x00261C, 0x00261C, cpw::ambiguous },
		{ 0x00261E, 0x00261E, cpw::ambiguous },
		{ 0x002640, 0x002640, cpw::ambiguous },
		{ 0x002642, 0x002642, cpw::ambiguous },
		{ 0x002648, 0x002653, cpw::wide      },
		{ 0x002660, 0x002661, cpw::ambiguous },
		{ 0x002663, 0x002665, cpw::ambiguous },
		{ 0x002667, 0x00266A, cpw::ambiguous },
		{ 0x00266C, 0x00266D, cpw::ambiguous },
		{ 0x00266F, 0x00266F, cpw::ambiguous },
		{ 0x00267F, 0x00267F, cpw::wide      },
		{ 0x002693, 0x002693, cpw::wide      },
		{ 0x00269E, 0x00269F, cpw::ambiguous },
		{ 0x0026A1, 0x0026A1, cpw::wide      },
		{ 0x0026AA, 0x0026AB, cpw::wide      },
		{ 0x0026BD, 0x0026BE, cpw::wide      },
		{ 0x0026BF, 0x0026BF, cpw::ambiguous },
		{ 0x0026C4, 0x0026C5, cpw::wide      },
		{ 0x0026C6, 0x0026CD, cpw::ambiguous },
		{ 0x0026CE, 0x0026CE, cpw::wide      },
		{ 0x0026CF, 0x0026D3, cpw::ambiguous },
		{ 0x0026D4, 0x0026D4, cpw::wide      },
		{ 0x0026D5, 0x0026E1, cpw::ambiguous },
		{ 0x0026E3, 0x0026E3, cpw::ambiguous },
		{ 0x0026E8, 0x0026E9, cpw::ambiguous },
		{ 0x0026EA, 0x0026EA, cpw::wide      },
		{ 0x0026EB, 0x0026F1, cpw::ambiguous },
		{ 0x0026F2, 0x0026F3, cpw::wide      },
		{ 0x0026F4, 0x0026F4, cpw::ambiguous },
		{ 0x0026F5, 0x0026F5, cpw::wide      },
		{ 0x0026F6, 0x0026F9, cpw::ambiguous },
		{ 0x0026FA, 0x0026FA, cpw::wide      },
		{ 0x0026FB, 0x0026FC, cpw::ambiguous },
		{ 0x0026FD, 0x0026FD, cpw::wide      },
		{ 0x0026FE, 0x0026FF, cpw::ambiguous },
		{ 0x002705, 0x002705, cpw::wide      },
		{ 0x00270A, 0x00270B, cpw::wide      },
		{ 0x002728, 0x002728, cpw::wide      },
		{ 0x00273D, 0x00273D, cpw::ambiguous },
		{ 0x00274C, 0x00274C, cpw::wide      },
		{ 0x00274E, 0x00274E, cpw::wide      },
		{ 0x002753, 0x002755, cpw::wide      },
		{ 0x002757, 0x002757, cpw::wide      },
		{ 0x002776, 0x00277F, cpw::ambiguous },
		{ 0x002795, 0x002797, cpw::wide      },
		{ 0x0027B0, 0x0027B0, cpw::wide      },
		{ 0x0027BF, 0x0027BF, cpw::wide      },
		{ 0x002B1B, 0x002B1C, cpw::wide      },
		{ 0x002B50, 0x002B50, cpw::wide      },
		{ 0x002B55, 0x002B55, cpw::wide      },
		{ 0x002B56, 0x002B59, cpw::ambiguous },
		{ 0x002E80, 0x002E99, cpw::wide      },
		{ 0x002E9B, 0x002EF3, cpw::wide      },
		{ 0x002F00, 0x002FD5, cpw::wide      },
		{ 0x002FF0, 0x002FFB, cpw::wide      },
		{ 0x003000, 0x00303E, cpw::wide      },
		{ 0x003041, 0x003096, cpw::wide      },
		{ 0x003099, 0x0030FF, cpw::wide      },
		{ 0x003105, 0x00312F, cpw::wide      },
		{ 0x003131, 0x00318E, cpw::wide      },
		{ 0x003190, 0x0031E3, cpw::wide      },
		{ 0x0031F0, 0x00321E, cpw::wide      },
		{ 0x003220, 0x003247, cpw::wide      },
		{ 0x003248, 0x00324F, cpw::ambiguous },
		{ 0x003250, 0x004DBF, cpw::wide      },
		{ 0x004DC0, 0x004DFF, cpw::ambiguous },
		{ 0x004E00, 0x00A48C, cpw::wide      },
		{ 0x00A490, 0x00A4C6, cpw::wide      },
		{ 0x00A960, 0x00A97C, cpw::wide      },
		{ 0x00AC00, 0x00D7A3, cpw::wide      },
		{ 0x00E000, 0x00F8FF, cpw::ambiguous },
		{ 0x00F900, 0x00FAFF, cpw::wide      },
		{ 0x00FE00, 0x00FE0F, cpw::ambiguous },
		{ 0x00FE10, 0x00FE19, cpw::wide      },
		{ 0x00FE20, 0x00FE2F, cpw::ambiguous },
		{ 0x00FE30, 0x00FE52, cpw::wide      },
		{ 0x00FE54, 0x00FE66, cpw::wide      },
		{ 0x00FE68, 0x00FE6B, cpw::wide      },
		{ 0x00FF01, 0x00FF60, cpw::wide      },
		{ 0x00FFE0, 0x00FFE6, cpw::wide      },
		{ 0x00FFFD, 0x00FFFD, cpw::ambiguous },
		{ 0x016FE0, 0x016FE4, cpw::wide      },
		{ 0x016FF0, 0x016FF1, cpw::wide      },
		{ 0x017000, 0x0187F7, cpw::wide      },
		{ 0x018800, 0x018CD5, cpw::wide      },
		{ 0x018D00, 0x018D08, cpw::wide      },
		{ 0x01B000, 0x01B11E, cpw::wide      },
		{ 0x01B150, 0x01B152, cpw::wide      },
		{ 0x01B164, 0x01B167, cpw::wide      },
		{ 0x01B170, 0x01B2FB, cpw::wide      },
		{ 0x01F004, 0x01F004, cpw::wide      },
		{ 0x01F0CF, 0x01F0CF, cpw::wide      },
		{ 0x01F100, 0x01F10A, cpw::ambiguous },
		{ 0x01F110, 0x01F12D, cpw::ambiguous },
		{ 0x01F130, 0x01F169, cpw::ambiguous },
		{ 0x01F170, 0x01F18D, cpw::ambiguous },
		{ 0x01F18E, 0x01F18E, cpw::wide      },
		{ 0x01F18F, 0x01F190, cpw::ambiguous },
		{ 0x01F191, 0x01F19A, cpw::wide      },
		{ 0x01F19B, 0x01F1AC, cpw::ambiguous },
		{ 0x01F1E6, 0x01F202, cpw::wide      },
		{ 0x01F210, 0x01F23B, cpw::wide      },
		{ 0x01F240, 0x01F248, cpw::wide      },
		{ 0x01F250, 0x01F251, cpw::wide      },
		{ 0x01F260, 0x01F265, cpw::wide      },
		{ 0x01F300, 0x01F320, cpw::wide      },
		{ 0x01F32D, 0x01F335, cpw::wide      },
		{ 0x01F337, 0x01F37C, cpw::wide      },
		{ 0x01F37E, 0x01F393, cpw::wide      },
		{ 0x01F3A0, 0x01F3CA, cpw::wide      },
		{ 0x01F3CF, 0x01F3D3, cpw::wide      },
		{ 0x01F3E0, 0x01F3F0, cpw::wide      },
		{ 0x01F3F4, 0x01F3F4, cpw::wide      },
		{ 0x01F3F8, 0x01F43E, cpw::wide      },
		{ 0x01F440, 0x01F440, cpw::wide      },
		{ 0x01F442, 0x01F4FC, cpw::wide      },
		{ 0x01F4FF, 0x01F53D, cpw::wide      },
		{ 0x01F54B, 0x01F54E, cpw::wide      },
		{ 0x01F550, 0x01F567, cpw::wide      },
		{ 0x01F57A, 0x01F57A, cpw::wide      },
		{ 0x01F595, 0x01F596, cpw::wide      },
		{ 0x01F5A4, 0x01F5A4, cpw::wide      },
		{ 0x01F5FB, 0x01F64F, cpw::wide      },
		{ 0x01F680, 0x01F6C5, cpw::wide      },
		{ 0x01F6CC, 0x01F6CC, cpw::wide      },
		{ 0x01F6D0, 0x01F6D2, cpw::wide      },
		{ 0x01F6D5, 0x01F6D7, cpw::wide      },
		{ 0x01F6EB, 0x01F6EC, cpw::wide      },
		{ 0x01F6F4, 0x01F6FC, cpw::wide      },
		{ 0x01F7E0, 0x01F7EB, cpw::wide      },
		{ 0x01F90C, 0x01F93A, cpw::wide      },
		{ 0x01F93C, 0x01F945, cpw::wide      },
		{ 0x01F947, 0x01F978, cpw::wide      },
		{ 0x01F97A, 0x01F9CB, cpw::wide      },
		{ 0x01F9CD, 0x01F9FF, cpw::wide      },
		{ 0x01FA70, 0x01FA74, cpw::wide      },
		{ 0x01FA78, 0x01FA7A, cpw::wide      },
		{ 0x01FA80, 0x01FA86, cpw::wide      },
		{ 0x01FA90, 0x01FAA8, cpw::wide      },
		{ 0x01FAB0, 0x01FAB6, cpw::wide      },
		{ 0x01FAC0, 0x01FAC2, cpw::wide      },
		{ 0x01FAD0, 0x01FAD6, cpw::wide      },
		{ 0x020000, 0x02FFFD, cpw::wide      },
		{ 0x030000, 0x03FFFD, cpw::wide      },
		{ 0x0E0100, 0x0E01EF, cpw::ambiguous },
		{ 0x0F0000, 0x0FFFFD, cpw::ambiguous },
		{ 0x100000, 0x10FFFD, cpw::ambiguous },
	};

	[[nodiscard]]
	static auto lookup_width(char_width::codepoint const Codepoint)
	{
		if (
			const auto Iterator = std::lower_bound(ALL_CONST_RANGE(s_WideAndAmbiguousTable), Codepoint);
			Iterator != std::end(s_WideAndAmbiguousTable) && in_closed_range(Iterator->LowerBound, Codepoint, Iterator->UpperBound)
		)
		{
			return Iterator->Width;
		}

		return codepoint_width::narrow;
	}

	[[nodiscard]]
	static auto is_bmp(char_width::codepoint const Codepoint)
	{
		return Codepoint <= std::numeric_limits<wchar_t>::max();
	}

	[[nodiscard]]
	static auto device_width(char_width::codepoint const Codepoint, bool const ClearCacheOnly = false)
	{
		static_assert(sizeof(wchar_t) == 2, "4 GB for a cache is too much, rewrite it.");
		static codepoint_width FastCache[std::numeric_limits<wchar_t>::max()];
		static std::unordered_map<char_width::codepoint, codepoint_width> SlowCache;

		if (ClearCacheOnly)
		{
			std::fill(ALL_RANGE(FastCache), codepoint_width::ambiguous);
			SlowCache.clear();
			(void)console.IsWidePreciseExpensive(0, true);
			return codepoint_width::ambiguous;
		}

		const auto IsBMP = is_bmp(Codepoint);

		if (IsBMP)
		{
			if (FastCache[Codepoint] != codepoint_width::ambiguous)
				return FastCache[Codepoint];
		}
		else
		{
			if (const auto Iterator = SlowCache.find(Codepoint); Iterator != SlowCache.cend())
				return Iterator->second;
		}

		const auto Result = console.IsWidePreciseExpensive(Codepoint)? codepoint_width::wide : codepoint_width::narrow;

		(IsBMP? FastCache[Codepoint] : SlowCache[Codepoint]) = Result;

		return Result;
	}

	[[nodiscard]]
	static auto is_fullwidth_needed()
	{
		return console.IsVtSupported() || locale.is_cjk();
	}

	[[nodiscard]]
	static auto get_width(char_width::codepoint const RawCodepoint)
	{
		const auto Codepoint = RawCodepoint > std::numeric_limits<wchar_t>::max()?
			RawCodepoint :
			ReplaceControlCharacter(static_cast<wchar_t>(RawCodepoint));

		if (const auto Width = quick_width(Codepoint); Width != codepoint_width::ambiguous)
			return Width;

		if (const auto Width = lookup_width(Codepoint); Width != codepoint_width::ambiguous)
			return Width;

		if (const auto Width = device_width(Codepoint); Width != codepoint_width::ambiguous)
			return Width;

		return codepoint_width::wide;
	}
}

namespace char_width
{
	[[nodiscard]]
	bool is_wide(unsigned int const Codepoint)
	{
		switch (s_FullWidthState)
		{
		default:
		case full_width::off:
			return !is_bmp(Codepoint);

		case full_width::automatic:
			if (!is_fullwidth_needed())
				return false;

			[[fallthrough]];

		case full_width::on:
			return get_width(Codepoint) == codepoint_width::wide;
		}
	}

	void enable(int const Value)
	{
		switch (Value)
		{
		case BSTATE_UNCHECKED:
			invalidate();
			s_FullWidthState = full_width::off;
			break;

		case BSTATE_CHECKED:
			s_FullWidthState = full_width::on;
			break;

		case BSTATE_3STATE:
			s_FullWidthState = full_width::automatic;
			break;
		}
	}

	[[nodiscard]]
	bool is_enabled()
	{
		return s_FullWidthState != full_width::off;
	}

	void invalidate()
	{
		if (is_enabled())
			(void)device_width(0, true);
	}
}

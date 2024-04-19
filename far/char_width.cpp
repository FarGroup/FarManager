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

	auto s_FullWidthState = full_width::off;

	enum class codepoint_width: char
	{
		ambiguous,
		narrow,
		wide,
	};

	[[nodiscard]]
	auto quick_width(char_width::codepoint const Codepoint) noexcept
	{
		return in_closed_range(U'\U00000020', Codepoint, U'\U0000007E')?
			codepoint_width::narrow :
			codepoint_width::ambiguous;
	}

	struct unicode_range
	{
		char_width::codepoint
			LowerBound,
			UpperBound;

		codepoint_width Width;
	};

	// These mappings are based on src\types\CodepointWidthDetector.cpp from Windows Terminal.
	// Generated on 2022-11-15 19:54:23Z from Unicode 15.0.0.

	// Note: unlike Terminal, we don't use any overrides since we don't control any drawing code
	// and have no other choice but to do as the Romans do.
	using cpw = codepoint_width;
	constexpr unicode_range s_WideAndAmbiguousTable[]
	{
		{ U'\U000000A1', U'\U000000A1', cpw::ambiguous },
		{ U'\U000000A4', U'\U000000A4', cpw::ambiguous },
		{ U'\U000000A7', U'\U000000A8', cpw::ambiguous },
		{ U'\U000000AA', U'\U000000AA', cpw::ambiguous },
		{ U'\U000000AD', U'\U000000AE', cpw::ambiguous },
		{ U'\U000000B0', U'\U000000B4', cpw::ambiguous },
		{ U'\U000000B6', U'\U000000BA', cpw::ambiguous },
		{ U'\U000000BC', U'\U000000BF', cpw::ambiguous },
		{ U'\U000000C6', U'\U000000C6', cpw::ambiguous },
		{ U'\U000000D0', U'\U000000D0', cpw::ambiguous },
		{ U'\U000000D7', U'\U000000D8', cpw::ambiguous },
		{ U'\U000000DE', U'\U000000E1', cpw::ambiguous },
		{ U'\U000000E6', U'\U000000E6', cpw::ambiguous },
		{ U'\U000000E8', U'\U000000EA', cpw::ambiguous },
		{ U'\U000000EC', U'\U000000ED', cpw::ambiguous },
		{ U'\U000000F0', U'\U000000F0', cpw::ambiguous },
		{ U'\U000000F2', U'\U000000F3', cpw::ambiguous },
		{ U'\U000000F7', U'\U000000FA', cpw::ambiguous },
		{ U'\U000000FC', U'\U000000FC', cpw::ambiguous },
		{ U'\U000000FE', U'\U000000FE', cpw::ambiguous },
		{ U'\U00000101', U'\U00000101', cpw::ambiguous },
		{ U'\U00000111', U'\U00000111', cpw::ambiguous },
		{ U'\U00000113', U'\U00000113', cpw::ambiguous },
		{ U'\U0000011B', U'\U0000011B', cpw::ambiguous },
		{ U'\U00000126', U'\U00000127', cpw::ambiguous },
		{ U'\U0000012B', U'\U0000012B', cpw::ambiguous },
		{ U'\U00000131', U'\U00000133', cpw::ambiguous },
		{ U'\U00000138', U'\U00000138', cpw::ambiguous },
		{ U'\U0000013F', U'\U00000142', cpw::ambiguous },
		{ U'\U00000144', U'\U00000144', cpw::ambiguous },
		{ U'\U00000148', U'\U0000014B', cpw::ambiguous },
		{ U'\U0000014D', U'\U0000014D', cpw::ambiguous },
		{ U'\U00000152', U'\U00000153', cpw::ambiguous },
		{ U'\U00000166', U'\U00000167', cpw::ambiguous },
		{ U'\U0000016B', U'\U0000016B', cpw::ambiguous },
		{ U'\U000001CE', U'\U000001CE', cpw::ambiguous },
		{ U'\U000001D0', U'\U000001D0', cpw::ambiguous },
		{ U'\U000001D2', U'\U000001D2', cpw::ambiguous },
		{ U'\U000001D4', U'\U000001D4', cpw::ambiguous },
		{ U'\U000001D6', U'\U000001D6', cpw::ambiguous },
		{ U'\U000001D8', U'\U000001D8', cpw::ambiguous },
		{ U'\U000001DA', U'\U000001DA', cpw::ambiguous },
		{ U'\U000001DC', U'\U000001DC', cpw::ambiguous },
		{ U'\U00000251', U'\U00000251', cpw::ambiguous },
		{ U'\U00000261', U'\U00000261', cpw::ambiguous },
		{ U'\U000002C4', U'\U000002C4', cpw::ambiguous },
		{ U'\U000002C7', U'\U000002C7', cpw::ambiguous },
		{ U'\U000002C9', U'\U000002CB', cpw::ambiguous },
		{ U'\U000002CD', U'\U000002CD', cpw::ambiguous },
		{ U'\U000002D0', U'\U000002D0', cpw::ambiguous },
		{ U'\U000002D8', U'\U000002DB', cpw::ambiguous },
		{ U'\U000002DD', U'\U000002DD', cpw::ambiguous },
		{ U'\U000002DF', U'\U000002DF', cpw::ambiguous },
		{ U'\U00000300', U'\U0000036F', cpw::ambiguous },
		{ U'\U00000391', U'\U000003A1', cpw::ambiguous },
		{ U'\U000003A3', U'\U000003A9', cpw::ambiguous },
		{ U'\U000003B1', U'\U000003C1', cpw::ambiguous },
		{ U'\U000003C3', U'\U000003C9', cpw::ambiguous },
		{ U'\U00000401', U'\U00000401', cpw::ambiguous },
		{ U'\U00000410', U'\U0000044F', cpw::ambiguous },
		{ U'\U00000451', U'\U00000451', cpw::ambiguous },
		{ U'\U00001100', U'\U0000115F', cpw::wide      },
		{ U'\U00002010', U'\U00002010', cpw::ambiguous },
		{ U'\U00002013', U'\U00002016', cpw::ambiguous },
		{ U'\U00002018', U'\U00002019', cpw::ambiguous },
		{ U'\U0000201C', U'\U0000201D', cpw::ambiguous },
		{ U'\U00002020', U'\U00002022', cpw::ambiguous },
		{ U'\U00002024', U'\U00002027', cpw::ambiguous },
		{ U'\U00002030', U'\U00002030', cpw::ambiguous },
		{ U'\U00002032', U'\U00002033', cpw::ambiguous },
		{ U'\U00002035', U'\U00002035', cpw::ambiguous },
		{ U'\U0000203B', U'\U0000203B', cpw::ambiguous },
		{ U'\U0000203E', U'\U0000203E', cpw::ambiguous },
		{ U'\U00002074', U'\U00002074', cpw::ambiguous },
		{ U'\U0000207F', U'\U0000207F', cpw::ambiguous },
		{ U'\U00002081', U'\U00002084', cpw::ambiguous },
		{ U'\U000020AC', U'\U000020AC', cpw::ambiguous },
		{ U'\U00002103', U'\U00002103', cpw::ambiguous },
		{ U'\U00002105', U'\U00002105', cpw::ambiguous },
		{ U'\U00002109', U'\U00002109', cpw::ambiguous },
		{ U'\U00002113', U'\U00002113', cpw::ambiguous },
		{ U'\U00002116', U'\U00002116', cpw::ambiguous },
		{ U'\U00002121', U'\U00002122', cpw::ambiguous },
		{ U'\U00002126', U'\U00002126', cpw::ambiguous },
		{ U'\U0000212B', U'\U0000212B', cpw::ambiguous },
		{ U'\U00002153', U'\U00002154', cpw::ambiguous },
		{ U'\U0000215B', U'\U0000215E', cpw::ambiguous },
		{ U'\U00002160', U'\U0000216B', cpw::ambiguous },
		{ U'\U00002170', U'\U00002179', cpw::ambiguous },
		{ U'\U00002189', U'\U00002189', cpw::ambiguous },
		{ U'\U00002190', U'\U00002199', cpw::ambiguous },
		{ U'\U000021B8', U'\U000021B9', cpw::ambiguous },
		{ U'\U000021D2', U'\U000021D2', cpw::ambiguous },
		{ U'\U000021D4', U'\U000021D4', cpw::ambiguous },
		{ U'\U000021E7', U'\U000021E7', cpw::ambiguous },
		{ U'\U00002200', U'\U00002200', cpw::ambiguous },
		{ U'\U00002202', U'\U00002203', cpw::ambiguous },
		{ U'\U00002207', U'\U00002208', cpw::ambiguous },
		{ U'\U0000220B', U'\U0000220B', cpw::ambiguous },
		{ U'\U0000220F', U'\U0000220F', cpw::ambiguous },
		{ U'\U00002211', U'\U00002211', cpw::ambiguous },
		{ U'\U00002215', U'\U00002215', cpw::ambiguous },
		{ U'\U0000221A', U'\U0000221A', cpw::ambiguous },
		{ U'\U0000221D', U'\U00002220', cpw::ambiguous },
		{ U'\U00002223', U'\U00002223', cpw::ambiguous },
		{ U'\U00002225', U'\U00002225', cpw::ambiguous },
		{ U'\U00002227', U'\U0000222C', cpw::ambiguous },
		{ U'\U0000222E', U'\U0000222E', cpw::ambiguous },
		{ U'\U00002234', U'\U00002237', cpw::ambiguous },
		{ U'\U0000223C', U'\U0000223D', cpw::ambiguous },
		{ U'\U00002248', U'\U00002248', cpw::ambiguous },
		{ U'\U0000224C', U'\U0000224C', cpw::ambiguous },
		{ U'\U00002252', U'\U00002252', cpw::ambiguous },
		{ U'\U00002260', U'\U00002261', cpw::ambiguous },
		{ U'\U00002264', U'\U00002267', cpw::ambiguous },
		{ U'\U0000226A', U'\U0000226B', cpw::ambiguous },
		{ U'\U0000226E', U'\U0000226F', cpw::ambiguous },
		{ U'\U00002282', U'\U00002283', cpw::ambiguous },
		{ U'\U00002286', U'\U00002287', cpw::ambiguous },
		{ U'\U00002295', U'\U00002295', cpw::ambiguous },
		{ U'\U00002299', U'\U00002299', cpw::ambiguous },
		{ U'\U000022A5', U'\U000022A5', cpw::ambiguous },
		{ U'\U000022BF', U'\U000022BF', cpw::ambiguous },
		{ U'\U00002312', U'\U00002312', cpw::ambiguous },
		{ U'\U0000231A', U'\U0000231B', cpw::wide      },
		{ U'\U00002329', U'\U0000232A', cpw::wide      },
		{ U'\U000023E9', U'\U000023EC', cpw::wide      },
		{ U'\U000023F0', U'\U000023F0', cpw::wide      },
		{ U'\U000023F3', U'\U000023F3', cpw::wide      },
		{ U'\U00002460', U'\U000024E9', cpw::ambiguous },
		{ U'\U000024EB', U'\U000024FF', cpw::ambiguous },
		{ U'\U00002500', U'\U0000259F', cpw::ambiguous },
		{ U'\U000025A0', U'\U000025A1', cpw::ambiguous },
		{ U'\U000025A3', U'\U000025A9', cpw::ambiguous },
		{ U'\U000025B2', U'\U000025B3', cpw::ambiguous },
		{ U'\U000025B6', U'\U000025B7', cpw::ambiguous },
		{ U'\U000025BC', U'\U000025BD', cpw::ambiguous },
		{ U'\U000025C0', U'\U000025C1', cpw::ambiguous },
		{ U'\U000025C6', U'\U000025C8', cpw::ambiguous },
		{ U'\U000025CB', U'\U000025CB', cpw::ambiguous },
		{ U'\U000025CE', U'\U000025D1', cpw::ambiguous },
		{ U'\U000025E2', U'\U000025E5', cpw::ambiguous },
		{ U'\U000025EF', U'\U000025EF', cpw::ambiguous },
		{ U'\U000025FD', U'\U000025FE', cpw::wide      },
		{ U'\U00002605', U'\U00002606', cpw::ambiguous },
		{ U'\U00002609', U'\U00002609', cpw::ambiguous },
		{ U'\U0000260E', U'\U0000260F', cpw::ambiguous },
		{ U'\U00002614', U'\U00002615', cpw::wide      },
		{ U'\U0000261C', U'\U0000261C', cpw::ambiguous },
		{ U'\U0000261E', U'\U0000261E', cpw::ambiguous },
		{ U'\U00002640', U'\U00002640', cpw::ambiguous },
		{ U'\U00002642', U'\U00002642', cpw::ambiguous },
		{ U'\U00002648', U'\U00002653', cpw::wide      },
		{ U'\U00002660', U'\U00002661', cpw::ambiguous },
		{ U'\U00002663', U'\U00002665', cpw::ambiguous },
		{ U'\U00002667', U'\U0000266A', cpw::ambiguous },
		{ U'\U0000266C', U'\U0000266D', cpw::ambiguous },
		{ U'\U0000266F', U'\U0000266F', cpw::ambiguous },
		{ U'\U0000267F', U'\U0000267F', cpw::wide      },
		{ U'\U00002693', U'\U00002693', cpw::wide      },
		{ U'\U0000269E', U'\U0000269F', cpw::ambiguous },
		{ U'\U000026A1', U'\U000026A1', cpw::wide      },
		{ U'\U000026AA', U'\U000026AB', cpw::wide      },
		{ U'\U000026BD', U'\U000026BE', cpw::wide      },
		{ U'\U000026BF', U'\U000026BF', cpw::ambiguous },
		{ U'\U000026C4', U'\U000026C5', cpw::wide      },
		{ U'\U000026C6', U'\U000026CD', cpw::ambiguous },
		{ U'\U000026CE', U'\U000026CE', cpw::wide      },
		{ U'\U000026CF', U'\U000026D3', cpw::ambiguous },
		{ U'\U000026D4', U'\U000026D4', cpw::wide      },
		{ U'\U000026D5', U'\U000026E1', cpw::ambiguous },
		{ U'\U000026E3', U'\U000026E3', cpw::ambiguous },
		{ U'\U000026E8', U'\U000026E9', cpw::ambiguous },
		{ U'\U000026EA', U'\U000026EA', cpw::wide      },
		{ U'\U000026EB', U'\U000026F1', cpw::ambiguous },
		{ U'\U000026F2', U'\U000026F3', cpw::wide      },
		{ U'\U000026F4', U'\U000026F4', cpw::ambiguous },
		{ U'\U000026F5', U'\U000026F5', cpw::wide      },
		{ U'\U000026F6', U'\U000026F9', cpw::ambiguous },
		{ U'\U000026FA', U'\U000026FA', cpw::wide      },
		{ U'\U000026FB', U'\U000026FC', cpw::ambiguous },
		{ U'\U000026FD', U'\U000026FD', cpw::wide      },
		{ U'\U000026FE', U'\U000026FF', cpw::ambiguous },
		{ U'\U00002705', U'\U00002705', cpw::wide      },
		{ U'\U0000270A', U'\U0000270B', cpw::wide      },
		{ U'\U00002728', U'\U00002728', cpw::wide      },
		{ U'\U0000273D', U'\U0000273D', cpw::ambiguous },
		{ U'\U0000274C', U'\U0000274C', cpw::wide      },
		{ U'\U0000274E', U'\U0000274E', cpw::wide      },
		{ U'\U00002753', U'\U00002755', cpw::wide      },
		{ U'\U00002757', U'\U00002757', cpw::wide      },
		{ U'\U00002776', U'\U0000277F', cpw::ambiguous },
		{ U'\U00002795', U'\U00002797', cpw::wide      },
		{ U'\U000027B0', U'\U000027B0', cpw::wide      },
		{ U'\U000027BF', U'\U000027BF', cpw::wide      },
		{ U'\U00002B1B', U'\U00002B1C', cpw::wide      },
		{ U'\U00002B50', U'\U00002B50', cpw::wide      },
		{ U'\U00002B55', U'\U00002B55', cpw::wide      },
		{ U'\U00002B56', U'\U00002B59', cpw::ambiguous },
		{ U'\U00002E80', U'\U00002E99', cpw::wide      },
		{ U'\U00002E9B', U'\U00002EF3', cpw::wide      },
		{ U'\U00002F00', U'\U00002FD5', cpw::wide      },
		{ U'\U00002FF0', U'\U00002FFB', cpw::wide      },
		{ U'\U00003000', U'\U0000303E', cpw::wide      },
		{ U'\U00003041', U'\U00003096', cpw::wide      },
		{ U'\U00003099', U'\U000030FF', cpw::wide      },
		{ U'\U00003105', U'\U0000312F', cpw::wide      },
		{ U'\U00003131', U'\U0000318E', cpw::wide      },
		{ U'\U00003190', U'\U000031E3', cpw::wide      },
		{ U'\U000031F0', U'\U0000321E', cpw::wide      },
		{ U'\U00003220', U'\U00003247', cpw::wide      },
		{ U'\U00003248', U'\U0000324F', cpw::ambiguous },
		{ U'\U00003250', U'\U00004DBF', cpw::wide      },
		{ U'\U00004DC0', U'\U00004DFF', cpw::ambiguous },
		{ U'\U00004E00', U'\U0000A48C', cpw::wide      },
		{ U'\U0000A490', U'\U0000A4C6', cpw::wide      },
		{ U'\U0000A960', U'\U0000A97C', cpw::wide      },
		{ U'\U0000AC00', U'\U0000D7A3', cpw::wide      },
		{ U'\U0000E000', U'\U0000F8FF', cpw::ambiguous },
		{ U'\U0000F900', U'\U0000FAFF', cpw::wide      },
		{ U'\U0000FE00', U'\U0000FE0F', cpw::ambiguous },
		{ U'\U0000FE10', U'\U0000FE19', cpw::wide      },
		{ U'\U0000FE20', U'\U0000FE2F', cpw::ambiguous },
		{ U'\U0000FE30', U'\U0000FE52', cpw::wide      },
		{ U'\U0000FE54', U'\U0000FE66', cpw::wide      },
		{ U'\U0000FE68', U'\U0000FE6B', cpw::wide      },
		{ U'\U0000FF01', U'\U0000FF60', cpw::wide      },
		{ U'\U0000FFE0', U'\U0000FFE6', cpw::wide      },
		{ U'\U0000FFFD', U'\U0000FFFD', cpw::ambiguous },
		{ U'\U00016FE0', U'\U00016FE4', cpw::wide      },
		{ U'\U00016FF0', U'\U00016FF1', cpw::wide      },
		{ U'\U00017000', U'\U000187F7', cpw::wide      },
		{ U'\U00018800', U'\U00018CD5', cpw::wide      },
		{ U'\U00018D00', U'\U00018D08', cpw::wide      },
		{ U'\U0001AFF0', U'\U0001AFF3', cpw::wide      },
		{ U'\U0001AFF5', U'\U0001AFFB', cpw::wide      },
		{ U'\U0001AFFD', U'\U0001AFFE', cpw::wide      },
		{ U'\U0001B000', U'\U0001B122', cpw::wide      },
		{ U'\U0001B132', U'\U0001B132', cpw::wide      },
		{ U'\U0001B150', U'\U0001B152', cpw::wide      },
		{ U'\U0001B155', U'\U0001B155', cpw::wide      },
		{ U'\U0001B164', U'\U0001B167', cpw::wide      },
		{ U'\U0001B170', U'\U0001B2FB', cpw::wide      },
		{ U'\U0001F004', U'\U0001F004', cpw::wide      },
		{ U'\U0001F0CF', U'\U0001F0CF', cpw::wide      },
		{ U'\U0001F100', U'\U0001F10A', cpw::ambiguous },
		{ U'\U0001F110', U'\U0001F12D', cpw::ambiguous },
		{ U'\U0001F130', U'\U0001F169', cpw::ambiguous },
		{ U'\U0001F170', U'\U0001F18D', cpw::ambiguous },
		{ U'\U0001F18E', U'\U0001F18E', cpw::wide      },
		{ U'\U0001F18F', U'\U0001F190', cpw::ambiguous },
		{ U'\U0001F191', U'\U0001F19A', cpw::wide      },
		{ U'\U0001F19B', U'\U0001F1AC', cpw::ambiguous },
		{ U'\U0001F1E6', U'\U0001F202', cpw::wide      },
		{ U'\U0001F210', U'\U0001F23B', cpw::wide      },
		{ U'\U0001F240', U'\U0001F248', cpw::wide      },
		{ U'\U0001F250', U'\U0001F251', cpw::wide      },
		{ U'\U0001F260', U'\U0001F265', cpw::wide      },
		{ U'\U0001F300', U'\U0001F320', cpw::wide      },
		{ U'\U0001F32D', U'\U0001F335', cpw::wide      },
		{ U'\U0001F337', U'\U0001F37C', cpw::wide      },
		{ U'\U0001F37E', U'\U0001F393', cpw::wide      },
		{ U'\U0001F3A0', U'\U0001F3CA', cpw::wide      },
		{ U'\U0001F3CF', U'\U0001F3D3', cpw::wide      },
		{ U'\U0001F3E0', U'\U0001F3F0', cpw::wide      },
		{ U'\U0001F3F4', U'\U0001F3F4', cpw::wide      },
		{ U'\U0001F3F8', U'\U0001F43E', cpw::wide      },
		{ U'\U0001F440', U'\U0001F440', cpw::wide      },
		{ U'\U0001F442', U'\U0001F4FC', cpw::wide      },
		{ U'\U0001F4FF', U'\U0001F53D', cpw::wide      },
		{ U'\U0001F54B', U'\U0001F54E', cpw::wide      },
		{ U'\U0001F550', U'\U0001F567', cpw::wide      },
		{ U'\U0001F57A', U'\U0001F57A', cpw::wide      },
		{ U'\U0001F595', U'\U0001F596', cpw::wide      },
		{ U'\U0001F5A4', U'\U0001F5A4', cpw::wide      },
		{ U'\U0001F5FB', U'\U0001F64F', cpw::wide      },
		{ U'\U0001F680', U'\U0001F6C5', cpw::wide      },
		{ U'\U0001F6CC', U'\U0001F6CC', cpw::wide      },
		{ U'\U0001F6D0', U'\U0001F6D2', cpw::wide      },
		{ U'\U0001F6D5', U'\U0001F6D7', cpw::wide      },
		{ U'\U0001F6DC', U'\U0001F6DF', cpw::wide      },
		{ U'\U0001F6EB', U'\U0001F6EC', cpw::wide      },
		{ U'\U0001F6F4', U'\U0001F6FC', cpw::wide      },
		{ U'\U0001F7E0', U'\U0001F7EB', cpw::wide      },
		{ U'\U0001F7F0', U'\U0001F7F0', cpw::wide      },
		{ U'\U0001F90C', U'\U0001F93A', cpw::wide      },
		{ U'\U0001F93C', U'\U0001F945', cpw::wide      },
		{ U'\U0001F947', U'\U0001F9FF', cpw::wide      },
		{ U'\U0001FA70', U'\U0001FA7C', cpw::wide      },
		{ U'\U0001FA80', U'\U0001FA88', cpw::wide      },
		{ U'\U0001FA90', U'\U0001FABD', cpw::wide      },
		{ U'\U0001FABF', U'\U0001FAC5', cpw::wide      },
		{ U'\U0001FACE', U'\U0001FADB', cpw::wide      },
		{ U'\U0001FAE0', U'\U0001FAE8', cpw::wide      },
		{ U'\U0001FAF0', U'\U0001FAF8', cpw::wide      },
		{ U'\U00020000', U'\U0002FFFD', cpw::wide      },
		{ U'\U00030000', U'\U0003FFFD', cpw::wide      },
		{ U'\U000E0100', U'\U000E01EF', cpw::ambiguous },
		{ U'\U000F0000', U'\U000FFFFD', cpw::ambiguous },
		{ U'\U00100000', U'\U0010FFFD', cpw::ambiguous },
	};

	[[nodiscard]]
	auto lookup_width(char_width::codepoint const Codepoint)
	{
		if (
			const auto Iterator = std::ranges::lower_bound(s_WideAndAmbiguousTable, Codepoint, {}, &unicode_range::UpperBound);
			Iterator != std::end(s_WideAndAmbiguousTable) && in_closed_range(Iterator->LowerBound, Codepoint, Iterator->UpperBound)
		)
		{
			return Iterator->Width;
		}

		return codepoint_width::narrow;
	}

	[[nodiscard]]
	auto is_bmp(char_width::codepoint const Codepoint)
	{
		return Codepoint <= std::numeric_limits<char16_t>::max();
	}

	[[nodiscard]]
	auto device_width(char_width::codepoint const Codepoint, bool const ClearCacheOnly = false)
	{
		static std::array<codepoint_width, std::numeric_limits<char16_t>::max()> FastCache;
		static std::unordered_map<char_width::codepoint, codepoint_width> SlowCache;

		if (ClearCacheOnly)
		{
			FastCache.fill(codepoint_width::ambiguous);
			SlowCache.clear();
			console.ClearWideCache();
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
	auto is_fullwidth_needed()
	{
		return console.IsVtSupported() || locale.is_cjk();
	}

	[[nodiscard]]
	auto get_width(char_width::codepoint const RawCodepoint)
	{
		const auto Codepoint = RawCodepoint > std::numeric_limits<char16_t>::max()?
			RawCodepoint :
			static_cast<char_width::codepoint>(ReplaceControlCharacter(static_cast<wchar_t>(RawCodepoint)));

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
	bool is_wide(codepoint const Codepoint)
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

	bool is_half_width_surrogate_broken()
	{
		// As of 23 Jun 2022 conhost and WT render half-width surrogates as half-width,
		// but advance the cursor position as if they were full-width.
		// We can workaround it by moving the cursor backwards each time.
		// They might fix it eventually, so it's better to detect it dynamically.

		// Mathematical Bold Fraktur Small A, U+1D586, half-width
		static const auto Result = console.IsWidePreciseExpensive(U'𝖆');
		return Result;
	}
}

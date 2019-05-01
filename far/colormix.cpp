/*
colormix.cpp

Работа с цветами
*/
/*
Copyright © 2011 Far Group
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

// Self:
#include "colormix.hpp"

// Internal:
#include "config.hpp"
#include "console.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"

// External:

//----------------------------------------------------------------------------

enum
{
	DefaultColor=0xf,
	ConsoleMask=0xf,
	ConsoleBgShift=4,
	ConsoleFgShift=0,
};

namespace colors
{
	COLORREF index_value(COLORREF const Colour)
	{
		return Colour & INDEXMASK;
	}

	COLORREF color_value(COLORREF const Colour)
	{
		return Colour & COLORMASK;
	}

	COLORREF alpha_value(COLORREF const Colour)
	{
		return Colour & ALPHAMASK;
	}

	bool is_opaque(COLORREF const Colour)
	{
		return alpha_value(Colour) == ALPHAMASK;
	}

	bool is_transparent(COLORREF const Colour)
	{
		return !alpha_value(Colour);
	}

	COLORREF opaque(COLORREF const Colour)
	{
		return Colour | ALPHAMASK;
	}

	COLORREF transparent(COLORREF const Colour)
	{
		return Colour & COLORMASK;
	}

	void make_opaque(COLORREF& Colour)
	{
		Colour = opaque(Colour);
	}

	void make_transparent(COLORREF& Colour)
	{
		Colour = transparent(Colour);
	}

	size_t color_hash(const FarColor& Value)
	{
		size_t Seed = 0;

		hash_combine(Seed, Value.Flags);
		hash_combine(Seed, Value.BackgroundColor);
		hash_combine(Seed, Value.ForegroundColor);
		hash_combine(Seed, Value.Reserved);

		return Seed;
	}

	FarColor merge(const FarColor& Bottom, const FarColor& Top)
	{
		FarColor Result = Bottom;

		const auto ApplyColorPart = [&](COLORREF FarColor::*ColorAccessor, const FARCOLORFLAGS Flag)
		{
			const auto TopPart = std::invoke(ColorAccessor, Top);
			if (is_opaque(TopPart))
			{
				std::invoke(ColorAccessor, Result) = TopPart;
				(Top.Flags & Flag)? (Result.Flags |= Flag) : (Result.Flags &= ~Flag);
			}
		};

		ApplyColorPart(&FarColor::BackgroundColor, FCF_BG_4BIT);
		ApplyColorPart(&FarColor::ForegroundColor, FCF_FG_4BIT);

		Result.Flags |= Top.Flags & FCF_EXTENDEDFLAGS;

		return Result;
	}

WORD FarColorToConsoleColor(const FarColor& Color)
{
	static COLORREF LastTrueColors[2] = {};
	static FARCOLORFLAGS LastFlags = 0;
	static WORD Result = 0;

	if (Color.BackgroundColor == LastTrueColors[0] && Color.ForegroundColor == LastTrueColors[1] && (Color.Flags & FCF_4BITMASK) == (LastFlags & FCF_4BITMASK))
		return Result;

	LastFlags = Color.Flags;

	static std::array<BYTE, 2> IndexColors{};

	const struct
	{
		const COLORREF Color;
		const rgba RGBA;
		FARCOLORFLAGS Flags;
		COLORREF* LastColor;
		BYTE* IndexColor;
	}
	data[] =
	{
		{Color.BackgroundColor, Color.BackgroundRGBA, FCF_BG_4BIT, &LastTrueColors[0], &IndexColors[0]},
		{Color.ForegroundColor, Color.ForegroundRGBA, FCF_FG_4BIT, &LastTrueColors[1], &IndexColors[1]}
	};

	enum console_mask
	{
		BlueMask      = 0_bit,
		GreenMask     = 1_bit,
		RedMask       = 2_bit,
		IntensityMask = 3_bit,
	};

	for (auto& i: data)
	{
		if (i.Color == *i.LastColor)
			continue;

		*i.LastColor = i.Color;
		if(Color.Flags & i.Flags)
		{
			*i.IndexColor = i.Color & ConsoleMask;
			continue;
		}

		int R = i.RGBA.r;
		int G = i.RGBA.g;
		int B = i.RGBA.b;

		// special case, silver color:
		if (in_range(160, R, 223) && in_range(160, G, 223) && in_range(160, B, 223))
		{
			*i.IndexColor = RedMask | GreenMask | BlueMask;
			continue;
		}

		int* p[] = { &R, &G, &B };
		size_t IntenseCount = 0;
		for (auto& component : p)
		{
			if(in_range(0, *component, 63))
			{
				*component = 0;
			}
			else if(in_range(64, *component, 191))
			{
				*component = 128;
			}
			else if(in_range(192, *component, 255))
			{
				*component = 255;
				++IntenseCount;
			}
		}

		// eliminate mixed intensity
		if(IntenseCount > 0 && IntenseCount < 3)
		{
			for(auto& component: p)
			{
				if(*component == 128)
				{
					*component = IntenseCount == 1? 0 : 255;
				}
			}
		}

		const auto ToMask = [](size_t component, console_mask mask) { return component? mask : 0; };
		*i.IndexColor = ToMask(R, RedMask) | ToMask(G, GreenMask) | ToMask(B, BlueMask) | ToMask(IntenseCount, IntensityMask);
	}

	auto FinalIndexColors = IndexColors;

	if (color_value(data[0].Color) != color_value(data[1].Color) && FinalIndexColors[0] == FinalIndexColors[1])
	{
		// oops, unreadable
		// since background is more pronounced we try to adjust the foreground first
		FinalIndexColors[1] & IntensityMask? FinalIndexColors[1] &= ~IntensityMask : FinalIndexColors[0] |= IntensityMask;
	}

	Result = (FinalIndexColors[0] << ConsoleBgShift) | (FinalIndexColors[1] << ConsoleFgShift) | (Color.Flags & FCF_RAWATTR_MASK);

	return Result;
}

FarColor ConsoleColorToFarColor(WORD Color)
{
	FarColor NewColor{};
	NewColor.Flags = FCF_FG_4BIT | FCF_BG_4BIT | (Color & FCF_RAWATTR_MASK);
	NewColor.ForegroundColor = opaque((Color >> ConsoleFgShift) & ConsoleMask);
	NewColor.BackgroundColor = opaque((Color >> ConsoleBgShift) & ConsoleMask);
	return NewColor;
}

COLORREF ConsoleIndexToTrueColor(int Index)
{
	std::array<COLORREF, 16> Palette;
	if (!console.GetPalette(Palette))
	{
		Palette =
		{
			//  BBGGRR
			0x00000000, // black
			0x00800000, // blue
			0x00008000, // green
			0x00808000, // cyan
			0x00000080, // red
			0x00800080, // magenta
			0x00008080, // yellow
			0x00C0C0C0, // white
			0x00808080, // bright black
			0x00FF0000, // bright blue
			0x0000FF00, // bright green
			0x00FFFF00, // bright cyan
			0x000000FF, // bright red
			0x00FF00FF, // bright magenta
			0x0000FFFF, // bright yellow
			0x00FFFFFF  // white
		};
	}

	return opaque(Palette[Index & ConsoleMask]);
}


const FarColor& PaletteColorToFarColor(PaletteColors ColorIndex)
{
	return Global->Opt->Palette[ColorIndex];
}

const FarColor* StoreColor(const FarColor& Value)
{
	static std::unordered_set<FarColor> ColorSet;
	return &*ColorSet.emplace(Value).first;
}

COLORREF ARGB2ABGR(int Color)
{
	return (Color & 0xFF000000) | ((Color & 0x00FF0000) >> 16) | (Color & 0x0000FF00) | ((Color & 0x000000FF) << 16);
}

static bool ExtractColor(string_view const Str, COLORREF& Target, FARCOLORFLAGS& TargetFlags, FARCOLORFLAGS SetFlag)
{
	const auto IsTrueColour = Str.front() == L'T';

	if (!from_string(Str.substr(IsTrueColour? 1 : 0), Target, nullptr, 16))
		return false;

	if (IsTrueColour)
	{
		Target = ARGB2ABGR(Target);
		TargetFlags &= ~SetFlag;
	}
	else
	{
		TargetFlags |= SetFlag;
	}

	return true;
}

string_view ExtractColorInNewFormat(string_view const Str, FarColor& Color, bool& Stop)
{
	Stop = false;

	if (!starts_with(Str, L'('))
		return Str;

	const auto FgColorBegin = Str.cbegin() + 1;
	const auto ColorEnd = std::find(FgColorBegin, Str.cend(), L')');
	if (ColorEnd == Str.cend())
	{
		Stop = true;
		return Str;
	}

	const auto FgColorEnd = std::find(FgColorBegin, ColorEnd, L':');
	const auto BgColorBegin = FgColorEnd == ColorEnd? ColorEnd : FgColorEnd + 1;
	const auto BgColorEnd = ColorEnd;

	auto NewColor = Color;
	if (
		(FgColorBegin == FgColorEnd || ExtractColor(make_string_view(FgColorBegin, FgColorEnd), NewColor.ForegroundColor, NewColor.Flags, FCF_FG_4BIT)) &&
		(BgColorBegin == BgColorEnd || ExtractColor(make_string_view(BgColorBegin, BgColorEnd), NewColor.BackgroundColor, NewColor.Flags, FCF_BG_4BIT))
	)
	{
		Color = NewColor;
		return make_string_view(ColorEnd + 1, Str.cend());
	}

	return Str;
}

}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("colors.parser")
{
	static const struct
	{
		string_view Input;
		FarColor Color;
	}
	ValidTests[]
	{
		{L"(E)"sv, { FCF_FG_4BIT, 0xE, 0 }},
		{L"(:F)"sv, { FCF_BG_4BIT, 0, 0xF }},
		{L"(B:C)"sv, { FCF_FG_4BIT | FCF_BG_4BIT, 0xB, 0xC }},
		{L"()"sv, {}},
		{L"(T00CCCC:TE34234)"sv, { 0, 0x00CCCC00, 0x003442E3}},
	};

	for (const auto& i: ValidTests)
	{
		FarColor Color{};
		bool Stop = false;
		const auto Tail = colors::ExtractColorInNewFormat(i.Input, Color, Stop);
		REQUIRE(Color == i.Color);
		REQUIRE(Tail.empty());
		REQUIRE(!Stop);
	}

	static const struct
	{
		string_view Input;
		bool Stop;
	}
	InvalidTests[]
	{
		{L""sv, false},
		{L"("sv, true},
		{L"(z"sv, true},
		{L"(z)"sv, false},
		{L"(0:z)"sv, false},
		{L"(z:0)"sv, false},
		{L"(Tz)"sv, false},
		{L"( )"sv, false},
		{L"( 0)"sv, false},
		{L"( -0)"sv, false},
		{L"( +0)"sv, false},
	};

	for (const auto& i: InvalidTests)
	{
		FarColor Color{};
		bool Stop = false;
		const auto Tail = colors::ExtractColorInNewFormat(i.Input, Color, Stop);
		REQUIRE(Tail.size() == i.Input.size());
		REQUIRE(Stop == i.Stop);
	}
}
#endif

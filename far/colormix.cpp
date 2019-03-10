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

#include "colormix.hpp"

#include "config.hpp"
#include "console.hpp"
#include "string_utils.hpp"
#include "global.hpp"

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

		const auto& ApplyColorPart = [&](COLORREF FarColor::*ColorAccessor, const FARCOLORFLAGS Flag)
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
		BlueMask      = bit(0),
		GreenMask     = bit(1),
		RedMask       = bit(2),
		IntensityMask = bit(3),
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
		if (InRange(160, R, 223) && InRange(160, G, 223) && InRange(160, B, 223))
		{
			*i.IndexColor = RedMask | GreenMask | BlueMask;
			continue;
		}

		int* p[] = { &R, &G, &B };
		size_t IntenseCount = 0;
		for (auto& component : p)
		{
			if(InRange(0, *component, 63))
			{
				*component = 0;
			}
			else if(InRange(64, *component, 191))
			{
				*component = 128;
			}
			else if(InRange(192, *component, 255))
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

		const auto& ToMask = [](size_t component, console_mask mask) { return component? mask : 0; };
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

static const wchar_t* ExtractColor(const wchar_t* Color, COLORREF& Target, FARCOLORFLAGS& TargetFlags, FARCOLORFLAGS SetFlag)
{
	// Empty string - default color
	if (!*Color)
		return Color;

	const auto& Convert = [](const wchar_t*& Ptr, COLORREF& Result)
	{
		wchar_t* EndPtr;
		const auto Value = std::wcstoul(Ptr, &EndPtr, 16);
		if (EndPtr == Ptr)
		{
			return false;
		}
		Result = Value;
		Ptr = EndPtr;
		return true;
	};

	if (upper(Color[0]) == L'T')
	{
		auto NewPtr = Color + 1;
		if (!Convert(NewPtr, Target))
		{
			return Color;
		}
		Color = NewPtr;
		Target = ARGB2ABGR(Target);
		TargetFlags &= ~SetFlag;
	}
	else
	{
		if (!Convert(Color, Target))
		{
			return Color;
		}
		TargetFlags |= SetFlag;
	}
	return Color;
}

string_view::const_iterator ExtractColorInNewFormat(string_view::const_iterator const Begin, string_view::const_iterator const End, FarColor& Color, bool& Stop)
{
	Stop = false;

	if (*Begin != L'(')
		return Begin;

	const auto FgColorBegin = Begin + 1;
	const auto ColorEnd = std::find(FgColorBegin, End, L')');
	if (ColorEnd == End)
	{
		Stop = true;
		return Begin;
	}

	const auto FgColorEnd = std::find(FgColorBegin, ColorEnd, L':');
	const auto BgColorBegin = FgColorEnd == ColorEnd? ColorEnd : FgColorEnd + 1;
	const auto BgColorEnd = ColorEnd;

	auto NewColor = Color;
	if ((FgColorBegin == FgColorEnd || ExtractColor(&*FgColorBegin, NewColor.ForegroundColor, NewColor.Flags, FCF_FG_4BIT)) &&
		(BgColorBegin == BgColorEnd || ExtractColor(&*BgColorBegin, NewColor.BackgroundColor, NewColor.Flags, FCF_BG_4BIT)))
	{
		Color = NewColor;
		return ColorEnd + 1;
	}

	return Begin;
}

}

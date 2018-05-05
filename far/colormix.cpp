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
#include "string_utils.hpp"
#include "global.hpp"

enum
{
	DefaultColor=0xf,
	ConsoleMask=0xf,
	ConsoleBgShift=4,
	ConsoleFgShift=0,
	ConsoleExtraMask = 0xff00
};

namespace colors
{
	size_t color_hash::operator()(const FarColor& Key) const
	{
		return
			make_hash(Key.Flags) ^
			make_hash(Key.BackgroundColor) ^
			make_hash(Key.ForegroundColor) ^
			make_hash(Key.Reserved);
	}

	FarColor merge(const FarColor& Bottom, const FarColor& Top)
	{
		FarColor Result = Bottom;

		const auto& ApplyColorPart = [&](COLORREF FarColor::*ColorAccessor, const FARCOLORFLAGS Flag)
		{
			const auto TopPart = std::invoke(ColorAccessor, Top);
			if (IS_OPAQUE(TopPart))
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

	static BYTE IndexColors[2] = {};

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

	if (COLORVALUE(data[0].Color) != COLORVALUE(data[1].Color) && IndexColors[0] == IndexColors[1])
	{
		// oops, unreadable
		IndexColors[0] & IntensityMask? IndexColors[0] &= ~IntensityMask : IndexColors[1] |= IntensityMask;
	}

	Result = (IndexColors[0] << ConsoleBgShift) | (IndexColors[1] << ConsoleFgShift) | (Color.Flags & ConsoleExtraMask);

	return Result;
}

FarColor ConsoleColorToFarColor(WORD Color)
{
	FarColor NewColor;
	static_assert(FCF_RAWATTR_MASK == ConsoleExtraMask);
	NewColor.Flags = FCF_FG_4BIT | FCF_BG_4BIT | (Color & ConsoleExtraMask);
	NewColor.ForegroundColor=(Color>>ConsoleFgShift)&ConsoleMask;
	NewColor.BackgroundColor=(Color>>ConsoleBgShift)&ConsoleMask;
	MAKE_OPAQUE(NewColor.ForegroundColor);
	MAKE_OPAQUE(NewColor.BackgroundColor);
	NewColor.Reserved=nullptr;
	return NewColor;
}


const FarColor& PaletteColorToFarColor(PaletteColors ColorIndex)
{
	return Global->Opt->Palette[ColorIndex];
}

const FarColor* StoreColor(const FarColor& Value)
{
	static std::unordered_set<FarColor, color_hash> ColorSet;
	return &*ColorSet.emplace(Value).first;
}

static COLORREF ARGB2ABGR(COLORREF Color)
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

string_view ExtractColorInNewFormat(const string_view Str, FarColor& Color, bool& Stop)
{
	Stop = false;
	if (Str[0] != L'(')
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
	if ((FgColorBegin == FgColorEnd || ExtractColor(&*FgColorBegin, NewColor.ForegroundColor, NewColor.Flags, FCF_FG_4BIT)) &&
		(BgColorBegin == BgColorEnd || ExtractColor(&*BgColorBegin, NewColor.BackgroundColor, NewColor.Flags, FCF_BG_4BIT)))
	{
		Color = NewColor;
		return Str.substr(ColorEnd + 1 - Str.cbegin());
	}

	return Str;
}

}

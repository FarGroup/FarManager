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

#include "headers.hpp"
#pragma hdrstop

#include "colormix.hpp"
#include "config.hpp"

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
WORD FarColorToConsoleColor(const FarColor& Color)
{
	static COLORREF LastTrueColors[2] = {};
	static FARCOLORFLAGS LastFlags = 0;
	static WORD Result = 0;
	if (Color.BackgroundColor != LastTrueColors[0] || Color.ForegroundColor != LastTrueColors[1] || (Color.Flags & FCF_4BITMASK) != (LastFlags & FCF_4BITMASK))
	{
		LastFlags = Color.Flags;

		static BYTE IndexColors[2] = {};
		const struct
		{
			COLORREF Color;
			rgba RGBA;
			FARCOLORFLAGS Flags;
			COLORREF* LastColor;
			BYTE* IndexColor;
		}
		data[2] =
		{
			{Color.BackgroundColor, Color.BackgroundRGBA, FCF_BG_4BIT, &LastTrueColors[0], &IndexColors[0]},
			{Color.ForegroundColor, Color.ForegroundRGBA, FCF_FG_4BIT, &LastTrueColors[1], &IndexColors[1]}
		};

		enum console_mask
		{
			BlueMask = 1,
			GreenMask = 2,
			RedMask = 4,
			IntensityMask = 8,
		};

		for (auto& i: data)
		{
			if(i.Color != *i.LastColor)
			{
				*i.LastColor = i.Color;
				if(Color.Flags & i.Flags)
				{
					*i.IndexColor = i.Color & ConsoleMask;
				}
				else
				{
					int R = i.RGBA.r;
					int G = i.RGBA.g;
					int B = i.RGBA.b;

					// special case, silver color:
					if (InRange(160, R, 223) && InRange(160, G, 223) && InRange(160, B, 223))
					{
						*i.IndexColor = RedMask|GreenMask|BlueMask;
					}
					else
					{
						int* p[] = { &R, &G, &B };
						size_t IntenseCount = 0;
						std::for_each(RANGE(p, component)
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
						});
						// eliminate mixed intensity
						if(IntenseCount > 0 && IntenseCount < 3)
						{
							std::for_each(RANGE(p, component)
							{
								if(*component == 128)
								{
									*component = IntenseCount==1? 0 : 255;
								}
							});
						}

						const auto ToMask = [](size_t component, console_mask mask) { return component ? mask : 0; };
						*i.IndexColor = ToMask(R, RedMask) | ToMask(G, GreenMask) | ToMask(B, BlueMask) | ToMask(IntenseCount, IntensityMask);
					}
				}
			}
		}

		if (COLORVALUE(data[0].Color) != COLORVALUE(data[1].Color) && IndexColors[0] == IndexColors[1])
		{
			// oops, unreadable
			IndexColors[0]&IntensityMask? IndexColors[0]&=~IntensityMask : IndexColors[1]|=IntensityMask;
		}
		Result = (IndexColors[0] << ConsoleBgShift) | (IndexColors[1] << ConsoleFgShift);
	}

	return (WORD)(Result | ((WORD)(Color.Flags & ConsoleExtraMask)));
}

FarColor ConsoleColorToFarColor(WORD Color)
{
	FarColor NewColor;
	static_assert(FCF_RAWATTR_MASK == ConsoleExtraMask, "Wrong FCF_RAWATTR_MASK");
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
	static std::unordered_set<FarColor> ColorSet;
	return &*ColorSet.emplace(Value).first;
}
}
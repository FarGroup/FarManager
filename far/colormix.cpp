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

WORD Colors::FarColorToConsoleColor(const FarColor& Color)
{
	static FARCOLORFLAGS Flags[2] = {FCF_BG_4BIT, FCF_FG_4BIT};
	static int Shifts[2] = {ConsoleBgShift, ConsoleFgShift};
	static int BlueMask = 1, GreenMask = 2, RedMask = 4, IntensityMask = 8;
	static BYTE IndexColors[2] = {};
	static COLORREF LastTrueColors[2] = {};
	static WORD Result = 0;
	if(Color.BackgroundColor != LastTrueColors[0] || Color.ForegroundColor != LastTrueColors[1])
	{
		COLORREF TrueColors[] = {Color.BackgroundColor, Color.ForegroundColor};

#define INSIDE(from, what, to) ((from) <= (what) && (what) <= (to))

		for(size_t i = 0; i < ARRAYSIZE(TrueColors); ++i)
		{
			if(TrueColors[i] != LastTrueColors[i])
			{
				LastTrueColors[i] = TrueColors[i];
				if(Color.Flags&Flags[i])
				{
					IndexColors[i] = TrueColors[i]&ConsoleMask;
				}
				else
				{
					int R = GetRValue(TrueColors[i]);
					int G = GetGValue(TrueColors[i]);
					int B = GetBValue(TrueColors[i]);

					// special case, silver color:
					if(INSIDE(160, R, 223) && INSIDE(160, G, 223) && INSIDE(160, B, 223))
					{
						IndexColors[i] = RedMask|GreenMask|BlueMask;
					}
					else
					{
						int* p[] = {&R, &G, &B};
						int IntenseCount = 0;
						for(size_t j = 0; j < ARRAYSIZE(p); ++j)
						{
							if(INSIDE(0, *p[j], 63))
							{
								*p[j]=0;
							}
							else if(INSIDE(64, *p[j], 191))
							{
								*p[j]=128;
							}
							else if(INSIDE(192, *p[j], 255))
							{
								*p[j]=255;
								++IntenseCount;
							}
						}
						// eliminate mixed intensity
						if(IntenseCount > 0 && IntenseCount < 3)
						{
							for(size_t j = 0; j < 3; ++j)
							{
								if(*p[j] == 128)
								{
									*p[j] = IntenseCount==1? 0 : 255;
								}
							}
						}
						IndexColors[i] = 0;
						if(R)
						{
							IndexColors[i]|=RedMask;
						}
						if(G)
						{
							IndexColors[i]|=GreenMask;
						}
						if(B)
						{
							IndexColors[i]|=BlueMask;
						}
						if(IntenseCount)
						{
							IndexColors[i]|=IntensityMask;
						}
					}
				}
			}
		}

		if(COLORVALUE(TrueColors[0]) != COLORVALUE(TrueColors[1]) && IndexColors[0] == IndexColors[1])
		{
			// oops, unreadable
			IndexColors[0]&IntensityMask? IndexColors[0]&=~IntensityMask : IndexColors[1]|=IntensityMask;
		}
		Result = (IndexColors[0] << Shifts[0]) | (IndexColors[1] << Shifts[1]);

#undef INSIDE

	}

	return Result;
}

void Colors::ConsoleColorToFarColor(WORD Color,FarColor& NewColor)
{
	NewColor.Flags=FCF_FG_4BIT|FCF_BG_4BIT;
	NewColor.ForegroundColor=(Color>>ConsoleFgShift)&ConsoleMask;
	NewColor.BackgroundColor=(Color>>ConsoleBgShift)&ConsoleMask;
	MAKE_OPAQUE(NewColor.ForegroundColor);
	MAKE_OPAQUE(NewColor.BackgroundColor);
	NewColor.Reserved=nullptr;
}

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

int Colors::FarColorToConsoleColor(const FarColor& Color)
{
	static FARCOLORFLAGS Flags[2] = {FCF_BG_4BIT, FCF_FG_4BIT};
	static int Shifts[2] = {ConsoleBgShift, ConsoleFgShift};
	int TrueColors[2]={Color.BackgroundColor, Color.ForegroundColor};
	BYTE IndexColors[2] = {0,7};

	for(size_t i = 0; i < ARRAYSIZE(TrueColors); ++i)
	{
		if(Color.Flags&Flags[i])
		{
			IndexColors[i] = TrueColors[i]&ConsoleMask;
		}
		else
		{
			switch(TrueColors[i]&0x00ffffff)
			{
				case 0x000000: IndexColors[i] = 0x0; break;
				case 0x800000: IndexColors[i] = 0x1; break;
				case 0x008000: IndexColors[i] = 0x2; break;
				case 0x808000: IndexColors[i] = 0x3; break;
				case 0x000080: IndexColors[i] = 0x4; break;
				case 0x800080: IndexColors[i] = 0x5; break;
				case 0x008080: IndexColors[i] = 0x6; break;
				case 0xc0c0c0: IndexColors[i] = 0x7; break;
				case 0x808080: IndexColors[i] = 0x8; break;
				case 0xff0000: IndexColors[i] = 0x9; break;
				case 0x00ff00: IndexColors[i] = 0xa; break;
				case 0xffff00: IndexColors[i] = 0xb; break;
				case 0x0000ff: IndexColors[i] = 0xc; break;
				case 0xff00ff: IndexColors[i] = 0xd; break;
				case 0x00ffff: IndexColors[i] = 0xe; break;
				case 0xffffff: IndexColors[i] = 0xf; break;
			}
		}
	}
	return (IndexColors[0] << Shifts[0]) | (IndexColors[1] << Shifts[1]);
}

void Colors::ConsoleColorToFarColor(int Color,FarColor& NewColor)
{
	NewColor.Flags=FCF_FG_4BIT|FCF_BG_4BIT;
	NewColor.ForegroundColor=((Color>>ConsoleFgShift)&ConsoleMask)|0xff000000;
	NewColor.BackgroundColor=((Color>>ConsoleBgShift)&ConsoleMask)|0xff000000;
	NewColor.Reserved=nullptr;
}

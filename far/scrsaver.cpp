/*
scrsaver.cpp

ScreenSaver
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "scrsaver.hpp"

#include "farcolor.hpp"
#include "chgprior.hpp"
#include "savescr.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "global.hpp"

#include "platform.chrono.hpp"

#include "common/function_ref.hpp"

enum {STAR_NONE,STAR_NORMAL,STAR_PLANET};

struct star
{
	int X;
	int Y;
	int Type;
	int Color;
	int Speed;
};

static std::array<star, 16> Star;

static const wchar_t StarSymbol[]=
{
	L'\x25A0',
	L'\x2219',
	L'\x2219',
	L'\x00B0',
	L'\x00B7',
};

static void ShowSaver(int Step, function_ref<void(star&)> const Fill)
{
	std::for_each(RANGE(Star, i)
	{
		if (i.Type!=STAR_NONE && !(Step % i.Speed))
		{
			SetColor(F_LIGHTCYAN|B_BLACK);
			GotoXY(i.X/100, i.Y/100);
			Text(L' ');
			int dx = i.X/100 - ScrX/2;
			i.X += dx*10+((dx<0) ? -1:1);
			int dy = i.Y/100-ScrY/2;
			i.Y+=dy*10+((dy<0) ? -1:1);

			if (i.X<0 || i.X/100>ScrX || i.Y<0 || i.Y/100>ScrY)
				i.Type=STAR_NONE;
			else
			{
				SetColor(i.Color|B_BLACK);
				GotoXY(i.X/100, i.Y/100);

				if (abs(dx)>3*ScrX/8 || abs(dy)>3*ScrY/8)
				{
					if (i.Type==STAR_PLANET)
					{
						SetColor(i.Color|FOREGROUND_INTENSITY|B_BLACK);
						Text(StarSymbol[0]);
					}
					else
					{
						SetColor(F_WHITE|B_BLACK);
						Text(StarSymbol[1]);
					}
				}
				else if (abs(dx)>ScrX/7 || abs(dy)>ScrY/7)
				{
					if (i.Type==STAR_PLANET)
					{
						SetColor(i.Color|FOREGROUND_INTENSITY|B_BLACK);
						Text(StarSymbol[1]);
					}
					else
					{
						if (abs(dx)>ScrX/4 || abs(dy)>ScrY/4)
							SetColor(F_LIGHTCYAN|B_BLACK);
						else
							SetColor(F_CYAN|B_BLACK);

						Text(StarSymbol[2]);
					}
				}
				else
				{
					if (i.Type==STAR_PLANET)
					{
						SetColor(i.Color|B_BLACK);
						Text(StarSymbol[3]);
					}
					else
					{
						SetColor(F_CYAN|B_BLACK);
						Text(StarSymbol[4]);
					}
				}
			}
		}
	});

	const auto NotStar = std::find_if(RANGE(Star, i) { return i.Type == STAR_NONE; });
	if (NotStar != Star.end())
	{
		Fill(*NotStar);
	}
}

int ScreenSaver()
{
	if (Global->ScreenSaverActive)
		return 1;

	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_IDLE);

	for (const auto WaitTime = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - WaitTime < 500ms;)
	{
		INPUT_RECORD rec;
		if (PeekInputRecord(&rec))
			return 1;

		os::chrono::sleep_for(100ms);
	}

	Global->ScreenSaverActive = true;
	CONSOLE_CURSOR_INFO CursorInfo;
	console.GetCursorInfo(CursorInfo);
	{
		SCOPED_ACTION(SaveScreen);
		SetCursorType(false, 10);

		SetScreen({ 0, 0, ScrX, ScrY }, L' ', colors::ConsoleColorToFarColor(F_LIGHTGRAY | B_BLACK));

		std::for_each(RANGE(Star, i)
		{
			i.Type=STAR_NONE;
			i.Color=0;
		});

		int Step=0;

		std::mt19937 mt(clock()); // std::random_device doesn't work in w2k
		std::uniform_int_distribution<int>
			XDist(100 * ScrX / 4, 100 * ScrX * 3 / 4),
			YDist(100 * ScrY / 4, 100 * ScrY * 3 / 4),
			TypeDist(0, 77),
			ColorDist(0, 2);

		INPUT_RECORD rec;
		while (!PeekInputRecord(&rec))
		{
			os::chrono::sleep_for(50ms);
			ShowSaver(Step++, [&](star& i)
			{
				static const int Colors[] = { F_MAGENTA, F_RED, F_BLUE };

				i.Type = TypeDist(mt) < 3? STAR_PLANET : STAR_NORMAL;
				i.X = XDist(mt);
				i.Y = YDist(mt);
				i.Color = Colors[ColorDist(mt)];
				i.Speed = i.Type == STAR_PLANET? 1 : 2;
			});
		}
	}
	SetCursorType(CursorInfo.bVisible!=FALSE, CursorInfo.dwSize);
	Global->ScreenSaverActive = false;
	FlushInputBuffer();
	Global->StartIdleTime = std::chrono::steady_clock::now();
	return 1;
}

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

#include "headers.hpp"
#pragma hdrstop

#include "colors.hpp"
#include "chgprior.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "console.hpp"
#include "palette.hpp"
#include "colormix.hpp"

#define randomize() srand(67898)
#define random(x) ((int)(((x)*rand())/(RAND_MAX+1)))

enum {STAR_NONE,STAR_NORMAL,STAR_PLANET};

static struct
{
	int X;
	int Y;
	int Type;
	int Color;
	int Speed;
} Star[16];

static const wchar_t* StarSymbol[]=
{
	L"\x25A0",
	L"\x2219",
	L"\x2219",
	L"\x00B0",
	L"\x00B7",
};

static void ShowSaver(int Step)
{
	for (size_t I=0; I<ARRAYSIZE(Star); I++)
		if (Star[I].Type!=STAR_NONE && !(Step%Star[I].Speed))
		{
			SetColor(F_LIGHTCYAN|B_BLACK);
			GotoXY(Star[I].X/100,Star[I].Y/100);
			Text(L" ");
			int dx=Star[I].X/100-ScrX/2;
			Star[I].X+=dx*10+((dx<0) ? -1:1);
			int dy=Star[I].Y/100-ScrY/2;
			Star[I].Y+=dy*10+((dy<0) ? -1:1);

			if (Star[I].X<0 || Star[I].X/100>ScrX || Star[I].Y<0 || Star[I].Y/100>ScrY)
				Star[I].Type=STAR_NONE;
			else
			{
				SetColor(Star[I].Color|B_BLACK);
				GotoXY(Star[I].X/100,Star[I].Y/100);

				if (abs(dx)>3*ScrX/8 || abs(dy)>3*ScrY/8)
				{
					if (Star[I].Type==STAR_PLANET)
					{
						SetColor(Star[I].Color|FOREGROUND_INTENSITY|B_BLACK);
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
					if (Star[I].Type==STAR_PLANET)
					{
						SetColor(Star[I].Color|FOREGROUND_INTENSITY|B_BLACK);
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
					if (Star[I].Type==STAR_PLANET)
					{
						SetColor(Star[I].Color|B_BLACK);
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

	for (size_t I=0; I<ARRAYSIZE(Star); I++)
		if (Star[I].Type==STAR_NONE)
		{
			static const int Colors[]={F_MAGENTA,F_RED,F_BLUE};
			Star[I].Type=random(77)<3 ? STAR_PLANET:STAR_NORMAL;
			Star[I].X=(ScrX/2-ScrX/4+random(ScrX/2))*100;
			Star[I].Y=(ScrY/2-ScrY/4+random(ScrY/2))*100;
			Star[I].Color=Colors[random(ARRAYSIZE(Colors))];
			Star[I].Speed=(Star[I].Type==STAR_PLANET) ? 1:2;
			break;
		}
}

int ScreenSaver(int EnableExit)
{
	INPUT_RECORD rec;
	clock_t WaitTime;

	if (ScreenSaverActive)
		return 1;

	ChangePriority ChPriority(THREAD_PRIORITY_IDLE);

	for (WaitTime=clock(); clock()-WaitTime<500;)
	{
		if (PeekInputRecord(&rec))
			return 1;

		Sleep(100);
	}

	ScreenSaverActive=TRUE;
	CONSOLE_CURSOR_INFO CursorInfo;
	Console.GetCursorInfo(CursorInfo);
	{
		SaveScreen SaveScr;
		SetCursorType(0,10);
		randomize();
		FarColor Color;
		Colors::ConsoleColorToFarColor(F_LIGHTGRAY|B_BLACK, Color);
		SetScreen(0,0,ScrX,ScrY,L' ', Color);

		for (size_t I=0; I<ARRAYSIZE(Star); I++)
		{
			Star[I].Type=STAR_NONE;
			Star[I].Color=0;
		}

		int Step=0;

		while (!PeekInputRecord(&rec))
		{
			Sleep(50);
			ShowSaver(Step++);
		}
	}
	SetCursorType(CursorInfo.bVisible!=FALSE, CursorInfo.dwSize);
	ScreenSaverActive=FALSE;
	FlushInputBuffer();
	StartIdleTime=clock();
	return 1;
}



/*
scrsaver.cpp

ScreenSaver

*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
#include "colors.hpp"
#include "chgprior.hpp"
#include "savescr.hpp"
#include "manager.hpp"

static void ShowSaver(int Step);

enum {STAR_NONE,STAR_NORMAL,STAR_PLANET};

static struct
{
	int X;
	int Y;
	int Type;
	int Color;
	int Speed;
} Star[16];

static char StarSymbol[5][2]=
{
	{0xFE,0x00},
	{0x07,0x00},
	{0xF9,0x00},
	{0xF8,0x00},
	{0xFA,0x00},
};

int ScreenSaver(int EnableExit)
{
	INPUT_RECORD rec;
	clock_t WaitTime;

	if (ScreenSaverActive)
		return(1);

	ChangePriority ChPriority(THREAD_PRIORITY_IDLE);

	for (WaitTime=clock(); clock()-WaitTime<500;)
	{
		if (PeekInputRecord(&rec))
			return(1);

		Sleep(100);
	}

	ScreenSaverActive=TRUE;
	int CursorVisible, CursorSize;
	GetRealCursorType(CursorVisible, CursorSize);
	{
		SaveScreen SaveScr;
		SetCursorType(0,10);
		randomize();
		SetScreen(0,0,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);

		for (int I=0; I<sizeof(Star)/sizeof(Star[0]); I++)
		{
			Star[I].Type=STAR_NONE;
			Star[I].Color=0;
		}

		int Step=0;

		while (!PeekInputRecord(&rec))
		{
			clock_t CurTime=clock();

			/* $ 05.05.2001 DJ
			   NWZ
			*/
			if (EnableExit && Opt.InactivityExit && Opt.InactivityExitTime>0 &&
			        CurTime-StartIdleTime>Opt.InactivityExitTime*60000 &&
			        FrameManager->GetFrameCount()==1)
			{
				FrameManager->ExitMainLoop(FALSE);
				return(0);
			}

			/* DJ $ */
			Sleep(50);
			ShowSaver(Step++);
		}
	}
	SetCursorType(CursorVisible, CursorSize);
	ScreenSaverActive=FALSE;
	FlushInputBuffer();
	StartIdleTime=clock();
	return 1;
}


static void ShowSaver(int Step)
{
	int I;

	for (I=0; I<sizeof(Star)/sizeof(Star[0]); I++)
		if (Star[I].Type!=STAR_NONE && (Step%Star[I].Speed)==0)
		{
			SetColor(F_LIGHTCYAN|B_BLACK);
			GotoXY(Star[I].X/100,Star[I].Y/100);
			Text(" ");
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

	for (I=0; I<sizeof(Star)/sizeof(Star[0]); I++)
		if (Star[I].Type==STAR_NONE)
		{
			static const int Colors[]={F_MAGENTA,F_RED,F_BLUE};
			Star[I].Type=random(77)<3 ? STAR_PLANET:STAR_NORMAL;
			Star[I].X=(ScrX/2-ScrX/4+random(ScrX/2))*100;
			Star[I].Y=(ScrY/2-ScrY/4+random(ScrY/2))*100;
			Star[I].Color=Colors[random(sizeof(Colors)/sizeof(Colors[0]))];
			Star[I].Speed=(Star[I].Type==STAR_PLANET) ? 1:2;
			break;
		}
}

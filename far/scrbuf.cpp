/*
scrbuf.cpp

Буферизация вывода на экран, весь вывод идет через этот буфер
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "scrbuf.hpp"
#include "colors.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "config.hpp"
#include "DList.hpp"
#include "adminmode.hpp"

enum
{
	SBFLAGS_FLUSHED         = 0x00000001,
	SBFLAGS_FLUSHEDCURPOS   = 0x00000002,
	SBFLAGS_FLUSHEDCURTYPE  = 0x00000004,
	SBFLAGS_USESHADOW       = 0x00000008,
};


//#if defined(SYSLOG_OT)
// #define DIRECT_SCREEN_OUT
//#endif

#ifdef DIRECT_RT
extern int DirectRT;
#endif

ScreenBuf ScrBuf;

ScreenBuf::ScreenBuf():
	Buf(nullptr),
	Shadow(nullptr),
	MacroCharUsed(false),
	ElevationCharUsed(false),
	BufX(0),
	BufY(0),
	LockCount(0)
{
	SBFlags.Set(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_FLUSHEDCURTYPE);
}


ScreenBuf::~ScreenBuf()
{
	if (Buf)    delete[] Buf;

	if (Shadow) delete[] Shadow;
}


void ScreenBuf::AllocBuf(int X,int Y)
{
	CriticalSectionLock Lock(CS);

	if (X==BufX && Y==BufY)
		return;

	if (Buf) delete[] Buf;

	if (Shadow) delete[] Shadow;

	unsigned Cnt=X*Y;
	Buf=new CHAR_INFO[Cnt]();
	Shadow=new CHAR_INFO[Cnt]();
	BufX=X;
	BufY=Y;
}

/* Заполнение виртуального буфера значением из консоли.
*/
void ScreenBuf::FillBuf()
{
	CriticalSectionLock Lock(CS);
	COORD Size,Corner;
	SMALL_RECT Coord;
	Size.X=BufX;
	Size.Y=BufY;
	Corner.X=0;
	Corner.Y=0;
	Coord.Left=0;
	Coord.Top=0;
	Coord.Right=BufX-1;
	Coord.Bottom=BufY-1;
	_tran(SysLog(L"BufX*BufY=%i",BufX*BufY));

	HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (BufX*BufY>6000)
	{
		_tran(SysLog(L"fucked method"));
		CHAR_INFO *ci=(CHAR_INFO*)Buf;

		for (int y=0; y<BufY; y++)
		{
			Size.Y=1;
			Coord.Top=y;
			Coord.Bottom=y;
			BOOL r;
			r=ReadConsoleOutput(hConOut,ci,Size,Corner,&Coord);
			ci+=BufX;
		}

		_tran(SysLog(L"fucked method end"));
	}
	else
	{
		ReadConsoleOutput(hConOut,Buf,Size,Corner,&Coord);
	}

	memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
	SBFlags.Set(SBFLAGS_USESHADOW);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConOut,&csbi);
	CurX=csbi.dwCursorPosition.X;
	CurY=csbi.dwCursorPosition.Y;
}

/* Записать Text в виртуальный буфер
*/
void ScreenBuf::Write(int X,int Y,const CHAR_INFO *Text,int TextLength)
{
	CriticalSectionLock Lock(CS);

	if (X<0)
	{
		Text-=X;
		TextLength=Max(0,TextLength+X);
		X=0;
	}

	if (X>=BufX || Y>=BufY || TextLength==0 || Y<0)
		return;

	if (X+TextLength >= BufX)
		TextLength=BufX-X; //??

	CHAR_INFO *PtrBuf=Buf+Y*BufX+X;

	for (int i=0; i<TextLength; i++)
	{
		SetVidChar(PtrBuf[i],Text[i].Char.UnicodeChar);
		PtrBuf[i].Attributes=Text[i].Attributes;
	}

	SBFlags.Clear(SBFLAGS_FLUSHED);
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (DirectRT)
		Flush();

#endif
}


/* Читать блок из виртуального буфера.
*/
void ScreenBuf::Read(int X1,int Y1,int X2,int Y2,CHAR_INFO *Text,int MaxTextLength)
{
	CriticalSectionLock Lock(CS);
	int Width=X2-X1+1;
	int Height=Y2-Y1+1;
	int I, Idx;

	for (Idx=I=0; I < Height; I++, Idx+=Width)
		memcpy(Text+Idx,Buf+(Y1+I)*BufX+X1,Min((int)sizeof(CHAR_INFO)*Width,(int)MaxTextLength));

	if (X1==0 && Y1==0 && MacroCharUsed)
	{
		Text[0]=MacroChar;
	}

	if (X2==BufX-1 && Y2==BufY-1 && ElevationCharUsed)
	{
		Text[Width*Height-1]=ElevationChar;
	}
}

/* Изменить значение цветовых атрибутов в соответствии с маской
   (в основном применяется для "создания" тени)
*/
void ScreenBuf::ApplyColorMask(int X1,int Y1,int X2,int Y2,WORD ColorMask)
{
	CriticalSectionLock Lock(CS);
	int Width=X2-X1+1;
	int Height=Y2-Y1+1;
	int I, J;

	for (I=0; I < Height; I++)
	{
		CHAR_INFO *PtrBuf=Buf+(Y1+I)*BufX+X1;

		for (J=0; J < Width; J++, ++PtrBuf)
		{
			if ((PtrBuf->Attributes&=~ColorMask) == 0)
				PtrBuf->Attributes=0x08;
		}
	}

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (DirectRT)
		Flush();

#endif
}

/* Непосредственное изменение цветовых атрибутов
*/
void ScreenBuf::ApplyColor(int X1,int Y1,int X2,int Y2,WORD Color)
{
	CriticalSectionLock Lock(CS);
	if(X1<=ScrX && Y1<=ScrY && X2>=0 && Y2>=0)
	{
		X1=Max(0,X1);
		X2=Min(static_cast<int>(ScrX),X2);
		Y1=Max(0,Y1);
		Y2=Min(static_cast<int>(ScrY),Y2);

		int Width=X2-X1+1;
		int Height=Y2-Y1+1;
		int I, J;

		for (I=0; I < Height; I++)
		{
			CHAR_INFO *PtrBuf=Buf+(Y1+I)*BufX+X1;

			for (J=0; J < Width; J++, ++PtrBuf)
				PtrBuf->Attributes=Color;

			//Buf[K+J].Attributes=Color;
		}

#ifdef DIRECT_SCREEN_OUT
		Flush();
#elif defined(DIRECT_RT)

		if (DirectRT)
			Flush();

#endif
	}
}

/* Непосредственное изменение цветовых атрибутов с заданым цетом исключением
*/
void ScreenBuf::ApplyColor(int X1,int Y1,int X2,int Y2,int Color,WORD ExceptColor)
{
	CriticalSectionLock Lock(CS);
	if(X1<=ScrX && Y1<=ScrY && X2>=0 && Y2>=0)
	{
		X1=Max(0,X1);
		X2=Min(static_cast<int>(ScrX),X2);
		Y1=Max(0,Y1);
		Y2=Min(static_cast<int>(ScrY),Y2);

		for (int I = 0; I < Y2-Y1+1; I++)
		{
			CHAR_INFO *PtrBuf = Buf+(Y1+I)*BufX+X1;

			for (int J = 0; J < X2-X1+1; J++, ++PtrBuf)
				if (PtrBuf->Attributes != ExceptColor)
					PtrBuf->Attributes = Color;
		}

#ifdef DIRECT_SCREEN_OUT
		Flush();
#elif defined(DIRECT_RT)

		if (DirectRT)
			Flush();

#endif
	}
}

/* Закрасить прямоугольник символом Ch и цветом Color
*/
void ScreenBuf::FillRect(int X1,int Y1,int X2,int Y2,WCHAR Ch,WORD Color)
{
	CriticalSectionLock Lock(CS);
	int Width=X2-X1+1;
	int Height=Y2-Y1+1;
	int I, J;
	CHAR_INFO CI,*PtrBuf;
	CI.Attributes=Color;
	SetVidChar(CI,Ch);

	for (I=0; I < Height; I++)
	{
		for (PtrBuf=Buf+(Y1+I)*BufX+X1, J=0; J < Width; J++, ++PtrBuf)
			*PtrBuf=CI;
	}

	SBFlags.Clear(SBFLAGS_FLUSHED);
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (DirectRT)
		Flush();

#endif
}

/* "Сбросить" виртуальный буфер на консоль
*/
void ScreenBuf::Flush()
{
	CriticalSectionLock Lock(CS);

	if (!LockCount)
	{
		HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);

		if (CtrlObject && (CtrlObject->Macro.IsRecording() || CtrlObject->Macro.IsExecuting()))
		{
			if (!MacroCharUsed)
			{
				MacroChar=Buf[0];
				MacroCharUsed=true;
			}

			if(CtrlObject->Macro.IsRecording())
			{
				Buf[0].Char.UnicodeChar=L'R';
				Buf[0].Attributes=0xCF;
			}
			else
			{
				Buf[0].Char.UnicodeChar=L'P';
				Buf[0].Attributes=0x2F;
			}
		}

		if(Admin.Elevated())
		{
			if (!ElevationCharUsed)
			{
				ElevationChar=Buf[BufX*BufY-1];
				ElevationCharUsed=true;
			}

			Buf[BufX*BufY-1].Char.UnicodeChar=L'A';
			Buf[BufX*BufY-1].Attributes=0xCF;
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && !CurVisible)
		{
			CONSOLE_CURSOR_INFO cci={CurSize,CurVisible};
			SetConsoleCursorInfo(hConOut,&cci);
			SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHED))
		{
			SBFlags.Set(SBFLAGS_FLUSHED);

			if (WaitInMainLoop && Opt.Clock && !ProcessShowClock)
			{
				ShowTime(FALSE);
			}

			DList<SMALL_RECT>WriteList;
			bool Changes=false;

			if (SBFlags.Check(SBFLAGS_USESHADOW))
			{
				PCHAR_INFO PtrBuf=Buf,PtrShadow=Shadow;

				if (Opt.ClearType)
				{
					//Для полного избавления от артефактов ClearType будем перерисовывать на всю ширину.
					//Чревато тормозами/миганием в зависимости от конфигурации системы.
					SMALL_RECT WriteRegion={0,0,BufX-1,0};

					for (SHORT I=0; I<BufY; I++, PtrBuf+=BufX, PtrShadow+=BufX)
					{
						WriteRegion.Top=I;
						WriteRegion.Bottom=I-1;

						while (I<BufY && memcmp(PtrBuf,PtrShadow,BufX*sizeof(CHAR_INFO)))
						{
							I++;
							PtrBuf+=BufX;
							PtrShadow+=BufX;
							WriteRegion.Bottom++;
						}

						if (WriteRegion.Bottom >= WriteRegion.Top)
						{
							WriteList.Push(&WriteRegion);
							Changes=true;
						}
					}
				}
				else
				{
					bool Started=false;
					SMALL_RECT WriteRegion={BufX-1,BufY-1,0,0};

					for (SHORT I=0; I<BufY; I++)
					{
						for (SHORT J=0; J<BufX; J++,++PtrBuf,++PtrShadow)
						{
							if (memcmp(PtrBuf,PtrShadow,sizeof(CHAR_INFO)))
							{
								WriteRegion.Left=Min(WriteRegion.Left,J);
								WriteRegion.Top=Min(WriteRegion.Top,I);
								WriteRegion.Right=Max(WriteRegion.Right,J);
								WriteRegion.Bottom=Max(WriteRegion.Bottom,I);
								Changes=true;
								Started=true;
							}
							else if (Started && I>WriteRegion.Bottom && J>=WriteRegion.Left)
							{
								//BUGBUG: при включенном СlearType-сглаживании на экране остаётся "мусор" - тонкие вертикальные полосы
								// кстати, и при выключенном тоже (но реже).
								// баг, конечно, не наш, но что делать.
								// расширяем область прорисовки влево-вправо на 1 символ:
								WriteRegion.Left=Max(static_cast<SHORT>(0),static_cast<SHORT>(WriteRegion.Left-1));
								WriteRegion.Right=Min(static_cast<SHORT>(WriteRegion.Right+1),static_cast<SHORT>(BufX-1));
								bool Merge=false;
								PSMALL_RECT Last=WriteList.Last();

								if (Last)
								{
#define MAX_DELTA 5

									if (WriteRegion.Top-1==Last->Bottom && ((WriteRegion.Left>=Last->Left && WriteRegion.Left-Last->Left<MAX_DELTA) || (Last->Right>=WriteRegion.Right && Last->Right-WriteRegion.Right<MAX_DELTA)))
									{
										Last->Bottom=WriteRegion.Bottom;
										Last->Left=Min(Last->Left,WriteRegion.Left);
										Last->Right=Max(Last->Right,WriteRegion.Right);
										Merge=true;
									}
								}

								if (!Merge)
									WriteList.Push(&WriteRegion);

								WriteRegion.Left=BufX-1;
								WriteRegion.Top=BufY-1;
								WriteRegion.Right=0;
								WriteRegion.Bottom=0;
								Started=false;
							}
						}
					}

					if (Started)
					{
						WriteList.Push(&WriteRegion);
					}
				}
			}
			else
			{
				Changes=true;
				SMALL_RECT WriteRegion={0,0,BufX-1,BufY-1};
				WriteList.Push(&WriteRegion);
			}

			if (Changes)
			{
				for (PSMALL_RECT PtrRect=WriteList.First(); PtrRect; PtrRect=WriteList.Next(PtrRect))
				{
					COORD Size={BufX,BufY},Corner={PtrRect->Left,PtrRect->Top};
					SMALL_RECT Coord=*PtrRect;
					// BUGBUG: в Windows 7 при 0xFFFF в консоли имеем мусор и падает conhost.
					// посему пишем по 32 K.
#define MAXSIZE 0x7FFF

					if (BufX*BufY*sizeof(CHAR_INFO)>MAXSIZE) // See REMINDER file section scrbuf.cpp
					{
						Corner.Y=0;
						int WriteY2=Coord.Bottom;

						for (int yy=Coord.Top; yy<=WriteY2;)
						{
							Coord.Top=yy;
							PCHAR_INFO BufPtr=Buf+yy*BufX;
							Size.Y=Min(Max(MAXSIZE/static_cast<int>(BufX*sizeof(CHAR_INFO)),1),WriteY2-yy+1);
							yy+=Size.Y;
							Coord.Bottom=yy-1;
							WriteConsoleOutput(hConOut,BufPtr,Size,Corner,&Coord);
						}
					}
					else
					{
						WriteConsoleOutput(hConOut,Buf,Size,Corner,&Coord);
					}
				}

				memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
			}
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURPOS))
		{
			COORD C={CurX,CurY};
			SetConsoleCursorPosition(hConOut,C);
			SBFlags.Set(SBFLAGS_FLUSHEDCURPOS);
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && CurVisible)
		{
			CONSOLE_CURSOR_INFO cci={CurSize,CurVisible};
			SetConsoleCursorInfo(hConOut,&cci);
			SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
		}

		SBFlags.Set(SBFLAGS_USESHADOW|SBFLAGS_FLUSHED);
	}
}


void ScreenBuf::Lock()
{
	LockCount++;
}


void ScreenBuf::Unlock()
{
	if (LockCount>0)
		LockCount--;
}


void ScreenBuf::ResetShadow()
{
	SBFlags.Clear(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURTYPE|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_USESHADOW);
}


void ScreenBuf::MoveCursor(int X,int Y)
{
	CriticalSectionLock Lock(CS);
	CurX=X;
	CurY=Y;

	if (CurX<0||CurY<0||CurX>ScrX||CurY>ScrY)
		CurVisible=FALSE;

	SBFlags.Clear(SBFLAGS_FLUSHEDCURPOS);
}


void ScreenBuf::GetCursorPos(SHORT& X,SHORT& Y)
{
	X=CurX;
	Y=CurY;
}


void ScreenBuf::SetCursorType(int Visible,int Size)
{
	/* $ 09.01.2001 SVS
	   По наводке ER - в SetCursorType не дергать раньше
	   времени установку курсора
	*/
	if (CurVisible!=Visible || CurSize!=Size)
	{
		CurVisible=Visible;
		CurSize=Size;
		SBFlags.Clear(SBFLAGS_FLUSHEDCURTYPE);
	}
}

void ScreenBuf::GetCursorType(int &Visible,int &Size)
{
	Visible=CurVisible;
	Size=CurSize;
}


void ScreenBuf::RestoreMacroChar()
{
	if(MacroCharUsed)
	{
		Buf[0]=MacroChar;
		MacroCharUsed=false;
	}
}

void ScreenBuf::RestoreElevationChar()
{
	if(ElevationCharUsed)
	{
		Buf[BufX*BufY-1]=ElevationChar;
		ElevationCharUsed=false;
	}
}

//  проскроллировать буффер на одну строку вверх.
void ScreenBuf::Scroll(int Num)
{
	CriticalSectionLock Lock(CS);

	if (Num > 0 && Num < BufY)
		memmove(Buf,Buf+Num*BufX,(BufY-Num)*BufX*sizeof(CHAR_INFO));

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (DirectRT)
		Flush();

#endif
}

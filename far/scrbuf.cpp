/*
scrbuf.cpp

Буферизация вывода на экран, весь вывод идет через этот буфер
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

#include "scrbuf.hpp"
#include "colors.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "elevation.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "constitle.hpp"

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

ScreenBuf::ScreenBuf():
	MacroChar(),
	ElevationChar(),
	SBFlags(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_FLUSHEDCURTYPE),
	LockCount(0),
	CurSize(0),
	BufX(0),
	BufY(0),
	CurX(0),
	CurY(0),
	MacroCharUsed(false),
	ElevationCharUsed(false),
	CurVisible(false)
{
}

void ScreenBuf::AllocBuf(int X,int Y)
{
	CriticalSectionLock Lock(CS);

	if (X==BufX && Y==BufY)
		return;

	size_t Cnt=X*Y;

	// don't call vector.resize() here:
	// - it's never shrink
	// - we don't care about old content
	resize_nomove(Buf, Cnt);
	resize_nomove(Shadow, Cnt);

	BufX=X;
	BufY=Y;
}

/* Заполнение виртуального буфера значением из консоли.
*/
void ScreenBuf::FillBuf()
{
	CriticalSectionLock Lock(CS);
	COORD BufferSize={BufX, BufY}, BufferCoord={};
	SMALL_RECT ReadRegion={0, 0, static_cast<SHORT>(BufX-1), static_cast<SHORT>(BufY-1)};
	Global->Console->ReadOutput(Buf.data(), BufferSize, BufferCoord, ReadRegion);
	std::copy(ALL_CONST_RANGE(Buf), Shadow.begin());
	SBFlags.Set(SBFLAGS_USESHADOW);
	COORD CursorPosition;
	Global->Console->GetCursorPosition(CursorPosition);
	CurX=CursorPosition.X;
	CurY=CursorPosition.Y;
}

/* Записать Text в виртуальный буфер
*/
void ScreenBuf::Write(int X,int Y,const FAR_CHAR_INFO *Text, size_t Size)
{
	CriticalSectionLock Lock(CS);

	if (X<0)
	{
		Text-=X;
		Size = std::max(0, static_cast<int>(Size) + X);
		X=0;
	}

	if (X>=BufX || Y>=BufY || !Size || Y<0)
		return;

	if (X+Size >= static_cast<size_t>(BufX))
		Size=BufX-X; //??

	FAR_CHAR_INFO *PtrBuf=Buf.data()+Y*BufX+X;

	for (size_t i=0; i < Size; i++)
	{
		SetVidChar(PtrBuf[i],Text[i].Char);
		PtrBuf[i].Attributes=Text[i].Attributes;
	}

	SBFlags.Clear(SBFLAGS_FLUSHED);
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (Global->DirectRT)
		Flush();

#endif
}


/* Читать блок из виртуального буфера.
*/
void ScreenBuf::Read(int X1,int Y1,int X2,int Y2,FAR_CHAR_INFO *Text, size_t Size)
{
	CriticalSectionLock Lock(CS);
	size_t Width=X2-X1+1;
	size_t Height=Y2-Y1+1;

	for (size_t Idx = 0, I = 0; I < Height; I++, Idx+=Width)
		std::copy(Buf.data() + (Y1 + I)*BufX + X1, Buf.data() + (Y1 + I)*BufX + X1 + std::min(Width, Size), Text + Idx);
}

/* Изменить значение цветовых атрибутов в соответствии с маской
   (применяется для "создания" тени)
*/
void ScreenBuf::ApplyShadow(int X1,int Y1,int X2,int Y2)
{
	CriticalSectionLock Lock(CS);
	int Width=X2-X1+1;
	int Height=Y2-Y1+1;
	int I, J;
	for (I=0; I < Height; I++)
	{
		FAR_CHAR_INFO *PtrBuf = Buf.data() + (Y1 + I)*BufX + X1;

		for (J=0; J < Width; J++, ++PtrBuf)
		{
			PtrBuf->Attributes.BackgroundColor = 0;

			if(PtrBuf->Attributes.Flags&FCF_FG_4BIT)
			{
				PtrBuf->Attributes.ForegroundColor&=~0x8;
				if(!COLORVALUE(PtrBuf->Attributes.ForegroundColor))
				{
					PtrBuf->Attributes.ForegroundColor=0x8;
				}
			}
			else
			{
				PtrBuf->Attributes.ForegroundColor&=~0x808080;
				if(!COLORVALUE(PtrBuf->Attributes.ForegroundColor))
				{
					PtrBuf->Attributes.ForegroundColor = 0x808080;
				}
			}
		}
	}

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (Global->DirectRT)
		Flush();

#endif
}

/* Непосредственное изменение цветовых атрибутов
*/
// used in block selection
void ScreenBuf::ApplyColor(int X1,int Y1,int X2,int Y2,const FarColor& Color, bool PreserveExFlags)
{
	CriticalSectionLock Lock(CS);
	if(X1<=ScrX && Y1<=ScrY && X2>=0 && Y2>=0)
	{
		X1=std::max(0,X1);
		X2=std::min(static_cast<int>(ScrX),X2);
		Y1=std::max(0,Y1);
		Y2=std::min(static_cast<int>(ScrY),Y2);

		int Width=X2-X1+1;
		int Height=Y2-Y1+1;
		int I, J;

		FAR_CHAR_INFO *PtrBuf;
		if(PreserveExFlags)
		{
			for (I=0; I < Height; I++)
			{
				PtrBuf = Buf.data() + (Y1 + I)*BufX + X1;
				for (J=0; J < Width; J++, ++PtrBuf)
				{
					FARCOLORFLAGS ExFlags = PtrBuf->Attributes.Flags&FCF_EXTENDEDFLAGS;
					PtrBuf->Attributes=Color;
					PtrBuf->Attributes.Flags = (PtrBuf->Attributes.Flags&~FCF_EXTENDEDFLAGS)|ExFlags;
				}
			}
		}
		else
		{
			for (I=0; I < Height; I++)
			{
				PtrBuf = Buf.data() + (Y1 + I)*BufX + X1;
				for (J=0; J < Width; J++, ++PtrBuf)
				{
					PtrBuf->Attributes=Color;
				}
			}
		}

#ifdef DIRECT_SCREEN_OUT
		Flush();
#elif defined(DIRECT_RT)

		if (Global->DirectRT)
			Flush();

#endif
	}
}

/* Непосредственное изменение цветовых атрибутов с заданым цетом исключением
*/
// used in stream selection
void ScreenBuf::ApplyColor(int X1,int Y1,int X2,int Y2,const FarColor& Color,const FarColor& ExceptColor, bool ForceExFlags)
{
	CriticalSectionLock Lock(CS);
	if(X1<=ScrX && Y1<=ScrY && X2>=0 && Y2>=0)
	{
		X1=std::max(0,X1);
		X2=std::min(static_cast<int>(ScrX),X2);
		Y1=std::max(0,Y1);
		Y2=std::min(static_cast<int>(ScrY),Y2);

		for (int I = 0; I < Y2-Y1+1; I++)
		{
			FAR_CHAR_INFO* PtrBuf = Buf.data() + (Y1 + I)*BufX + X1;
			for (int J = 0; J < X2-X1+1; J++, ++PtrBuf)
			{
				if (PtrBuf->Attributes.ForegroundColor != ExceptColor.ForegroundColor || PtrBuf->Attributes.BackgroundColor != ExceptColor.BackgroundColor)
				{
					PtrBuf->Attributes=Color;
				}
				else if (ForceExFlags)
				{
					PtrBuf->Attributes.Flags = (PtrBuf->Attributes.Flags&~FCF_EXTENDEDFLAGS)|(Color.Flags&FCF_EXTENDEDFLAGS);
				}
			}
		}

#ifdef DIRECT_SCREEN_OUT
		Flush();
#elif defined(DIRECT_RT)

		if (Global->DirectRT)
			Flush();

#endif
	}
}

/* Закрасить прямоугольник символом Ch и цветом Color
*/
void ScreenBuf::FillRect(int X1,int Y1,int X2,int Y2,WCHAR Ch,const FarColor& Color)
{
	CriticalSectionLock Lock(CS);
	int Width=X2-X1+1;
	int Height=Y2-Y1+1;
	int I, J;
	FAR_CHAR_INFO CI,*PtrBuf;
	CI.Attributes=Color;
	SetVidChar(CI,Ch);

	for (I=0; I < Height; I++)
	{
		for (PtrBuf = Buf.data() + (Y1 + I)*BufX + X1, J = 0; J < Width; J++, ++PtrBuf)
			*PtrBuf=CI;
	}

	SBFlags.Clear(SBFLAGS_FLUSHED);
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (Global->DirectRT)
		Flush();

#endif
}

/* "Сбросить" виртуальный буфер на консоль
*/
void ScreenBuf::Flush(bool SuppressIndicators)
{
	CriticalSectionLock Lock(CS);

	if (!LockCount)
	{
		if (!SuppressIndicators)
		{
			if(Global->CtrlObject &&
				(Global->CtrlObject->Macro.IsRecording() ||
				(Global->CtrlObject->Macro.IsExecuting() && Global->Opt->Macro.ShowPlayIndicator))
			)
			{
				MacroChar=Buf[0];
				MacroCharUsed=true;

				if(Global->CtrlObject->Macro.IsRecording())
				{
					Buf[0].Char=L'R';
					Buf[0].Attributes = Colors::ConsoleColorToFarColor(B_LIGHTRED|F_WHITE);
				}
				else
				{
					Buf[0].Char=L'P';
					Buf[0].Attributes = Colors::ConsoleColorToFarColor(B_GREEN|F_WHITE);
				}
			}

			if(Global->Elevation->Elevated())
			{
				ElevationChar=Buf[BufX*BufY-1];
				ElevationCharUsed=true;

				Buf[BufX*BufY-1].Char=L'A';
				Buf[BufX*BufY-1].Attributes = Colors::ConsoleColorToFarColor(B_LIGHTRED|F_WHITE);
			}
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && !CurVisible)
		{
			CONSOLE_CURSOR_INFO cci={CurSize,CurVisible};
			Global->Console->SetCursorInfo(cci);
			SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHED))
		{
			SBFlags.Set(SBFLAGS_FLUSHED);

			if (Global->WaitInMainLoop && Global->Opt->Clock && !Global->ProcessShowClock)
			{
				ShowTime(FALSE);
			}

			std::list<SMALL_RECT>WriteList;
			bool Changes=false;

			if (SBFlags.Check(SBFLAGS_USESHADOW))
			{
				FAR_CHAR_INFO* PtrBuf = Buf.data(), *PtrShadow = Shadow.data();

				if (Global->Opt->ClearType)
				{
					//Для полного избавления от артефактов ClearType будем перерисовывать на всю ширину.
					//Чревато тормозами/миганием в зависимости от конфигурации системы.
					SMALL_RECT WriteRegion={0,0,static_cast<SHORT>(BufX-1),0};

					for (SHORT I=0; I<BufY; I++, PtrBuf+=BufX, PtrShadow+=BufX)
					{
						WriteRegion.Top=I;
						WriteRegion.Bottom=I-1;

						while (I<BufY && memcmp(PtrBuf,PtrShadow,BufX*sizeof(FAR_CHAR_INFO))!=0)
						{
							I++;
							PtrBuf+=BufX;
							PtrShadow+=BufX;
							WriteRegion.Bottom++;
						}

						if (WriteRegion.Bottom >= WriteRegion.Top)
						{
							WriteList.emplace_back(WriteRegion);
							Changes=true;
						}
					}
				}
				else
				{
					bool Started=false;
					SMALL_RECT WriteRegion={static_cast<SHORT>(BufX-1),static_cast<SHORT>(BufY-1),0,0};

					for (SHORT I=0; I<BufY; I++)
					{
						for (SHORT J=0; J<BufX; J++,++PtrBuf,++PtrShadow)
						{
							if (memcmp(PtrBuf,PtrShadow,sizeof(FAR_CHAR_INFO))!=0)
							{
								WriteRegion.Left=std::min(WriteRegion.Left,J);
								WriteRegion.Top=std::min(WriteRegion.Top,I);
								WriteRegion.Right=std::max(WriteRegion.Right,J);
								WriteRegion.Bottom=std::max(WriteRegion.Bottom,I);
								Changes=true;
								Started=true;
							}
							else if (Started && I>WriteRegion.Bottom && J>=WriteRegion.Left)
							{
								//BUGBUG: при включенном СlearType-сглаживании на экране остаётся "мусор" - тонкие вертикальные полосы
								// кстати, и при выключенном тоже (но реже).
								// баг, конечно, не наш, но что делать.
								// расширяем область прорисовки влево-вправо на 1 символ:
								WriteRegion.Left=std::max(static_cast<SHORT>(0),static_cast<SHORT>(WriteRegion.Left-1));
								WriteRegion.Right=std::min(static_cast<SHORT>(WriteRegion.Right+1),static_cast<SHORT>(BufX-1));
								bool Merge=false;
								if (!WriteList.empty())
								{
									SMALL_RECT& Last=WriteList.back();
									const int MAX_DELTA = 5;
									if (WriteRegion.Top-1==Last.Bottom && ((WriteRegion.Left>=Last.Left && WriteRegion.Left-Last.Left<MAX_DELTA) || (Last.Right>=WriteRegion.Right && Last.Right-WriteRegion.Right<MAX_DELTA)))
									{
										Last.Bottom=WriteRegion.Bottom;
										Last.Left=std::min(Last.Left,WriteRegion.Left);
										Last.Right=std::max(Last.Right,WriteRegion.Right);
										Merge=true;
									}
								}

								if (!Merge)
									WriteList.emplace_back(WriteRegion);

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
						WriteList.emplace_back(WriteRegion);
					}
				}
			}
			else
			{
				Changes=true;
				SMALL_RECT WriteRegion={0,0,static_cast<SHORT>(BufX-1),static_cast<SHORT>(BufY-1)};
				WriteList.emplace_back(WriteRegion);
			}

			if (Changes)
			{
				std::for_each(CONST_RANGE(WriteList, i)
				{
					COORD BufferSize={BufX, BufY}, BufferCoord={i.Left, i.Top};
					SMALL_RECT WriteRegion = i;
					Global->Console->WriteOutput(Buf.data(), BufferSize, BufferCoord, WriteRegion);
				});
				Global->Console->Commit();
				std::copy(ALL_CONST_RANGE(Buf), Shadow.begin());
			}
		}

		if (MacroCharUsed)
		{
			Buf[0]=MacroChar;
		}

		if (ElevationCharUsed)
		{
			Buf[BufX*BufY-1]=ElevationChar;
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURPOS))
		{
			COORD C={CurX,CurY};
			Global->Console->SetCursorPosition(C);
			SBFlags.Set(SBFLAGS_FLUSHEDCURPOS);
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && CurVisible)
		{
			CONSOLE_CURSOR_INFO cci={CurSize,CurVisible};
			Global->Console->SetCursorInfo(cci);
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
		SetLockCount(LockCount-1);
}

void ScreenBuf::SetLockCount(int Count)
{
	LockCount=Count;
	if (!LockCount && ConsoleTitle::WasTitleModified())
		ConsoleTitle::RestoreTitle();
}

void ScreenBuf::ResetShadow()
{
	SBFlags.Clear(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURTYPE|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_USESHADOW);
}


void ScreenBuf::MoveCursor(int X,int Y)
{
	CriticalSectionLock Lock(CS);

	if (CurX<0||CurY<0||CurX>ScrX||CurY>ScrY)
	{
		CurVisible=FALSE;
	}

	if(X!=CurX || Y!=CurY || !CurVisible)
	{
		CurX=X;
		CurY=Y;
		SBFlags.Clear(SBFLAGS_FLUSHEDCURPOS);
	}
}


void ScreenBuf::GetCursorPos(SHORT& X, SHORT& Y) const
{
	X=CurX;
	Y=CurY;
}


void ScreenBuf::SetCursorType(bool Visible, DWORD Size)
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

void ScreenBuf::GetCursorType(bool& Visible, DWORD& Size) const
{
	Visible=CurVisible;
	Size=CurSize;
}


void ScreenBuf::RestoreMacroChar()
{
	if(MacroCharUsed)
	{
		Write(0,0,&MacroChar,1);
		MacroCharUsed=false;
	}
}

void ScreenBuf::RestoreElevationChar()
{
	if(ElevationCharUsed)
	{
		Write(BufX-1,BufY-1,&ElevationChar,1);
		ElevationCharUsed=false;
	}
}

//  проскроллировать буффер на одну строку вверх.
void ScreenBuf::Scroll(int Num)
{
	CriticalSectionLock Lock(CS);

	if (Num > 0 && Num < BufY)
	{
		size_t size = Buf.size();
		Buf.erase(Buf.begin(), Buf.begin() + Num * BufX);
		Buf.resize(size);
	}

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (Global->DirectRT)
		Flush();

#endif
}

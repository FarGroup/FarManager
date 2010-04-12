/*
scrbuf.cpp

Буферизация вывода на экран, весь вывод идет через этот буфер

*/

#include "headers.hpp"
#pragma hdrstop

#include "scrbuf.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "ctrlobj.hpp"

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

#if defined(USE_WFUNC)
extern WCHAR Oem2Unicode[];
#endif

ScreenBuf ScrBuf;

ScreenBuf::ScreenBuf()
{
	Buf=Shadow=NULL;
	BufX=BufY=0;
	SBFlags.Set(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_FLUSHEDCURTYPE);
	LockCount=0;
	hScreen=NULL;
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
	Buf=new CHAR_INFO[Cnt];
	Shadow=new CHAR_INFO[Cnt];
#if !defined(ALLOC)
	/* а вот здесь самая интересность: при переопределенных функция запроса
	   памяти - память уже проинициализированна нулями ;-)
	   поэтому этот кусок пропустится...
	*/
	memset(Buf,0,Cnt*sizeof(CHAR_INFO));
	memset(Shadow,0,Cnt*sizeof(CHAR_INFO));
#endif
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
	_tran(SysLog("BufX*BufY=%i",BufX*BufY));

	if (BufX*BufY>6000)
	{
		_tran(SysLog("fucked method"));
		CHAR_INFO *ci=(CHAR_INFO*)Buf;

		for (int y=0; y<BufY; y++)
		{
			Size.Y=1;
			Coord.Top=y;
			Coord.Bottom=y;
			BOOL r;
#if defined(USE_WFUNC)

			if (Opt.UseUnicodeConsole)
				r=ReadConsoleOutputW(hScreen,ci,Size,Corner,&Coord);
			else
				r=ReadConsoleOutputA(hScreen,ci,Size,Corner,&Coord);

#else
			r=ReadConsoleOutput(hScreen,ci,Size,Corner,&Coord);
#endif
			_tran(SysLog("r=%i, le=%i",r,GetLastError()));
			ci+=BufX;
		}

		_tran(SysLog("fucked method end"));
	}
	else
	{
#if defined(USE_WFUNC)

		if (Opt.UseUnicodeConsole)
			ReadConsoleOutputW(hScreen,Buf,Size,Corner,&Coord);
		else
			ReadConsoleOutputA(hScreen,Buf,Size,Corner,&Coord);

#else
		ReadConsoleOutput(hScreen,Buf,Size,Corner,&Coord);
#endif
	}

	memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
	SBFlags.Set(SBFLAGS_USESHADOW);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hScreen,&csbi);
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

	memcpy(Buf+Y*BufX+X,Text,sizeof(CHAR_INFO)*TextLength);
	SBFlags.Clear(SBFLAGS_FLUSHED);
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (DirectRT)
		Flush();

#endif
}

#if defined(USE_WFUNC)
void ScreenBuf::WriteA(int X,int Y,const CHAR_INFO *Text,int TextLength)
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

	for (int I=0; I < TextLength; ++I, ++PtrBuf, ++Text)
	{
		SetVidChar(*PtrBuf,Text->Char.AsciiChar);
		PtrBuf->Attributes=Text->Attributes;
	}

	SBFlags.Clear(SBFLAGS_FLUSHED);
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (DirectRT)
		Flush();

#endif
}
#endif

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

	if (X1==0 && Y1==0 &&
	        CtrlObject!=NULL &&
	        CtrlObject->Macro.IsRecording() &&
	        GetVidChar(MacroChar) != 'R')
		Text[0]=MacroChar;
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
void ScreenBuf::ApplyColor(int X1,int Y1,int X2,int Y2,int Color)
{
	CriticalSectionLock Lock(CS);
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

/* Закрасить прямоугольник символом Ch и цветом Color
*/
void ScreenBuf::FillRect(int X1,int Y1,int X2,int Y2,int Ch,int Color)
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

	if (LockCount>0)
		return;

	if (CtrlObject!=NULL && CtrlObject->Macro.IsRecording())
	{
		if (GetVidChar(Buf[0])!='R')
			MacroChar=Buf[0];

		SetVidChar(Buf[0],'R');
		Buf[0].Attributes=FarColorToReal(COL_WARNDIALOGTEXT);
	}

	if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && !CurVisible)
	{
		CONSOLE_CURSOR_INFO cci;
		cci.dwSize=CurSize;
		cci.bVisible=CurVisible;
		SetConsoleCursorInfo(hScreen,&cci);
		SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
	}

	if (!SBFlags.Check(SBFLAGS_FLUSHED))
	{
		SBFlags.Set(SBFLAGS_FLUSHED);

		if (WaitInMainLoop && Opt.Clock && !ProcessShowClock)
			ShowTime(FALSE);

		int WriteX1=BufX-1,WriteY1=BufY-1,WriteX2=0,WriteY2=0;
		int NoChanges=TRUE;

		if (SBFlags.Check(SBFLAGS_USESHADOW))
		{
			CHAR_INFO *PtrBuf=Buf, *PtrShadow=Shadow;

			for (int I=0; I < BufY; I++)
			{
				for (int J=0; J < BufX; J++, ++PtrBuf, ++PtrShadow)
				{
					if (PtrBuf->Attributes  != PtrShadow->Attributes ||
					        GetVidChar(*PtrBuf) != GetVidChar(*PtrShadow))
					{
						if (WriteX1>J)
							WriteX1=J;

						if (WriteX2<J)
							WriteX2=J;

						if (WriteY1>I)
							WriteY1=I;

						if (WriteY2<I)
							WriteY2=I;

						NoChanges=FALSE;
					}
				}
			}

			if (!NoChanges)
			{
				// при включенном СlearType-сглаживании на экране остаётся "мусор" - тонкие вертикальные полосы
				// кстати, и при выключенном тоже (но реже).
				// баг, конечно, не наш, но что делать.
				// расширяем область прорисовки влево-вправо на 1 символ:
				WriteX1=Max(0,WriteX1-1);
				WriteX2=Min(WriteX2+1,ScrX);
			}
		}
		else
		{
			NoChanges=FALSE;
			WriteX1=0;
			WriteY1=0;
			WriteX2=BufX-1;
			WriteY2=BufY-1;
		}

		if (!NoChanges)
		{
			COORD Size,Corner;
			SMALL_RECT Coord;
			Size.X=BufX;
			Size.Y=BufY;
			Corner.X=WriteX1;
			Corner.Y=WriteY1;
			Coord.Left=WriteX1;
			Coord.Top=WriteY1;
			Coord.Right=WriteX2;
			Coord.Bottom=WriteY2;
			// BUGBUG: в Windows 7 при 0xFFFF в консоли имеем мусор и падает conhost.
			// посему пишем по 32 K.
#define MAXSIZE 0x7FFF

			if (BufX*BufY*sizeof(CHAR_INFO) > MAXSIZE) // See REMINDER file section scrbuf.cpp
			{
				Corner.Y=0;
				PCHAR_INFO BufPtr;
				int maxY;

				for (int yy=WriteY1; yy<=WriteY2;)
				{
					Coord.Top=yy;
					BufPtr=Buf+yy*BufX;
					maxY=MAXSIZE/(BufX*sizeof(CHAR_INFO));

					if (maxY==0)
					{
						maxY=1;
					}
					else if (maxY-1+yy>WriteY2)
					{
						maxY=WriteY2-yy+1;
					}

					Size.Y=maxY;
					yy+=maxY;
					Coord.Bottom=yy-1;
#if defined(USE_WFUNC)

					if (Opt.UseUnicodeConsole)
						WriteConsoleOutputW(hScreen, BufPtr, Size, Corner, &Coord);
					else
						WriteConsoleOutputA(hScreen, BufPtr, Size, Corner, &Coord);

#else
					WriteConsoleOutput(hScreen, BufPtr, Size, Corner, &Coord);
#endif
				}
			}
			else
			{
#if defined(USE_WFUNC)

				if (Opt.UseUnicodeConsole)
					WriteConsoleOutputW(hScreen, Buf, Size, Corner, &Coord);
				else
					WriteConsoleOutputA(hScreen, Buf, Size, Corner, &Coord);

#else
				WriteConsoleOutput(hScreen, Buf, Size, Corner, &Coord);
#endif
			}

			memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
		}
	}

	if (!SBFlags.Check(SBFLAGS_FLUSHEDCURPOS))
	{
		COORD C;
		C.X=CurX;
		C.Y=CurY;
		SetConsoleCursorPosition(hScreen,C);
		SBFlags.Set(SBFLAGS_FLUSHEDCURPOS);
	}

	if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && CurVisible)
	{
		CONSOLE_CURSOR_INFO cci;
		cci.dwSize=CurSize;
		cci.bVisible=CurVisible;
		SetConsoleCursorInfo(hScreen,&cci);
		SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
	}

	SBFlags.Set(SBFLAGS_USESHADOW|SBFLAGS_FLUSHED);
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


void ScreenBuf::SetHandle(HANDLE hScreen)
{
	ScreenBuf::hScreen=hScreen;
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


void ScreenBuf::GetCursorPos(int& X,int& Y)
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
	Write(0,0,&MacroChar,1);
	ResetShadow();
}

/*$ 23.07.2001 SKV
  проскроллировать буффер на одну строку вверх.
*/
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

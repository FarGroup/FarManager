/*
savescr.cpp

Сохраняем и восстанавливааем экран кусками и целиком
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

#include "savescr.hpp"
#include "colors.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "console.hpp"
#include "colormix.hpp"

SaveScreen::SaveScreen()
{
	_OT(SysLog(L"[%p] SaveScreen::SaveScreen()", this));
	SaveArea(0,0,ScrX,ScrY);
}

SaveScreen::SaveScreen(int X1,int Y1,int X2,int Y2)
{
	_OT(SysLog(L"[%p] SaveScreen::SaveScreen(X1=%i,Y1=%i,X2=%i,Y2=%i)",this,X1,Y1,X2,Y2));

	X1=Min(static_cast<int>(ScrX), Max(0, X1));
	X2=Min(static_cast<int>(ScrX), Max(0, X2));
	Y1=Min(static_cast<int>(ScrY), Max(0, Y1));
	Y2=Min(static_cast<int>(ScrY), Max(0, Y2));

	SaveArea(X1,Y1,X2,Y2);
}


SaveScreen::~SaveScreen()
{
	if (!ScreenBuf)
		return;

	_OT(SysLog(L"[%p] SaveScreen::~SaveScreen()", this));
	RestoreArea();
	delete[] ScreenBuf;
}


void SaveScreen::Discard()
{
	if (!ScreenBuf)
		return;

	delete[] ScreenBuf;
	ScreenBuf=nullptr;
}


void SaveScreen::RestoreArea(int RestoreCursor)
{
	if (!ScreenBuf)
		return;

	PutText(X1,Y1,X2,Y2,ScreenBuf);

	if (RestoreCursor)
	{
		SetCursorType(CurVisible,CurSize);
		MoveCursor(CurPosX,CurPosY);
	}
}


void SaveScreen::SaveArea(int X1,int Y1,int X2,int Y2)
{
	SaveScreen::X1=X1;
	SaveScreen::Y1=Y1;
	SaveScreen::X2=X2;
	SaveScreen::Y2=Y2;
	ScreenBuf=new FAR_CHAR_INFO[ScreenBufCharCount()];

	if (!ScreenBuf)
		return;

	GetText(X1,Y1,X2,Y2,ScreenBuf,ScreenBufCharCount()*sizeof(FAR_CHAR_INFO));
	GetCursorPos(CurPosX,CurPosY);
	GetCursorType(CurVisible,CurSize);
}

void SaveScreen::SaveArea()
{
	if (!ScreenBuf)
		return;

	GetText(X1,Y1,X2,Y2,ScreenBuf,ScreenBufCharCount()*sizeof(FAR_CHAR_INFO));
	GetCursorPos(CurPosX,CurPosY);
	GetCursorType(CurVisible,CurSize);
}

void SaveScreen::CorrectRealScreenCoord()
{
	if (X1 < 0) X1=0;

	if (Y1 < 0) Y1=0;

	if (X2 >= ScrX) X2=ScrX;

	if (Y2 >= ScrY) Y2=ScrY;
}

void SaveScreen::AppendArea(SaveScreen *NewArea)
{
	FAR_CHAR_INFO *Buf=ScreenBuf,*NewBuf=NewArea->ScreenBuf;

	if (!Buf || !NewBuf)
		return;

	for (int X=X1; X<=X2; X++)
		if (X>=NewArea->X1 && X<=NewArea->X2)
			for (int Y=Y1; Y<=Y2; Y++)
				if (Y>=NewArea->Y1 && Y<=NewArea->Y2)
					Buf[X-X1+(X2-X1+1)*(Y-Y1)]=NewBuf[X-NewArea->X1+(NewArea->X2-NewArea->X1+1)*(Y-NewArea->Y1)];
}

void SaveScreen::Resize(int NewX,int NewY, DWORD Corner, bool SyncWithConsole)
//  Corner definition:
//  0 --- 1
//  |     |
//  2 --- 3
{
	int OWi=X2-X1+1, OHe=Y2-Y1+1, iY=0;

	if (OWi==NewX && OHe==NewY)
	{
		return;
	}

	int NX1,NX2,NY1,NY2;
	NX1=NX2=NY1=NY2=0;
	FAR_CHAR_INFO* NewBuf = new FAR_CHAR_INFO[NewX*NewY];
	CleanupBuffer(NewBuf,NewX*NewY);
	int NewWidth=Min(OWi,NewX);
	int NewHeight=Min(OHe,NewY);
	int iYReal;
	int ToIndex=0;
	int FromIndex=0;

	if (Corner & 2)
	{
		NY2=Y1+NewY-1; NY1=NY2-NewY+1;
	}
	else
	{
		NY1=Y1; NY2=NY1+NewY-1;
	}

	if (Corner & 1)
	{
		NX2=X1+NewX-1; NX1=NX2-NewX+1;
	}
	else
	{
		NX1=X1; NX2=NX1+NewX-1;
	}

	for (iY=0; iY<NewHeight; iY++)
	{
		if (Corner & 2)
		{
			if (OHe>NewY)
			{
				iYReal=OHe-NewY+iY;
				FromIndex=iYReal*OWi;
				ToIndex=iY*NewX;
			}
			else
			{
				iYReal=NewY-OHe+iY;
				ToIndex=iYReal*NewX;
				FromIndex=iY*OWi;
			}
		}

		if (Corner & 1)
		{
			if (OWi>NewX)
			{
				FromIndex+=OWi-NewX;
			}
			else
			{
				ToIndex+=NewX-OWi;
			}
		}

		CharCopy(&NewBuf[ToIndex],&ScreenBuf[FromIndex],NewWidth);
	}

	// achtung, experimental
	if((Corner&2) && SyncWithConsole)
	{
		Console.ResetPosition();
		if(NewY!=OHe)
		{
			COORD Size={static_cast<SHORT>(Max(NewX,OWi)), static_cast<SHORT>(abs(OHe-NewY))};
			COORD Coord={};
			FAR_CHAR_INFO* Tmp=new FAR_CHAR_INFO[Size.X*Size.Y];
			if(NewY>OHe)
			{
				SMALL_RECT ReadRegion={0, 0, static_cast<SHORT>(NewX-1), static_cast<SHORT>(NewY-OHe-1)};
				Console.ReadOutput(Tmp, Size, Coord, ReadRegion);
				for(int i=0; i<Size.Y;i++)
				{
					CharCopy(&NewBuf[i*Size.X],&Tmp[i*Size.X], Size.X);
				}
			}
			else
			{
				SMALL_RECT WriteRegion={0, static_cast<SHORT>(NewY-OHe), static_cast<SHORT>(NewX-1), -1};
				for(int i=0; i<Size.Y;i++)
				{
					CharCopy(&Tmp[i*Size.X],&ScreenBuf[i*OWi], Size.X);
				}
				Console.WriteOutput(Tmp, Size, Coord, WriteRegion);
				Console.Commit();
			}
			delete[] Tmp;
		}

		if(NewX!=OWi)
		{
			COORD Size={static_cast<SHORT>(abs(NewX-OWi)), static_cast<SHORT>(Max(NewY,OHe))};
			COORD Coord={};
			FAR_CHAR_INFO* Tmp=new FAR_CHAR_INFO[Size.X*Size.Y];
			if(NewX>OWi)
			{
				SMALL_RECT ReadRegion={static_cast<SHORT>(OWi), 0, static_cast<SHORT>(NewX-1), static_cast<SHORT>(NewY-1)};
				Console.ReadOutput(Tmp, Size, Coord, ReadRegion);
				for(int i=0; i<NewY;i++)
				{
					CharCopy(&NewBuf[i*NewX+OWi],&Tmp[i*Size.X], Size.X);
				}
			}
			else
			{
				SMALL_RECT WriteRegion={static_cast<SHORT>(NewX), static_cast<SHORT>(NewY-OHe), static_cast<SHORT>(OWi-1), static_cast<SHORT>(NewY-1)};
				for(int i=0; i<Size.Y;i++)
				{
					if (i < OHe)
						CharCopy(&Tmp[i*Size.X],&ScreenBuf[i*OWi+NewX], Size.X);
					else
						CleanupBuffer(&Tmp[i*Size.X], Size.X);
				}
				Console.WriteOutput(Tmp, Size, Coord, WriteRegion);
				Console.Commit();
			}
			delete[] Tmp;
		}
	}

	delete [] ScreenBuf;
	ScreenBuf=NewBuf;
	X1=NX1; Y1=NY1; X2=NX2; Y2=NY2;
}


int SaveScreen::ScreenBufCharCount()
{
	return (X2-X1+1)*(Y2-Y1+1);
}

void SaveScreen::CharCopy(FAR_CHAR_INFO* ToBuffer,FAR_CHAR_INFO* FromBuffer,int Count)
{
	memcpy(ToBuffer,FromBuffer,Count*sizeof(FAR_CHAR_INFO));
}

void SaveScreen::CleanupBuffer(FAR_CHAR_INFO* Buffer, size_t BufSize)
{
	FarColor Color=ColorIndexToColor(COL_COMMANDLINEUSERSCREEN);

	for (size_t i=0; i<BufSize; i++)
	{
		Buffer[i].Attributes=Color;
		Buffer[i].Char=L' ';
	}
}

void SaveScreen::DumpBuffer(const wchar_t *Title)
{
	SaveScreenDumpBuffer(Title,GetBufferAddress(),X1,Y1,X2,Y2,nullptr);
}

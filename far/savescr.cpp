/*
savescr.cpp

—охран€ем и восстанавливааем экран кусками и целиком

*/

#include "headers.hpp"
#pragma hdrstop

#include "savescr.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "global.hpp"

SaveScreen::SaveScreen()
{
	_OT(SysLog("[%p] SaveScreen::SaveScreen()", this));
	RealScreen=FALSE;
	SaveArea(0,0,ScrX,ScrY);
}


SaveScreen::SaveScreen(int RealScreen)
{
	_OT(SysLog("[%p] SaveScreen::SaveScreen(RealScreen=%i)",this,RealScreen));
	SaveScreen::RealScreen=RealScreen;
	SaveArea(0,0,ScrX,ScrY);
}


SaveScreen::SaveScreen(int X1,int Y1,int X2,int Y2,int RealScreen)
{
	_OT(SysLog("[%p] SaveScreen::SaveScreen(X1=%i,Y1=%i,X2=%i,Y2=%i)",this,X1,Y1,X2,Y2));
	SaveScreen::RealScreen=RealScreen;

	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;

	SaveArea(X1,Y1,X2,Y2);
}


SaveScreen::~SaveScreen()
{
	if (!ScreenBuf)
		return;

	_OT(SysLog("[%p] SaveScreen::~SaveScreen()", this));
	RestoreArea();
	delete[] ScreenBuf;
}


void SaveScreen::Discard()
{
	if (!ScreenBuf)
		return;

	delete[] ScreenBuf;
	ScreenBuf=NULL;
}


void SaveScreen::RestoreArea(int RestoreCursor)
{
	if (!ScreenBuf)
		return;

	if (RealScreen)
		PutRealText(X1,Y1,X2,Y2,ScreenBuf);
	else
		PutText(X1,Y1,X2,Y2,ScreenBuf);

	if (RestoreCursor)
		if (RealScreen)
		{
			SetRealCursorType(CurVisible,CurSize);
			MoveRealCursor(CurPosX,CurPosY);
		}
		else
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
	ScreenBuf=new CHAR_INFO[ScreenBufCharCount()];

	if (!ScreenBuf)
		return;

	if (RealScreen)
	{
		GetRealText(X1,Y1,X2,Y2,ScreenBuf);
		GetRealCursorPos(CurPosX,CurPosY);
		GetRealCursorType(CurVisible,CurSize);
	}
	else
	{
		GetText(X1,Y1,X2,Y2,ScreenBuf,ScreenBufCharCount()*sizeof(CHAR_INFO));
		GetCursorPos(CurPosX,CurPosY);
		GetCursorType(CurVisible,CurSize);
	}
}

void SaveScreen::SaveArea()
{
	if (!ScreenBuf)
		return;

	if (RealScreen)
	{
		GetRealText(X1,Y1,X2,Y2,ScreenBuf);
		GetRealCursorPos(CurPosX,CurPosY);
		GetRealCursorType(CurVisible,CurSize);
	}
	else
	{
		GetText(X1,Y1,X2,Y2,ScreenBuf,ScreenBufCharCount()*sizeof(CHAR_INFO));
		GetCursorPos(CurPosX,CurPosY);
		GetCursorType(CurVisible,CurSize);
	}
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
	CHAR_INFO *Buf=ScreenBuf,*NewBuf=NewArea->ScreenBuf;

	if (Buf==NULL || NewBuf==NULL)
		return;

	for (int X=X1; X<=X2; X++)
		if (X>=NewArea->X1 && X<=NewArea->X2)
			for (int Y=Y1; Y<=Y2; Y++)
				if (Y>=NewArea->Y1 && Y<=NewArea->Y2)
					Buf[X-X1+(X2-X1+1)*(Y-Y1)]=NewBuf[X-NewArea->X1+(NewArea->X2-NewArea->X1+1)*(Y-NewArea->Y1)];
}

/* $ 21.05.2001 OT  ћетоды дл€ работы с измен€ющимс€ буфером экрана */
void SaveScreen::Resize(int NewX,int NewY, DWORD Corner)
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
	PCHAR_INFO NewBuf = new CHAR_INFO[NewX*NewY];
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

	delete[] ScreenBuf;
	ScreenBuf=NewBuf;
	X1=NX1; Y1=NY1; X2=NX2; Y2=NY2;
}


int SaveScreen::ScreenBufCharCount()
{
	return (X2-X1+1)*(Y2-Y1+1);
}

void SaveScreen::CharCopy(PCHAR_INFO ToBuffer,PCHAR_INFO FromBuffer,int Count)
{
	memcpy(ToBuffer,FromBuffer,Count*sizeof(CHAR_INFO));
}

void SaveScreen::CleanupBuffer(PCHAR_INFO Buffer, size_t BufSize)
{
	WORD Attr=FarColorToReal(COL_COMMANDLINEUSERSCREEN);

	for (size_t i=0; i<BufSize; i++)
	{
		Buffer[i].Attributes=Attr;
		Buffer[i].Char.UnicodeChar=L' ';
	}
}
/* 21.05.2001 OT  $ */

void SaveScreen::DumpBuffer(const char *Title)
{
	SaveScreenDumpBuffer(Title,GetBufferAddress(),X1,Y1,X2,Y2,RealScreen,NULL);
}

/*
savescr.cpp

Сохраняем и восстанавливаем экран кусками и целиком
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
	SaveArea(X1,Y1,X2,Y2);
}


SaveScreen::~SaveScreen()
{
	_OT(SysLog(L"[%p] SaveScreen::~SaveScreen()", this));
	RestoreArea();
}


void SaveScreen::Discard()
{
	// don't call vector.resize() here, it's never shrink
	clear_and_shrink(ScreenBuf);
}


void SaveScreen::RestoreArea(int RestoreCursor)
{
	if (ScreenBuf.empty())
		return;

	PutText(m_X1, m_Y1, m_X2, m_Y2, ScreenBuf.data());

	if (RestoreCursor)
	{
		SetCursorType(CurVisible,CurSize);
		MoveCursor(CurPosX,CurPosY);
	}
}


void SaveScreen::SaveArea(int X1,int Y1,int X2,int Y2)
{
	m_X1=X1;
	m_Y1=Y1;
	m_X2=X2;
	m_Y2=Y2;

	ScreenBuf.allocate(Y2 - Y1 + 1, X2 - X1 + 1);

	GetText(X1, Y1, X2, Y2, ScreenBuf);
	GetCursorPos(CurPosX,CurPosY);
	GetCursorType(CurVisible,CurSize);
}

void SaveScreen::SaveArea()
{
	if (ScreenBuf.empty())
		return;

	GetText(m_X1, m_Y1, m_X2, m_Y2, ScreenBuf);
	GetCursorPos(CurPosX,CurPosY);
	GetCursorType(CurVisible,CurSize);
}

void SaveScreen::AppendArea(const SaveScreen *NewArea)
{
	for (int X = m_X1; X <= m_X2; X++)
		if (X >= NewArea->m_X1 && X <= NewArea->m_X2)
			for (int Y = m_Y1; Y <= m_Y2; Y++)
				if (Y >= NewArea->m_Y1 && Y <= NewArea->m_Y2)
					ScreenBuf.vector()[X - m_X1 + (m_X2 - m_X1 + 1)*(Y - m_Y1)] = (NewArea->ScreenBuf.vector())[X - NewArea->m_X1 + (NewArea->m_X2 - NewArea->m_X1 + 1)*(Y - NewArea->m_Y1)];
}

void SaveScreen::Resize(int NewX,int NewY, DWORD Corner, bool SyncWithConsole)
//  Corner definition:
//  0 --- 1
//  |     |
//  2 --- 3
{
	int OWi = m_X2 - m_X1 + 1, OHe = m_Y2 - m_Y1 + 1, iY = 0;

	if (OWi==NewX && OHe==NewY)
	{
		return;
	}

	int NX1 = 0, NX2 = 0, NY1 = 0, NY2 = 0;
	matrix<FAR_CHAR_INFO> NewBuf(NewY, NewX);
	CleanupBuffer(NewBuf.data(), NewBuf.size());
	int NewWidth=std::min(OWi,NewX);
	int NewHeight=std::min(OHe,NewY);
	int iYReal;
	int ToIndex=0;
	int FromIndex=0;

	if (Corner & 2)
	{
		NY2 = m_Y1 + NewY - 1; NY1 = NY2 - NewY + 1;
	}
	else
	{
		NY1 = m_Y1; NY2 = NY1 + NewY - 1;
	}

	if (Corner & 1)
	{
		NX2 = m_X1 + NewX - 1; NX1 = NX2 - NewX + 1;
	}
	else
	{
		NX1 = m_X1; NX2 = NX1 + NewX - 1;
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

		std::copy_n(ScreenBuf.data() + FromIndex, NewWidth, NewBuf.data() + ToIndex);
	}

	// achtung, experimental
	if((Corner&2) && SyncWithConsole)
	{
		Console().ResetPosition();
		if(NewY!=OHe)
		{
			matrix<FAR_CHAR_INFO> Tmp(abs(OHe - NewY), std::max(NewX, OWi));
			if(NewY>OHe)
			{
				SMALL_RECT ReadRegion={0, 0, static_cast<SHORT>(NewX-1), static_cast<SHORT>(NewY-OHe-1)};
				Console().ReadOutput(Tmp, ReadRegion);
				for(size_t i = 0; i != Tmp.height(); ++i)
				{
					std::copy_n(Tmp[i].data(), Tmp.width(), NewBuf[i].data());
				}
			}
			else
			{
				SMALL_RECT WriteRegion={0, static_cast<SHORT>(NewY-OHe), static_cast<SHORT>(NewX-1), -1};
				for(size_t i = 0; i != Tmp.height(); ++i)
				{
					std::copy_n(ScreenBuf[i].data(), Tmp.width(), Tmp[i].data());
				}
				Console().WriteOutput(Tmp, WriteRegion);
				Console().Commit();
			}
		}

		if(NewX!=OWi)
		{
			matrix<FAR_CHAR_INFO> Tmp(std::max(NewY, OHe), abs(NewX - OWi));
			if(NewX>OWi)
			{
				SMALL_RECT ReadRegion={static_cast<SHORT>(OWi), 0, static_cast<SHORT>(NewX-1), static_cast<SHORT>(NewY-1)};
				Console().ReadOutput(Tmp, ReadRegion);
				for(size_t i = 0; i != NewBuf.height(); ++i)
				{
					std::copy_n(Tmp[i].data(), Tmp.width(), &NewBuf[i][OWi]);
				}
			}
			else
			{
				SMALL_RECT WriteRegion={static_cast<SHORT>(NewX), static_cast<SHORT>(NewY-OHe), static_cast<SHORT>(OWi-1), static_cast<SHORT>(NewY-1)};
				for(size_t i = 0; i != Tmp.height(); ++i)
				{
					if (static_cast<int>(i) < OHe)
						std::copy_n(&ScreenBuf[i][NewX], Tmp.width(), Tmp[i].data());
					else
						CleanupBuffer(Tmp[i].data(), Tmp.width());
				}
				Console().WriteOutput(Tmp, WriteRegion);
				Console().Commit();
			}
		}
	}

	ScreenBuf.swap(NewBuf);
	m_X1 = NX1; m_Y1 = NY1; m_X2 = NX2; m_Y2 = NY2;
}

void SaveScreen::CleanupBuffer(FAR_CHAR_INFO* Buffer, size_t BufSize)
{
	const FAR_CHAR_INFO Value = { L' ', colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN) };
	std::fill_n(Buffer, BufSize, Value);
}

void SaveScreen::DumpBuffer(const wchar_t *Title)
{
	SaveScreenDumpBuffer(Title, ScreenBuf.data(), m_X1, m_Y1, m_X2, m_Y2, nullptr);
}

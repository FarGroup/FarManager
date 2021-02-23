﻿/*
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "savescr.hpp"

// Internal:
#include "farcolor.hpp"
#include "interf.hpp"
#include "console.hpp"
#include "colormix.hpp"

// Platform:

// Common:
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

static void CleanupBuffer(FAR_CHAR_INFO* Buffer, size_t BufSize)
{
	const FAR_CHAR_INFO Value = { L' ', colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN) };
	std::fill_n(Buffer, BufSize, Value);
}

SaveScreen::SaveScreen()
{
	SaveArea({ 0, 0, ScrX, ScrY });
}

SaveScreen::SaveScreen(rectangle Where)
{
	fix_coordinates(Where);
	SaveArea(Where);
}


SaveScreen::~SaveScreen()
{
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

	PutText(m_Where, ScreenBuf.data());

	if (RestoreCursor)
	{
		SetCursorType(CurVisible,CurSize);
		MoveCursor(m_Cursor);
	}
}


void SaveScreen::SaveArea(rectangle Where)
{
	fix_coordinates(Where);

	m_Where = Where;

	ScreenBuf.allocate(height(), width());
	SaveArea();
}

void SaveScreen::SaveArea()
{
	if (ScreenBuf.empty())
		return;

	GetText(m_Where, ScreenBuf);
	m_Cursor = GetCursorPos();
	GetCursorType(CurVisible,CurSize);
}

void SaveScreen::AppendArea(const SaveScreen *NewArea)
{
	const auto Offset = [](const SaveScreen* Ptr, int X, int Y)
	{
		return X - Ptr->m_Where.left + Ptr->width() * (Y - Ptr->m_Where.top);
	};

	for (int X = m_Where.left; X <= m_Where.right; ++X)
		if (X >= NewArea->m_Where.left && X <= NewArea->m_Where.right)
			for (int Y = m_Where.top; Y <= m_Where.bottom; ++Y)
				if (Y >= NewArea->m_Where.top && Y <= NewArea->m_Where.bottom)
					ScreenBuf.vector()[Offset(this, X, Y)] = NewArea->ScreenBuf.vector()[Offset(NewArea, X, Y)];
}

void SaveScreen::Resize(int DesiredWidth, int DesiredHeight, bool SyncWithConsole)
{
	const auto OriginalWidth = width();
	const auto OriginalHeight = height();

	if (OriginalWidth == DesiredWidth && OriginalHeight == DesiredHeight)
	{
		return;
	}

	matrix<FAR_CHAR_INFO> NewBuf(DesiredHeight, DesiredWidth);
	CleanupBuffer(NewBuf.data(), NewBuf.size());

	const rectangle NewWhere = { m_Where.left, m_Where.top, m_Where.left + DesiredWidth - 1, m_Where.top + DesiredHeight - 1 };

	const auto DeltaY = abs(DesiredHeight - OriginalHeight);
	const size_t CopyWidth = std::min(OriginalWidth, DesiredWidth);
	const size_t CopyHeight = std::min(OriginalHeight, DesiredHeight);

	if (DesiredHeight > OriginalHeight)
	{
		for (size_t i = 0; i != CopyHeight; ++i)
		{
			const auto FromIndex = i * OriginalWidth;
			const auto ToIndex = (i + DeltaY) * DesiredWidth;
			std::copy_n(ScreenBuf.data() + FromIndex, CopyWidth, NewBuf.data() + ToIndex);
		}
	}
	else
	{
		for (size_t i = 0; i != CopyHeight; ++i)
		{
			const auto FromIndex = (i + DeltaY) * OriginalWidth;
			const auto ToIndex = i * DesiredWidth;
			std::copy_n(ScreenBuf.data() + FromIndex, CopyWidth, NewBuf.data() + ToIndex);
		}
	}

	// achtung, experimental
	if (SyncWithConsole)
	{
		std::pair<rectangle, bool> WindowRect;
		WindowRect.second = console.GetWindowRect(WindowRect.first);
		const auto IsExtraTop = WindowRect.second && !(WindowRect.first.top == 0 && WindowRect.first.bottom == OriginalHeight);
		const auto IsExtraRight = WindowRect.second && !(WindowRect.first.left == 0 && WindowRect.first.right == OriginalWidth);

		if (DesiredHeight != OriginalHeight)
		{
			matrix<FAR_CHAR_INFO> Tmp(abs(OriginalHeight - DesiredHeight), std::max(DesiredWidth, OriginalWidth));
			if (DesiredHeight > OriginalHeight)
			{
				if (IsExtraTop)
				{
					rectangle const ReadRegion{ 0, 0, DesiredWidth - 1, DesiredHeight - OriginalHeight - 1 };
					if (console.ReadOutput(Tmp, ReadRegion))
					{
						for (size_t i = 0; i != Tmp.height(); ++i)
						{
							std::copy_n(Tmp[i].data(), Tmp.width(), NewBuf[i].data());
						}
					}
				}
			}
			else
			{
				rectangle const WriteRegion{ 0, DesiredHeight - OriginalHeight, DesiredWidth - 1, -1 };
				for (size_t i = 0; i != Tmp.height(); ++i)
				{
					std::copy_n(ScreenBuf[i].data(), Tmp.width(), Tmp[i].data());
				}
				console.WriteOutput(Tmp, WriteRegion);
				console.Commit();
			}
		}

		if (DesiredWidth != OriginalWidth)
		{
			matrix<FAR_CHAR_INFO> Tmp(std::max(DesiredHeight, OriginalHeight), abs(DesiredWidth - OriginalWidth));
			if (DesiredWidth > OriginalWidth)
			{
				if (IsExtraRight)
				{
					rectangle const ReadRegion{ OriginalWidth, 0, DesiredWidth - 1, DesiredHeight - 1 };
					console.ReadOutput(Tmp, ReadRegion);
					for (size_t i = 0; i != NewBuf.height(); ++i)
					{
						std::copy_n(Tmp[i].data(), Tmp.width(), &NewBuf[i][OriginalWidth]);
					}
				}
			}
			else
			{
				rectangle const WriteRegion{ DesiredWidth, DesiredHeight - OriginalHeight, OriginalWidth - 1, DesiredHeight - 1 };
				for (size_t i = 0; i != Tmp.height(); ++i)
				{
					if (static_cast<int>(i) < OriginalHeight)
						std::copy_n(&ScreenBuf[i][DesiredWidth], Tmp.width(), Tmp[i].data());
					else
						CleanupBuffer(Tmp[i].data(), Tmp.width());
				}
				console.WriteOutput(Tmp, WriteRegion);
				console.Commit();
			}
		}
	}

	using std::swap;
	swap(ScreenBuf, NewBuf);
	m_Where = NewWhere;
}

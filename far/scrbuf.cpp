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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "scrbuf.hpp"

// Internal:
#include "farcolor.hpp"
#include "ctrlobj.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "elevation.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/2d/algorithm.hpp"

// External:

//----------------------------------------------------------------------------

enum
{
	SBFLAGS_FLUSHED         = 0_bit,
	SBFLAGS_FLUSHEDCURPOS   = 1_bit,
	SBFLAGS_FLUSHEDCURTYPE  = 2_bit,
	SBFLAGS_FLUSHEDTITLE    = 3_bit,
};

//#if defined(SYSLOG_OT)
// #define DIRECT_SCREEN_OUT
//#endif

static bool is_visible(point const& Where)
{
	return
		in_closed_range(0, Where.x, ScrX) &&
		in_closed_range(0, Where.y, ScrY);
}

static bool is_visible(rectangle const& Where)
{
	return Where.left <= ScrX && Where.top <= ScrY && Where.right >= 0 && Where.bottom >= 0;
}

ScreenBuf::ScreenBuf():
	SBFlags(SBFLAGS_FLUSHED | SBFLAGS_FLUSHEDCURPOS | SBFLAGS_FLUSHEDCURTYPE | SBFLAGS_FLUSHEDTITLE)
{
}

void ScreenBuf::DebugDump() const
{
#ifdef _DEBUG
	string s;
	s.reserve(Buf.width() + 1);

	for (size_t row_num = 0; row_num != Buf.height(); ++row_num)
	{
		const auto& row = Buf[row_num];
		std::transform(ALL_CONST_RANGE(row), std::back_inserter(s), [](const auto& i) { return i.Char; });
		s.push_back(L'\n');
		os::debug::print(s);
		s.clear();
	}
#endif
}

void ScreenBuf::AllocBuf(size_t rows, size_t cols)
{
	SCOPED_ACTION(std::lock_guard)(CS);

	if (rows == Buf.height() && cols == Buf.width())
		return;

	Buf.allocate(rows, cols);
	Shadow.allocate(rows, cols);
}

/* Заполнение виртуального буфера значением из консоли.
*/
void ScreenBuf::FillBuf()
{
	SCOPED_ACTION(std::lock_guard)(CS);

	rectangle const ReadRegion{ 0, 0, static_cast<int>(Buf.width() - 1), static_cast<int>(Buf.height() - 1) };
	console.ReadOutput(Buf, ReadRegion);
	Shadow = Buf;
	point CursorPosition;
	console.GetCursorPosition(CursorPosition);
	m_CurPos = CursorPosition;
}

/* Записать Text в виртуальный буфер
*/
void ScreenBuf::Write(int X, int Y, span<const FAR_CHAR_INFO> Text)
{
	SCOPED_ACTION(std::lock_guard)(CS);

	if (X<0)
	{
		Text.pop_front(std::min(static_cast<size_t>(-X), Text.size()));
		X=0;
	}

	if (X >= static_cast<int>(Buf.width()) || Y >= static_cast<int>(Buf.height()) || Text.empty() || Y < 0)
		return;

	if (static_cast<size_t>(X + Text.size()) > Buf.width())
		Text.pop_back(Text.size() - (Buf.width() - X));

	for (size_t i = 0; i != Text.size(); ++i)
	{
		Buf[Y][X + i] = Text[i];
	}

	SBFlags.Clear(SBFLAGS_FLUSHED);

	debug_flush();
}


/* Читать блок из виртуального буфера.
*/
void ScreenBuf::Read(rectangle Where, matrix<FAR_CHAR_INFO>& Dest)
{
	SCOPED_ACTION(std::lock_guard)(CS);

	fix_coordinates(Where);

	for (auto i = Where.top; i <= Where.bottom; ++i)
	{
		const auto Row = Buf[i];
		std::copy_n(Row.cbegin() + Where.left, Where.width(), Dest[i - Where.top].begin());
	}
}

/* Изменить значение цветовых атрибутов в соответствии с маской
   (применяется для "создания" тени)
*/
void ScreenBuf::ApplyShadow(rectangle Where)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(std::lock_guard)(CS);

	fix_coordinates(Where);

	for_submatrix(Buf, Where, [](FAR_CHAR_INFO& Element)
	{
		Element.Attributes.BackgroundColor = 0;

		const auto Mask = Element.Attributes.IsFg4Bit()? FOREGROUND_INTENSITY : 0x808080;

		Element.Attributes.ForegroundColor &= ~Mask;
		if (!colors::color_value(Element.Attributes.ForegroundColor))
		{
			Element.Attributes.ForegroundColor = Mask;
		}
	});

	debug_flush();
}

/* Непосредственное изменение цветовых атрибутов
*/
// used in block selection
void ScreenBuf::ApplyColor(rectangle Where, const FarColor& Color, bool PreserveExFlags)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(std::lock_guard)(CS);

	fix_coordinates(Where);

	if(PreserveExFlags)
	{
		for_submatrix(Buf, Where, [&Color](FAR_CHAR_INFO& Element)
		{
			const auto ExFlags = Element.Attributes.Flags&FCF_EXTENDEDFLAGS;
			Element.Attributes = Color;
			Element.Attributes.Flags = (Element.Attributes.Flags&~FCF_EXTENDEDFLAGS) | ExFlags;
		});
	}
	else
	{
		for_submatrix(Buf, Where, [&Color](FAR_CHAR_INFO& Element)
		{
			Element.Attributes = Color;
		});
	}

	debug_flush();
}

/* Непосредственное изменение цветовых атрибутов с заданным цветом исключением
*/
// used in stream selection
void ScreenBuf::ApplyColor(rectangle Where, const FarColor& Color, const FarColor& ExceptColor, bool ForceExFlags)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(std::lock_guard)(CS);

	fix_coordinates(Where);

	for_submatrix(Buf, Where, [&](FAR_CHAR_INFO& Element)
	{
		if (Element.Attributes.ForegroundColor != ExceptColor.ForegroundColor || Element.Attributes.BackgroundColor != ExceptColor.BackgroundColor)
		{
			Element.Attributes = Color;
		}
		else if (ForceExFlags)
		{
			Element.Attributes.Flags = (Element.Attributes.Flags&~FCF_EXTENDEDFLAGS) | (Color.Flags&FCF_EXTENDEDFLAGS);
		}
	});

	debug_flush();
}

/* Закрасить прямоугольник символом Ch и цветом Color
*/
void ScreenBuf::FillRect(rectangle Where, const FAR_CHAR_INFO& Info)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(std::lock_guard)(CS);

	fix_coordinates(Where);

	for_submatrix(Buf, Where, [&Info](FAR_CHAR_INFO& Element)
	{
		Element = Info;
	});

	SBFlags.Clear(SBFLAGS_FLUSHED);

	debug_flush();
}

/* "Сбросить" виртуальный буфер на консоль
*/
void ScreenBuf::Flush(flush_type FlushType)
{
	SCOPED_ACTION(std::lock_guard)(CS);

	if (FlushType & flush_type::title && !SBFlags.Check(SBFLAGS_FLUSHEDTITLE))
	{
		console.SetTitle(m_Title);
		SBFlags.Set(SBFLAGS_FLUSHEDTITLE);
	}

	if (LockCount)
		return;

	if (!console.IsViewportVisible())
		return;

	if (FlushType & flush_type::screen)
	{
		if (!Global->SuppressIndicators)
		{
			const auto SetMacroChar = [this](FAR_CHAR_INFO& Where, wchar_t Char, WORD Color)
			{
				Where.Char = Char;
				Where.Attributes = colors::ConsoleColorToFarColor(Color);
				SBFlags.Clear(SBFLAGS_FLUSHED);
			};

			if (Global->CtrlObject &&
				(Global->CtrlObject->Macro.IsRecording() ||
				(Global->CtrlObject->Macro.IsExecuting() && Global->Opt->Macro.ShowPlayIndicator))
				)
			{
				auto& Where = Buf[0][0];
				MacroChar = Where;
				MacroCharUsed = true;

				Global->CtrlObject->Macro.IsRecording() ?
					SetMacroChar(Where, L'R', B_LIGHTRED | F_WHITE) :
					SetMacroChar(Where, L'P', B_GREEN | F_WHITE);
			}

			if (elevation::instance().Elevated())
			{
				auto& Where = Buf.back().back();
				ElevationChar = Where;
				ElevationCharUsed = true;

				SetMacroChar(Where, L'A', B_LIGHTRED | F_WHITE);
			}
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHED))
		{
			if (Global->IsPanelsActive() && Global->Opt->Clock)
			{
				ShowTime();
			}

			std::vector<rectangle> WriteList;
			bool Changes=false;

			if (m_ClearTypeFix == BSTATE_CHECKED)
			{
				//Для полного избавления от артефактов ClearType будем перерисовывать на всю ширину.
				//Чревато тормозами/миганием в зависимости от конфигурации системы.
				rectangle WriteRegion{ 0, 0, static_cast<int>(Buf.width() - 1), 0 };

				for (size_t I = 0, Height = Buf.height(); I < Height; ++I)
				{
					auto BufRow = Buf[I], ShadowRow = Shadow[I];

					WriteRegion.top = static_cast<int>(I);
					WriteRegion.bottom = static_cast<int>(I - 1);

					while (I < Height && BufRow != ShadowRow)
					{
						I++;
						BufRow = Buf[I];
						ShadowRow = Shadow[I];
						WriteRegion.bottom++;
					}

					if (WriteRegion.bottom >= WriteRegion.top)
					{
						WriteList.emplace_back(WriteRegion);
						Changes=true;
					}
				}
			}
			else
			{
				bool Started=false;
				rectangle WriteRegion = { static_cast<int>(Buf.width() - 1), static_cast<int>(Buf.height() - 1), 0, 0 };

				auto PtrBuf = Buf.data(), PtrShadow = Shadow.data();
				for (size_t I = 0, Height = Buf.height(); I < Height; ++I)
				{
					for (size_t J = 0, Width = Buf.width(); J < Width; ++J, ++PtrBuf, ++PtrShadow)
					{
						if (*PtrBuf != *PtrShadow)
						{
							WriteRegion.left = std::min(WriteRegion.left, static_cast<int>(J));
							WriteRegion.top = std::min(WriteRegion.top, static_cast<int>(I));
							WriteRegion.right = std::max(WriteRegion.right, static_cast<int>(J));
							WriteRegion.bottom = std::max(WriteRegion.bottom, static_cast<int>(I));
							Changes=true;
							Started=true;
						}
						else if (Started && static_cast<int>(I) > WriteRegion.bottom && static_cast<int>(J) >= WriteRegion.left)
						{
							if (m_ClearTypeFix == BSTATE_3STATE)
							{
								//BUGBUG: при включенном СlearType-сглаживании на экране остаётся "мусор" - тонкие вертикальные полосы
								// кстати, и при выключенном тоже (но реже).
								// баг, конечно, не наш, но что делать.
								// расширяем область прорисовки влево-вправо на 1 символ:
								WriteRegion.left = std::max(static_cast<int>(0), static_cast<int>(WriteRegion.left - 1));
								WriteRegion.right = std::min(static_cast<int>(WriteRegion.right + 1), static_cast<int>(Buf.width() - 1));
							}

							bool Merge=false;
							if (!WriteList.empty())
							{
								auto& Last = WriteList.back();
								const int MAX_DELTA = 5;
								if (WriteRegion.top - 1 == Last.bottom && ((WriteRegion.left >= Last.left && WriteRegion.left - Last.left < MAX_DELTA) || (Last.right >= WriteRegion.right && Last.right - WriteRegion.right < MAX_DELTA)))
								{
									Last.bottom = WriteRegion.bottom;
									Last.left = std::min(Last.left, WriteRegion.left);
									Last.right = std::max(Last.right, WriteRegion.right);
									Merge=true;
								}
							}

							if (!Merge)
								WriteList.emplace_back(WriteRegion);

							WriteRegion.left = static_cast<int>(Buf.width() - 1);
							WriteRegion.top = static_cast<int>(Buf.height() - 1);
							WriteRegion.right=0;
							WriteRegion.bottom=0;
							Started=false;
						}
					}
				}

				if (Started)
				{
					WriteList.emplace_back(WriteRegion);
				}
			}

			if (Changes)
			{
				if (IsConsoleSizeChanged())
				{
					// We must draw something, but canvas has been changed, drawing on it will make things only worse
					Changes = false;
					GenerateWINDOW_BUFFER_SIZE_EVENT();
				}
			}

			if (Changes)
			{
				for (const auto& i: WriteList)
				{
					console.WriteOutput(Buf, { i.left, i.top }, i);
				}

				console.Commit();
				Shadow = Buf;
			}

			if (MacroCharUsed)
			{
				Buf[0][0] = MacroChar;
			}

			if (ElevationCharUsed)
			{
				Buf.back().back() = ElevationChar;
			}

			SBFlags.Set(SBFLAGS_FLUSHED);
		}
	}

	if (FlushType & flush_type::cursor)
	{
		// Example: a dialog with an edit control, dragged beyond the screen
		const auto IsCursorInBuffer = is_visible(m_CurPos);

		// Skip setting cursor position if it's not in the viewport to prevent Windows from repositioning the console window
		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURPOS) && IsCursorInBuffer && console.IsPositionVisible(m_CurPos))
		{
			console.SetCursorPosition(m_CurPos);
			SBFlags.Set(SBFLAGS_FLUSHEDCURPOS);
		}

		if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE))
		{
			console.SetCursorInfo({ static_cast<DWORD>(CurSize), CurVisible && IsCursorInBuffer });
			SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
		}
	}
}

void ScreenBuf::Lock()
{
	++LockCount;
}

void ScreenBuf::Unlock()
{
	if (LockCount>0)
		SetLockCount(LockCount-1);
}

void ScreenBuf::SetLockCount(int Count)
{
	LockCount=Count;
}

void ScreenBuf::MoveCursor(point const Point)
{
	SCOPED_ACTION(std::lock_guard)(CS);

	if (!is_visible(m_CurPos))
	{
		CurVisible = false;
	}

	if(Point != m_CurPos || !CurVisible)
	{
		m_CurPos = Point;
		SBFlags.Clear(SBFLAGS_FLUSHEDCURPOS);
	}
}

point ScreenBuf::GetCursorPos() const
{
	return m_CurPos;
}

void ScreenBuf::SetCursorType(bool Visible, size_t Size)
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

void ScreenBuf::GetCursorType(bool& Visible, size_t& Size) const
{
	Visible=CurVisible;
	Size=CurSize;
}

void ScreenBuf::SetTitle(string_view const Title)
{
	if (Title != m_Title)
	{
		m_Title = Title;
		SBFlags.Clear(SBFLAGS_FLUSHEDTITLE);
	}
}

void ScreenBuf::RestoreMacroChar()
{
	if(MacroCharUsed)
	{
		SBFlags.Clear(SBFLAGS_FLUSHED);
		MacroCharUsed=false;
	}
}

void ScreenBuf::RestoreElevationChar()
{
	if(ElevationCharUsed)
	{
		Write(static_cast<int>(Buf.width() - 1), static_cast<int>(Buf.height() - 1), { &ElevationChar, 1 });
		ElevationCharUsed=false;
	}
}

//  проскроллировать буфер вверх.
void ScreenBuf::Scroll(size_t Count)
{
	assert(Count);

	SCOPED_ACTION(std::lock_guard)(CS);

	const FAR_CHAR_INFO Fill{ L' ', colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN) };

	if (Global->Opt->WindowMode)
	{
		if (console.IsScrollbackPresent())
		{
			rectangle Region = { 0, 0, ScrX, static_cast<int>(Count - 1) };

			// TODO: matrix_view to avoid copying
			matrix<FAR_CHAR_INFO> BufferBlock(Count, ScrX + 1);
			Read(Region, BufferBlock);

			console.ScrollNonClientArea(Count, Fill);

			Region.top = -static_cast<int>(Count);
			Region.bottom = -1;
			console.WriteOutput(BufferBlock, Region);
		}
		else
		{
			// Even if there's no scrollback there might be the right area
			console.ScrollNonClientArea(Count, Fill);
		}
	}

	if (Count && Count < Buf.height())
	{
		auto& RawBuf = Buf.vector();
		const auto size = RawBuf.size();
		RawBuf.erase(RawBuf.begin(), RawBuf.begin() + Count * Buf.width());
		RawBuf.resize(size, Fill);

		SBFlags.Clear(SBFLAGS_FLUSHED);
	}

	debug_flush();
}

void ScreenBuf::SetClearTypeFix(int const ClearTypeFix)
{
	// 'New' console doesn't need this
	if (::console.IsVtSupported())
		return;

	m_ClearTypeFix = ClearTypeFix;
}

void ScreenBuf::debug_flush()
{
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)
	if (Global->DirectRT)
		Flush();
#endif
}

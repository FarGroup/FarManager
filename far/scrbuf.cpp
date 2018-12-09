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

#include "scrbuf.hpp"

#include "farcolor.hpp"
#include "ctrlobj.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "elevation.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "global.hpp"

#include "common/2d/algorithm.hpp"

enum
{
	SBFLAGS_FLUSHED         = bit(0),
	SBFLAGS_FLUSHEDCURPOS   = bit(1),
	SBFLAGS_FLUSHEDCURTYPE  = bit(2),
	SBFLAGS_FLUSHEDTITLE    = bit(3),
};

//#if defined(SYSLOG_OT)
// #define DIRECT_SCREEN_OUT
//#endif

static bool is_visible(rectangle const Where)
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
		OutputDebugString(s.c_str());
		s.clear();
	}
#endif
}

void ScreenBuf::AllocBuf(size_t rows, size_t cols)
{
	SCOPED_ACTION(os::critical_section_lock)(CS);

	if (rows == Buf.height() && cols == Buf.width())
		return;

	Buf.allocate(rows, cols);
	Shadow.allocate(rows, cols);
}

/* Заполнение виртуального буфера значением из консоли.
*/
void ScreenBuf::FillBuf()
{
	SCOPED_ACTION(os::critical_section_lock)(CS);

	SMALL_RECT ReadRegion={0, 0, static_cast<SHORT>(Buf.width() - 1), static_cast<SHORT>(Buf.height() - 1)};
	console.ReadOutput(Buf, ReadRegion);
	Shadow = Buf;
	COORD CursorPosition;
	console.GetCursorPosition(CursorPosition);
	m_CurPos = CursorPosition;
}

/* Записать Text в виртуальный буфер
*/
void ScreenBuf::Write(int X, int Y, span<const FAR_CHAR_INFO> Text)
{
	SCOPED_ACTION(os::critical_section_lock)(CS);

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
#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (Global->DirectRT)
		Flush();

#endif
}


/* Читать блок из виртуального буфера.
*/
void ScreenBuf::Read(rectangle Where, matrix<FAR_CHAR_INFO>& Dest)
{
	SCOPED_ACTION(os::critical_section_lock)(CS);

	fix_coordinates(Where);

	for (auto i = Where.top; i <= Where.bottom; ++i)
	{
		const auto Row = Buf[i];
		std::copy(&Row[Where.left], &Row[Where.right + 1], &Dest[i - Where.top][0]);
	}
}

/* Изменить значение цветовых атрибутов в соответствии с маской
   (применяется для "создания" тени)
*/
void ScreenBuf::ApplyShadow(rectangle Where)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(os::critical_section_lock)(CS);

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
void ScreenBuf::ApplyColor(rectangle Where, const FarColor& Color, bool PreserveExFlags)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(os::critical_section_lock)(CS);

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

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)
	if (Global->DirectRT)
		Flush();
#endif
}

/* Непосредственное изменение цветовых атрибутов с заданным цветом исключением
*/
// used in stream selection
void ScreenBuf::ApplyColor(rectangle Where, const FarColor& Color, const FarColor& ExceptColor, bool ForceExFlags)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(os::critical_section_lock)(CS);

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

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)
	if (Global->DirectRT)
		Flush();
#endif
}

/* Закрасить прямоугольник символом Ch и цветом Color
*/
void ScreenBuf::FillRect(rectangle Where, const FAR_CHAR_INFO& Info)
{
	if (!is_visible(Where))
		return;

	SCOPED_ACTION(os::critical_section_lock)(CS);

	fix_coordinates(Where);

	for_submatrix(Buf, Where, [&Info](FAR_CHAR_INFO& Element)
	{
		Element = Info;
	});

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
void ScreenBuf::Flush(flush_type FlushType)
{
	SCOPED_ACTION(os::critical_section_lock)(CS);

	if (FlushType & flush_type::title && !SBFlags.Check(SBFLAGS_FLUSHEDTITLE))
	{
		console.SetTitle(m_Title);
		SBFlags.Set(SBFLAGS_FLUSHEDTITLE);
	}

	if (LockCount)
		return;

	if (!console.IsViewportVisible())
		return;

	if (FlushType & flush_type::cursor && !SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && !CurVisible)
	{
		console.SetCursorInfo({ CurSize,CurVisible });
		SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
	}

	if (FlushType & flush_type::screen)
	{
		if (!Global->SuppressIndicators)
		{
			const auto& SetMacroChar = [this](FAR_CHAR_INFO& Where, wchar_t Char, WORD Color)
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
				ShowTimeInBackground();
			}

			std::vector<SMALL_RECT>WriteList;
			bool Changes=false;

			if (m_ClearTypeFix == BSTATE_CHECKED)
			{
				//Для полного избавления от артефактов ClearType будем перерисовывать на всю ширину.
				//Чревато тормозами/миганием в зависимости от конфигурации системы.
				SMALL_RECT WriteRegion={0, 0, static_cast<SHORT>(Buf.width() - 1), 0};

				for (size_t I = 0, Height = Buf.height(); I < Height; ++I)
				{
					auto BufRow = Buf[I], ShadowRow = Shadow[I];

					WriteRegion.Top = static_cast<SHORT>(I);
					WriteRegion.Bottom = static_cast<SHORT>(I - 1);

					while (I < Height && BufRow != ShadowRow)
					{
						I++;
						BufRow = Buf[I];
						ShadowRow = Shadow[I];
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
				SMALL_RECT WriteRegion = { static_cast<SHORT>(Buf.width() - 1), static_cast<SHORT>(Buf.height() - 1), 0, 0 };

				auto PtrBuf = Buf.data(), PtrShadow = Shadow.data();
				for (size_t I = 0, Height = Buf.height(); I < Height; ++I)
				{
					for (size_t J = 0, Width = Buf.width(); J < Width; ++J, ++PtrBuf, ++PtrShadow)
					{
						if (*PtrBuf != *PtrShadow)
						{
							WriteRegion.Left = std::min(WriteRegion.Left, static_cast<SHORT>(J));
							WriteRegion.Top = std::min(WriteRegion.Top, static_cast<SHORT>(I));
							WriteRegion.Right = std::max(WriteRegion.Right, static_cast<SHORT>(J));
							WriteRegion.Bottom = std::max(WriteRegion.Bottom, static_cast<SHORT>(I));
							Changes=true;
							Started=true;
						}
						else if (Started && static_cast<SHORT>(I) > WriteRegion.Bottom && static_cast<SHORT>(J) >= WriteRegion.Left)
						{
							if (m_ClearTypeFix == BSTATE_3STATE)
							{
								//BUGBUG: при включенном СlearType-сглаживании на экране остаётся "мусор" - тонкие вертикальные полосы
								// кстати, и при выключенном тоже (но реже).
								// баг, конечно, не наш, но что делать.
								// расширяем область прорисовки влево-вправо на 1 символ:
								WriteRegion.Left=std::max(static_cast<SHORT>(0),static_cast<SHORT>(WriteRegion.Left-1));
								WriteRegion.Right = std::min(static_cast<SHORT>(WriteRegion.Right + 1), static_cast<SHORT>(Buf.width() - 1));
							}
							
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

							WriteRegion.Left = static_cast<SHORT>(Buf.width() - 1);
							WriteRegion.Top = static_cast<SHORT>(Buf.height() - 1);
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
				std::for_each(CONST_RANGE(WriteList, i)
				{
					console.WriteOutput(Buf, { i.Left, i.Top }, i);
				});
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

	if (FlushType & flush_type::cursor && !SBFlags.Check(SBFLAGS_FLUSHEDCURPOS))
	{
		if (is_visible({ m_CurPos.x, m_CurPos.y, m_CurPos.x, m_CurPos.x }))
		{
			console.SetCursorPosition({ static_cast<SHORT>(m_CurPos.x), static_cast<SHORT>(m_CurPos.y) });
		}
		SBFlags.Set(SBFLAGS_FLUSHEDCURPOS);
	}

	if (FlushType & flush_type::cursor && !SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE))
	{
		console.SetCursorInfo({ CurSize, CurVisible && is_visible({ m_CurPos.x, m_CurPos.y, m_CurPos.x, m_CurPos.x }) });
		SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
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
	SCOPED_ACTION(os::critical_section_lock)(CS);

	if (!is_visible({ m_CurPos.x, m_CurPos.y, m_CurPos.x, m_CurPos.x }))
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

void ScreenBuf::SetTitle(const string& Title)
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

//  проскроллировать буфер на одну строку вверх.
void ScreenBuf::Scroll(size_t Count)
{
	SCOPED_ACTION(os::critical_section_lock)(CS);

	const FAR_CHAR_INFO Fill{ L' ', colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN) };

	if (Global->Opt->WindowMode)
	{
		SMALL_RECT Region = { 0, 0, ScrX, static_cast<SHORT>(Count - 1) };

		// TODO: matrix_view to avoid copying
		matrix<FAR_CHAR_INFO> BufferBlock(Count, ScrX + 1);
		Read({ Region.Left, Region.Top, Region.Right, Region.Bottom }, BufferBlock);

		console.ScrollNonClientArea(Count, Fill);

		Region.Top = static_cast<SHORT>(-static_cast<SHORT>(Count));
		Region.Bottom = -1;
		console.WriteOutput(BufferBlock, Region);
	}

	if (Count && Count < Buf.height())
	{
		auto& RawBuf = Buf.vector();
		size_t size = RawBuf.size();
		RawBuf.erase(RawBuf.begin(), RawBuf.begin() + Count * Buf.width());
		RawBuf.resize(size, Fill);

		SBFlags.Clear(SBFLAGS_FLUSHED);
	}

#ifdef DIRECT_SCREEN_OUT
	Flush();
#elif defined(DIRECT_RT)

	if (Global->DirectRT)
		Flush();

#endif
}

void ScreenBuf::SetClearTypeFix(int const ClearTypeFix)
{
	m_ClearTypeFix = ClearTypeFix;
}

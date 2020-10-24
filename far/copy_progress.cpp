/*
copy_progress.cpp
*/
/*
Copyright © 2016 Far Group
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
#include "copy_progress.hpp"

// Internal:
#include "colormix.hpp"
#include "lang.hpp"
#include "config.hpp"
#include "keyboard.hpp"
#include "constitle.hpp"
#include "mix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "scrbuf.hpp"
#include "global.hpp"

// Platform:

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/* Общее время ожидания пользователя */
extern std::chrono::steady_clock::duration WaitUserTime;

copy_progress::copy_progress(bool Move, bool Total, bool Time):
	m_CurrentBarSize(CanvasWidth()),
	m_CurrentBar(make_progressbar(m_CurrentBarSize, 0, false, false)),
	m_TotalBarSize(CanvasWidth()),
	m_TotalBar(make_progressbar(m_TotalBarSize, 0, false, false)),
	m_Move(Move),
	m_Total(Total),
	m_ShowTime(Time),
	m_Color(colors::PaletteColorToFarColor(COL_DIALOGTEXT)),
	m_TimeCheck(time_check::mode::immediate, GetRedrawTimeout()),
	m_SpeedUpdateCheck(time_check::mode::immediate, 3s),
	m_SecurityTimeCheck(time_check::mode::immediate, GetRedrawTimeout())
{
}

size_t copy_progress::CanvasWidth()
{
	return 52;
}

void copy_progress::UpdateAllBytesInfo(unsigned long long FileSize)
{
	m_Bytes.Copied += m_Bytes.CurrCopied;
	if (m_Bytes.CurrCopied < FileSize)
	{
		m_Bytes.Skipped += FileSize - m_Bytes.CurrCopied;
	}
	Flush();
}

void copy_progress::UpdateCurrentBytesInfo(unsigned long long NewValue)
{
	m_Bytes.Copied -= m_Bytes.CurrCopied;
	m_Bytes.CurrCopied = NewValue;
	m_Bytes.Copied += m_Bytes.CurrCopied;
	Flush();
}

bool copy_progress::CheckEsc()
{
	if (!m_IsCancelled)
	{
		if (CheckForEscSilent())
		{
			m_IsCancelled = ConfirmAbortOp() != 0;
		}
	}
	return m_IsCancelled;
}

string copy_progress::FormatCounter(lng CounterId, lng AnotherId, unsigned long long CurrentValue, unsigned long long TotalValue, bool ShowTotal, size_t MaxWidth)
{
	string Label = msg(CounterId);
	const auto PaddedLabelSize = std::max(Label.size(), msg(AnotherId).size()) + 1;
	Label.resize(PaddedLabelSize, L' ');

	const auto StrCurrent = GroupDigits(CurrentValue);
	const auto StrTotal = ShowTotal? GroupDigits(TotalValue) : string();

	auto Value = ShowTotal? concat(StrCurrent, L" / "sv, StrTotal) : StrCurrent;
	if (MaxWidth > PaddedLabelSize)
	{
		const auto PaddedValueSize = MaxWidth - PaddedLabelSize;
		if (PaddedValueSize > Value.size())
		{
			Value.insert(0, PaddedValueSize - Value.size(), L' ');
		}
	}
	return Label + Value;
}

void copy_progress::Flush()
{
	if (!m_TimeCheck || CheckEsc())
		return;

	CreateBackground();

	Text({ m_Rect.left + 5, m_Rect.top + 3 }, m_Color, m_Src);
	Text({ m_Rect.left + 5, m_Rect.top + 5 }, m_Color, m_Dst);
	Text({ m_Rect.left + 5, m_Rect.top + 8 }, m_Color, m_FilesCopied);

	const auto Result = FormatCounter(lng::MCopyBytesTotalInfo, lng::MCopyFilesTotalInfo, GetBytesDone(), m_Bytes.Total, m_Total, CanvasWidth() - 5);
	Text({ m_Rect.left + 5, m_Rect.top + 9 }, m_Color, Result);

	Text({ m_Rect.left + 5, m_Rect.top + 6 }, m_Color, m_CurrentBar);

	if (m_Total)
	{
		Text({ m_Rect.left + 5, m_Rect.top + 10 }, m_Color, m_TotalBar);
	}

	if (!m_Time.empty())
	{
		const size_t Width = m_Rect.width() - 10;
		const auto XPos = m_Rect.left + 5;
		const auto YPos = m_Rect.top + (m_Total? 12 : 11);

		const auto ConsumedSpace = m_Time.size() + 1 + m_TimeLeft.size() + 1 + m_Speed.size();
		const auto FillerWidth = ConsumedSpace >= Width? 0 : (Width - ConsumedSpace) / 2;

		Text({ XPos, YPos }, m_Color, m_Time);
		Text({ static_cast<int>(XPos + m_Time.size() + 1 + FillerWidth), YPos }, m_Color, m_TimeLeft);
		Text({ static_cast<int>(m_Rect.right + 1 - 5 - m_Speed.size()), YPos }, m_Color, m_Speed);
	}

	if (m_Total || (m_Files.Total == 1))
	{
		ConsoleTitle::SetFarTitle(concat(
			L'{', str(m_Total ? ToPercent(GetBytesDone(), m_Bytes.Total) : m_CurrentPercent), L"%} "sv,
			msg(m_Move? lng::MCopyMovingTitle : lng::MCopyCopyingTitle))
		);
	}

	Global->ScrBuf->Flush();
}

void copy_progress::SetProgressValue(unsigned long long CompletedSize, unsigned long long TotalSize)
{
	SetCurrentProgress(CompletedSize, TotalSize);

	const auto BytesDone = GetBytesDone();

	if (m_Total)
	{
		SetTotalProgress(BytesDone, m_Bytes.Total);
	}

	if (m_ShowTime)
	{
		const auto SizeToGo = (m_Bytes.Total > BytesDone) ? (m_Bytes.Total - BytesDone) : 0;
		UpdateTime(BytesDone, SizeToGo);
	}

	Flush();
}

void copy_progress::CreateBackground()
{
	const auto& Title = msg(m_Move? lng::MMoveDlgTitle : lng::MCopyDlgTitle);

	std::vector<string> Items =
	{
		msg(m_Move? lng::MCopyMoving :lng::MCopyCopying),
		{}, // source name
		msg(lng::MCopyTo),
		{}, // dest path
		m_CurrentBar,
		L'\x1' + msg(lng::MCopyDlgTotal),
		{}, // files [total] <processed>
		{}  // bytes [total] <processed>
	};

	// total progress bar
	if (m_Total)
	{
		Items.emplace_back(m_TotalBar);
	}

	// time & speed
	if (m_ShowTime)
	{
		Items.emplace_back(L"\x1"sv);
		Items.emplace_back();
	}

	m_Rect = Message(MSG_LEFTALIGN | MSG_NOFLUSH,
		Title,
		std::move(Items),
		{}).GetPosition();
}

void copy_progress::SetNames(const string& Src, const string& Dst)
{
	if (m_ShowTime && !m_Files.Copied)
	{
		m_CopyStartTime = std::chrono::steady_clock::now();
		WaitUserTime = 0s;
		m_CalcTime = 0s;
	}

	const auto NameWidth = static_cast<int>(CanvasWidth());
	m_Src = truncate_path(Src, NameWidth);
	m_Dst = truncate_path(Dst, NameWidth);
	m_FilesCopied = FormatCounter(lng::MCopyFilesTotalInfo, lng::MCopyBytesTotalInfo, m_Files.Copied, m_Files.Total, m_Total, CanvasWidth() - 5);

	Flush();
}

void copy_progress::SetCurrentProgress(unsigned long long CompletedSize, unsigned long long TotalSize)
{
	m_CurrentPercent = ToPercent(std::min(CompletedSize, TotalSize), TotalSize);
	m_CurrentBar = make_progressbar(m_CurrentBarSize, m_CurrentPercent, true, !m_Total);
}

void copy_progress::SetTotalProgress(unsigned long long CompletedSize, unsigned long long TotalSize)
{
	m_TotalPercent = ToPercent(std::min(CompletedSize, TotalSize), TotalSize);
	m_TotalBar = make_progressbar(m_TotalBarSize, m_TotalPercent, true, true);
}

void copy_progress::UpdateTime(unsigned long long SizeDone, unsigned long long SizeToGo)
{
	m_CalcTime = std::chrono::steady_clock::now() - m_CopyStartTime - WaitUserTime;

	if (const auto CalcTime = m_CalcTime / 1s * 1s; CalcTime != 0s)
	{
		SizeDone -= m_Bytes.Skipped;

		m_Time = concat(msg(lng::MCopyTimeInfoElapsed), L' ', ConvertDurationToHMS(CalcTime));

		if (m_SpeedUpdateCheck)
		{
			if (SizeToGo)
			{
				// double to avoid potential overflows with large files
				m_TimeLeft = concat(msg(lng::MCopyTimeInfoRemaining), L' ', ConvertDurationToHMS(std::chrono::duration_cast<std::chrono::seconds>(CalcTime * 1.0 / SizeDone * SizeToGo)));
			}

			m_Speed = concat(trim(FileSizeToStr(SizeDone / (CalcTime / 1s), 8, COLFLAGS_FLOATSIZE | COLFLAGS_GROUPDIGITS)), msg(lng::MCopyTimeInfoSpeed));
		}
	}
	else
	{
		m_Time.clear();
		m_TimeLeft.clear();
		m_Speed.clear();
	}
}

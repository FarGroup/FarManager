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

#include "copy_progress.hpp"

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

#include "format.hpp"

/* Общее время ожидания пользователя */
extern std::chrono::steady_clock::duration WaitUserTime;

copy_progress::copy_progress(bool Move, bool Total, bool Time):
	m_Rect(),
	m_CurrentBarSize(CanvasWidth()),
	m_CurrentPercent(0),
	m_TotalBarSize(CanvasWidth()),
	m_TotalPercent(0),
	m_Move(Move),
	m_Total(Total),
	m_ShowTime(Time),
	m_IsCancelled(false),
	m_Color(colors::PaletteColorToFarColor(COL_DIALOGTEXT)),
	m_TimeCheck(time_check::mode::immediate, GetRedrawTimeout()),
	m_SpeedUpdateCheck(time_check::mode::immediate, 3s),
	m_SecurityTimeCheck(time_check::mode::immediate, GetRedrawTimeout()),
	m_Files(),
	m_Bytes()
{
	m_CurrentBar = make_progressbar(m_CurrentBarSize, 0, false, false);
	m_TotalBar = make_progressbar(m_TotalBarSize, 0, false, false);
}

size_t copy_progress::CanvasWidth()
{
	return 52;
}

static string GetTimeText(std::chrono::seconds Seconds)
{
	string Days, Time;
	ConvertDuration(Seconds, Days, Time);
	if (Days != L"0")
	{
		// BUGBUG copy time > 4.166 days (100 hrs) will not be displayed correctly
		const auto Hours = str(std::min(std::chrono::duration_cast<std::chrono::hours>(Seconds), 99h).count());
		Time[0] = Hours[0];
		Time[1] = Hours[1];
	}

	// drop msec
	return Time.substr(0, Time.size() - 4);
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

	auto Value = ShowTotal? concat(StrCurrent, L" / "_sv, StrTotal) : StrCurrent;
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

	Text(m_Rect.Left + 5, m_Rect.Top + 3, m_Color, m_Src);
	Text(m_Rect.Left + 5, m_Rect.Top + 5, m_Color, m_Dst);
	Text(m_Rect.Left + 5, m_Rect.Top + 8, m_Color, m_FilesCopied);

	const auto Result = FormatCounter(lng::MCopyBytesTotalInfo, lng::MCopyFilesTotalInfo, GetBytesDone(), m_Bytes.Total, m_Total, CanvasWidth() - 5);
	Text(m_Rect.Left + 5, m_Rect.Top + 9, m_Color, Result);

	Text(m_Rect.Left + 5, m_Rect.Top + 6, m_Color, m_CurrentBar);

	if (m_Total)
	{
		Text(m_Rect.Left + 5, m_Rect.Top + 10, m_Color, m_TotalBar);
	}

	Text(m_Rect.Left + 5, m_Rect.Top + (m_Total ? 12 : 11), m_Color, m_Time);

	if (m_Total || (m_Files.Total == 1))
	{
		ConsoleTitle::SetFarTitle(concat(
			L'{', str(m_Total ? ToPercent(GetBytesDone(), m_Bytes.Total) : m_CurrentPercent), L"%} "_sv,
			msg(m_Move? lng::MCopyMovingTitle : lng::MCopyCopyingTitle))
		);
	}

	Global->ScrBuf->Flush();
}

void copy_progress::SetProgressValue(unsigned long long CompletedSize, unsigned long long TotalSize)
{
	SetCurrentProgress(CompletedSize, TotalSize);

	auto BytesDone = GetBytesDone();

	if (m_Total)
	{
		SetTotalProgress(BytesDone, m_Bytes.Total);
	}

	if (m_ShowTime)
	{
		auto SizeToGo = (m_Bytes.Total > BytesDone) ? (m_Bytes.Total - BytesDone) : 0;
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
		L"", // source name
		msg(lng::MCopyTo),
		L"", // dest path
		m_CurrentBar,
		string(L"\x1") + msg(lng::MCopyDlgTotal),
		L"", // files [total] <processed>
		L""  // bytes [total] <processed>
	};

	// total progress bar
	if (m_Total)
	{
		Items.emplace_back(m_TotalBar);
	}

	// time & speed
	if (m_ShowTime)
	{
		Items.emplace_back(L"\x1");
		Items.emplace_back(L"");
	}

	Message m(MSG_LEFTALIGN | MSG_NOFLUSH,
		Title,
		std::move(Items),
		{});

	int MX1, MY1, MX2, MY2;
	m.GetMessagePosition(MX1, MY1, MX2, MY2);
	m_Rect.Left = MX1;
	m_Rect.Right = MX2;
	m_Rect.Top = MY1;
	m_Rect.Bottom = MY2;
}

void copy_progress::SetNames(const string& Src, const string& Dst)
{
	if (m_ShowTime)
	{
		if (!m_Files.Copied)
		{
			m_CopyStartTime = std::chrono::steady_clock::now();
			WaitUserTime = 0s;
			m_CalcTime = 0s;
		}
	}

	const auto NameWidth = static_cast<int>(CanvasWidth());
	m_Src = Src;
	TruncPathStr(m_Src, NameWidth);
	m_Dst = Dst;
	TruncPathStr(m_Dst, NameWidth);
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

	string tmp[3];
	const auto CalcTime = std::chrono::duration_cast<std::chrono::seconds>(m_CalcTime);
	if (CalcTime != CalcTime.zero())
	{
		SizeDone -= m_Bytes.Skipped;

		const auto CPS = SizeDone / CalcTime.count();

		const auto strCalcTimeStr = GetTimeText(CalcTime);

		if (m_SpeedUpdateCheck)
		{
			if (SizeToGo)
			{
				m_TimeLeft = GetTimeText(std::chrono::seconds(CPS? SizeToGo / CPS : 0));
			}

			m_Speed = FileSizeToStr(CPS, 8, COLUMN_FLOATSIZE | COLUMN_COMMAS);
			if (!m_Speed.empty() && m_Speed.front() == L' ' && std::iswdigit(m_Speed.back()))
			{
				m_Speed.erase(0, 1);
				m_Speed += L' ';
			}
		}

		tmp[0] = strCalcTimeStr;
		tmp[1] = m_TimeLeft;
		tmp[2] = m_Speed;
	}
	else
	{
		tmp[0] = tmp[1] = tmp[2] = string(8, L' ');
	}

	m_Time = format(msg(lng::MCopyTimeInfo), tmp[0], tmp[1], tmp[2]);
}

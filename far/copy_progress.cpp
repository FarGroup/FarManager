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
#include "lang.hpp"
#include "config.hpp"
#include "keyboard.hpp"
#include "mix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "dialog.hpp"
#include "uuids.far.dialogs.hpp"

// Platform:

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/* Общее время ожидания пользователя */
extern std::chrono::steady_clock::duration WaitUserTime;

namespace
{
	enum
	{
		DlgW = 76,
		DlgH = 15,
	};

	enum progress_items
	{
		pr_console_title,
		pr_doublebox,
		pr_src_label,
		pr_src_name,
		pr_dst_label,
		pr_dst_name,
		pr_current_progress,
		pr_separator1,
		pr_total_files,
		pr_total_bytes,
		pr_total_progress,
		pr_separator2,
		pr_stats,

		pr_count
	};
}

copy_progress::copy_progress(bool Move, bool Total, bool Time):
	m_Move(Move),
	m_Total(Total),
	m_ShowTime(Time),
	m_TimeCheck(time_check::mode::immediate, GetRedrawTimeout()),
	m_SpeedUpdateCheck(time_check::mode::immediate, 3s)
{
	auto ProgressDlgItems = MakeDialogItems<progress_items::pr_count>(
	{
		{ DI_TEXT,      {{ 0, 0 }, { 0,               0 }}, DIF_HIDDEN,    {}, },
		{ DI_DOUBLEBOX, {{ 3, 1 }, { DlgW - 4, DlgH - 2 }}, DIF_NONE,      msg(m_Move? lng::MMoveDlgTitle : lng::MCopyDlgTitle), },
		{ DI_TEXT,      {{ 5, 2 }, { DlgW - 6,        2 }}, DIF_NONE,      msg(m_Move? lng::MCopyMoving :lng::MCopyCopying), },
		{ DI_TEXT,      {{ 5, 3 }, { DlgW - 6,        3 }}, DIF_SHOWAMPERSAND, {}, },
		{ DI_TEXT,      {{ 5, 4 }, { DlgW - 6,        4 }}, DIF_NONE,      msg(lng::MCopyTo), },
		{ DI_TEXT,      {{ 5, 5 }, { DlgW - 6,        5 }}, DIF_SHOWAMPERSAND, {}, },
		{ DI_TEXT,      {{ 5, 6 }, { DlgW - 6,        6 }}, DIF_NONE,      make_progressbar(CanvasWidth(), 0, true, false) },
		{ DI_TEXT,      {{-1, 7 }, { DlgW - 6,        7 }}, DIF_SEPARATOR, msg(lng::MCopyDlgTotal), },
		{ DI_TEXT,      {{ 5, 8 }, { DlgW - 6,        8 }}, DIF_NONE,      {}, },
		{ DI_TEXT,      {{ 5, 9 }, { DlgW - 6,        9 }}, DIF_NONE,      {}, },
		{ DI_TEXT,      {{ 5, 10}, { DlgW - 6,        10}}, DIF_NONE,      make_progressbar(CanvasWidth(), 0, true, false), },
		{ DI_TEXT,      {{-1, 11}, { DlgW - 6,        11}}, DIF_SEPARATOR, {}, },
		{ DI_TEXT,      {{ 5, 12}, { DlgW - 6,        12}}, DIF_NONE,      {}, },
	});

	if (!m_Total)
	{
		ProgressDlgItems[progress_items::pr_total_progress].Flags |= DIF_HIDDEN;
		ProgressDlgItems[progress_items::pr_console_title].strData = ProgressDlgItems[progress_items::pr_doublebox].strData;

		for (const auto i: std::views::iota(progress_items::pr_total_progress + 1, progress_items::pr_count))
		{
			--ProgressDlgItems[i].Y1;
			--ProgressDlgItems[i].Y2;
		}

		--ProgressDlgItems[progress_items::pr_doublebox].Y2;
	}

	if (!m_ShowTime)
	{
		ProgressDlgItems[progress_items::pr_separator2].Flags |= DIF_HIDDEN;
		ProgressDlgItems[progress_items::pr_stats].Flags |= DIF_HIDDEN;
		ProgressDlgItems[progress_items::pr_doublebox].Y2 -= 2;
	}

	const int DialogHeight = ProgressDlgItems[progress_items::pr_doublebox].Y2 - ProgressDlgItems[progress_items::pr_doublebox].Y1 + 1 + 2;
	init(ProgressDlgItems, { -1, -1, DlgW, DialogHeight }, &CopyProgressId);
}

size_t copy_progress::CanvasWidth()
{
	return DlgW - 10;
}

void copy_progress::skip(unsigned long long const Size)
{
	m_BytesTotal.Copied -= m_BytesCurrent.Copied;
	m_BytesTotal.Total -= m_BytesCurrent.Total? m_BytesCurrent.Total : Size;

	m_BytesCurrent = {};

	--m_Files.Total;

	Flush();
}

void copy_progress::next()
{
	++m_Files.Copied;

	m_BytesCurrent = {};

	Flush();
}

void copy_progress::undo()
{
	m_BytesTotal.Copied -= m_BytesCurrent.Copied;

	m_BytesCurrent.Copied = 0;

	Flush();
}

unsigned long long copy_progress::get_total_bytes() const
{
	return m_BytesTotal.Total;
}

bool copy_progress::CheckEsc()
{
	if (m_IsCancelled)
		return m_IsCancelled;

	m_IsCancelled = CheckForEscAndConfirmAbort();
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

	SCOPED_ACTION(Dialog::suppress_redraw)(m_Dialog.get());

	m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_src_name, UNSAFE_CSTR(m_Src));
	m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_dst_name, UNSAFE_CSTR(m_Dst));

	const auto CurrentProgress = make_progressbar(GetWidth(progress_items::pr_current_progress), m_CurrentPercent, true, !m_Total);
	m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_current_progress, UNSAFE_CSTR(CurrentProgress));

	if (m_FilesLastRendered != m_Files)
	{
		m_FilesCopied = FormatCounter(lng::MCopyFilesTotalInfo, lng::MCopyBytesTotalInfo, m_Files.Copied, m_Files.Total, m_Total, GetWidth(progress_items::pr_total_files) - 5);
		m_FilesLastRendered = m_Files;
	}

	m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_total_files, UNSAFE_CSTR(m_FilesCopied));

	const auto Result = FormatCounter(lng::MCopyBytesTotalInfo, lng::MCopyFilesTotalInfo, m_BytesTotal.Copied, m_BytesTotal.Total, m_Total, GetWidth(progress_items::pr_total_bytes) - 5);
	m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_total_bytes, UNSAFE_CSTR(Result));

	if (m_Total)
	{
		const auto TotalProgress = make_progressbar(GetWidth(progress_items::pr_total_progress), m_TotalPercent, true, true);
		m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_total_progress, UNSAFE_CSTR(TotalProgress));
	}

	if (!m_Time.empty())
	{
		const auto Width = GetWidth(progress_items::pr_stats);
		const auto ConsumedSpace = m_Time.size() + 1 + m_TimeLeft.size() + 1 + m_Speed.size();
		const auto RemainingSpace = ConsumedSpace > Width? 0 : Width - ConsumedSpace;
		const auto FirstFillerWidth = RemainingSpace / 2;
		const auto SecondFillerWidth = RemainingSpace - FirstFillerWidth;

		const auto Stat = cut_right(
			concat(
				fit_to_left(m_Time, m_Time.size() + 1 + FirstFillerWidth),
				fit_to_left(m_TimeLeft, m_TimeLeft.size() + 1 + SecondFillerWidth),
				m_Speed),
			Width
		);

		m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_stats, UNSAFE_CSTR(Stat));
	}

	if (m_Total || (m_Files.Total == 1))
	{
		m_Dialog->SendMessage(DM_SETTEXTPTR, progress_items::pr_console_title, UNSAFE_CSTR(concat(
			L'{', str(m_Total? ToPercent(m_BytesTotal.Copied, m_BytesTotal.Total) : m_CurrentPercent), L"%} "sv,
			msg(m_Move? lng::MCopyMovingTitle : lng::MCopyCopyingTitle))
		));
	}
}

void copy_progress::reset_current()
{
	m_BytesCurrent = {};
}

void copy_progress::set_current_total(unsigned long long const Value)
{
	m_BytesCurrent.Copied = 0;
	m_BytesCurrent.Total = Value;

	Flush();
}

void copy_progress::set_current_copied(unsigned long long const Value)
{
	const auto Increment = Value - m_BytesCurrent.Copied;
	m_BytesCurrent.Copied = Value;
	m_BytesTotal.Copied += Increment;

	SetCurrentProgress(m_BytesCurrent.Copied, m_BytesCurrent.Total);

	if (m_Total)
	{
		SetTotalProgress(m_BytesTotal.Copied, m_BytesTotal.Total);
	}

	if (m_ShowTime)
	{
		const auto SizeToGo = m_BytesTotal.Total > m_BytesTotal.Copied? m_BytesTotal.Total - m_BytesTotal.Copied : 0;
		UpdateTime(m_BytesTotal.Copied, SizeToGo);
	}

	Flush();
}

void copy_progress::set_total_files(unsigned long long const Value)
{
	m_Files.Total = Value;
}

void copy_progress::set_total_bytes(unsigned long long const Value)
{
	m_BytesTotal.Copied = 0;
	m_BytesTotal.Total = Value;
}

void copy_progress::add_total_bytes(unsigned long long const Value)
{
	m_BytesTotal.Total += Value;
}

void copy_progress::SetNames(const string_view Src, const string_view Dst)
{
	if (m_ShowTime && !m_Files.Copied)
	{
		m_CopyStartTime = std::chrono::steady_clock::now();
		WaitUserTime = 0s;
		m_CalcTime = 0s;
	}

	m_Src = truncate_path(Src, static_cast<int>(GetWidth(progress_items::pr_src_name)));
	m_Dst = truncate_path(Dst, static_cast<int>(GetWidth(progress_items::pr_dst_name)));

	set_current_total(0);
	set_current_copied(0);

	Flush();
}

void copy_progress::SetCurrentProgress(unsigned long long CompletedSize, unsigned long long TotalSize)
{
	m_CurrentPercent = ToPercent(std::min(CompletedSize, TotalSize), TotalSize);
}

void copy_progress::SetTotalProgress(unsigned long long CompletedSize, unsigned long long TotalSize)
{
	m_TotalPercent = ToPercent(std::min(CompletedSize, TotalSize), TotalSize);
}

void copy_progress::UpdateTime(unsigned long long SizeDone, unsigned long long SizeToGo)
{
	m_CalcTime = std::chrono::steady_clock::now() - m_CopyStartTime - WaitUserTime;

	if (const auto CalcTime = m_CalcTime / 1s * 1s; CalcTime != 0s)
	{
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

size_t copy_progress::GetWidth(intptr_t Index)
{
	FarDialogItem Item;
	if (!m_Dialog->SendMessage(DM_GETDLGITEMSHORT, Index, &Item))
		return CanvasWidth();
	return Item.X2 - Item.X1 + 1;
}

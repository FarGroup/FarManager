/*
viewer.cpp

Internal viewer
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
#include "viewer.hpp"

// Internal:
#include "encoding.hpp"
#include "macroopcode.hpp"
#include "keyboard.hpp"
#include "farcolor.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "fileview.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "taskbar.hpp"
#include "drivemix.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "delete.hpp"
#include "pathmix.hpp"
#include "filestr.hpp"
#include "mix.hpp"
#include "console.hpp"
#include "RegExp.hpp"
#include "colormix.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "plugins.hpp"
#include "manager.hpp"
#include "lang.hpp"
#include "datetime.hpp"
#include "keybar.hpp"
#include "stddlg.hpp"
#include "strmix.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "panel.hpp"
#include "global.hpp"
#include "uuids.far.dialogs.hpp"
#include "log.hpp"
#include "encoding.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/bytes_view.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace
{
	enum SHOW_MODES
	{
		SHOW_RELOAD,
		SHOW_HEX,
		SHOW_UP,
		SHOW_DOWN,
		SHOW_DUMP
	};

	enum saved_modes
	{
		m_none = 0,

		m_mode_changed = 0x10,

		m_mode_text = VMT_TEXT,
		m_mode_hex = VMT_HEX,
		m_mode_dump = VMT_DUMP,

		m_mode_last = m_mode_dump,
		m_mode_mask = 0xF,

		m_mode_wrap = 0x20,
		m_mode_wrap_words = 0x40,
	};

	bool is_code_page_supported_in_viewer(uintptr_t cp)
	{
		return IsCodePageSupported(cp, 2);
	}

	int get_char_size(uintptr_t const cp)
	{
		return IsUtf16CodePage(cp)? sizeof(char16_t) : 1;
	}

	// 0000000000: 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D  --------------------------
	// 0         1         2         3         4         5         6         7         8         9         0         1         2
	// 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012

	constexpr int s_HexModeBytesPerStripe{ 8 };

	constexpr int s_HexModeAddressAreaWidth{ 11 };
	constexpr int s_HexModeByteWidth{ 3 };
	constexpr int s_HexModeStripePadding{ 2 };
	constexpr int s_HexModeCharsAreaLeader{ 2 };

	constexpr int hex_mode_bytes_area_wdth(const int Bytes)
	{
		assert(Bytes > 0);
		return s_HexModeByteWidth * Bytes + s_HexModeStripePadding * ((Bytes - 1) / s_HexModeBytesPerStripe);
	}

	constexpr int hex_mode_chars_area_width(const int Bytes, const int CharSize)
	{
		assert(CharSize == 1 || CharSize == 2);
		return (Bytes + CharSize / 2) / CharSize;
	}

	constexpr int hex_mode_line_width_from_bytes(const int Bytes, const int CharSize)
	{
		assert(Bytes >= s_HexModeBytesPerStripe);
		return s_HexModeAddressAreaWidth + hex_mode_bytes_area_wdth(Bytes) + s_HexModeCharsAreaLeader + hex_mode_chars_area_width(Bytes, CharSize);
	}

	constexpr int hex_mode_columns_per_stripe(const int CharSize)
	{
		return s_HexModeBytesPerStripe * s_HexModeByteWidth + s_HexModeStripePadding + hex_mode_chars_area_width(s_HexModeBytesPerStripe, CharSize);
	}

	constexpr int hex_mode_bytes_from_line_width(const int LineWidth, const int CharSize)
	{
		if (LineWidth <= hex_mode_line_width_from_bytes(s_HexModeBytesPerStripe, CharSize))
			return s_HexModeBytesPerStripe;

		const auto BytesAndCharsWidth{ LineWidth - s_HexModeAddressAreaWidth };
		const auto ColumnsPerStripe{ hex_mode_columns_per_stripe(CharSize) };
		const auto FullStripes{ BytesAndCharsWidth / ColumnsPerStripe };
		const auto ColumnsInFullStripes{ ColumnsPerStripe * FullStripes };
		const auto RemainingColumns{ BytesAndCharsWidth - s_HexModeCharsAreaLeader - ColumnsInFullStripes };

		// If CharSize is 2, one byte accupies 3.5 columns. Double the nominator and denominator to stay in integral numbers.
		const auto RemainingBytes{ (2 * RemainingColumns) / (2 * s_HexModeByteWidth + 2 / CharSize)};

		return s_HexModeBytesPerStripe * FullStripes + RemainingBytes;
	}

	int hex_mode_byte_from_column(int Column, const int Bytes, const int CharSize)
	{
		if (Column < s_HexModeAddressAreaWidth) return 0;
		Column -= s_HexModeAddressAreaWidth;

		const auto BytesAreaWidth{ hex_mode_bytes_area_wdth(Bytes) };
		if (Column < BytesAreaWidth)
		{
			const auto ColumnsPerStripe{ s_HexModeBytesPerStripe * s_HexModeByteWidth + s_HexModeStripePadding };
			const auto FullStripes{ Column / ColumnsPerStripe };
			const auto ColumnsInFullStripes{ ColumnsPerStripe * FullStripes };
			const auto RemainingColumns{ Column - ColumnsInFullStripes };
			const auto RemainingBytes{ RemainingColumns / s_HexModeByteWidth };
			return s_HexModeBytesPerStripe * FullStripes + RemainingBytes;
		}
		Column -= BytesAreaWidth;

		if (Column < s_HexModeCharsAreaLeader) return 0;
		Column -= s_HexModeCharsAreaLeader;

		if (Column < hex_mode_chars_area_width(Bytes, CharSize))
		{
			return Column * CharSize;
		}

		return Bytes;
	}
}

enum class Viewer::SearchDisposition
{
	Cancel,
	Prev,
	Next,
	ContinueBackward,
	ContinueForward,
};

static int ViewerID=0;

// seems like this initialization list is toooooo long
Viewer::Viewer(window_ptr Owner, bool bQuickView, uintptr_t aCodePage):
	SimpleScreenObject(std::move(Owner)),
	ViOpt(Global->Opt->ViOpt),
	Reader(ViewFile, (Global->Opt->ViOpt.MaxLineSize*2*64 > 64*1024 ? Global->Opt->ViOpt.MaxLineSize*2*64 : 64*1024)),
	m_SearchDlgParams
	{
		.SearchStr = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).SearchStr,
		.SearchBytes = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).SearchBytes.value_or(bytes{}),
		.Hex = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).Hex.value_or(false),
		.CaseSensitive = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).CaseSensitive.value_or(false),
		.WholeWords = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).WholeWords.value_or(false),
		.Regex = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).Regex.value_or(false),
		.Fuzzy = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).Fuzzy.value_or(false),
	},
	m_DefCodepage(aCodePage),
	m_Codepage(m_DefCodepage),
	m_Wrap(Global->Opt->ViOpt.ViewerIsWrap),
	m_WordWrap(Global->Opt->ViOpt.ViewerWrap),
	m_DisplayMode(VMT_TEXT),
	ViewerID(::ViewerID++),
	m_bQuickView(bQuickView),
	vread_buffer(std::max(MaxViewLineBufferSize(), 8192uz)),
	lcache_lines(16*1000),
	// dirty magic numbers, fix them!
	max_backward_size(std::min(Options::ViewerOptions::eMaxLineSize*3ll, std::max(Global->Opt->ViOpt.MaxLineSize*2, 1024ll) * 32)),
	llengths(max_backward_size / 40),
	Search_buffer(3 * std::max(MaxViewLineBufferSize(), 8192uz)),
	ReadBuffer(MaxViewLineBufferSize())
{
	if (m_DefCodepage != CP_DEFAULT)
		MB.SetCP(m_DefCodepage);
}

Viewer::~Viewer()
{
	KeepInitParameters();

	if (ViewFile)
	{
		ViewFile.Close();
		SavePosition();
	}

	/* $ 11.10.2001 IS
	   Удаляем файл только, если нет открытых окон с таким именем.
	*/

	if (!strTempViewName.empty() && !Global->WindowManager->CountWindowsWithName(strTempViewName))
	{
		/* $ 14.06.2002 IS
		   Если DeleteFolder сброшен, то удаляем только файл. Иначе - удаляем еще
		   и каталог.
		*/
		if (m_DeleteFolder)
			DeleteFileWithFolder(strTempViewName);
		else
		{
			if (!os::fs::set_file_attributes(strTempViewName, FILE_ATTRIBUTE_NORMAL)) // BUGBUG
			{
				LOGWARNING(L"set_file_attributes({}): {}"sv, strTempViewName, os::last_error());
			}

			if (!os::fs::delete_file(strTempViewName)) //BUGBUG
			{
				LOGWARNING(L"delete_file({}): {}"sv, strTempViewName, os::last_error());
			}
		}
	}
}

wchar_t Viewer::ZeroChar() const
{
	return ViOpt.Visible0x00 && ViOpt.ZeroChar > 0 ? static_cast<wchar_t>(ViOpt.ZeroChar) : 0;
}

void Viewer::ChangeHexModeBytesPerLine(int const Amount)
{
	assert(m_DisplayMode == VMT_HEX);

	if (Amount == 0) return;

	const auto AbsAmount = std::abs(Amount);
	const auto MinBytesPerLine = std::max(s_HexModeBytesPerStripe, AbsAmount);
	const auto MaxBytesPerLine = hex_mode_bytes_from_line_width(ScrollbarAdjustedWidth, get_char_size(m_Codepage)) / AbsAmount * AbsAmount;

	if (MinBytesPerLine > MaxBytesPerLine) return;

	const auto GetNextGridPoint = [&](const auto BytesPerLine)
		{
			assert(BytesPerLine > 0);
			return (BytesPerLine / AbsAmount + 1) * AbsAmount;
		};

	const int RoundedBytesPerLine = [&]()
		{
			const auto NextGridPoint{ GetNextGridPoint(m_HexModeBytesPerLine) };

			if (Amount > 0) return NextGridPoint;

			// Do the math in the inverted space X' = NextGridPoint - X, then return to the original space.
			return NextGridPoint - GetNextGridPoint(NextGridPoint - m_HexModeBytesPerLine);
		}();

	const auto NewBytesPerLine = std::clamp(RoundedBytesPerLine, MinBytesPerLine, MaxBytesPerLine);

	if (NewBytesPerLine == m_HexModeBytesPerLine) return;

	m_HexModeBytesPerLine = NewBytesPerLine;
	Show();
}

void Viewer::AdjustHexModeBytesPerLineToViewWidth()
{
	if (m_HexModePrevScrollbarAdjustedWidth != ScrollbarAdjustedWidth)
	{
		m_HexModePrevScrollbarAdjustedWidth = ScrollbarAdjustedWidth;
		// TODO: Add an option
		if constexpr ((false))
		{
			m_HexModeBytesPerLine = std::min(m_HexModeBytesPerLine, hex_mode_bytes_from_line_width(ScrollbarAdjustedWidth, get_char_size(m_Codepage)));
		}
	}
}

void Viewer::HorizontalScroll(const int Shift)
{
	assert(m_DisplayMode == VMT_TEXT || m_DisplayMode == VMT_HEX);

	if (!ViewFile) return;

	const long long MaxLeftPos{ m_DisplayMode == VMT_TEXT
		? (!m_Wrap ? MaxViewLineSize() : LeftPos ) // If m_Wrap, do not scroll right
		: hex_mode_line_width_from_bytes(m_HexModeBytesPerLine, get_char_size(m_Codepage)) - ScrollbarAdjustedWidth
	};

	const auto NewLeftPos = std::max(std::min(LeftPos + Shift, MaxLeftPos), 0LL);

	if (NewLeftPos == LeftPos) return;

	LeftPos = NewLeftPos;
	Show();
}

void Viewer::RollContents(const long long OffsetInChars)
{
	assert(m_DisplayMode == VMT_HEX || m_DisplayMode == VMT_DUMP);

	if (!ViewFile) return;

	const auto CharSize = GetModeDependentCharSize();
	const auto AlignToChar{ [&](const auto Pos) { return Pos / CharSize * CharSize; } };

	const auto NewFilePos =
		AlignToChar(std::max(std::min(FilePos + OffsetInChars * CharSize, FileSize - 1), 0LL));

	if (NewFilePos == FilePos) return;

	FilePos = NewFilePos;
	Show();
}

struct Viewer::ViewerUndoData
{
	ViewerUndoData(long long UndoAddr, long long UndoLeft):
		UndoAddr(UndoAddr),
		UndoLeft(UndoLeft)
	{
	}
	long long UndoAddr;
	long long UndoLeft;
};

void Viewer::SavePosition()
{
	const auto& vo = Global->Opt->ViOpt;
	if (vo.SaveShortPos || vo.SavePos || vo.SaveCodepage || vo.SaveViewMode || vo.SaveWrapMode)
	{
		ViewerPosCache poscache;

		poscache.cur.FilePos = FilePos;
		poscache.cur.LeftPos = LeftPos;

		poscache.ViewModeAndWrapState = (m_DisplayMode.touched() || m_Wrap.touched() || m_WordWrap.touched())
			? m_mode_changed | static_cast<saved_modes>(m_DisplayMode.value()) | (m_Wrap ? m_mode_wrap : 0) | (m_WordWrap ? m_mode_wrap_words : 0)
			: m_none;

		poscache.CodePage = m_Codepage;
		poscache.bm = BMSavePos;

		FilePositionCache::AddPosition(strPluginData.empty()? strFullFileName : strPluginData+PointToName(strFileName), poscache);
	}
}

void Viewer::KeepInitParameters() const
{
	m_SearchDlgParams.SaveToShared(SearchReplaceDlgParams::SharedGroup::view_edit);
	Global->Opt->ViOpt.ViewerIsWrap = m_Wrap;
	Global->Opt->ViOpt.ViewerWrap = m_WordWrap;
}

bool Viewer::OpenFile(string_view const Name, bool const Warn)
{
	m_Codepage=m_DefCodepage;
	m_DefCodepage=CP_DEFAULT;
	OpenFailed=false;

	ViewFile.Close();
	Reader.Clear();
	lcache_ready = false;

	SelectSize = -1; // Сбросим выделение
	strFileName = Name;

	const auto& vo = Global->Opt->ViOpt;

	if (Global->OnlyEditorViewerUsed && strFileName == L"-"sv)
	{
		const auto strTempName = MakeTemp();
		if (!ViewFile.Open(strTempName, GENERIC_READ | GENERIC_WRITE, os::fs::file_share_all, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE))
		{
			OpenFailed = true;
			return false;
		}

		DWORD ReadSize = 0;
		while (ReadFile(console.GetOriginalInputHandle(), vread_buffer.data(), static_cast<DWORD>(vread_buffer.size()), &ReadSize, nullptr) && ReadSize)
		{
			// BUGBUG check result
			if (!ViewFile.Write(vread_buffer.data(), ReadSize))
			{
				LOGWARNING(L"Write({}): {}"sv, ViewFile.GetName(), os::last_error());
			}
		}
		ViewFile.SetPointer(0, nullptr, FILE_BEGIN);

		ReadStdin=TRUE;
	}
	else
	{
		while (!ViewFile.Open(strFileName, FILE_READ_DATA, os::fs::file_share_all, nullptr, OPEN_EXISTING))
		{
			/* $ 04.07.2000 tran
			   + 'warning' flag processing, in QuickView it is FALSE
				 so don't show red message box */
			if (Warn)
			{
				const auto ErrorState = os::last_error();

				if (OperationFailed(ErrorState, strFileName, lng::MViewerTitle, msg(lng::MViewerCannotOpenFile), false) == operation::retry)
					continue;
			}

			OpenFailed = true;
			return false;
		}
	}

	Reader.AdjustAlignment();

	strFullFileName = ConvertNameToFull(strFileName);
	// BUGBUG check result
	if (!os::fs::get_find_data(strFileName, ViewFindData))
	{
		LOGWARNING(L"get_find_data({}): {}"sv, strFileName, os::last_error());
	}

	uintptr_t CachedCodePage=0;

	if ((vo.SavePos || vo.SaveShortPos || vo.SaveCodepage || vo.SaveViewMode || vo.SaveWrapMode) && !ReadStdin)
	{
		const auto strCacheName = strPluginData.empty()? strFileName : strPluginData + PointToName(strFileName);
		ViewerPosCache poscache;
		if (FilePositionCache::GetPosition(strCacheName, poscache))
		{
			if (vo.SavePos)
			{
				LastSelectPos = FilePos = std::max(poscache.cur.FilePos, 0LL);
				LeftPos = poscache.cur.LeftPos;
			}
			if (vo.SaveCodepage || vo.SavePos)
			{
				CachedCodePage = poscache.CodePage;
				if (CachedCodePage && !is_code_page_supported_in_viewer(CachedCodePage))
					CachedCodePage = 0;
			}

			if (vo.SaveShortPos)
			{
				BMSavePos = poscache.bm;
			}

			if (!m_DisplayMode.touched()) // keep Mode if file listed (Gray+-)
			{
				if (vo.SaveViewMode && (poscache.ViewModeAndWrapState & m_mode_changed) != 0)
				{
					auto ViewMode = poscache.ViewModeAndWrapState & m_mode_mask;
					if (ViewMode <= m_mode_last)
					{
						m_DisplayMode = static_cast<VIEWER_MODE_TYPE>(ViewMode);
					}
				}
				if (m_DisplayMode != VMT_HEX)
					m_DumpTextMode = (m_DisplayMode == VMT_DUMP);

				if (vo.SaveWrapMode && (poscache.ViewModeAndWrapState & m_mode_changed) != 0)
				{
					m_Wrap = (poscache.ViewModeAndWrapState & m_mode_wrap) != 0;
					m_WordWrap = (poscache.ViewModeAndWrapState & m_mode_wrap_words) != 0;
				}
			}
		}
	}

	if (m_Codepage == CP_DEFAULT)
	{
		if (CachedCodePage)
		{
			m_Codepage = CachedCodePage;
		}
		else
		{
			const auto DefaultCodepage = GetDefaultCodePage();
			const auto DetectedCodepage = GetFileCodepage(ViewFile, DefaultCodepage, &Signature, vo.AutoDetectCodePage);
			m_Codepage = is_code_page_supported_in_viewer(DetectedCodepage)? DetectedCodepage : DefaultCodepage;
		}

		MB.SetCP(m_Codepage);
	}

	ViewFile.SetPointer(0, nullptr, FILE_BEGIN);

	SetFileSize();

	if (!m_DisplayMode.touched())
	{
		m_DumpTextMode = vo.DetectDumpMode && isBinaryFile(m_Codepage);
		m_DisplayMode = m_DumpTextMode ? VMT_DUMP : VMT_TEXT;
		m_DisplayMode.forget();
	}

	if (FilePos > FileSize)
		FilePos=0;
	if ( FilePos )
		AdjustFilePos();

	ChangeViewKeyBar();
	AdjustWidth();

	if (!HostFileViewer) ReadEvent();

	return true;
}

bool Viewer::isBinaryFile(uintptr_t cp) // very approximate: looks for '\0' in first 2k bytes
{
	alignas(wchar_t) char Buffer[2048];

	const auto CurrentPos = vtell();
	vseek(0, FILE_BEGIN);
	size_t BytesRead = 0;
	const auto Result = ViewFile.Read(Buffer, sizeof(Buffer), BytesRead);
	vseek(CurrentPos, FILE_BEGIN);

	if (!Result)
		return true;

	if (IsUtf16CodePage(cp))
	{
		return contains(std::span(std::bit_cast<const wchar_t*>(&Buffer), BytesRead / sizeof(wchar_t)), L'\0');
	}
	else
	{
		return contains(std::span(Buffer, BytesRead), '\0');
	}
}

void Viewer::AdjustWidth()
{
	ScrollbarAdjustedWidth = m_Where.width();
	ScrollbarAdjustedRight = m_Where.right;

	if (ViOpt.ShowScrollbar && !m_bQuickView)
	{
		ScrollbarAdjustedWidth--;
		ScrollbarAdjustedRight--;
	}
}

bool Viewer::CheckChanged()
{
	os::fs::find_data NewViewFindData;
	if (!os::fs::get_find_data(strFullFileName, NewViewFindData))
		return true;

	// Smart file change check -- thanks Dzirt2005
	//
	bool changed = ViewFindData.LastWriteTime != NewViewFindData.LastWriteTime || ViewFindData.FileSize != NewViewFindData.FileSize;
	if (changed)
	{
		ViewFindData = NewViewFindData;
	}
	else
	{
		if (!ViewFile.GetSize(NewViewFindData.FileSize) || FileSize == static_cast<long long>(NewViewFindData.FileSize))
			return true;

		changed = FileSize > static_cast<long long>(NewViewFindData.FileSize); // true if file shrank
	}

	SetFileSize();
	if (changed) // do not reset caches if file just enlarged [make sense on Win7, doesn't matter on XP]
	{
		Reader.Clear(); // иначе зачем вся эта возня?
		ViewFile.FlushBuffers();
		vseek(0, FILE_CURRENT); // reset vgetc state
		lcache_ready = false; // reset start-lines cache
	}

	return changed;
}

void Viewer::ShowPage(int nMode)
{
	redraw_selection = false;

	AdjustWidth();

	if (!ViewFile)
	{
		if (!strFileName.empty() && ((nMode == SHOW_RELOAD) || (nMode == SHOW_HEX)|| (nMode == SHOW_DUMP)))
		{
			SetScreen(m_Where, L' ', colors::PaletteColorToFarColor(COL_VIEWERTEXT));
			GotoXY(m_Where.left, m_Where.top);
			SetColor(COL_WARNDIALOGTEXT);
			Text(cut_right(msg(lng::MViewerCannotOpenFile), ScrollbarAdjustedWidth));
			ShowStatus();
		}

		return;
	}

	if (m_HideCursor)
		HideCursor();

	vseek(FilePos, FILE_BEGIN);
	LastPage = false;

	if ( SelectSize < 0 )
		SelectPos=FilePos;

	switch (nMode)
	{
		case SHOW_HEX:
			ShowHex();
			break;
		case SHOW_DUMP:
			ShowDump();
			break;
		case SHOW_RELOAD:
			{
				CheckChanged();

				Strings.clear();

				for (const auto Y: std::views::iota(m_Where.top, m_Where.bottom + 1))
				{
					ViewerString NewString;
					NewString.nFilePos = vtell();

					if (Y == m_Where.top + 1 && !veof())
						SecondPos=vtell();

					ReadString(&NewString, -1);
					Strings.emplace_back(NewString);
				}
			}
			break;

		case SHOW_UP:
			{
				ViewerString NewString;
				NewString.nFilePos = FilePos;

				SecondPos = Strings.front().nFilePos;

				Strings.emplace_front(NewString);
				Strings.pop_back();

				ReadString(&Strings.front(), static_cast<int>(SecondPos - FilePos));
			}
			break;

		case SHOW_DOWN:
			{
				ViewerString NewString;
				NewString.nFilePos = Strings.back().nFilePos + Strings.back().linesize;

				Strings.emplace_back(NewString);
				Strings.pop_front();

				FilePos = Strings.front().nFilePos;
				SecondPos = FilePos + Strings.front().linesize;

				vseek(Strings.back().nFilePos, FILE_BEGIN);
				ReadString(&Strings.back(), -1);
			}
			break;
	}

	if (nMode != SHOW_HEX && nMode != SHOW_DUMP)
	{
		int Y = m_Where.top - 1;
		for (auto& i: Strings)
		{
			++Y;
			SetColor(COL_VIEWERTEXT);
			GotoXY(m_Where.left, Y);

			if (static_cast<long long>(i.Data.size()) > LeftPos)
			{
				Text(fit_to_left(i.Data.substr(LeftPos), ScrollbarAdjustedWidth));
			}
			else
			{
				Text(string(ScrollbarAdjustedWidth, L' '));
			}

			if (SelectSize >= 0 && i.bSelection)
			{
				long long SelX1;

				if (LeftPos > i.nSelStart)
					SelX1 = m_Where.left;
				else
					SelX1 = i.nSelStart - LeftPos;

				if (!m_Wrap && (i.nSelEnd < LeftPos || i.nSelStart >= LeftPos + ScrollbarAdjustedWidth))
				{
					if (AdjustSelPosition)
					{
						LeftPos = std::max(i.nSelStart - 1, 0LL);
						AdjustSelPosition = false;
						Show();
						return;
					}
				}
				else
				{
					SetColor(COL_VIEWERSELECTEDTEXT);
					GotoXY(static_cast<int>(m_Where.left + SelX1), Y);
					long long Length = i.nSelEnd - i.nSelStart;

					if (LeftPos > i.nSelStart)
						Length = i.nSelEnd - LeftPos;

					if (LeftPos > i.nSelEnd)
						Length = 0;

					Text(cut_right(i.Data.substr(SelX1 + LeftPos), Length));
				}
			}

			if (static_cast<long long>(i.Data.size()) > LeftPos + ScrollbarAdjustedWidth && ViOpt.ShowArrows)
			{
				GotoXY(ScrollbarAdjustedRight,Y);
				SetColor(COL_VIEWERARROWS);
				Text(L'»');
			}

			if (LeftPos>0 && !i.Data.empty() && ViOpt.ShowArrows)
			{
				GotoXY(m_Where.left, Y);
				SetColor(COL_VIEWERARROWS);
				Text(L'«');
			}
		}
	}

	DrawScrollbar();
	ShowStatus();
}

void Viewer::DisplayObject()
{
	SHOW_MODES ShowMode;

	switch (m_DisplayMode)
	{
	default:
	case VMT_TEXT:
		ShowMode = SHOW_RELOAD;
		break;

	case VMT_HEX:
		ShowMode = SHOW_HEX;
		break;

	case VMT_DUMP:
		ShowMode = SHOW_DUMP;
		break;
	}

	ShowPage(ShowMode);
}

int Viewer::getCharSize() const
{
	if (CP_UTF8 == m_Codepage)
		return -1;
	else if (IsUtf16CodePage(m_Codepage))
		return sizeof(char16_t);
	else
		return m_Codepage == MB.GetCP()? -static_cast<int>(MB.GetSize()) : +1;
}

int Viewer::GetModeDependentCharSize() const
{
	return m_DisplayMode == VMT_HEX? 1 : get_char_size(m_Codepage);
}

int Viewer::GetModeDependentLineSize() const
{
	return m_DisplayMode == VMT_HEX? m_HexModeBytesPerLine : ScrollbarAdjustedWidth * get_char_size(m_Codepage);
}

int Viewer::txt_dump(std::string_view const Str, size_t ClientWidth, string& OutStr, wchar_t ZeroChar, int tail) const
{
	OutStr.clear();

	if (IsUtf16CodePage(m_Codepage))
	{
		OutStr.assign(std::bit_cast<const wchar_t*>(Str.data()), Str.size() / sizeof(wchar_t));
		if (m_Codepage == CP_UTF16BE)
		{
			static_assert(std::endian::native == std::endian::little, "No way");
			swap_bytes(OutStr.data(), OutStr.data(), OutStr.size() * sizeof(char16_t), sizeof(char16_t));
		}
		if (Str.size() & 1)
		{
			OutStr.push_back(encoding::replace_char);
		}
	}
	else if (m_Codepage == CP_UTF8)
	{
		std::vector<wchar_t> Buffer(ClientWidth);
		int dummy_tail;
		const auto WideCharsNumber = Utf8::get_chars(Str, Buffer, dummy_tail);
		for (size_t iw = 0; OutStr.size() < ClientWidth && iw != WideCharsNumber; ++iw)
		{
			if (tail)
			{
				--tail;
				OutStr.push_back(encoding::continue_char);
				continue;
			}

			OutStr.push_back(Buffer[iw] == encoding::bom_char? encoding::replace_char : Buffer[iw]); // BOM can be Zero Length
			const auto clen = encoding::utf8::get_bytes_count({ Buffer.data() + iw, 1 });
			const auto PaddingSize = std::min(clen - 1, ClientWidth - OutStr.size());
			OutStr.append(PaddingSize, encoding::continue_char);
			tail = static_cast<int>(clen - 1 - PaddingSize); // char continues on the next line?
		}
	}
	else if (m_Codepage == MB.GetCP())
	{
		while (OutStr.size() < ClientWidth && OutStr.size() < Str.size())
		{
			if (tail)
			{
				--tail;
				OutStr.push_back(encoding::continue_char);
				continue;
			}
			wchar_t Char;
			const auto clen = MB.GetChar(Str.substr(OutStr.size()), Char);
			if (clen)
			{
				OutStr.push_back(Char);
			}
			else
			{
				OutStr.push_back(encoding::replace_char);
				continue;
			}

			const auto PaddingSize = std::min(clen - 1, ClientWidth - OutStr.size());
			OutStr.append(PaddingSize, encoding::continue_char);

			tail = static_cast<int>(clen - 1 - PaddingSize); // char continues on the next line?
		}
	}
	else
	{
		encoding::get_chars(m_Codepage, Str, OutStr);
	}

	OutStr.resize(ClientWidth, L' ');
	std::ranges::replace(OutStr, L'\0', ZeroChar);

	return tail;
}


void Viewer::ShowDump()
{
	const int CharSize = get_char_size(m_Codepage);
	const int xl = m_Codepage == CP_UTF8? 4 - 1 : m_Codepage == MB.GetCP()? static_cast<int>(MB.GetSize() - 1) : 0;
	std::vector<char> line(ScrollbarAdjustedWidth * CharSize + xl);
	const DWORD mb = ScrollbarAdjustedWidth * CharSize;

	FilePos -= FilePos % CharSize;
	vseek(SecondPos = FilePos, FILE_BEGIN);

	bool EndFile = false;
	int tail = 0;
	string OutStr;

	for (const auto Y: std::views::iota(m_Where.top, m_Where.bottom + 1))
	{
		SetColor(COL_VIEWERTEXT);
		GotoXY(m_Where.left, Y);

		if (EndFile)
		{
			Text(string(ObjWidth(), L' '));
			continue;
		}
		const auto bpos = vtell();
		if (Y == m_Where.top + 1)
			SecondPos = bpos;

		size_t BytesRead = 0;
		Reader.Read(line.data(), line.size(), &BytesRead);
		if (BytesRead > mb)
			Reader.Unread(BytesRead-mb);
		else
			LastPage = EndFile = veof();

		tail = txt_dump({ line.data(), BytesRead }, ScrollbarAdjustedWidth, OutStr, ZeroChar(), tail);

		Text(fit_to_left(OutStr, ObjWidth()));
		if ( SelectSize > 0 && bpos < SelectPos+SelectSize && bpos+mb > SelectPos )
		{
			const int bsel = SelectPos > bpos? static_cast<int>(SelectPos - bpos) / CharSize : 0;
			const int esel = SelectPos + SelectSize < bpos + mb? (static_cast<int>(SelectPos + SelectSize - bpos) + CharSize - 1) / CharSize : ScrollbarAdjustedWidth;
			SetColor(COL_VIEWERSELECTEDTEXT);
			GotoXY(bsel, Y);
			Text(cut_right(OutStr.substr(bsel), esel - bsel));
		}
	}
}

void Viewer::ShowHex()
{
	const string BorderLine{ BoxSymbols[BS_V1], L' '};

	int tail = 0;
	bool EndFile = false;

	LastPage = false;

	AdjustHexModeBytesPerLineToViewWidth();

	for (const auto Y: std::views::iota(m_Where.top, m_Where.bottom + 1))
	{
		bool bSelStartFound = false;
		bool bSelEndFound = false;

		SetColor(COL_VIEWERTEXT);
		GotoXY(m_Where.left, Y);

		if (EndFile)
		{
			Text(string(ObjWidth(), L' '));
			continue;
		}

		if (Y == m_Where.top + 1 && !veof())
			SecondPos=vtell();

		auto OutStr = far::format(L"{:010X}: "sv, vtell());
		int SelStart = static_cast<int>(OutStr.size()), SelEnd = SelStart;
		const auto fpos = vtell();

		if (fpos > SelectPos)
			bSelStartFound = true;

		if (fpos < SelectPos+SelectSize-1)
			bSelEndFound = true;

		if ( SelectSize < 0 )
			bSelStartFound = bSelEndFound = false;

		std::vector<char> RawBuffer(m_HexModeBytesPerLine + 3, 0);
		size_t BytesRead = 0;
		const auto BytesToRead = CP_UTF8 == m_Codepage
			? m_HexModeBytesPerLine + 4 - 1
			: (m_Codepage == MB.GetCP() ? m_HexModeBytesPerLine + MB.GetSize() - 1 : m_HexModeBytesPerLine);
		Reader.Read(RawBuffer.data(), BytesToRead, &BytesRead);
		if (BytesRead > static_cast<size_t>(m_HexModeBytesPerLine))
			Reader.Unread(BytesRead - m_HexModeBytesPerLine);
		else
			LastPage = EndFile = veof();

		string TextStr;
		if (!BytesRead)
		{
			OutStr.clear();
		}
		else
		{
			if ( SelectSize >= 0 )
			{
				if (SelectPos >= fpos && SelectPos < fpos + m_HexModeBytesPerLine)
				{
					const auto off = static_cast<int>(SelectPos - fpos);
					bSelStartFound = true;
					SelStart = static_cast<int>(OutStr.size() + 3 * off + (off < s_HexModeBytesPerStripe? 0 : BorderLine.size()));
					if (!SelectSize)
						--SelStart;
				}
				const auto selectEnd = SelectPos + SelectSize - 1;
				if (selectEnd >= fpos && selectEnd < fpos + m_HexModeBytesPerLine)
				{
					const auto off = static_cast<int>(selectEnd - fpos);
					bSelEndFound = true;
					SelEnd = SelectSize ? static_cast<int>(OutStr.size() + 3 * off + (off < s_HexModeBytesPerStripe? 0 : BorderLine.size()) + 1) : SelStart;
				}
				else if ( SelectSize == 0 && SelectPos == fpos )
				{
					bSelEndFound = true;
					SelEnd = SelStart;
				}
			}

			for (const auto X: std::views::iota(0, m_HexModeBytesPerLine))
			{
				if (static_cast<size_t>(X) < BytesRead)
					far::format_to(OutStr, L"{:02X} "sv, static_cast<int>(RawBuffer[X]));
				else
					OutStr.append(3, L' ');

				if (X + 1 != m_HexModeBytesPerLine && (X + 1) % s_HexModeBytesPerStripe == 0)
					OutStr += BorderLine;
			}
			tail = txt_dump({ RawBuffer.data(), BytesRead }, m_HexModeBytesPerLine, TextStr, ZeroChar(), tail);
		}

		if ((SelEnd <= SelStart) && bSelStartFound && bSelEndFound && SelectSize > 0 )
			SelEnd = static_cast<int>(OutStr.size()) - 2;

		OutStr.push_back(L' ');
		OutStr += TextStr;

		const auto HexLeftPos = std::min(
			LeftPos,
			static_cast<long long>(std::max(hex_mode_line_width_from_bytes(m_HexModeBytesPerLine, get_char_size(m_Codepage)) - ScrollbarAdjustedWidth, 0)));

		if (static_cast<int>(OutStr.size()) > HexLeftPos)
		{
			Text(fit_to_left(OutStr.substr(HexLeftPos), ObjWidth()));
		}
		else
		{
			Text(string(ObjWidth(), L' '));
		}

		if (bSelStartFound && bSelEndFound)
		{
			SetColor(COL_VIEWERSELECTEDTEXT);
			GotoXY(static_cast<int>(static_cast<long long>(m_Where.left) + SelStart - HexLeftPos), Y);
			Text(cut_right(string_view(OutStr).substr(SelStart), SelEnd - SelStart + 1));
		}
	}
}

/* $ 27.04.2001 DJ
   отрисовка скроллбара - в отдельную функцию
*/
void Viewer::DrawScrollbar()
{
	if (ViOpt.ShowScrollbar)
	{
		SetColor(m_bQuickView? COL_PANELSCROLLBAR : COL_VIEWERSCROLLBAR);

		const auto x = m_Where.right + (m_bQuickView? 1 : 0);
		const auto h = m_Where.height();
		unsigned long long start, end, total;

		if (m_DisplayMode == VMT_TEXT)
		{
			total = static_cast<unsigned long long>(FileSize);
			start = static_cast<unsigned long long>(FilePos);
			const auto& last_line = Strings.back();
			end = last_line.nFilePos + last_line.linesize;
			if ( end == static_cast<unsigned long long>(FileSize) && last_line.linesize > 0 && last_line.eol_length != 0 )
				++total;
		}
		else
		{
			const auto LineSize = GetModeDependentLineSize();
			total = FileSize / LineSize + ((FileSize % LineSize)? 1 : 0);
			start = FilePos / LineSize + ((FilePos % LineSize)? 1 : 0);
			end = start + h;
		}
		ScrollBarEx(x, m_Where.top, h, start, end, total);
	}
}


string Viewer::GetTitle() const
{
	return strTitle.empty()? strFullFileName : strTitle;
}

void Viewer::ShowStatus() const
{
	if (HostFileViewer)
		HostFileViewer->ShowStatus();
}


void Viewer::SetStatusMode(int Mode)
{
	ShowStatusLine=Mode;
}

static bool is_word_div(const wchar_t ch, const string& word_div)
{
	return !ch || std::iswspace(ch) || contains(word_div, ch);
}

static string get_word_div()
{
	static constexpr std::array extra_word_div{ encoding::bom_char, encoding::replace_char };

	auto word_div{ Global->Opt->strWordDiv.Get() };
	word_div.append(extra_word_div.cbegin(), extra_word_div.cend());
	return word_div;
}

void Viewer::ReadString(ViewerString *pString, int MaxSize, bool update_cache)
{
	const auto& WordDiv{ Global->Opt->strWordDiv.Get() };
	auto CanWrapLineAt{ [&WordDiv](wchar_t ch) { return IsBlankOrEos(ch) || contains(WordDiv, ch); } };

	AdjustWidth();

	int OutPtr = 0, nTab = 0, wrap_out = -1;
	wchar_t ch, eol_char{};
	long long wrap_pos = -1;
	bool skip_space = false;

	if (m_DisplayMode != VMT_TEXT)
	{
		vseek(GetModeDependentLineSize(), FILE_CURRENT);
		ReadBuffer[OutPtr] = {};
		LastPage = veof();
		return;
	}

	bool bSelStartFound = false, bSelEndFound = false;
	pString->bSelection = false;
	const auto sel_end = SelectPos + SelectSize;

	long long fpos1 = vtell();
	for (;;)
	{
		const auto fpos = fpos1;

		if (OutPtr >= MaxViewLineSize())
			break;

		if (--nTab >= 0)
			ch = L' ';
		else
		{
			if (!MaxSize-- || !vgetc(&ch))
				break;
		}
		fpos1 = vtell();

		if (SelectSize >= 0)
		{
			if (fpos == SelectPos || (fpos < SelectPos && fpos1 > SelectPos))
			{
				pString->nSelStart = OutPtr;
				bSelStartFound = true;
			}
			if (fpos == sel_end || (fpos < sel_end && fpos1 > sel_end))
			{
				pString->nSelEnd = OutPtr + (fpos < sel_end ? 1 : 0);
				bSelEndFound = true;
			}
		}

		if (!fpos && encoding::bom_char == ch)
		{
			continue; // skip BOM
		}
		else if (L'\t' == ch)
		{
			nTab = ViOpt.TabSize - (OutPtr % ViOpt.TabSize);
			continue;
		}
		if (IsEol(ch))
		{
			eol_char = ch;
			break;
		}

		ReadBuffer[OutPtr++] = ch? ch : ZeroChar();
		if ( !m_Wrap )
			continue;

		if (m_WordWrap && OutPtr <= ScrollbarAdjustedWidth && CanWrapLineAt(ch))
		{
			wrap_out = OutPtr;
			wrap_pos = fpos1;
		}

		if ( OutPtr < ScrollbarAdjustedWidth )
			continue;
		if ( !m_WordWrap )
			break;

		if ( OutPtr > ScrollbarAdjustedWidth )
		{
			if ( wrap_out <= 0 || IsBlankOrEos(ch) )
			{
				wrap_out = OutPtr - 1;
				wrap_pos = fpos;
			}

			OutPtr = wrap_out;
			vseek(wrap_pos, FILE_BEGIN);
			while (OutPtr > 0 && IsBlankOrEos(ReadBuffer[OutPtr-1]))
				--OutPtr;

			if ( bSelEndFound && pString->nSelEnd > OutPtr )
				pString->nSelEnd = OutPtr;
			if ( bSelStartFound && pString->nSelStart >= OutPtr )
				bSelStartFound = bSelEndFound = false;

			skip_space = true;
			break;
		}
	}

	int eol_len = (eol_char ? 1 : 0);
	if (skip_space || eol_char != L'\n') // skip spaces and/or eol-s if required
	{
		for (;;)
		{
			vgetc(nullptr);
			const auto Iterator = VgetcCache.begin();
			if (!vgetc(&ch))
				break;

			if (skip_space && !eol_char && IsBlankOrEos(ch))
				continue;

			if ( ch == L'\n' )
			{
				++eol_len;            // LF or CRLF
				assert(eol_len <= 2);
			}
			else if ( ch != L'\r' ) // nor LF nor CR
			{
				VgetcCache.m_Iterator = Iterator; // ungetc(1)
				assert(eol_len <= 1); // CR or unterminated
			}
			else                     // CR
			{
				eol_char = ch;
				if (++eol_len == 1) // single CR - continue
					continue;

				assert(eol_len == 2); // CRCR...
				if (vgetc(&ch) && ch == L'\n')
					++eol_len;         // CRCRLF
				else
				{
					//assert(Iterator + 2 < VgetcCache.cbegin()); // ???
					VgetcCache.m_Iterator = Iterator; // CR ungetc(2)
					eol_len = 1;
				}
			}
			break;
		}
	}

	pString->eol_length = eol_len;
	pString->linesize = static_cast<int>(vtell() - pString->nFilePos);

	if ( update_cache )
		CacheLine(pString->nFilePos, pString->linesize, pString->eol_length != 0);

	if (SelectSize >= 0 && OutPtr > 0)
	{
		if (!bSelStartFound && pString->nFilePos >= SelectPos && pString->nFilePos <= sel_end)
		{
			bSelStartFound = true;
			pString->nSelStart = 0;
		}
		if (bSelStartFound && !bSelEndFound)
		{
			bSelEndFound = true;
			pString->nSelEnd = OutPtr;
		}
		if (bSelEndFound && pString->nSelEnd > OutPtr)
			pString->nSelEnd = OutPtr;

		pString->bSelection = bSelStartFound && bSelEndFound;
	}

	if (!eol_char && veof())
		LastPage = true;

	pString->Data.assign(ReadBuffer.data(), OutPtr);
}


long long Viewer::EndOfScreen(int line)
{
	long long pos;

	if (m_DisplayMode == VMT_TEXT)
	{
		const auto i = std::next(Strings.begin(), m_Where.height() - 1 + line);
		pos = i->nFilePos + i->linesize;
		if (!line && !m_Wrap && Strings.back().linesize > 0)
		{
			vseek(Strings.back().nFilePos, FILE_BEGIN);
			int col = 0;
			const auto rmargin = static_cast<int>(LeftPos) + ScrollbarAdjustedWidth;
			wchar_t ch;
			while (vgetc(&ch))
			{
				if (IsEol(ch))
					break;
				if ( ch == L'\t' )
					col += ViOpt.TabSize - (col % ViOpt.TabSize);
				else
					++col;
				if ( col >= rmargin )
				{
					pos = vtell();
					break;
				}
			}
		}
	}
	else
	{
		pos = FilePos + GetModeDependentLineSize() * (m_Where.height() + line);
	}
	if (pos < 0)
		pos = 0;
	else if (pos > FileSize)
		pos = FileSize;

	return pos;
}

long long Viewer::BegOfScreen()
{
	long long pos = FilePos;

	if (m_DisplayMode == VMT_TEXT && !m_Wrap && LeftPos > 0)
	{
		vseek(FilePos, FILE_BEGIN);
		int col = 0;
		wchar_t ch;
		pos = -1;
		long long prev_pos;
		for (;;)
		{
			prev_pos = vtell();
			if ( !vgetc(&ch) )
				break;
			if (IsEol(ch))
			{
				pos = std::next(Strings.begin())->nFilePos;
				break;
			}
			if ( ch == L'\t' )
				col += ViOpt.TabSize - (col % ViOpt.TabSize);
			else
				++col;
			if ( col > LeftPos ) //!! шеврон закрывает первый символ
				break;           //!! при LeftPos=1 не видны 2 символа
		}
		if ( pos < 0 )
			pos = (col > LeftPos ? prev_pos : vtell());
	}

	return pos;
}

long long Viewer::XYfilepos(int col, int row)
{
	long long pos = -1;

	const auto csz = get_char_size(m_Codepage);
	switch (m_DisplayMode)
	{
	case VMT_DUMP:
	{
		pos = FilePos + csz * (static_cast<long long>(ScrollbarAdjustedWidth) * row + col);
		break;
	}
	case VMT_HEX:
	{
		const long long byte{ hex_mode_byte_from_column(col, m_HexModeBytesPerLine, csz) };
		pos = FilePos + static_cast<long long>(m_HexModeBytesPerLine) * row + byte / csz * csz;
		break;
	}
	case VMT_TEXT:
	{
		for (const auto& i : Strings)
		{
			if (i.linesize <= 0)
			{
				pos = i.nFilePos;
				break;
			}

			if (--row < 0)
			{
				vseek(i.nFilePos, FILE_BEGIN);
				if (!m_Wrap)
					col += static_cast<int>(LeftPos);
				int clm = 0;
				wchar_t ch;
				for (;;)
				{
					pos = vtell();
					if (clm >= col)
						break;

					if (!vgetc(&ch))
					{
						pos = i.nFilePos + i.linesize;
						break;
					}
					else if (IsEol(ch))
					{
						pos = i.nFilePos + i.linesize - 1;
						break;
					}
					else if (ch == L'\t')
					{
						clm += ViOpt.TabSize - (clm % ViOpt.TabSize);
						if (clm > col)
							break;
					}
					else
						++clm;
				}
				break;
			}
		}
		break;
	}
	default:
		return -1;
	}
	return std::clamp(pos, 0ll, FileSize);
}


long long Viewer::VMProcess(int OpCode,void *vParam,long long iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return !FileSize;
		case MCODE_C_SELECTED:
			return SelectSize >= 0;
		case MCODE_C_EOF:
			return LastPage || !ViewFile;
		case MCODE_C_BOF:
			return !FilePos || !ViewFile;
		case MCODE_V_VIEWERSTATE:
		{
			DWORD MacroViewerState = 0;
			MacroViewerState |= ViOpt.AutoDetectCodePage?                             0_bit : 0;
			MacroViewerState |= IsUtf16CodePage(m_Codepage)?                          2_bit : 0;
			MacroViewerState |= m_Wrap?                                               3_bit : 0;
			MacroViewerState |= m_WordWrap?                                           4_bit : 0;
			MacroViewerState |= m_DisplayMode == VMT_HEX?                             5_bit : 0;
			MacroViewerState |= m_DisplayMode == VMT_DUMP?                            6_bit : 0;
			MacroViewerState |= HostFileViewer && !HostFileViewer->GetCanLoseFocus()? 11_bit : 0;
			MacroViewerState |= Global->OnlyEditorViewerUsed?                         27_bit | 11_bit : 0;
			return MacroViewerState;
		}
		case MCODE_V_ITEMCOUNT: // ItemCount - число элементов в текущем объекте
			return GetViewFileSize();
		case MCODE_V_CURPOS: // CurPos - текущий индекс в текущем объекте
			return GetViewFilePos()+1;
	}

	return 0;
}

bool Viewer::ProcessKey(const Manager::Key& Key)
{
	const auto ret = process_key(Key);
	if (redraw_selection)
		Show();
	return ret;
}

bool Viewer::process_key(const Manager::Key& Key)
{
	unsigned int LocalKey = Key();

	if (!ViOpt.PersistentBlocks &&
		none_of(LocalKey,
			KEY_NONE,
			KEY_SHIFT,
			KEY_CTRL, KEY_RCTRL,
			KEY_ALT, KEY_RALT,
			KEY_CTRLSHIFT, KEY_RCTRLSHIFT,
			KEY_ALTSHIFT, KEY_RALTSHIFT,
			KEY_CTRLALT, KEY_RCTRLALT, KEY_CTRLRALT, KEY_RCTRLRALT,
			KEY_CTRLALTSHIFT, KEY_CTRLRALTSHIFT, KEY_RCTRLALTSHIFT, KEY_RCTRLRALTSHIFT,
			KEY_CTRLINS, KEY_RCTRLINS,
			KEY_CTRLNUMPAD0, KEY_RCTRLNUMPAD0,
			KEY_CTRLC, KEY_RCTRLC,
			KEY_SHIFTF7, KEY_SPACE,
			KEY_ALTF7, KEY_RALTF7))
	{
		redraw_selection = SelectSize >= 0;
		SelectSize = -1;
	}

	if (!InternalKey && !LastKeyUndo && (UndoData.empty() || FilePos!=UndoData.back().UndoAddr || LeftPos!=UndoData.back().UndoLeft))
	{
		enum { VIEWER_UNDO_COUNT = 65536 };

		if (UndoData.size() == VIEWER_UNDO_COUNT)
			UndoData.pop_front();
		UndoData.emplace_back(FilePos, LeftPos);
	}

	if (none_of(LocalKey, KEY_ALTBS, KEY_RALTBS, KEY_CTRLZ, KEY_RCTRLZ, KEY_NONE))
		LastKeyUndo=FALSE;

	if (in_closed_range(KEY_CTRL0, LocalKey, KEY_CTRL9))
	{
		const auto Pos = LocalKey - KEY_CTRL0;

		if (BMSavePos[Pos].FilePos != POS_NONE)
		{
			FilePos = BMSavePos[Pos].FilePos;
			LeftPos = BMSavePos[Pos].LeftPos;
			Show();
		}

		return true;
	}

	if (LocalKey>=KEY_CTRLSHIFT0 && LocalKey<=KEY_CTRLSHIFT9)
		LocalKey=LocalKey-KEY_CTRLSHIFT0+KEY_RCTRL0;
	else if (LocalKey>=KEY_RCTRLSHIFT0 && LocalKey<=KEY_RCTRLSHIFT9)
		LocalKey&=~KEY_SHIFT;

	if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
	{
		const auto Pos = LocalKey - KEY_RCTRL0;
		BMSavePos[Pos].FilePos = FilePos;
		BMSavePos[Pos].LeftPos = LeftPos;
		return true;
	}

	switch (LocalKey)
	{
		case KEY_F1:
			help::show(L"Viewer"sv);
			return true;

		case KEY_CTRLU:
		case KEY_RCTRLU:
		{
			SelectSize = -1;
			Show();
			return true;
		}
		case KEY_CTRLC:
		case KEY_RCTRLC:
		case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS: case KEY_RCTRLNUMPAD0:
		{
			if (SelectSize >= 0 && ViewFile)
			{
				const wchar_t_ptr_n<256> SelData(SelectSize);
				const auto CurFilePos = vtell();
				vseek(SelectPos, FILE_BEGIN);
				const auto ReadSize = vread(SelData.data(), static_cast<int>(SelectSize));
				SetClipboardText({ SelData.data(), static_cast<size_t>(ReadSize) });
				vseek(CurFilePos, FILE_BEGIN);
			}
			return true;
		}
		//   включить/выключить скролбар
		case KEY_CTRLS:
		case KEY_RCTRLS:
		{
			ViOpt.ShowScrollbar=!ViOpt.ShowScrollbar;
			Global->Opt->ViOpt.ShowScrollbar=ViOpt.ShowScrollbar;

			if (m_bQuickView)
				Global->CtrlObject->Cp()->ActivePanel()->Redraw();

			Show();
			return true;
		}

		case KEY_NONE:
		{
			if (ViewFile)
			{
				CheckChanged();

				if (FilePos > FileSize)
				{
					ProcessKey(Manager::Key(KEY_CTRLEND));
				}
				else
				{
					const auto PrevLastPage = LastPage;
					LastPage = false;
					Show();

					if (PrevLastPage && !LastPage)
					{
						ProcessKey(Manager::Key(KEY_CTRLEND));
						LastPage = true;
					}
				}
			}
			return true;
		}
		case KEY_ALTBS:
		case KEY_RALTBS:
		case KEY_CTRLZ:
		case KEY_RCTRLZ:
		{
			if (!UndoData.empty())
			{
				UndoData.pop_back();
				if (!UndoData.empty())
				{
					FilePos=UndoData.back().UndoAddr;
					LeftPos=UndoData.back().UndoLeft;
					Show();
				}
			}
			return true;
		}
		case KEY_ADD:
		case KEY_SUBTRACT:
		{
			if (!ViewNamesList.empty())
			{
				if (const auto Name = LocalKey == KEY_ADD? ViewNamesList.GetNextName() : ViewNamesList.GetPrevName())
				{
					SavePosition();
					BMSavePos.Clear(); //Prepare for new file loading

					if (OpenFile(*Name, true))
					{
						SecondPos=0;
						Show();
						if (HostFileViewer) HostFileViewer->OnReload();
					}

					ShowConsoleTitle();
				}
			}

			return true;
		}
		case KEY_SHIFTF2:
		{
			ProcessTypeWrapMode(!m_WordWrap);
			return true;
		}
		case KEY_F2:
		{
			if (m_DisplayMode == VMT_TEXT)
				ProcessWrapMode(!m_Wrap);
			else
			{
				m_DisplayMode = m_DisplayMode == VMT_DUMP || m_DumpTextMode ? VMT_TEXT : VMT_DUMP;
				ProcessDisplayMode(m_DisplayMode);
			}
			return true;
		}
		case KEY_F4:
		{
			m_DisplayMode = m_DisplayMode == VMT_HEX? (m_DumpTextMode? VMT_DUMP : VMT_TEXT) : VMT_HEX;
			ProcessDisplayMode(m_DisplayMode);
			return true;
		}
		case KEY_SHIFTF4:
		{
			const menu_item ModeListMenu[]
			{
				{ msg(lng::MViewF4Text), 0 },
				{ msg(lng::MViewF4), 0 },
				{ msg(lng::MViewF4Dump), 0},
			};
			int MenuResult;
			{
				const auto vModes = VMenu2::create(msg(lng::MViewMode), ModeListMenu, ScrY - 4);
				vModes->SetMenuFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
				vModes->SetSelectPos(m_DisplayMode, +1);
				MenuResult = vModes->Run();
			}
			if (MenuResult >= 0)
			{
				const auto NewMode = static_cast<VIEWER_MODE_TYPE>(MenuResult);
				if (NewMode != m_DisplayMode)
				{
					if (NewMode != VMT_HEX)
						m_DumpTextMode = NewMode == VMT_DUMP;
					ProcessDisplayMode(NewMode);
				}
			}
			return true;
		}

		case KEY_F7:
		{
			DoSearchReplace(ShowSearchReplaceDialog());
			return true;
		}
		case KEY_SHIFTF7:
		case KEY_SPACE:
		{
			DoSearchReplace(SearchDisposition::ContinueForward);
			return true;
		}
		case KEY_ALTF7:
		case KEY_RALTF7:
		{
			DoSearchReplace(SearchDisposition::ContinueBackward);
			return true;
		}
		case KEY_F8:
		{
			m_Codepage = f8cps.NextCP(m_Codepage);
			MB.SetCP(m_Codepage);
			lcache_ready = false;
			AdjustFilePos();
			ChangeViewKeyBar();
			Show();
			return true;
		}
		case KEY_SHIFTF8:
		{
			uintptr_t nCodePage = m_Codepage;
			if (codepages::instance().SelectCodePage(nCodePage, true, true))
			{
				if (nCodePage == CP_DEFAULT)
				{
					const auto DefaultCodepage = GetDefaultCodePage();
					const auto fpos = vtell();
					const auto DecectedCodepage = GetFileCodepage(ViewFile, DefaultCodepage, &Signature, true);
					vseek(fpos, FILE_BEGIN);
					nCodePage = is_code_page_supported_in_viewer(DecectedCodepage)? DecectedCodepage : DefaultCodepage;
				}
				m_Codepage = nCodePage;
				MB.SetCP(m_Codepage);
				lcache_ready = false;
				AdjustFilePos();
				ChangeViewKeyBar();
				Show();
			}

			return true;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			if (ViewFile)
			{
				LastPage = false;
				GoTo();
			}

			return true;
		}
		case KEY_F11:
		{
			Global->CtrlObject->Plugins->CommandsMenu(windowtype_viewer,0,L"Viewer");
			Show();
			return true;
		}
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_UP | KEY_ALT:
		case KEY_MSWHEEL_UP | KEY_RALT:
		{
			const auto Roll = (LocalKey == KEY_MSWHEEL_UP? get_wheel_scroll_lines(Global->Opt->MsWheelDeltaView) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&]{ ProcessKey(Manager::Key(KEY_UP)); });
			return true;
		}
		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
		{
			const auto Roll = (LocalKey == KEY_MSWHEEL_DOWN? get_wheel_scroll_lines(Global->Opt->MsWheelDeltaView) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&]{ ProcessKey(Manager::Key(KEY_DOWN)); });
			return true;
		}
		case KEY_MSWHEEL_LEFT:
		case KEY_MSWHEEL_LEFT | KEY_ALT:
		case KEY_MSWHEEL_LEFT | KEY_RALT:
		{
			const auto Roll = (LocalKey == KEY_MSWHEEL_LEFT? get_wheel_scroll_chars(Global->Opt->MsHWheelDeltaView) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&]{ ProcessKey(Manager::Key(KEY_LEFT)); });
			return true;
		}
		case KEY_MSWHEEL_RIGHT:
		case KEY_MSWHEEL_RIGHT | KEY_ALT:
		case KEY_MSWHEEL_RIGHT | KEY_RALT:
		{
			const auto Roll = (LocalKey == KEY_MSWHEEL_RIGHT? get_wheel_scroll_chars(Global->Opt->MsHWheelDeltaView) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&]{ ProcessKey(Manager::Key(KEY_RIGHT)); });
			return true;
		}
		case KEY_UP: case KEY_NUMPAD8: case KEY_SHIFTNUMPAD8:
		{
			if (FilePos>0 && ViewFile)
			{
				Up(1, false); // LastPage = 0

				if (m_DisplayMode == VMT_TEXT)
				{
					ShowPage(SHOW_UP);
					const auto& end = Strings.back();
					LastPage = end.nFilePos >= FileSize || (end.eol_length == 0 && end.nFilePos + end.linesize >= FileSize);
				}
				else
				{
					Show();
				}
			}

			return true;
		}
		case KEY_DOWN: case KEY_NUMPAD2:  case KEY_SHIFTNUMPAD2:
		{
			if (!LastPage && ViewFile)
			{
				if (m_DisplayMode == VMT_TEXT)
				{
					ShowPage(SHOW_DOWN);

				}
				else
				{
					FilePos=SecondPos;
					Show();
				}
			}

			return true;
		}
		case KEY_PGUP: case KEY_NUMPAD9: case KEY_SHIFTNUMPAD9: case KEY_CTRLUP: case KEY_RCTRLUP:
		{
			if (ViewFile)
			{
				Up(m_Where.height() - 1, false);
				Show();
			}

			return true;
		}
		case KEY_PGDN: case KEY_NUMPAD3:  case KEY_SHIFTNUMPAD3: case KEY_CTRLDOWN: case KEY_RCTRLDOWN:
		{
			if (LastPage || !ViewFile)
				return true;

			FilePos = EndOfScreen(-1); // start of last screen line

			if (any_of(LocalKey, KEY_CTRLDOWN, KEY_RCTRLDOWN))
			{
				vseek(vString.nFilePos = FilePos, FILE_BEGIN);
				repeat(m_Where.height(), [&]
				{
					ReadString(&vString,-1);
					vString.nFilePos += vString.linesize;
				});

				if (LastPage)
				{
					InternalKey++;
					ProcessKey(Manager::Key(KEY_CTRLPGDN));
					InternalKey--;
					return true;
				}
			}

			Show();
			return true;
		}
		case KEY_LEFT: case KEY_NUMPAD4: case KEY_SHIFTNUMPAD4:
		{
			if (m_DisplayMode != VMT_DUMP)
				HorizontalScroll(-1);

			return true;
		}
		case KEY_RIGHT: case KEY_NUMPAD6: case KEY_SHIFTNUMPAD6:
		{
			if (m_DisplayMode != VMT_DUMP)
				HorizontalScroll(1);

			return true;
		}
		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			if (m_DisplayMode == VMT_TEXT)
				HorizontalScroll(-20);
			else
				RollContents(-1);

			return true;
		}
		case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
		{
			if (m_DisplayMode == VMT_TEXT)
				HorizontalScroll(20);
			else
				RollContents(1);

			return true;
		}

		case KEY_ALTLEFT:
		case KEY_RALTLEFT:
			if (m_DisplayMode == VMT_HEX)
				ChangeHexModeBytesPerLine(-1);

			return true;

		case KEY_CTRLALTLEFT: case KEY_RCTRLALTLEFT:
		case KEY_CTRLRALTLEFT: case KEY_RCTRLRALTLEFT:
			if (m_DisplayMode == VMT_HEX)
				ChangeHexModeBytesPerLine(-16);

			return true;

		case KEY_ALTRIGHT:
		case KEY_RALTRIGHT:
			if (m_DisplayMode == VMT_HEX)
				ChangeHexModeBytesPerLine(1);

			return true;

		case KEY_CTRLALTRIGHT: case KEY_RCTRLALTRIGHT:
		case KEY_CTRLRALTRIGHT: case KEY_RCTRLRALTRIGHT:
			if (m_DisplayMode == VMT_HEX)
				ChangeHexModeBytesPerLine(16);

			return true;

		case KEY_CTRLSHIFTLEFT:    case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT:   case KEY_RCTRLSHIFTNUMPAD4:
		{
			// Перейти на начало строк
			if (ViewFile)
			{
				LeftPos = 0;
				Show();
			}

			return true;
		}
		case KEY_CTRLSHIFTRIGHT:     case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT:    case KEY_RCTRLSHIFTNUMPAD6:
		{
			// Перейти на конец строк
			if (ViewFile)
			{
				const size_t MaxLen = std::ranges::fold_left(Strings, 0uz, [](size_t Value, const ViewerString& i)
				{
					return std::max(Value, i.Data.size());
				});
				LeftPos = (MaxLen > static_cast<size_t>(ScrollbarAdjustedWidth))? MaxLen - ScrollbarAdjustedWidth : 0;
				Show();
			}

			return true;
		}

		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		case KEY_HOME:        case KEY_NUMPAD7:   case KEY_SHIFTNUMPAD7:
			// Перейти на начало файла
			if (ViewFile)
				LeftPos=0;
			[[fallthrough]];
		case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:   case KEY_RCTRLNUMPAD9:
			if (ViewFile)
			{
				FilePos=0;
				Show();
			}
			return true;

		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:    case KEY_RCTRLNUMPAD1:
		case KEY_END:         case KEY_NUMPAD1: case KEY_SHIFTNUMPAD1:
			// Перейти на конец файла
			if (ViewFile)
				LeftPos=0;
			[[fallthrough]];
		case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:   case KEY_RCTRLNUMPAD3:
			if (ViewFile)
			{
				int max_counter = m_Where.height() - 1;
				const auto CharSize = get_char_size(m_Codepage);

				if (m_DisplayMode == VMT_TEXT)
				{
					vseek(0, FILE_END);
					FilePos = vtell();
					if (FilePos > 0)
					{
						--FilePos;
						FilePos -= FilePos % CharSize;
						vseek(FilePos, FILE_BEGIN);
						wchar_t LastSym{};
						if (vgetc(&LastSym) && !IsEol(LastSym))
							++max_counter;

						FilePos = vtell();
					}
					Up(max_counter, false);
				}
				else
				{
					const auto LineSize = GetModeDependentLineSize();
					vseek(0, FILE_END);
					FilePos = vtell();
					FilePos -= FilePos % CharSize;
					if ((FilePos % LineSize) == 0)
						FilePos -= LineSize * m_Where.height();
					else
						FilePos -= (FilePos % LineSize) + LineSize * (m_Where.height() - 1);
					if (FilePos < 0)
						FilePos = 0;
				}

				Show();
			}

			return true;
	}

	return false;
}

bool Viewer::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		return false;

	if (ViOpt.ShowScrollbar && IntKeyState.MousePos.x == m_Where.right + (m_bQuickView? 1 : 0))
	{
		if (IntKeyState.MousePos.y == m_Where.top)
		{
			// Press and hold the [▲] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});
		}
		else if (IntKeyState.MousePos.y == m_Where.bottom)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});
		}
		else if (IntKeyState.MousePos.y == m_Where.top + 1)
		{
			// Top click approximation
			ProcessKey(Manager::Key(KEY_CTRLHOME));
		}
		else if (IntKeyState.MousePos.y == m_Where.bottom - 1)
		{
			// Bottom click approximation
			ProcessKey(Manager::Key(KEY_CTRLEND));
		}
		else
		{
			// Drag the thumb
			while (IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				FilePos = (FileSize - 1) / (m_Where.height() - 2) * (IntKeyState.MousePos.y - m_Where.top);
				int Perc;

				if (FilePos > FileSize)
				{
					FilePos=FileSize;
					Perc=100;
				}
				else if (FilePos < 0)
				{
					FilePos=0;
					Perc=0;
				}
				else
					Perc=ToPercent(FilePos,FileSize);

				if (Perc == 100)
					ProcessKey(Manager::Key(KEY_CTRLEND));
				else if (!Perc)
					ProcessKey(Manager::Key(KEY_CTRLHOME));
				else
				{
					AdjustFilePos();
					Show();
				}
			}
		}

		return true;
	}

	if (!m_Where.contains(IntKeyState.MousePos))
		return false;

	if (GetAsyncKeyState(VK_SHIFT)<0 && GetAsyncKeyState(VK_CONTROL)>=0 && GetAsyncKeyState(VK_MENU)>=0)
	{
		long long filepos = XYfilepos(IntKeyState.MousePos.x - m_Where.left, IntKeyState.MousePos.y - m_Where.top), mpos = -1;
		if (filepos < 0)
			return false;

		if (ManualSelectPos < 0)
			ManualSelectPos = mpos = filepos;
		else if (filepos < ManualSelectPos)
		{
			std::ranges::swap(filepos, ManualSelectPos);
		}

		vseek(filepos, FILE_BEGIN);
		wchar_t ch;
		vgetc(&ch);
		SelectSize = vtell() - (SelectPos = ManualSelectPos);

		ManualSelectPos = mpos;
		Show();
		return true;
	}

	const auto DoKey = [&](int Key)
	{
		ProcessKey(Manager::Key(Key));
	};

	while_mouse_button_pressed([&](DWORD)
	{
		if (IntKeyState.MousePos.x < m_Where.left + 7)
			DoKey(KEY_LEFT);
		else if (IntKeyState.MousePos.x > m_Where.right - 7)
			DoKey(KEY_RIGHT);

		DoKey(IntKeyState.MousePos.y < m_Where.top + (m_Where.height() - 1) / 2? KEY_UP : KEY_DOWN);
		return true;
	});

	return true;
}


void Viewer::CacheLine( long long start, int length, bool have_eol )
{
	assert(start >= 0 && length >= 0);
	if (!length) // empty lines beyond EOF
		return;

	if ( lcache_ready
		&& (lcache_wrap != static_cast<int>(m_Wrap) || lcache_wwrap != static_cast<int>(m_WordWrap) || lcache_width != ScrollbarAdjustedWidth)
	){
		lcache_ready = false;
	}

	if (!lcache_ready || start > lcache_last || start+length < lcache_first)
	{
		lcache_first = start;
		lcache_last = start + length;

		lcache_count = 2;
		lcache_base = 0;
		lcache_lines[0] = (have_eol ? -start : +start);
		lcache_lines[1] = start + length;

		lcache_wrap = m_Wrap; lcache_wwrap = m_WordWrap; lcache_width = ScrollbarAdjustedWidth;
		lcache_ready = true;
	}
	else if (start == lcache_last)
	{
		int i = (lcache_base + lcache_count - 1) % lcache_lines.size();
		lcache_lines[i] = (have_eol ? -start : +start);
		i = (i + 1) % lcache_lines.size();
		lcache_lines[i] = lcache_last = start + length;
		if (static_cast<size_t>(lcache_count) < lcache_lines.size())
			++lcache_count;
		else
		{
			lcache_base = (lcache_base + 1) % lcache_lines.size(); // ++start
			lcache_first = std::abs(lcache_lines[lcache_base]);
		}
	}
	else if (start+length == lcache_first)
	{
		lcache_base = static_cast<int>((lcache_base + lcache_lines.size() - 1) % lcache_lines.size()); // --start
		lcache_lines[lcache_base] = (have_eol ? -start : +start);
		lcache_first = start;
		if (static_cast<size_t>(lcache_count) < lcache_lines.size())
			++lcache_count;
		else
		{
			const auto i = (lcache_base + lcache_lines.size() - 1) % lcache_lines.size(); // i = start - 1
			lcache_last = std::abs(lcache_lines[i]);
		}
	}
	else
	{
		bool reset = (start < lcache_first || start+length > lcache_last);
		if ( reset )
		{
			const auto i = CacheFindUp(start+length);
			reset = (i < 0 || std::abs(lcache_lines[i]) != start);
			if ( !reset )
			{
				const auto j = (i + 1) % lcache_lines.size();
				reset = std::abs(lcache_lines[j]) != start + length;
			}
		}
#if defined(_DEBUG) && 0 // it is legal case if file changed...
		assert( !reset );
#endif
		if ( reset )
		{
			lcache_first = start;
			lcache_last = start + length;
			lcache_count = 2;
			lcache_base = 0;
			lcache_lines[0] = (have_eol ? -start : +start);
			lcache_lines[1] = start + length;
		}
	}
}

int Viewer::CacheFindUp( long long start )
{
	if ( lcache_ready
		&& (lcache_wrap != static_cast<int>(m_Wrap) || lcache_wwrap != static_cast<int>(m_WordWrap) || lcache_width != ScrollbarAdjustedWidth)
	){
		lcache_ready = false;
	}
	if ( !lcache_ready || start <= lcache_first || start > lcache_last )
		return -1;

	int i1 = 0, i2 = lcache_count - 1;
	for (;;)
	{
		if ( i1+1 >= i2 )
			return (lcache_base + i1) % lcache_lines.size();

		const auto i = (i1 + i2) / 2;
		const auto j = (lcache_base + i) % lcache_lines.size();
		if (std::abs(lcache_lines[j]) < start)
			i1 = i;
		else
			i2 = i;
	}
}

static const int portion_size = 250;

template<typename T>
static int process_back(int BufferSize, int pos, long long& fpos, const auto& Reader, const raw_eol& eol)
{
	T Buffer[portion_size/sizeof(T)];
	int nr = Reader({ Buffer, static_cast<size_t>(BufferSize) });

	if (nr != static_cast<int>(BufferSize / sizeof(T)))
	{
		throw far_exception(L"Wrong size"sv);
	}

	if (!pos)
	{
		const auto PopEol = [&](T Char) { return nr && Buffer[nr - 1] == Char && --nr; };

		if (PopEol(eol.lf()))
		{
			if (PopEol(eol.cr()))
			{
				PopEol(eol.cr());
			}
		}
		else
		{
			PopEol(eol.cr());
		}
	}

	const T crlf[]{ eol.cr(), eol.lf() };
	const auto REnd = std::make_reverse_iterator(Buffer);
	const auto RBegin = REnd - nr;
	const auto Iterator = std::find_first_of(RBegin, REnd, ALL_CONST_RANGE(crlf));
	if (Iterator != REnd)
	{
		fpos += sizeof(T) * (REnd - Iterator);
		return true;
	}
	return false;
}

void Viewer::Up(int nlines, bool adjust)
{
	assert( nlines > 0 );

	if (!ViewFile)
		return;

	LastPage = false;

	if (FilePos <= 0)
		return;

	if (m_DisplayMode != VMT_TEXT)
	{
		const long long LineSize = GetModeDependentLineSize();
		FilePos = FilePos > LineSize * nlines? FilePos - LineSize * nlines : 0;
		return;
	}

	long long fpos = FilePos;

	int i = CacheFindUp(fpos);
	if ( i >= 0 )
	{
		for (;;)
		{
			fpos = std::abs(lcache_lines[i]);
			if (--nlines == 0)
			{
				FilePos = fpos;
				return;
			}
			if (i == lcache_base)
				break;
			i = static_cast<int>((i + lcache_lines.size() - 1) % lcache_lines.size());
		}
	}

	const auto ch_size = getCharSize();

	const raw_eol eol(m_Codepage);

	while ( nlines > 0 )
	{
		if ( fpos <= 0 )
		{
			FilePos = 0;
			return;
		}

		long long fpos1 = fpos;

		// backward CR-LF search
		//
		for (const auto j: std::views::iota(0, max_backward_size / portion_size))
		{
			int buff_size = (fpos > static_cast<long long>(portion_size)? portion_size : static_cast<int>(fpos));
			if ( buff_size <= 0 )
				break;
			fpos -= buff_size;
			vseek(fpos, FILE_BEGIN);

			if ( ch_size <= 1 )
			{
				const auto BufferReader = [&](std::span<char> Buffer)
				{
					size_t nread = 0;
					Reader.Read(Buffer.data(), buff_size, &nread);
					return static_cast<int>(nread);
				};
				try
				{
					if (process_back<char>(buff_size, j, fpos, BufferReader, eol))
						break;
				}
				catch (far_exception const&)
				{
					return; //??? error handling
				}
			}
			else
			{
				const auto BufferReader = [&](std::span<wchar_t> Buffer)
				{
					return vread(Buffer.data(), static_cast<int>(Buffer.size()));
				};
				try
				{
					if (process_back<wchar_t>(buff_size, j, fpos, BufferReader, eol))
						break;
				}
				catch (far_exception const&)
				{
					return; //??? error handling
				}
			}
		}

		// split read portion
		//
		vseek(vString.nFilePos = fpos, FILE_BEGIN);
		for (i = 0; i < static_cast<int>(llengths.size()); ++i)
		{
			ReadString(&vString, -1, false);
			llengths[i] = (vString.eol_length != 0 ? -1 : +1) * vString.linesize;
			if ((vString.nFilePos += vString.linesize) >= fpos1)
			{
				if (adjust)
					fpos1 = vString.nFilePos;
				else
					llengths[i] = vString.linesize - static_cast<int>(vString.nFilePos - fpos1);
				break;
			}
		}
		assert(i < static_cast<int>(llengths.size()));
		if (i >= static_cast<int>(llengths.size()))
			--i;

		while ( i >= 0 )
		{
			int l = llengths[i--];
			bool IsEol = false;
			if (l < 0)
			{
				IsEol = true;
				l = -l;
			}
			fpos1 -= l;
			CacheLine(fpos1, l, IsEol);
			if (--nlines == 0)
				FilePos = fpos1;
		}
	}
}

int Viewer::GetStrBytesNum(const wchar_t* const Str, int const Length) const
{
	const auto ch_size = getCharSize();
	if (ch_size > 0)
		return Length * ch_size;

	return static_cast<int>(encoding::get_bytes_count(m_Codepage, { Str, static_cast<size_t>(Length) }));
}

void Viewer::SetViewKeyBar(KeyBar *ViewKeyBar)
{
	m_ViewKeyBar = ViewKeyBar;
	ChangeViewKeyBar();
}

void Viewer::UpdateViewKeyBar(KeyBar& ViewKeyBar)
{
	string f2_label, shiftf2_label;
	if (m_DisplayMode == VMT_TEXT)
	{
		f2_label = msg(m_Wrap? lng::MViewF2Unwrap : m_WordWrap? lng::MViewShiftF2 : lng::MViewF2);
		shiftf2_label = msg(m_WordWrap? lng::MViewF2 : lng::MViewShiftF2);
	}
	else
		f2_label = msg(m_DisplayMode == VMT_DUMP || m_DumpTextMode ? lng::MViewF4Text : lng::MViewF4Dump);

	ViewKeyBar[KBL_MAIN][F2] = f2_label;
	ViewKeyBar[KBL_SHIFT][F2] = shiftf2_label;

	ViewKeyBar[KBL_MAIN][F4] = msg(m_DisplayMode != VMT_HEX? lng::MViewF4 : (m_DumpTextMode? lng::MViewF4Dump : lng::MViewF4Text));
	ViewKeyBar[KBL_MAIN][F8] = f8cps.NextCPname(m_Codepage);
}

void Viewer::ChangeViewKeyBar()
{
	if (m_ViewKeyBar)
	{
		UpdateViewKeyBar(*m_ViewKeyBar);
		m_ViewKeyBar->Redraw();
	}
}

struct Viewer::search_data
{
	long long CurPos{-1}; // IN: LastSelectPos in file, in bytes always. OUT: If Search_ NotFound/Eof/Bof/Cycle, current search position in file, in bytes always
	long long MatchPos{-1}; // OUT: If found, position of the found sequence in file, in bytes always
	bytes_view search_bytes; // IN: Needle if Hex
	string_view search_text; // IN: Needle if Text / Regex
	int search_len{}; // IN: Needle length; in wide characters if Text / Regex, in bytes if Hex. OUT: If found, length of found sequence in file, in bytes always
	int  ch_size{}; // IN: getCharSize() if Text / Regex; 1 if Hex
	const i_searcher* searcher{}; // IN: The searcher to use if Text; nullptr otherwise. TODO: use i_searcherfor for all types of search
	string word_div; // IN: Word delimiter characters if Text; empty otherwise
	bool first_Rex{true};
	RegExp Rex; // IN: Compiled regex if Regex
	regex_match RexMatch;
};

enum SEARCHER_RESULT: int
{
	Search_NotFound  = 0,
	Search_Continue  = 1,
	Search_Eof       = 2,
	Search_Bof       = 3,
	Search_Cycle     = 4,
	Search_Found     = 5,
};

constexpr auto
	SearchWrap_NO    = BSTATE_UNCHECKED,
	SearchWrap_END   = BSTATE_CHECKED,
	SearchWrap_CYCLE = BSTATE_3STATE;

SEARCHER_RESULT Viewer::search_hex_forward(search_data* sd)
{
	const auto buff = edit_as<std::byte*>(Search_buffer.data());
	const auto bsize = static_cast<int>(Search_buffer.size() * sizeof(wchar_t)), slen = sd->search_len;
	long long to;
	const auto cpos = sd->CurPos;
	const auto swrap = ViOpt.SearchWrapStop;

	const auto tail_part = cpos >= StartSearchPos;
	if (swrap == SearchWrap_CYCLE || tail_part)
	{
		if ( FileSize - cpos <= bsize )
			SetFileSize();
		to = FileSize;
	}
	else
	{
		to = StartSearchPos + slen - 1;
		if ( to > FileSize )
			to = FileSize;
	}

	const auto nb = (to - cpos < bsize ? static_cast<int>(to - cpos) : bsize);
	vseek(cpos, FILE_BEGIN);
	size_t nr = 0;
	Reader.Read(buff, nb, &nr);
	to = cpos + nr;
	int n1 = static_cast<int>(nr);
	if ( n1 != nb )
		SetFileSize();

	auto ps = buff;
	while (n1 >= slen)
	{
		const auto ps_end = ps + n1 - slen + 1;
		ps = std::find(ps, ps_end, sd->search_bytes[0]);
		if (ps == ps_end)
			break;

		if (slen <= 1 || std::equal(ps + 1, ps + slen, sd->search_bytes.cbegin() + 1))
		{
			sd->MatchPos = cpos + (ps - buff);
			return Search_Found;
		}
		++ps;
		n1 = static_cast<int>(nr - (ps - buff));
	}

	if ((LastSelectPos <= 0 && to >= FileSize) || (LastSelectPos > 0 && cpos < LastSelectPos && to >= LastSelectPos))
		return Search_NotFound;

	sd->CurPos = cpos + nr - slen + 1;
	if (sd->CurPos + slen > FileSize)
	{
		sd->CurPos = 0;
		if (swrap == SearchWrap_CYCLE && StartSearchPos == 0)
			return Search_Cycle;
		else if (swrap == SearchWrap_END)
			return Search_Eof;
		else if (swrap == SearchWrap_NO)
			return Search_Continue;
	}
	if (swrap == SearchWrap_CYCLE && !tail_part && sd->CurPos >= StartSearchPos)
		return Search_Cycle;
	else
		return Search_Continue;
}

SEARCHER_RESULT Viewer::search_hex_backward(search_data* sd)
{
	const auto buff = edit_as<std::byte*>(Search_buffer.data());
	const auto bsize = static_cast<int>(Search_buffer.size() * sizeof(wchar_t)), slen = sd->search_len;
	long long to, cpos = sd->CurPos;
	const auto swrap = ViOpt.SearchWrapStop;

	const auto lo_half = cpos <= StartSearchPos;
	if ( swrap != SearchWrap_CYCLE || lo_half )
	{
		to = 0;
	}
	else
	{
		to = StartSearchPos - slen + 1;
		if ( to < 0 )
			to = 0;
	}

	const auto nb = (cpos - to < bsize? static_cast<int>(cpos - to) : bsize);
	vseek(to = cpos-nb, FILE_BEGIN);
	size_t nr = 0;
	Reader.Read(buff, static_cast<DWORD>(nb), &nr);
	int n1 = static_cast<int>(nr);
	if ( n1 != nb )
	{
		SetFileSize();
		cpos = vtell();
	}

	auto ps = buff + n1 - 1;
	const auto last_char = sd->search_bytes[slen - 1];
	while (n1 >= slen)
	{
		if (*ps == last_char)
		{
			if (slen <= 1 || std::equal(ps - slen + 1, ps, sd->search_bytes.cbegin()))
			{
				sd->MatchPos = cpos - nr + n1 - slen;
				return Search_Found;
			}
		}
		--ps;
		--n1;
	}

	if ((LastSelectPos >= FileSize && to <= 0) || (LastSelectPos > 0 && to <= LastSelectPos && cpos > LastSelectPos))
		return Search_NotFound;

	sd->CurPos = cpos - nr + slen - 1;
	if (sd->CurPos < slen)
	{
		sd->CurPos = FileSize;
		if (swrap == SearchWrap_CYCLE && StartSearchPos >= FileSize)
			return Search_Cycle;
		else if (swrap == SearchWrap_END)
			return Search_Bof;
		else
			return Search_Continue;
	}
	if (swrap == SearchWrap_CYCLE && !lo_half && sd->CurPos <= StartSearchPos)
		return Search_Cycle;
	else
		return Search_Continue;
}

SEARCHER_RESULT Viewer::search_text_forward(search_data* sd)
{
	assert(sd->searcher);

	const auto bsize = 8192, slen = sd->search_len, ww = (m_SearchDlgParams.WholeWords.value() ? 1 : 0);
	wchar_t prev_char{}, *buff = Search_buffer.data(), *t_buff = (sd->ch_size < 0 ? buff + bsize : nullptr);
	long long to;
	const auto cpos = sd->CurPos;
	const auto swrap = ViOpt.SearchWrapStop;

	vseek(cpos, FILE_BEGIN);
	if ( ww )
		prev_char = vgetc_prev();

	const auto tail_part = cpos >= StartSearchPos;
	if (swrap != SearchWrap_CYCLE || tail_part )
	{
		if (FileSize - cpos <= bsize )
			SetFileSize();

		to = FileSize;
	}
	else
	{
		to = std::min(FileSize, StartSearchPos);
	}

	const auto nb = (to - cpos > bsize? bsize : static_cast<int>(to - cpos));
	int nw = vread(buff, nb, t_buff);
	auto to1 = vtell();
	if ( swrap == SearchWrap_CYCLE && !tail_part && nb + 3*(slen+ww) < bsize && !veof() )
	{
		int nw1 = vread(buff+nw, 3*(slen+ww), t_buff ? t_buff+nw : nullptr);
		nw1 = std::min(nw1, slen+ww-1);
		to1 = to + (t_buff ? GetStrBytesNum(t_buff+nw, static_cast<size_t>(nw1)) : sd->ch_size * nw1);
		nw += nw1;
	}

	const auto is_eof = (to1 >= FileSize ? 1 : 0), iLast = nw - slen - ww + ww*is_eof;

	if (int CurPos{}, SearchLength{};
		SearchString(
		{ buff, buff + nw },
		sd->search_text,
		*sd->searcher,
		sd->Rex,
		sd->RexMatch,
		{},
		CurPos,
		{
			.CaseSensitive = m_SearchDlgParams.CaseSensitive.value(),
			.WholeWords = m_SearchDlgParams.WholeWords.value(),
		},
		SearchLength,
		sd->word_div))
	{
		auto matchWholeWords{ true };
		if (ww)
		{
			if (CurPos == 0 && !is_word_div(prev_char, sd->word_div)) matchWholeWords = false;
			if (matchWholeWords && CurPos == iLast && !is_eof && !is_word_div(buff[CurPos + SearchLength], sd->word_div)) matchWholeWords = false;
		}
		if (matchWholeWords)
		{
			sd->MatchPos = cpos + GetStrBytesNum(t_buff, CurPos);
			sd->search_len = GetStrBytesNum(t_buff + CurPos, SearchLength);
			return Search_Found;
		}
	}

	if (is_eof)
	{
		sd->CurPos = 0;

		if (LastSelectPos <= 0 || cpos < LastSelectPos)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && StartSearchPos == 0)
			return Search_Cycle;
		else if (swrap == SearchWrap_END)
			return Search_Eof;
	}
	else
	{
		sd->CurPos = to1 - GetStrBytesNum(t_buff + iLast + 1, static_cast<size_t>(nw - iLast - 1));

		if (LastSelectPos > 0 && cpos < LastSelectPos && sd->CurPos >= LastSelectPos)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && !tail_part && sd->CurPos >= StartSearchPos)
			return Search_Cycle;
	}
	return Search_Continue;
}

SEARCHER_RESULT Viewer::search_text_backward(search_data* sd)
{
	assert(sd->searcher);

	const auto bsize = 8192, slen = sd->search_len, ww = (m_SearchDlgParams.WholeWords.value() ? 1 : 0);
	const auto buff = Search_buffer.data();
	const auto t_buff = (sd->ch_size < 0 ? buff + bsize : nullptr);
	auto cpos = sd->CurPos;
	const auto swrap = ViOpt.SearchWrapStop;

	const auto tail_part = cpos > StartSearchPos;
	const auto to = (tail_part && swrap == SearchWrap_CYCLE ? StartSearchPos : 0);

	int nb = (cpos - to > bsize ? bsize : static_cast<int>(cpos- to));
	if (tail_part && swrap == SearchWrap_CYCLE && nb + 3*(slen+ww) < bsize && cpos > nb)
	{
		if ( sd->ch_size > 0 )
		{
			nb += sd->ch_size * (slen + ww - 1);
			if (cpos < nb)
				nb = static_cast<int>(cpos);
		}
		else
		{
			long long to1 = cpos - nb - 3*(slen + ww - 1);
			if (to1 < 0)
				to1 = 0;
			int nb1 = static_cast<int>(cpos - nb - to1);
			vseek(to1, FILE_BEGIN);
			const auto nw1 = vread(buff, nb1, t_buff);
			if (nw1 > slen + ww - 1)
				nb1 = GetStrBytesNum(t_buff + nw1 - (slen + ww - 1), static_cast<size_t>(slen + ww - 1));
			nb += nb1;
		}
	}

	cpos -= nb;
	vseek(cpos, FILE_BEGIN);
	const auto nw = vread(buff, nb, t_buff);

	const auto is_eof = (veof() ? 1 : 0), iFirst = ww * (cpos > 0 ? 1 : 0), iLast = nw - slen - ww + ww*is_eof;

	if (int CurPos{ iLast + slen }, SearchLength{};
		SearchString(
		{ buff + iFirst, buff + iLast + slen },
		sd->search_text,
		*sd->searcher,
		sd->Rex,
		sd->RexMatch,
		{},
		CurPos,
		{
			.CaseSensitive = m_SearchDlgParams.CaseSensitive.value(),
			.WholeWords = m_SearchDlgParams.WholeWords.value(),
			.Reverse = true
		},
		SearchLength,
		sd->word_div))
	{
		auto matchWholeWords{ true };
		if (ww)
		{
			if (CurPos == iFirst && cpos > 0 && !is_word_div(buff[iFirst - 1], sd->word_div)) matchWholeWords = false;
			if (matchWholeWords && CurPos == iLast && !is_eof && !is_word_div(buff[CurPos + SearchLength], sd->word_div)) matchWholeWords = false;
		}
		if (matchWholeWords)
		{
			sd->MatchPos = cpos + iFirst + GetStrBytesNum(t_buff, CurPos);
			sd->search_len = GetStrBytesNum(t_buff + CurPos, SearchLength);
			return Search_Found;
		}
	}

	if (cpos <= 0) // bof()
	{
		SetFileSize();
		sd->CurPos = FileSize;

		if (LastSelectPos > FileSize || tail_part)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && StartSearchPos >= FileSize)
			return Search_Cycle;
		else if (swrap == SearchWrap_END)
			return Search_Bof;
	}
	else
	{
		sd->CurPos = cpos + GetStrBytesNum(t_buff, static_cast<size_t>(iFirst + slen - 1));

		if (cpos+nb > LastSelectPos && sd->CurPos <= LastSelectPos)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && tail_part && sd->CurPos <= StartSearchPos)
			return Search_Cycle;
	}
	return Search_Continue;
}

int Viewer::read_line(wchar_t *buf, wchar_t *tbuf, long long cpos, int adjust, long long& lpos, int &lsize)
{
	const auto OldFilePos = FilePos;
	const auto OldLastPage = LastPage;
	const auto OldWrap = m_Wrap;
	const auto OldWrapTouched = m_Wrap.touched();
	const auto OldWordWrap = m_WordWrap;
	const auto OldWordWrapTouched = m_WordWrap.touched();
	const auto OldDisplayMode = m_DisplayMode;

	m_DisplayMode = VMT_TEXT;
	m_Wrap = m_WordWrap = false;

	FilePos = cpos;
	if (adjust)
	{
		if (adjust > 0)
			AdjustFilePos();
		else
			Up(1, true);
	}

	vseek(lpos = vString.nFilePos = FilePos, FILE_BEGIN);
	vString.linesize = 0;
	ReadString(&vString, -1, false); // read unwrapped text line

	vseek(FilePos, FILE_BEGIN);
	int llen = vread(buf, lsize = vString.linesize, tbuf);
	if (llen > 0)
		llen -= vString.eol_length; // remove eol-s
	buf[llen >= 0 ? llen : 0] = {};

	m_DisplayMode = OldDisplayMode;

	m_Wrap = OldWrap;
	m_WordWrap = OldWordWrap;
	if (!OldWrapTouched)
	{
		m_Wrap.forget();
	}
	if (!OldWordWrapTouched)
	{
		m_WordWrap.forget();
	}
	FilePos = OldFilePos;
	LastPage = OldLastPage;

	return llen;
}

SEARCHER_RESULT Viewer::search_regex_forward(search_data* sd)
{
	assert(Search_buffer.size() >= 2 * MaxViewLineBufferSize());

	const auto line = Search_buffer.data();
	const auto t_line = sd->ch_size < 0? Search_buffer.data() + MaxViewLineBufferSize() : nullptr;
	const auto cpos = sd->CurPos;
	long long bpos = 0;

	const auto first = (sd->first_Rex ? +1 : 0);
	sd->first_Rex = false;
	const auto tail_part = cpos >= StartSearchPos;
	const auto swrap = ViOpt.SearchWrapStop;

	int lsize = 0;
	const auto nw = read_line(line, t_line, cpos, first, bpos, lsize);
	if (lsize <= 0) //eof() -- TODO: error handling
	{
		sd->CurPos = 0;
		if (!tail_part || StartSearchPos <= 0)
			return Search_NotFound;
		else
			return swrap == SearchWrap_END ? Search_Eof : Search_Continue;
	}

	int off = 0;
	for (;;)
	{
		if ( off > nw )
			break;

		if (!sd->Rex.SearchEx({ line, static_cast<size_t>(nw) }, off, sd->RexMatch))  // doesn't match
			break;

		const auto fpos = bpos + GetStrBytesNum(t_line, sd->RexMatch.Matches[0].start);
		if ( fpos < cpos )
		{
			off = sd->RexMatch.Matches[0].start + 1; // skip
			continue;
		}
		else if (swrap == SearchWrap_CYCLE && !tail_part && fpos >= StartSearchPos)
		{
			break; // done - not found
		}
		else // found
		{
			sd->MatchPos = fpos;
			sd->search_len = GetStrBytesNum(t_line + off + sd->RexMatch.Matches[0].start, sd->RexMatch.Matches[0].end - sd->RexMatch.Matches[0].start);
			return Search_Found;
		}
	}

	if (veof())
	{
		sd->CurPos = 0;

		if (LastSelectPos <= 0 || cpos < LastSelectPos)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && StartSearchPos == 0)
			return Search_Cycle;
		else if (swrap == SearchWrap_END)
			return Search_Eof;
	}
	else
	{
		const auto pos = vtell();
		sd->CurPos = pos;

		if (LastSelectPos > 0 && cpos < LastSelectPos && pos >= LastSelectPos)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && !tail_part && pos >= StartSearchPos)
			return Search_Cycle;
	}
	return Search_Continue;
}

SEARCHER_RESULT Viewer::search_regex_backward(search_data* sd)
{
	assert(Search_buffer.size() >= 2 * MaxViewLineBufferSize());

	wchar_t *line = Search_buffer.data(), *t_line = sd->ch_size < 0 ? Search_buffer.data() + MaxViewLineBufferSize() : nullptr;
	const auto cpos = sd->CurPos;
	long long bpos = 0, prev_pos = -1;

	const auto tail_part = cpos > StartSearchPos;
	const auto swrap = ViOpt.SearchWrapStop;

	int off=0, lsize=0, prev_len = -1;
	const auto nw = read_line(line, t_line, cpos, -1, bpos, lsize);
	for (;;)
	{
		if (lsize <= 0 || off > nw)
			break;

		if (!sd->Rex.SearchEx({ line, static_cast<size_t>(nw) }, off, sd->RexMatch))
			break;

		const auto fpos = bpos + GetStrBytesNum(t_line, sd->RexMatch.Matches[0].start);
		const auto flen = GetStrBytesNum(t_line + sd->RexMatch.Matches[0].start, sd->RexMatch.Matches[0].end - sd->RexMatch.Matches[0].start);
		if (fpos+flen > cpos)
			break;

		if (!(tail_part && fpos+flen <= StartSearchPos))
		{
			prev_pos = fpos;
			prev_len = flen;
		}

		off = sd->RexMatch.Matches[0].start + 1; // skip
	}

	if (prev_len >= 0)
	{
		sd->MatchPos = prev_pos;
		sd->search_len = prev_len;
		return Search_Found;
	}

	if (bpos <= 0) // bof()
	{
		SetFileSize();
		sd->CurPos = FileSize;

		if (LastSelectPos > FileSize || tail_part)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && StartSearchPos >= FileSize)
			return Search_Cycle;
		else if (swrap == SearchWrap_END)
			return Search_Bof;
	}
	else
	{
		sd->CurPos = bpos;

		if (cpos > LastSelectPos && bpos <= LastSelectPos)
			return Search_NotFound;
		else if (swrap == SearchWrap_CYCLE && tail_part && bpos <= StartSearchPos)
			return Search_Cycle;
	}
	return Search_Continue;
}


Viewer::SearchDisposition Viewer::ShowSearchReplaceDialog()
{
	switch (GetSearchReplaceString(
		{ .ShowButtonsPrevNext = true },
		m_SearchDlgParams,
		L"SearchText"sv,
		{},
		m_Codepage,
		L"ViewerSearch"sv,
		&ViewerSearchId))
	{
		case SearchReplaceDlgResult::Cancel:
			return SearchDisposition::Cancel;

		case SearchReplaceDlgResult::Prev:
			return SearchDisposition::Prev;

		case SearchReplaceDlgResult::Next:
			return SearchDisposition::Next;

		case SearchReplaceDlgResult::Ok:
		case SearchReplaceDlgResult::All:
		default:
			std::unreachable();
	}
}

void Viewer::DoSearchReplace(SearchDisposition Disposition)
{
	if (!ViewFile || Disposition == SearchDisposition::Cancel || m_SearchDlgParams.SearchStr.empty())
		return;

	const auto Backward{ Disposition == SearchDisposition::Prev || Disposition == SearchDisposition::ContinueBackward };
	const auto Continue{ Disposition == SearchDisposition::ContinueBackward || Disposition == SearchDisposition::ContinueForward };

	searchers Searchers;

	search_data sd;
	decltype(&Viewer::search_hex_forward) searcher;
	string strMsgStr;

	if (m_SearchDlgParams.Hex.value())
	{
		sd.ch_size = 1;
		sd.search_bytes = m_SearchDlgParams.SearchBytes.value();
		sd.search_len = static_cast<int>(m_SearchDlgParams.SearchBytes.value().size());
		searcher = (Backward ? &Viewer::search_hex_backward : &Viewer::search_hex_forward);
		strMsgStr = BlobToHexString(m_SearchDlgParams.SearchBytes.value(), L' ');
	}
	else
	{
		sd.ch_size = getCharSize();
		sd.search_text = m_SearchDlgParams.SearchStr;
		sd.search_len = static_cast<int>(m_SearchDlgParams.SearchStr.size());

		if (m_SearchDlgParams.Regex.value())
		{
			searcher = (Backward ? &Viewer::search_regex_backward : &Viewer::search_regex_forward);

			const auto strSlash = InsertRegexpQuote(m_SearchDlgParams.SearchStr);
			try
			{
				sd.Rex.Compile(strSlash, OP_PERLSTYLE | OP_OPTIMIZE | (m_SearchDlgParams.CaseSensitive.value() ? 0 : OP_IGNORECASE));
			}
			catch (regex_exception const& e)
			{
				ReCompileErrorMessage(e, strSlash);
				return;
			}

			strMsgStr = strSlash;
		}
		else
		{
			sd.searcher = &init_searcher(Searchers, m_SearchDlgParams.CaseSensitive.value(), m_SearchDlgParams.Fuzzy.value(), m_SearchDlgParams.SearchStr);
			sd.word_div = get_word_div();
			searcher = (Backward ? &Viewer::search_text_backward : &Viewer::search_text_forward);
			strMsgStr = m_SearchDlgParams.SearchStr;
			inplace::quote_unconditional(strMsgStr);
		}
	}

	auto advanceSelectPositionForward{ [&]()
	{
		if (sd.ch_size >= 1) return SelectPos + sd.ch_size;

		vseek(SelectPos, FILE_BEGIN);
		wchar_t ch;
		vgetc(&ch);
		return vtell();
	} };

	auto advanceSelectPositionBackward{ [&]()
	{
		if (sd.ch_size >= 1) return  SelectPos + LastSelectSize - sd.ch_size;

		auto prev_pos = SelectPos;
		vseek(SelectPos, FILE_BEGIN);
		for (;;)
		{
			wchar_t ch;
			const auto has_next = vgetc(&ch);
			const auto next_pos = vtell();

			if (!has_next) return next_pos;
			if (next_pos >= SelectPos + LastSelectSize) return prev_pos;

			prev_pos = next_pos;
		}
	} };

	if (Continue && SelectPos >= 0 && SelectSize >= 0)
	{
		LastSelectPos = Backward ? advanceSelectPositionBackward() : advanceSelectPositionForward();
		if (Backward != LastSearchBackward)
			StartSearchPos = LastSelectPos;
	}
	else
	{
		if (LastSelectSize < 0)
			LastSelectSize = SelectSize = -1;
		StartSearchPos = LastSelectPos = (Backward ? EndOfScreen(0) : BegOfScreen());
	}

	LastSearchBackward = Backward;

	if (!sd.search_len || !FileSize)
		return;
	const auto can_be_found =
		((m_SearchDlgParams.Regex.value() || m_SearchDlgParams.Fuzzy.value()) && !m_SearchDlgParams.Hex.value())
		|| static_cast<long long>(sd.search_len) <= FileSize;

	sd.CurPos = LastSelectPos;
	{
		SCOPED_ACTION(taskbar::indeterminate);
		HideCursor();

		std::optional<single_progress> Progress;
		const time_check TimeCheck;

		for (;;)
		{
			const auto found = can_be_found ? (this->*searcher)(&sd) : Search_NotFound;
			if (found == Search_Found)
				break;

			else if (found == Search_Continue)
				;

			else if (found == Search_NotFound)
			{
				Message(MSG_WARNING,
					msg(lng::MSearchReplaceSearchTitle),
					{
						msg(m_SearchDlgParams.Hex.value() ? lng::MViewSearchCannotFindHex : lng::MViewSearchCannotFind),
						strMsgStr
					},
					{ lng::MOk });

				return;
			}
			else // Search_ Eof/Bof/Cycle
			{
				// MviewSearch Eod,FromBegin/Bod,FromEnd/Cycle,Repeat
				static_assert(Search_Bof - Search_Eof == 1 && Search_Cycle - Search_Eof == 2, "Wrong enum order");
				static_assert(lng::MViewSearchEod + 1 == lng::MViewSearchFromBegin, "Wrong .lng file order");
				static_assert(lng::MViewSearchBod + 1 == lng::MViewSearchFromEnd, "Wrong .lng file order");
				static_assert(lng::MViewSearchCycle + 1 == lng::MViewSearchRepeat, "Wrong .lng file order");
				static_assert(lng::MViewSearchEod + 2 == lng::MViewSearchBod && lng::MViewSearchEod + 4 == lng::MViewSearchCycle, "Wrong .lng file order");

				if (Message(0,
					msg(lng::MSearchReplaceSearchTitle),
					{
						msg(lng::MViewSearchEod + 2 * (found - Search_Eof)),
						msg(lng::MViewSearchFromBegin + 2 * (found - Search_Eof)),
						strMsgStr
					},
					{ lng::MYes, lng::MCancel }) != message_result::first_button) // cancel search
				{
					return;
				}
			}

			if (TimeCheck)
			{
				if (CheckForEscAndConfirmAbort())
				{
					Redraw();
					return;
				}

				int percent{};
				long long total = FileSize;
				if ( total > 0 )
				{
					long long done;
					if (!Backward)
					{
						if ( sd.CurPos >= StartSearchPos )
							done = sd.CurPos - StartSearchPos;
						else
							done = (FileSize - StartSearchPos) + sd.CurPos;
					}
					else
					{
						if ( sd.CurPos <= StartSearchPos )
							done = StartSearchPos - sd.CurPos;
						else
							done = StartSearchPos + (FileSize - sd.CurPos);
					}
					percent = ToPercent(done, total);
				}

				if (!Progress)
				{
					Progress.emplace(
						msg(lng::MSearchReplaceSearchTitle), concat(msg(m_SearchDlgParams.Hex.value() ? lng::MViewSearchingHex : lng::MViewSearchingFor), L' ', strMsgStr), 0);
				}

				Progress->update(percent);
			}
		}
	}

	if ( sd.MatchPos >= 0 )
	{
		DWORD flags = Backward ? 0x2 : 0;

		if (sd.search_len < 0
		 || (sd.MatchPos >= BegOfScreen() && sd.MatchPos + sd.search_len <= EndOfScreen(0)))
		{
			SelectPos = sd.MatchPos;
			SelectSize = LastSelectSize = sd.search_len;
			SelectFlags = flags;
		}
		else
		{
			SelectText(sd.MatchPos, sd.search_len, flags);
			LastSelectSize = SelectSize;

			// Покажем найденное на расстоянии четверти экрана от верха.
			int FromTop = (ScrY - (Global->Opt->ViOpt.ShowKeyBar ? 2 : 1)) / 4;

			if (FromTop<0 || FromTop>ScrY)
				FromTop = 0;

			Up(FromTop, false);
		}

		AdjustSelPosition = true;
		Show();
		AdjustSelPosition = false;
	}
}


bool Viewer::GetWrapMode() const
{
	return m_Wrap;
}

void Viewer::SetWrapMode(bool Wrap)
{
	m_Wrap = Wrap;
}

void Viewer::EnableHideCursor(int HideCursor)
{
	m_HideCursor=HideCursor;
}

bool Viewer::GetWrapType() const
{
	return m_WordWrap;
}

void Viewer::SetWrapType(bool TypeWrap)
{
	m_WordWrap = TypeWrap;
}

void Viewer::SetTempViewName(string_view const Name, bool DeleteFolder)
{
	if (!Name.empty())
		strTempViewName = ConvertNameToFull(Name);
	else
	{
		strTempViewName.clear();
		DeleteFolder = false;
	}

	m_DeleteFolder=DeleteFolder;
}

void Viewer::SetTitle(string_view const Title)
{
	strTitle = Title;
}

void Viewer::SetFilePos(long long Pos)
{
	FilePos=Pos;
	AdjustFilePos();
}

void Viewer::SetPluginData(string_view const PluginData)
{
	strPluginData = PluginData;
}

void Viewer::SetNamesList(NamesList& List)
{
	ViewNamesList = std::move(List);
}

int Viewer::vread(wchar_t *Buf, int Count, wchar_t *Buf2)
{
	if (Count <= 0)
		return 0;

	size_t ReadSize = 0;

	if (IsUtf16CodePage(m_Codepage))
	{
		Reader.Read(Buf, Count, &ReadSize);

		if (CP_UTF16BE == m_Codepage)
		{
			(void)encoding::get_chars(m_Codepage, { view_as<char const*>(Buf), ReadSize }, { Buf, ReadSize });
		}

		if (ReadSize & 1)
		{
			Buf[ReadSize / sizeof(wchar_t)] = encoding::replace_char;
		}

		ReadSize /= sizeof(wchar_t);
	}
	else
	{
		char *TmpBuf = vread_buffer.data();
		char_ptr Buffer;
		if (static_cast<size_t>(Count) > vread_buffer.size())
		{
			Buffer.reset(Count);
			TmpBuf = Buffer.data();
		}
		Reader.Read(TmpBuf, Count, &ReadSize);
		const auto ConvertSize = ReadSize;

		if (m_Codepage == CP_UTF8)
		{
			int tail;
			ReadSize = Utf8::get_chars({ TmpBuf, ConvertSize }, { Buf, static_cast<size_t>(Count) }, tail);

			if (Buf2)
			{
				std::copy_n(Buf, ReadSize, Buf2);
			}

			if (tail)
			{
				Reader.Unread(tail);
			}
		}
		else if (m_Codepage == MB.GetCP())
		{
			ReadSize = 0;
			for (size_t i = 0; i < ConvertSize; )
			{
				bool EndOfData = false;
				if (const auto clen = MB.GetChar({ TmpBuf + i, ConvertSize - i }, Buf[ReadSize], &EndOfData))
				{
					if (Buf2)
						Buf2[ReadSize] = Buf[ReadSize];

					i += clen;
					++ReadSize;
				}
				else
				{
					if (EndOfData)
					{
						Reader.Unread(ConvertSize - i);
						break;
					}
					else // invalid sequence
					{
						if (Buf2)
							Buf2[ReadSize] = L'?';

						Buf[ReadSize++] = encoding::replace_char;
						++i;
					}
				}
			}
		}
		else
		{
			ReadSize = encoding::get_chars(m_Codepage, { TmpBuf, ConvertSize }, { Buf, static_cast<size_t>(Count) });
		}
	}

	return static_cast<int>(ReadSize);
}

bool Viewer::vseek(long long Offset, int Whence)
{
	if (FILE_CURRENT == Whence)
	{
		if (VgetcCache.ready())
		{
			Offset += VgetcCache.size();
			VgetcCache.clear();
		}
		if (0 == Offset)
			return true;
	}

	VgetcCache.clear();
	return ViewFile.SetPointer(Offset, nullptr, Whence);
}

long long Viewer::vtell() const
{
	auto Ptr = ViewFile.GetPointer();

	if (VgetcCache.ready())
		Ptr -= VgetcCache.size();

	return Ptr;
}

bool Viewer::veof() const
{
	if (VgetcCache.ready() && !VgetcCache.empty())
		return false;
	else
		return ViewFile.Eof();
}

bool Viewer::vgetc(wchar_t* pCh)
{
	if (!VgetcCache.ready())
	{
		vgetc_composite = 0;
	}

	if (VgetcCache.size() < (4u + (pCh? 0 : 1)) && !ViewFile.Eof())
	{
		VgetcCache.compact();

		size_t nr = 0;
		Reader.Read(VgetcCache.end(), VgetcCache.free_size(), &nr);
		VgetcCache.m_End += nr;
	}

	if (vgetc_composite)
	{
		if (pCh)
			*pCh = vgetc_composite;
		vgetc_composite = 0;
		return true;
	}

	if (!pCh)
		return true;

	if (VgetcCache.empty())
		return false;

	switch (m_Codepage)
	{
	case CP_UTF16LE:
	case CP_UTF16BE:
		{
			const auto First = VgetcCache.pop();

			if (VgetcCache.empty())
			{
				*pCh = encoding::replace_char;
			}
			else
			{
				const auto Second = VgetcCache.pop();

				*pCh = m_Codepage == CP_UTF16LE?
					make_integer<wchar_t>(First, Second) :
					make_integer<wchar_t>(Second, First);
			}
		}
		break;

	case CP_UTF8:
		{
			wchar_t w[2];
			std::string_view const View(VgetcCache.cbegin(), VgetcCache.size());
			auto Iterator = View.cbegin();
			encoding::diagnostics Diagnostics;
			const auto WideCharsNumber = Utf8::get_char(Iterator, View.cend(), w[0], w[1], Diagnostics);
			VgetcCache.pop(Iterator - View.cbegin());
			*pCh = w[0];
			if (WideCharsNumber > 1)
				vgetc_composite = w[1];
		}
		break;

	default:
		if (m_Codepage == MB.GetCP())
		{
			bool DataEnd = false;
			if (const auto clen = MB.GetChar({ VgetcCache.cbegin(), VgetcCache.size() }, *pCh, &DataEnd))
			{
				VgetcCache.pop(clen);
			}
			else
			{
				if (DataEnd)
				{
					*pCh = encoding::replace_char;
					VgetcCache.m_Iterator = VgetcCache.end();
				}
				else // bad sequence
				{
					*pCh = encoding::replace_char;
					VgetcCache.pop();
				}
			}
		}
		else
		{
			const auto Ch = VgetcCache.pop();
			// BUGBUG, error checking
			if (!encoding::get_chars(m_Codepage, { &Ch, 1 }, { pCh, 1 }))
			{
				LOGWARNING(L"get_chars({:02X})"sv, static_cast<int>(Ch));
			}
		}

		break;
	}

	return true;
}

wchar_t Viewer::vgetc_prev()
{
	const auto pos = vtell();
	if ( pos <= 0 )
		return {};

	const auto CharSize = getCharSize();
	if ( pos < CharSize )
		return encoding::replace_char;

	size_t BytesToRead;

	if (CharSize < 0) // UTF-8 or MB decoder
	{
		BytesToRead = (CharSize == -1? 4 : -CharSize); // -1 -- UTF8
		BytesToRead = std::min(BytesToRead, static_cast<size_t>(pos));
	}
	else
	{
		BytesToRead = static_cast<size_t>(CharSize);
	}

	size_t BytesRead = 0;
	char RawBuffer[4]{};
	if (vseek(-static_cast<int>(BytesToRead), FILE_CURRENT))
		Reader.Read(RawBuffer, BytesToRead, &BytesRead);

	vseek(pos, FILE_BEGIN);

	wchar_t Result = encoding::replace_char;
	if (BytesRead == BytesToRead)
	{
		switch (m_Codepage)
		{
		case CP_UTF16BE:
			Result = make_integer<wchar_t>(RawBuffer[1], RawBuffer[0]);
			break;

		case CP_UTF16LE:
			Result = make_integer<wchar_t>(RawBuffer[0], RawBuffer[1]);
			break;

		case CP_UTF8:
			{
				int tail = 0;
				wchar_t CharBuffer[4];
				const auto Length = Utf8::get_chars({ RawBuffer, BytesRead }, CharBuffer, tail);
				if (!tail && Length)
				{
					Result = CharBuffer[Length - 1];
				}
			}
			break;

		default:
			if (CharSize == 1)
			{
				// BUGBUG, error checking
				if (!encoding::get_chars(m_Codepage, { RawBuffer, 1 }, { &Result, 1 }))
				{
					LOGWARNING(L"get_chars({:02X})"sv, static_cast<int>(RawBuffer[0]));
				}
			}
			else
			{
				assert(MB.GetCP() == m_Codepage);
				for (const auto i: std::views::iota(0uz, BytesRead))
				{
					wchar_t Char;
					if (MB.GetChar({ RawBuffer + i, BytesRead - i }, Char) == BytesRead - i)
					{
						Result = Char;
						break;
					}
				}
			}
			break;
		}
	}
	return Result;
}

void Viewer::GoTo(bool ShowDlg, long long Offset, unsigned long long Flags)
{
	long long NewLeftPos = -1;

	int IsOffsetRelative = 0;

	if (ShowDlg)
	{
		if (!m_GotoHex)
		{
			m_GotoHex = m_DisplayMode == VMT_HEX;
		}

		goto_coord Row{};
		goto_coord Col{};
		if (!GoToRowCol(Row, Col, *m_GotoHex, L"ViewerGotoPos"sv))
			return;

		if (Row.exist)
		{
			Offset = Row.percent? FromPercent(Row.value, FileSize) : Row.value;
			IsOffsetRelative = Row.relative;
		}

		if (m_DisplayMode == VMT_TEXT && !m_Wrap && Col.exist)
		{
			NewLeftPos = Col.percent? 0 : Col.value;
			if (Col.relative)
				NewLeftPos = LeftPos + NewLeftPos * Row.relative;
		}
	}
	else
	{
		IsOffsetRelative = Flags & VSP_RELATIVE? Offset < 0? -1 : 1 : 0;

		if (Flags&VSP_PERCENT)
		{
			Offset = FromPercent(Offset, FileSize);
		}
	}

	FilePos = IsOffsetRelative? FilePos + Offset * IsOffsetRelative : Offset;
	FilePos = std::clamp(FilePos, 0ll, FileSize);

	AdjustFilePos();
	if (NewLeftPos != -1)
		LeftPos = NewLeftPos;

	if (!(Flags&VSP_NOREDRAW))
		Show();
}

void Viewer::AdjustFilePos()
{
	if (m_DisplayMode == VMT_TEXT)
	{
		wchar_t ch;
		FilePos -= FilePos % get_char_size(m_Codepage);

		vseek(FilePos, FILE_BEGIN);
		if (m_Codepage != CP_UTF8)
		{
			vgetc(&ch);
		}
		else
		{
			vgetc(nullptr);

			const auto Skip = [&]
			{
				if (VgetcCache.empty() || (VgetcCache.top() & 0xC0) != 0x80)
					return false;

				VgetcCache.pop();
				return true;
			};

			if (!VgetcCache.empty())
			{
				if (Skip())
				{
					Skip() && Skip();
				}
				else
				{
					vgetc(&ch);
				}
			}
		}

		FilePos = vtell();
		Up(1, true);
	}
}

void Viewer::SetFileSize()
{
	if (!ViewFile)
		return;

	unsigned long long uFileSize = 0; // BUGBUG, sign
	// BUGBUG check result
	if (!ViewFile.GetSize(uFileSize))
	{
		LOGWARNING(L"GetSize({}): {}"sv, ViewFile.GetName(), os::last_error());
	}

	FileSize=uFileSize;
}


void Viewer::GetSelectedParam(long long &Pos, long long &Length, DWORD &Flags) const
{
	Pos=SelectPos;
	Length=SelectSize;
	Flags=SelectFlags;
}

// Flags=0x01 - показывать [делать Show()]
//       0x02 - "обратный поиск" ?
//
void Viewer::SelectText(const long long& MatchPos,const long long& SearchLength, const DWORD Flags)
{
	if (!ViewFile)
		return;

	SelectPos = MatchPos;
	SelectSize = SearchLength;
	SelectFlags = Flags;
	if ( SelectSize < 0 )
		return;

	if (m_DisplayMode == VMT_TEXT)
	{
		FilePos = SelectPos;
		Up(1, true);
		LeftPos = 0;

		if ( !m_Wrap )
		{
			vseek(vString.nFilePos = FilePos, FILE_BEGIN);
			vString.Data.clear();
			ReadString(&vString, static_cast<int>(SelectPos - FilePos), false);

			if ( vString.eol_length == 0 )
			{
				const auto found_offset = static_cast<int>(vString.Data.size());
				if ( found_offset > ScrollbarAdjustedWidth-10 )
					LeftPos = (ScrollbarAdjustedWidth <= 10 ? found_offset : found_offset + 10 - ScrollbarAdjustedWidth);
			}
		}
	}
	else
	{
		const auto LineSize = GetModeDependentLineSize();
		FilePos = (FilePos % LineSize) + LineSize * (SelectPos / LineSize);
		FilePos = (FilePos < SelectPos? FilePos : (FilePos > LineSize? FilePos - LineSize : 0));
	}

	if (Flags & 1)
	{
		AdjustSelPosition = true;
		Show();
		AdjustSelPosition = false;
	}
}


int Viewer::ViewerControl(int Command, intptr_t Param1, void *Param2)
{
	switch (Command)
	{
		case VCTL_GETINFO:
		{
			const auto Info=static_cast<ViewerInfo*>(Param2);
			if (CheckStructSize(Info))
			{
				std::memset(&Info->ViewerID,0,Info->StructSize-sizeof(Info->StructSize));
				Info->ViewerID = ViewerID;
				Info->WindowSizeX=ObjWidth();
				Info->WindowSizeY = m_Where.height();
				Info->FilePos=FilePos;
				Info->FileSize=FileSize;
				Info->CurMode.CodePage=m_Codepage;
				Info->CurMode.Flags=0;
				if (m_Wrap) Info->CurMode.Flags|=VMF_WRAP;
				if (m_WordWrap) Info->CurMode.Flags|=VMF_WORDWRAP;
				Info->CurMode.ViewMode = m_DisplayMode;
				Info->Options=0;

				if (Global->Opt->ViOpt.SavePos)
					Info->Options|=VOPT_SAVEFILEPOSITION;

				if (ViOpt.AutoDetectCodePage)
					Info->Options|=VOPT_AUTODETECTCODEPAGE;

				if (ViOpt.ShowScrollbar)
					Info->Options |= VOPT_SHOWSCROLLBAR;

				if (m_bQuickView)
					Info->Options |= VOPT_QUICKVIEW;

				Info->TabSize=ViOpt.TabSize;
				Info->LeftPos=LeftPos;
				return TRUE;
			}

			break;
		}
		/*
		   Param2 = ViewerSetPosition
		           сюда же будет записано новое смещение
		           В основном совпадает с переданным
		*/
		case VCTL_SETPOSITION:
		{
			const auto vsp = static_cast<ViewerSetPosition*>(Param2);
			if (CheckStructSize(vsp))
			{
				const auto isReShow = vsp->StartPos != FilePos;

				if ((LeftPos=vsp->LeftPos) < 0)
					LeftPos=0;

				GoTo(false, vsp->StartPos, vsp->Flags);

				if (isReShow && !(vsp->Flags&VSP_NOREDRAW))
					Global->ScrBuf->Flush();

				if (!(vsp->Flags&VSP_NORETNEWPOS))
				{
					vsp->StartPos=FilePos;
					vsp->LeftPos=LeftPos;
				}

				return TRUE;
			}

			break;
		}
		// Param2=ViewerSelect
		case VCTL_SELECT:
		{
			const auto vs = static_cast<const ViewerSelect*>(Param2);
			if (CheckStructSize(vs))
			{
				const auto SPos = vs->BlockStartPos;
				int SSize=vs->BlockLen;

				if (SPos < FileSize)
				{
					if (SPos+SSize > FileSize)
					{
						SSize = static_cast<int>(FileSize - SPos);
					}

					SelectText(SPos,SSize,0x1);
					Global->ScrBuf->Flush();
					return TRUE;
				}
			}
			else if (!Param2)
			{
				SelectSize = -1;
				Show();
			}

			break;
		}
		/* Функция установки Keybar Labels
		     Param2 = nullptr - восстановить, пред. значение
		     Param2 = -1   - обновить полосу (перерисовать)
		     Param2 = KeyBarTitles
		*/
		case VCTL_SETKEYBAR:
		{
			const auto Kbt = static_cast<const FarSetKeyBarTitles*>(Param2);

			if (!Kbt)
			{        // восстановить пред значение!
				if (HostFileViewer)
					HostFileViewer->InitKeyBar();
			}
			else
			{
				if (std::bit_cast<intptr_t>(Param2) != -1) // не только перерисовать?
				{
					if(CheckStructSize(Kbt))
						m_ViewKeyBar->Change(Kbt->Titles);
					else
						return FALSE;
				}
				m_ViewKeyBar->Show();
				Global->ScrBuf->Flush(); //?????
			}

			return TRUE;
		}
		// Param2=0
		case VCTL_REDRAW:
		{
			Global->WindowManager->RefreshWindow(GetOwner());
			Global->WindowManager->PluginCommit();
			Global->ScrBuf->Flush();
			return TRUE;
		}
		// Param2=0
		case VCTL_QUIT:
		{
			/* $ 28.12.2002 IS
			   Разрешаем выполнение VCTL_QUIT только для viewer-а, который
			   не является панелью информации и быстрого просмотра (т.е.
			   фактически панелей на экране не видно)
			*/
			if (!Global->WindowManager->IsPanelsActive())
			{
				/* $ 29.09.2002 IS
				   без этого не закрывался viewer, а просили именно это
				*/
				if (HostFileViewer)
				{
					Global->WindowManager->DeleteWindow(HostFileViewer->shared_from_this());
					HostFileViewer->SetExitCode(0);
					Global->WindowManager->PluginCommit();
				}

				return TRUE;
			}
		}
		[[fallthrough]];
		/* Функция установки режимов
		     Param2 = ViewerSetMode
		*/
		case VCTL_SETMODE:
		{
			const auto vsmode = static_cast<const ViewerSetMode*>(Param2);

			if (CheckStructSize(vsmode))
			{
				const auto isRedraw = (vsmode->Flags&VSMFL_REDRAW) != 0;

				switch (vsmode->Type)
				{
					case VSMT_VIEWMODE:
						return ProcessDisplayMode(static_cast<VIEWER_MODE_TYPE>(vsmode->iParam), isRedraw);
					case VSMT_WRAP:
						ProcessWrapMode(vsmode->iParam,isRedraw);
						return TRUE;
					case VSMT_WORDWRAP:
						ProcessTypeWrapMode(vsmode->iParam,isRedraw);
						return TRUE;
				}
			}

			return FALSE;
		}
		case VCTL_GETFILENAME:
		{
			if (Param2 && static_cast<size_t>(Param1) > strFullFileName.size())
			{
				*copy_string(strFullFileName, static_cast<wchar_t*>(Param2)) = {};
			}

			return static_cast<int>(strFullFileName.size()+1);
		}
	}

	return FALSE;
}

bool Viewer::ProcessDisplayMode(VIEWER_MODE_TYPE newMode, bool isRedraw)
{
	switch (newMode)
	{
	case VMT_TEXT:
		m_DisplayMode = VMT_TEXT;
		break;

	case VMT_HEX:
		m_DisplayMode = VMT_HEX;
		break;

	case VMT_DUMP:
		m_DisplayMode = VMT_DUMP;
		break;

	default:
		return false;
	}

	AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	return true;
}

int Viewer::ProcessWrapMode(int newMode, bool isRedraw)
{
	const auto oldWrap = m_Wrap;
	m_Wrap=newMode&1;

	if (m_Wrap)
		LeftPos = 0;

	AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	Global->Opt->ViOpt.ViewerIsWrap = m_Wrap != 0;
	return oldWrap;
}

int Viewer::ProcessTypeWrapMode(int newMode, bool isRedraw)
{
	const auto oldTypeWrap = m_WordWrap;
	m_WordWrap=newMode&1;

	if (!m_Wrap)
	{
		m_Wrap=!m_Wrap;
		LeftPos = 0;
	}

	AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	Global->Opt->ViOpt.ViewerWrap = m_WordWrap;
	return oldTypeWrap;
}

uintptr_t Viewer::GetDefaultCodePage()
{
	const auto cp = encoding::codepage::normalise(Global->Opt->ViOpt.DefaultCodePage);
	return cp == CP_DEFAULT || !is_code_page_supported_in_viewer(cp)?
		encoding::codepage::ansi() :
		cp;
}

void Viewer::ReadEvent()
{
	Global->CtrlObject->Plugins->ProcessViewerEvent(VE_READ,nullptr, this);
	bVE_READ_Sent = true;
}

void Viewer::CloseEvent()
{
	if (!OpenFailed && bVE_READ_Sent)
	{
		bVE_READ_Sent=false;
		Global->CtrlObject->Plugins->ProcessViewerEvent(VE_CLOSE,nullptr, this);
	}
}

void Viewer::OnDestroy()
{
	CloseEvent();
}

void Viewer::vgetc_cache::compact()
{
	m_End = std::copy(m_Iterator, m_End, m_Buffer);
	m_Iterator = m_Buffer;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("viewer.hex_mode_bytes_area_width")
{
	struct test_data
	{
		int Bytes;
		int Width;
	};

	static const test_data TestDataPoints[] =
	{
		{  8,  24 }, // > 2D 2D 2D 2D 2D 2D 2D 2D<
		{  9,  29 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D<
		{ 10,  32 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D<
		{ 13,  41 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D<
		{ 15,  47 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D<
		{ 16,  50 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D<
		{ 17,  55 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D<
		{ 20,  64 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D<
		{ 24,  76 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D<
		{ 25,  81 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D<
		{ 30,  96 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D<
		{ 32, 102 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D<
		{ 33, 107 }, // > 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D 2D 2D 2D 2D 2D 2D 2D │ 2D<
	};

	for (const auto& TestDataPoint : TestDataPoints)
	{
		REQUIRE(TestDataPoint.Width == hex_mode_bytes_area_wdth(TestDataPoint.Bytes));
	}
}

TEST_CASE("viewer.hex_mode_bytes_from_line_width")
{
	for (const auto CharSize : { 1, 2 })
	{
		for (const auto Bytes: std::views::iota(8, 260))
		{
			for (const auto LineWidth: std::views::iota(hex_mode_line_width_from_bytes(Bytes, CharSize), hex_mode_line_width_from_bytes(Bytes + 1, CharSize)))
			{
				REQUIRE(Bytes == hex_mode_bytes_from_line_width(LineWidth, CharSize));
			}
		}
	}
}

static int HexModeByteFromColumn16BytesFixed(const int Column, const int CharSize)
{
/*
0000000000: 32 30 2E 30 31 2E 32 30 | 31 35 20 31 30 3A 33 39  20.01.2015 10:39 - 1-byte
0000000020: 31 00 2E 00 30 00 22 00 | 20 00 65 00 6E 00 63 00  1.0" enc         - 2-byte
*/
	if (Column < 11) return 0;
	if (Column < 35) return (Column - 11) / 3;
	if (Column < 37) return s_HexModeBytesPerStripe;
	if (Column < 61) return s_HexModeBytesPerStripe + (Column - 37) / 3;
	if (Column < 63) return 0;
	if (Column < 63 + 16 / CharSize) return (Column - 63) * CharSize;
	return 16;
}

TEST_CASE("viewer.hex_mode_byte_from_column")
{
	for (const auto CharSize : { 1, 2 })
	{
		for (const auto Column: std::views::iota(0, 100))
		{
			REQUIRE(HexModeByteFromColumn16BytesFixed(Column, CharSize) == hex_mode_byte_from_column(Column, 16, CharSize));
		}
	}
}

#endif

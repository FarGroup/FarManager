/*
qview.cpp

Quick view panel
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

#include "qview.hpp"

#include "macroopcode.hpp"
#include "flink.hpp"
#include "farcolor.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "help.hpp"
#include "viewer.hpp"
#include "interf.hpp"
#include "execute.hpp"
#include "dirinfo.hpp"
#include "pathmix.hpp"
#include "mix.hpp"
#include "colormix.hpp"
#include "lang.hpp"
#include "keybar.hpp"
#include "strmix.hpp"
#include "keyboard.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "datetime.hpp"
#include "global.hpp"

#include "platform.fs.hpp"
#include "platform.reg.hpp"

#include "format.hpp"

static bool LastMode = false;
static bool LastWrapMode = false;
static bool LastWrapType = false;

qview_panel_ptr QuickView::create(window_ptr Owner)
{
	return std::make_shared<QuickView>(private_tag(), Owner);
}

QuickView::QuickView(private_tag, window_ptr Owner):
	Panel(std::move(Owner)),
	Data(),
	OldWrapMode(false),
	OldWrapType(false),
	m_TemporaryFile(),
	uncomplete_dirscan(false)
{
	m_Type = panel_type::QVIEW_PANEL;
	if (!LastMode)
	{
		LastMode = true;
		LastWrapMode = Global->Opt->ViOpt.ViewerIsWrap;
		LastWrapType = Global->Opt->ViOpt.ViewerWrap;
	}
}


QuickView::~QuickView()
{
	QuickView::CloseFile();
}


string QuickView::GetTitle() const
{
	return msg(lng::MQuickViewTitle);
}

void QuickView::DisplayObject()
{
	if (m_Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	m_Flags.Set(FSCROBJ_ISREDRAWING);

	if (!QView && !ProcessingPluginCommand)
		Parent()->GetAnotherPanel(this)->UpdateViewPanel();

	if (QView)
		QView->SetPosition(m_X1+1,m_Y1+1,m_X2-1,m_Y2-3);

	Box(m_X1,m_Y1,m_X2,m_Y2,colors::PaletteColorToFarColor(COL_PANELBOX),DOUBLE_BOX);
	SetScreen(m_X1+1,m_Y1+1,m_X2-1,m_Y2-1,L' ',colors::PaletteColorToFarColor(COL_PANELTEXT));
	SetColor(IsFocused()? COL_PANELSELECTEDTITLE:COL_PANELTITLE);

	const auto& strTitle = GetTitleForDisplay();
	if (!strTitle.empty())
	{
		GotoXY(m_X1+(m_X2-m_X1+1-(int)strTitle.size())/2,m_Y1);
		Text(strTitle);
	}

	DrawSeparator(m_Y2-2);
	SetColor(COL_PANELTEXT);
	GotoXY(m_X1+1,m_Y2-1);
	Text(fit_to_left(string(PointToName(strCurFileName)), m_X2 - m_X1 - 1));

	if (!strCurFileType.empty())
	{
		auto strTypeText = concat(L' ', strCurFileType, L' ');
		TruncStr(strTypeText,m_X2-m_X1-1);
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(m_X1+(m_X2-m_X1+1-(int)strTypeText.size())/2,m_Y2-2);
		Text(strTypeText);
	}

	if (m_DirectoryScanStatus != scan_status::none)
	{
		SetColor(COL_PANELTEXT);
		GotoXY(m_X1+2,m_Y1+2);
		auto DisplayName = strCurFileName;
		TruncPathStr(DisplayName, std::max(0, m_X2 - m_X1 - 1 - static_cast<int>(msg(lng::MQuickViewFolder).size() - 5)));
		PrintText(format(LR"({0} "{1}")", msg(lng::MQuickViewFolder), DisplayName));

		const auto currAttr = os::fs::get_file_attributes(strCurFileName); // обламывается, если нет доступа
		if (currAttr != INVALID_FILE_ATTRIBUTES && (currAttr&FILE_ATTRIBUTE_REPARSE_POINT))
		{
			string Target;
			DWORD ReparseTag=0;
			string TypeName;
			if (GetReparsePointInfo(strCurFileName, Target, &ReparseTag))
			{
				NormalizeSymlinkName(Target);
				switch(ReparseTag)
				{
				// 0xA0000003L = Directory Junction or Volume Mount Point
				case IO_REPARSE_TAG_MOUNT_POINT:
					{
						auto ID_Msg = lng::MQuickViewJunction;
						bool Root;
						if(ParsePath(Target, nullptr, &Root) == root_type::volume && Root)
						{
							ID_Msg = lng::MQuickViewVolMount;
						}
						TypeName = msg(ID_Msg);
					}
					break;
				// 0xA000000CL = Directory or File Symbolic Link
				case IO_REPARSE_TAG_SYMLINK:
					TypeName = msg(lng::MQuickViewSymlink);
					break;
				// 0x8000000AL = Distributed File System
				case IO_REPARSE_TAG_DFS:
					TypeName = msg(lng::MQuickViewDFS);
					break;
				// 0x80000012L = Distributed File System Replication
				case IO_REPARSE_TAG_DFSR:
					TypeName = msg(lng::MQuickViewDFSR);
					break;
				// 0xC0000004L = Hierarchical Storage Management
				case IO_REPARSE_TAG_HSM:
					TypeName = msg(lng::MQuickViewHSM);
					break;
				// 0x80000006L = Hierarchical Storage Management2
				case IO_REPARSE_TAG_HSM2:
					TypeName = msg(lng::MQuickViewHSM2);
					break;
				// 0x80000007L = Single Instance Storage
				case IO_REPARSE_TAG_SIS:
					TypeName = msg(lng::MQuickViewSIS);
					break;
				// 0x80000008L = Windows Imaging Format
				case IO_REPARSE_TAG_WIM:
					TypeName = msg(lng::MQuickViewWIM);
					break;
				// 0x80000009L = Cluster Shared Volumes
				case IO_REPARSE_TAG_CSV:
					TypeName = msg(lng::MQuickViewCSV);
					break;
				case IO_REPARSE_TAG_DEDUP:
					TypeName = msg(lng::MQuickViewDEDUP);
					break;
				case IO_REPARSE_TAG_NFS:
					TypeName = msg(lng::MQuickViewNFS);
					break;
				case IO_REPARSE_TAG_FILE_PLACEHOLDER:
					TypeName = msg(lng::MQuickViewPlaceholder);
					break;
					// 0x????????L = anything else
				default:
					if (Global->Opt->ShowUnknownReparsePoint)
					{
						TypeName = format(L":{0:0>8X}", ReparseTag);
					}
					else
					{
						TypeName = msg(lng::MQuickViewUnknownReparsePoint);
					}
				}
			}
			else
			{
				TypeName = msg(lng::MQuickViewUnknownReparsePoint);
				Target = msg(lng::MQuickViewNoData);
			}

			TruncPathStr(Target, std::max(0, m_X2 - m_X1 - 1 - static_cast<int>(TypeName.size()) - 5));
			SetColor(COL_PANELTEXT);
			GotoXY(m_X1+2,m_Y1+3);
			PrintText(format(LR"({0} "{1}")", TypeName, Target));
		}

		const auto bytes_suffix = upper(msg(lng::MListBytes));
		const auto& size2str = [&bytes_suffix](ULONGLONG Size)
		{
			if (Global->Opt->ShowBytes)
			{
				return GroupDigits(Size); // + L' ' + bytes_suffix;
			}
			else
			{
				const auto str = trim(FileSizeToStr(Size, 10, COLUMN_FLOATSIZE | COLUMN_SHOWUNIT));
				return str.back() == bytes_suffix[0]? str : str + bytes_suffix;
			}
		};

		if (m_DirectoryScanStatus == scan_status::real_ok || m_DirectoryScanStatus == scan_status::plugin_ok)
		{
			static const lng TableLabels[] =
			{
				lng::MQuickViewFolders,
				lng::MQuickViewFiles,
				lng::MQuickViewBytes,
				lng::MQuickViewAllocated,
				lng::MQuickViewCluster,
				lng::MQuickViewSlack,
			};

			const auto ColumnSize = msg(*std::max_element(ALL_CONST_RANGE(TableLabels), [](lng a, lng b) { return msg(a).size() < msg(b).size(); })).size() + 1;

			const auto iColor = uncomplete_dirscan? COL_PANELHIGHLIGHTTEXT : COL_PANELINFOTEXT;
			const auto prefix = uncomplete_dirscan? L"~" : L"";

			const auto& PrintRow = [&](int Y, lng Id, string_view const Value)
			{
				GotoXY(m_X1 + 2, m_Y1 + Y);
				SetColor(COL_PANELTEXT);
				PrintText(pad_right(msg(Id), ColumnSize));

				SetColor(iColor);
				PrintText(prefix);
				PrintText(Value);
			};

			PrintRow(4, lng::MQuickViewFolders, str(Data.DirCount));
			PrintRow(5, lng::MQuickViewFiles, str(Data.FileCount));
			PrintRow(6, lng::MQuickViewBytes, size2str(Data.FileSize));

			const auto Format = L"{0} ({1}%)";
			PrintRow(7, lng::MQuickViewAllocated, format(Format, size2str(Data.AllocationSize), ToPercent(Data.AllocationSize, Data.FileSize)));

			if (m_DirectoryScanStatus == scan_status::real_ok)
			{
				PrintRow(9, lng::MQuickViewCluster, GroupDigits(Data.ClusterSize));
				PrintRow(10, lng::MQuickViewSlack, format(Format, size2str(Data.FilesSlack), ToPercent(Data.FilesSlack, Data.AllocationSize)));
				PrintRow(11, lng::MQuickViewMFTOverhead, format(Format, size2str(Data.MFTOverhead), ToPercent(Data.MFTOverhead, Data.AllocationSize)));
			}
		}
	}
	else if (QView)
	{
		QView->Show();
	}

	m_Flags.Clear(FSCROBJ_ISREDRAWING);
}

Viewer* QuickView::GetViewer()
{
	return QView.get();
}


long long QuickView::VMProcess(int OpCode, void* vParam, long long iParam)
{
	if (m_DirectoryScanStatus == scan_status::none && QView)
		return QView->VMProcess(OpCode,vParam,iParam);

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return 1;
	}

	return 0;
}

bool QuickView::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (!IsVisible())
		return false;

	if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
	{
		ExecShortcutFolder(LocalKey-KEY_RCTRL0);
		return true;
	}

	if (LocalKey == KEY_F1)
	{
		Help::create(L"QViewPanel");
		return true;
	}

	if (LocalKey==KEY_F3 || LocalKey==KEY_NUMPAD5 || LocalKey == KEY_SHIFTNUMPAD5)
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
			AnotherPanel->ProcessKey(Manager::Key(KEY_F3));

		return true;
	}

	if (LocalKey==KEY_ADD || LocalKey==KEY_SUBTRACT || LocalKey == KEY_SHIFTDOWN || LocalKey==KEY_SHIFTUP)
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
			AnotherPanel->ProcessKey(Manager::Key((LocalKey==KEY_ADD||LocalKey==KEY_SHIFTDOWN)?KEY_DOWN:KEY_UP));

		return true;
	}

	if (QView && m_DirectoryScanStatus == scan_status::none && LocalKey>=256)
	{
		const auto ret = QView->ProcessKey(Manager::Key(LocalKey));

		if (LocalKey == KEY_F2 || LocalKey == KEY_SHIFTF2
		 || LocalKey == KEY_F4 || LocalKey == KEY_SHIFTF4
		 || LocalKey == KEY_F8 || LocalKey == KEY_SHIFTF8)
		{
			DynamicUpdateKeyBar();
			Parent()->GetKeybar().Redraw();
		}

		if (LocalKey == KEY_F7 || LocalKey == KEY_SHIFTF7)
		{
			//long long Pos;
			//int Length;
			//DWORD Flags;
			//QView->GetSelectedParam(Pos,Length,Flags);
			Redraw();
			Parent()->GetAnotherPanel(this)->Redraw();
			//QView->SelectText(Pos,Length,Flags|1);
		}

		return ret;
	}

	return false;
}


bool QuickView::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsMouseInClientArea(MouseEvent))
		return false;

	if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		return false;

	Parent()->SetActivePanel(this);

	if (QView && m_DirectoryScanStatus == scan_status::none)
		return QView->ProcessMouse(MouseEvent);

	return false;
}

void QuickView::Update(int Mode)
{
	if (!m_EnableUpdate)
		return;

	if (strCurFileName.empty())
		Parent()->GetAnotherPanel(this)->UpdateViewPanel();

	Redraw();
}

void QuickView::ShowFile(const string& FileName, bool const TempFile, const plugin_panel* const hDirPlugin)
{
	CloseFile();

	if (!IsVisible())
		return;

	if (FileName.empty())
	{
		ProcessingPluginCommand++;
		Show();
		ProcessingPluginCommand--;
		return;
	}

	m_CurDir = Parent()->GetAnotherPanel(this)->GetCurDir();

	auto FileFullName = hDirPlugin? FileName : ConvertNameToFull(FileName);
	bool SameFile = strCurFileName == FileFullName;
	strCurFileName = FileFullName;

	size_t pos = strCurFileName.rfind(L'.');
	if (pos != string::npos)
	{
		string strValue;

		if (GetShellType(string_view(strCurFileName).substr(pos), strValue))
		{
			os::reg::key::classes_root.get(strValue, L"", strCurFileType);
		}
	}

	if (hDirPlugin || os::fs::is_directory(strCurFileName))
	{
		// Не показывать тип файла для каталогов в "Быстром просмотре"
		strCurFileType.clear();

		if (SameFile && !hDirPlugin)
		{
			m_DirectoryScanStatus = scan_status::real_ok;
		}
		else if (hDirPlugin)
		{
			const auto Result = GetPluginDirInfo(hDirPlugin, strCurFileName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize);
			m_DirectoryScanStatus = Result? scan_status::plugin_ok : scan_status::plugin_fail;
			uncomplete_dirscan = !Result;
		}
		else
		{
			time_check TimeCheck(time_check::mode::delayed, GetRedrawTimeout());

			const auto& DirInfoCallback = [&](string_view const Name, unsigned long long const Items, unsigned long long const Size) mutable
			{
				if (TimeCheck)
					DirInfoMsg(msg(lng::MQuickViewTitle), Name, Items, Size);
			};

			const auto ExitCode = GetDirInfo(strCurFileName, Data, nullptr, DirInfoCallback, GETDIRINFO_ENHBREAK | GETDIRINFO_SCANSYMLINKDEF);
			m_DirectoryScanStatus = ExitCode == -1? scan_status::real_fail : scan_status::real_ok; // ExitCode: 1=done; 0=Esc,CtrlBreak; -1=Other
			uncomplete_dirscan = ExitCode != 1;

			if (const auto Window = m_Owner.lock())
			{
				Window->Redraw();
			}
		}
	}
	else
	{
		if (!strCurFileName.empty())
		{
			QView = std::make_unique<Viewer>(GetOwner(), true);
			QView->SetRestoreScreenMode(false);
			QView->SetPosition(m_X1+1,m_Y1+1,m_X2-1,m_Y2-3);
			QView->SetStatusMode(0);
			QView->EnableHideCursor(0);
			OldWrapMode = QView->GetWrapMode();
			OldWrapType = QView->GetWrapType();
			QView->SetWrapMode(LastWrapMode);
			QView->SetWrapType(LastWrapType);
			QView->OpenFile(strCurFileName,FALSE);
		}
	}

	m_TemporaryFile = TempFile;

	Redraw();

	if (IsFocused())
	{
		DynamicUpdateKeyBar();
		Parent()->GetKeybar().Redraw();
	}
}


void QuickView::CloseFile()
{
	if (QView)
	{
		LastWrapMode=QView->GetWrapMode();
		LastWrapType=QView->GetWrapType();
		QView->SetWrapMode(OldWrapMode);
		QView->SetWrapType(OldWrapType);
		QView=nullptr;
	}

	strCurFileType.clear();
	QViewDelTempName();
	m_DirectoryScanStatus = scan_status::none;
}


void QuickView::QViewDelTempName()
{
	if (m_TemporaryFile)
	{
		if (QView)
		{
			LastWrapMode=QView->GetWrapMode();
			LastWrapType=QView->GetWrapType();
			QView->SetWrapMode(OldWrapMode);
			QView->SetWrapType(OldWrapType);
			QView=nullptr;
		}

		os::fs::set_file_attributes(strCurFileName, FILE_ATTRIBUTE_ARCHIVE);
		os::fs::delete_file(strCurFileName);  //BUGBUG
		string TempDirectoryName = strCurFileName;
		CutToSlash(TempDirectoryName);
		os::fs::remove_directory(TempDirectoryName);
		m_TemporaryFile = false;
	}
}


void QuickView::PrintText(string_view const Str) const
{
	if (WhereY()>m_Y2-3 || WhereX()>m_X2-2)
		return;

	Text(cut_right(Str, m_X2 - 2 - WhereX() + 1));
}


bool QuickView::UpdateIfChanged(bool Idle)
{
	if (IsVisible() && !strCurFileName.empty() && m_DirectoryScanStatus == scan_status::real_fail)
	{
		const auto strViewName = strCurFileName;
		ShowFile(strViewName, m_TemporaryFile, nullptr);
		return true;
	}

	return false;
}

void QuickView::RefreshTitle()
{
	m_Title = L'{';
	if (!strCurFileName.empty())
	{
		append(m_Title, strCurFileName, L" - "sv);
	}
	append(m_Title, msg(lng::MQuickViewTitle), L'}');
}

bool QuickView::GetCurName(string &strName, string &strShortName) const
{
	if (strCurFileName.empty())
		return false;

	strName = strCurFileName;
	strShortName = strName;
	return true;
}

void QuickView::UpdateKeyBar()
{
	Parent()->GetKeybar().SetLabels(lng::MQViewF1);
	DynamicUpdateKeyBar();
}

void QuickView::DynamicUpdateKeyBar() const
{
	auto& Keybar = Parent()->GetKeybar();

	if (m_DirectoryScanStatus != scan_status::none || !QView)
	{
		Keybar[KBL_MAIN][F2] = msg(lng::MF2);
		Keybar[KBL_MAIN][F4].clear();
		Keybar[KBL_MAIN][F8].clear();
		Keybar[KBL_SHIFT][F2].clear();
		Keybar[KBL_SHIFT][F8].clear();
		Keybar[KBL_ALT][F8] = msg(lng::MAltF8);  // стандартный для панели - "хистори"
	}
	else
	{
		QView->UpdateViewKeyBar(Keybar);
	}

	Keybar.SetCustomLabels(KBA_QUICKVIEW);
}

Viewer* QuickView::GetById(int ID)
{
	return QView && ID==QView->GetId()?GetViewer():nullptr;
}

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "qview.hpp"

// Internal:
#include "macroopcode.hpp"
#include "flink.hpp"
#include "farcolor.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "help.hpp"
#include "viewer.hpp"
#include "interf.hpp"
#include "imports.hpp"
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

// Platform:
#include "platform.fs.hpp"
#include "platform.reg.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

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
		QView->SetPosition({ m_Where.left + 1, m_Where.top + 1, m_Where.right - 1, m_Where.bottom - 3 });

	Box(m_Where, colors::PaletteColorToFarColor(COL_PANELBOX), DOUBLE_BOX);
	SetScreen({ m_Where.left + 1, m_Where.top + 1, m_Where.right - 1, m_Where.bottom - 1 }, L' ', colors::PaletteColorToFarColor(COL_PANELTEXT));
	SetColor(IsFocused()? COL_PANELSELECTEDTITLE:COL_PANELTITLE);

	const auto& strTitle = GetTitleForDisplay();
	if (!strTitle.empty())
	{
		GotoXY(m_Where.left + (m_Where.width() - static_cast<int>(strTitle.size())) / 2, m_Where.top);
		Text(strTitle);
	}

	DrawSeparator(m_Where.bottom - 2);
	SetColor(COL_PANELTEXT);
	GotoXY(m_Where.left + 1, m_Where.bottom - 1);
	Text(fit_to_left(string(PointToName(strCurFileName)), m_Where.width() - 2));

	if (!strCurFileType.empty())
	{
		const auto strTypeText = truncate_left(concat(L' ', strCurFileType, L' '), m_Where.width() - 2);
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(m_Where.left + (m_Where.width() - static_cast<int>(strTypeText.size())) / 2, m_Where.bottom - 2);
		Text(strTypeText);
	}

	if (m_DirectoryScanStatus != scan_status::none)
	{
		SetColor(COL_PANELTEXT);
		GotoXY(m_Where.left + 2, m_Where.top + 2);
		const auto DisplayName = truncate_path(strCurFileName, std::max(size_t{}, m_Where.width() - 2 - msg(lng::MListFolder).size() - 5));
		PrintText(format(FSTR(LR"({0} "{1}")"), msg(lng::MListFolder), DisplayName));

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
				// Directory Junction or Volume Mount Point
				case IO_REPARSE_TAG_MOUNT_POINT:
					{
						auto ID_Msg = lng::MListJunction;
						bool Root;
						if(ParsePath(Target, nullptr, &Root) == root_type::volume && Root)
						{
							ID_Msg = lng::MListVolMount;
						}
						TypeName = msg(ID_Msg);
					}
					break;

				default:
					if (!reparse_tag_to_string(ReparseTag, TypeName) && !Global->Opt->ShowUnknownReparsePoint)
						TypeName = msg(lng::MQuickViewUnknownReparsePoint);
					break;
				}
			}
			else
			{
				TypeName = msg(lng::MQuickViewUnknownReparsePoint);
				Target = msg(lng::MQuickViewNoData);
			}

			inplace::truncate_path(Target, std::max(size_t{}, m_Where.width() - 2 - TypeName.size() - 5));
			SetColor(COL_PANELTEXT);
			GotoXY(m_Where.left + 2, m_Where.top + 3);
			PrintText(format(FSTR(LR"({0} "{1}")"), TypeName, Target));
		}

		const auto bytes_suffix = upper(msg(lng::MListBytes));
		const auto size2str = [&bytes_suffix](uint64_t const Size)
		{
			if (Global->Opt->ShowBytes)
			{
				return GroupDigits(Size); // + L' ' + bytes_suffix;
			}
			else
			{
				const auto str = trim(FileSizeToStr(Size, 10, COLFLAGS_FLOATSIZE | COLFLAGS_SHOW_MULTIPLIER));
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
			const auto prefix = uncomplete_dirscan? L"~"sv : L""sv;

			const auto PrintRow = [&](int Y, lng Id, string_view const Value)
			{
				GotoXY(m_Where.left + 2, m_Where.top + Y);
				SetColor(COL_PANELTEXT);
				PrintText(pad_right(msg(Id), ColumnSize));

				SetColor(iColor);
				PrintText(prefix);
				PrintText(Value);
			};

			PrintRow(4, lng::MQuickViewFolders, str(Data.DirCount));
			PrintRow(5, lng::MQuickViewFiles, str(Data.FileCount));
			PrintRow(6, lng::MQuickViewBytes, size2str(Data.FileSize));

			const auto Format = FSTR(L"{0} ({1}%)");
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
		help::show(L"QViewPanel"sv);
		return true;
	}

	if (any_of(LocalKey, KEY_F3, KEY_NUMPAD5, KEY_SHIFTNUMPAD5))
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
			AnotherPanel->ProcessKey(Manager::Key(KEY_F3));

		return true;
	}

	if (any_of(LocalKey, KEY_ADD, KEY_SUBTRACT))
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
			AnotherPanel->ProcessKey(Manager::Key(LocalKey==KEY_ADD?KEY_DOWN:KEY_UP));

		return true;
	}

	if (QView && m_DirectoryScanStatus == scan_status::none && LocalKey>=256)
	{
		const auto ret = QView->ProcessKey(Manager::Key(LocalKey));

		if (any_of(LocalKey, KEY_F2, KEY_SHIFTF2, KEY_F4, KEY_SHIFTF4, KEY_F8, KEY_SHIFTF8))
		{
			DynamicUpdateKeyBar();
			Parent()->GetKeybar().Redraw();
		}

		if (any_of(LocalKey, KEY_F7, KEY_SHIFTF7))
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

static bool IsProperProgID(string_view const ProgID)
{
	return !ProgID.empty() && os::reg::key::open(os::reg::key::classes_root, ProgID, KEY_QUERY_VALUE);
}

static bool GetShellType(const string_view Ext, string& strType)
{
	if (imports.SHCreateAssociationRegistration)
	{
		os::com::ptr<IApplicationAssociationRegistration> AAR;
		if (FAILED(imports.SHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, IID_PPV_ARGS_Helper(&ptr_setter(AAR)))))
			return false;

		os::com::memory<wchar_t*> Association;
		if (FAILED(AAR->QueryCurrentDefault(null_terminated(Ext).c_str(), AT_FILEEXTENSION, AL_EFFECTIVE, &ptr_setter(Association))))
			return false;

		strType = Association.get();
		return true;
	}

	if (const auto UserKey = os::reg::key::open(os::reg::key::current_user, concat(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"sv, Ext), KEY_QUERY_VALUE))
	{
		if (string Value; UserKey.get(L"ProgId"sv, Value) && IsProperProgID(Value))
		{
			strType = std::move(Value);
			return true;
		}

		if (string Value; UserKey.get(L"Application"sv, Value))
		{
			if (auto ProgId = L"Applications\\"sv + Value; IsProperProgID(ProgId))
			{
				strType = std::move(ProgId);
				return true;
			}
		}
	}

	if (const auto CRKey = os::reg::key::open(os::reg::key::classes_root, Ext, KEY_QUERY_VALUE))
	{
		if (string Value; CRKey.get({}, Value) && IsProperProgID(Value))
		{
			strType = std::move(Value);
			return true;
		}
	}

	return false;
}

void QuickView::ShowFile(string_view const FileName, const UserDataItem* const UserData, bool const TempFile, const plugin_panel* const hDirPlugin)
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

	const auto FileFullName = hDirPlugin? string(FileName) : ConvertNameToFull(FileName);
	const auto SameFile = strCurFileName == FileFullName;
	strCurFileName = FileFullName;
	CurUserData = {};
	if (UserData)
		CurUserData = *UserData;

	const auto pos = strCurFileName.rfind(L'.');
	if (pos != string::npos)
	{
		string strValue;

		if (GetShellType(string_view(strCurFileName).substr(pos), strValue))
		{
			// BUGBUG check result
			(void)os::reg::key::classes_root.get(strValue, {}, strCurFileType);
		}
	}

	if (hDirPlugin || os::fs::is_directory(strCurFileName))
	{
		// Не показывать тип файла для каталогов в "Быстром просмотре"
		strCurFileType.clear();

		const time_check TimeCheck;

		const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
		{
			if (TimeCheck)
				DirInfoMsg(msg(lng::MQuickViewTitle), Name, ItemsCount, Size);
		};

		if (SameFile && !hDirPlugin)
		{
			m_DirectoryScanStatus = scan_status::real_ok;
		}
		else if (hDirPlugin)
		{
			const auto Result = GetPluginDirInfo(hDirPlugin, strCurFileName, UserData, Data, DirInfoCallback);
			m_DirectoryScanStatus = Result? scan_status::plugin_ok : scan_status::plugin_fail;
			uncomplete_dirscan = !Result;
		}
		else
		{
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
			QView->SetPosition({ m_Where.left + 1, m_Where.top + 1, m_Where.right - 1, m_Where.bottom - 3 });
			QView->SetStatusMode(0);
			QView->EnableHideCursor(0);
			OldWrapMode = QView->GetWrapMode();
			OldWrapType = QView->GetWrapType();
			QView->SetWrapMode(LastWrapMode);
			QView->SetWrapType(LastWrapType);
			QView->OpenFile(strCurFileName, false);
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

		(void)os::fs::set_file_attributes(strCurFileName, FILE_ATTRIBUTE_ARCHIVE); // BUGBUG
		(void)os::fs::delete_file(strCurFileName);  //BUGBUG
		string_view TempDirectoryName = strCurFileName;
		CutToSlash(TempDirectoryName);
		// BUGBUG check result
		(void)os::fs::remove_directory(TempDirectoryName);
		m_TemporaryFile = false;
	}
}


void QuickView::PrintText(string_view const Str) const
{
	if (WhereY() > m_Where. bottom - 3 || WhereX() > m_Where.right - 2)
		return;

	Text(cut_right(Str, m_Where.right - 2 - WhereX() + 1));
}


void QuickView::UpdateIfChanged(bool Idle)
{
	if (!IsVisible() || strCurFileName.empty() || m_DirectoryScanStatus != scan_status::real_fail)
		return;

	const auto strViewName = strCurFileName;
	ShowFile(strViewName, &CurUserData, m_TemporaryFile, nullptr);
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

void QuickView::OnDestroy()
{
	if (QView)
		QView->OnDestroy();
}

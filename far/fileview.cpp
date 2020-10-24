/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp
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
#include "fileview.hpp"

// Internal:
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "history.hpp"
#include "manager.hpp"
#include "fileedit.hpp"
#include "cmdline.hpp"
#include "savescr.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "stddlg.hpp"
#include "macroopcode.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "exitcode.hpp"
#include "keybar.hpp"
#include "constitle.hpp"
#include "pathmix.hpp"
#include "global.hpp"
#include "exception.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

FileViewer::FileViewer(private_tag, bool const DisableEdit, string_view const Title):
	m_DisableEdit(DisableEdit),
	m_StrTitle(Title)
{
}

fileviewer_ptr FileViewer::create(
	const string& Name,
	bool EnableSwitch,
	bool DisableHistory,
	bool DisableEdit,
	long long ViewStartPos,
	string_view PluginData,
	NamesList *ViewNamesList,
	bool ToSaveAs,
	uintptr_t aCodePage,
	string_view Title,
	int DeleteOnClose,
	window_ptr Update)
{
	const auto FileViewerPtr = std::make_shared<FileViewer>(private_tag(), DisableEdit, Title);
	FileViewerPtr->SetPosition({ 0, 0, ScrX, ScrY });
	FileViewerPtr->Init(Name, EnableSwitch, DisableHistory, ViewStartPos, PluginData, ViewNamesList, ToSaveAs, aCodePage, std::move(Update));

	if (DeleteOnClose)
	{
		FileViewerPtr->m_DeleteOnClose = DeleteOnClose == 1 ? 1 : 2;
		FileViewerPtr->SetTempViewName(Name, DeleteOnClose == 1);
	}

	return FileViewerPtr;
}


fileviewer_ptr FileViewer::create(
	const string& Name,
	bool EnableSwitch,
	bool DisableHistory,
	string_view Title,
	rectangle Position,
	uintptr_t aCodePage)
{
	const auto FileViewerPtr = std::make_shared<FileViewer>(private_tag(), true, Title);

	_OT(SysLog(L"[%p] FileViewer::FileViewer(II variant...)", this));

	// BUGBUG WHY ALL THIS?
	if (Position.left < 0)
		Position.left = 0;

	if (Position.right < 0 || Position.right > ScrX)
		Position.right = ScrX;

	if (Position.top < 0)
		Position.top = 0;

	if (Position.bottom < 0 || Position.bottom > ScrY)
		Position.bottom = ScrY;

	if (Position.left > Position.right)
	{
		Position.left = 0;
		Position.right = ScrX;
	}

	if (Position.top > Position.bottom)
	{
		Position.top = 0;
		Position.bottom = ScrY;
	}

	FileViewerPtr->SetPosition(Position);
	FileViewerPtr->m_FullScreen = (!Position.left && !Position.top && Position.right == ScrX && Position.bottom == ScrY);
	FileViewerPtr->Init(Name, EnableSwitch, DisableHistory, -1, {}, nullptr, false, aCodePage);
	return FileViewerPtr;
}


void FileViewer::Init(
	const string& Name,
	bool const EnableSwitch,
	bool const DisableHistory,
	long long const ViewStartPos,
	string_view const PluginData,
	NamesList* const ViewNamesList,
	bool const ToSaveAs,
	uintptr_t const aCodePage,
	window_ptr const Update)
{
	m_View = std::make_unique<Viewer>(shared_from_this(), false, aCodePage);
	m_View->SetTitle(m_StrTitle);
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	m_RedrawTitle = false;
	SetMacroMode(MACROAREA_VIEWER);
	m_View->SetPluginData(PluginData);
	m_View->SetHostFileViewer(this);
	m_DisableHistory=DisableHistory; ///
	m_Name = Name;
	SetCanLoseFocus(EnableSwitch);
	m_SaveToSaveAs=ToSaveAs;
	InitKeyBar();
	// Note: bottom - bottom
	m_windowKeyBar->SetPosition({ m_Where.left, m_Where.bottom, m_Where.right, m_Where.bottom });

	if (ViewNamesList)
		m_View->SetNamesList(*ViewNamesList);

	if (!m_View->OpenFile(m_Name, true))
	{
		m_DisableHistory = true;  // $ 26.03.2002 DJ - при неудаче открытия - не пишем мусор в историю
		// WindowManager->DeleteWindow(this); // ЗАЧЕМ? Вьювер то еще не помещен в очередь манагера!
		m_ExitCode=FALSE;
		return;
	}

	if (ViewStartPos != -1)
		m_View->SetFilePos(ViewStartPos);

	m_ExitCode=TRUE;

	if (IsKeyBarVisible())
	{
		m_windowKeyBar->Show();
	}
	else
	{
		m_windowKeyBar->Hide();
	}

	ShowConsoleTitle();
	m_F3KeyOnly=true;

	if (EnableSwitch)
	{
		if (Update) Global->WindowManager->ReplaceWindow(Update, shared_from_this());
		else Global->WindowManager->InsertWindow(shared_from_this());
	}
	else
	{
		if (Update) Global->WindowManager->DeleteWindow(Update);
		Global->WindowManager->ExecuteWindow(shared_from_this());
	}
	ReadEvent();
}


void FileViewer::InitKeyBar()
{
	auto& Keybar = *m_windowKeyBar;

	Keybar.SetLabels(lng::MViewF1);

	if (Global->OnlyEditorViewerUsed)
		Keybar[KBL_CTRL][F10].clear();

	if (m_DisableEdit)
		Keybar[KBL_MAIN][F6].clear();

	if (!GetCanLoseFocus())
	{
		Keybar[KBL_MAIN][F12].clear();
		Keybar[KBL_ALT][F11].clear();
	}

	Keybar.SetCustomLabels(KBA_VIEWER);

	m_View->SetPosition({ m_Where.left, m_Where.top + (IsTitleBarVisible()? 1 : 0), m_Where.right, m_Where.bottom - (IsKeyBarVisible()? 1 : 0) });
	m_View->SetViewKeyBar(m_windowKeyBar.get());
}

void FileViewer::Show()
{
	if (m_FullScreen)
	{
		if (IsKeyBarVisible())
		{
			m_windowKeyBar->SetPosition({ 0, ScrY, ScrX, ScrY });
		}
		SetPosition({ 0, 0, ScrX, ScrY });
	}
	if (IsKeyBarVisible())
	{
		m_windowKeyBar->Redraw();
	}
	m_View->SetPosition({ m_Where.left, m_Where.top + (IsTitleBarVisible()? 1 : 0), m_Where.right, m_Where.bottom - (IsKeyBarVisible()? 1 : 0) });
	ScreenObjectWithShadow::Show();
	ShowStatus();
}


void FileViewer::DisplayObject()
{
	m_View->Show();
}

long long FileViewer::VMProcess(int OpCode,void *vParam,long long iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=IsKeyBarVisible()?2:1;
		switch (iParam)
		{
			case 0:
				break;

			case 1:
				Global->Opt->ViOpt.ShowKeyBar = true;
				m_windowKeyBar->Show();
				Show();
				break;

			case 2:
				Global->Opt->ViOpt.ShowKeyBar = false;
				m_windowKeyBar->Hide();
				Show();
				break;

			case 3:
				ProcessKey(Manager::Key(KEY_CTRLB));
				break;

			default:
				PrevMode=0;
				break;
		}
		return PrevMode;
	}
	return m_View->VMProcess(OpCode,vParam,iParam);
}

bool FileViewer::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (m_RedrawTitle && ((LocalKey & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal(LocalKey & 0x00ffffff)))
		ShowConsoleTitle();

	if (none_of(LocalKey, KEY_F3, KEY_IDLE))
		m_F3KeyOnly=false;

	switch (LocalKey)
	{
#if 0
			/* $ 30.05.2003 SVS
			   Фича :-) Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер
			   Пока закомментим
			*/
		case KEY_SHIFTF4:
		{
			if (!Global->OnlyEditorViewerUsed)
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Key);

			return true;
		}
#endif
		/* $ 22.07.2000 tran
		   + выход по ctrl-f10 с установкой курсора на файл */
		case KEY_CTRLF10:
		case KEY_RCTRLF10:
		{
			if (Global->WindowManager->InModal())
			{
				return true;
			}

			SCOPED_ACTION(SaveScreen);
			Global->CtrlObject->Cp()->GoToFile(m_View->GetFileName());
			m_RedrawTitle = TRUE;
			return true;
		}

		// $ 15.07.2000 tran + CtrlB switch KeyBar
		case KEY_CTRLB:
		case KEY_RCTRLB:
			Global->Opt->ViOpt.ShowKeyBar=!Global->Opt->ViOpt.ShowKeyBar;

			if (IsKeyBarVisible())
				m_windowKeyBar->Show();
			else
				m_windowKeyBar->Hide();

			Global->WindowManager->RefreshWindow();
			return true;

		case KEY_CTRLSHIFTB:
		case KEY_RCTRLSHIFTB:
			Global->Opt->ViOpt.ShowTitleBar=!Global->Opt->ViOpt.ShowTitleBar;
			Show();
			return true;

		case KEY_CTRLO:
		case KEY_RCTRLO:
			if (Global->WindowManager->ShowBackground())
			{
				SetCursorType(false, 0);
				WaitKey();
				Global->WindowManager->RefreshAll();
			}
			return true;

		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
			if (m_F3KeyOnly)
				return true;
			[[fallthrough]];
		case KEY_ESC:
		case KEY_F10:
			Global->WindowManager->DeleteWindow();
			return true;

		case KEY_F6:
			if (!m_DisableEdit)
			{
				const auto cp = m_View->m_Codepage;
				const auto strViewFileName = m_View->GetFileName();
				while(!os::fs::file(strViewFileName, FILE_READ_DATA, FILE_SHARE_READ|(Global->Opt->EdOpt.EditOpenedForWrite?FILE_SHARE_WRITE:0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
				{
					if (OperationFailed(error_state::fetch(), strViewFileName, lng::MEditTitle, msg(lng::MEditCannotOpen), false) != operation::retry)
						return true;
				}
				const auto FilePos = m_View->GetFilePos();
				const auto flags = (GetCanLoseFocus()? FFILEEDIT_ENABLEF6 : 0) | (m_SaveToSaveAs? FFILEEDIT_SAVETOSAVEAS : 0) | (m_DisableHistory? FFILEEDIT_DISABLEHISTORY : 0);
				const auto ShellEditor = FileEditor::create(
					strViewFileName, cp, flags, -2,
					static_cast<int>(FilePos), // TODO: Editor StartChar should be long long
					m_StrTitle.empty() ? nullptr: &m_StrTitle,
					{ -1, -1, -1, -1 }, m_DeleteOnClose, shared_from_this());

				const auto load = ShellEditor->GetExitCode();
				if (!(load == XC_LOADING_INTERRUPTED || load == XC_OPEN_ERROR))
				{
					ShellEditor->SetEnableF6(true);
					/* $ 07.05.2001 DJ сохраняем NamesList */
					ShellEditor->SetNamesList(m_View->GetNamesList());

					// Если переключаемся в редактор, то удалять файл уже не нужно
					SetTempViewName({});
					SetExitCode(0);
				}
			}
			return true;

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			// Работа с локальной копией ViewerOptions
			Global->Opt->LocalViewerConfig(m_View->ViOpt);

			if (IsKeyBarVisible())
				m_windowKeyBar->Show();

			m_View->Show();
			return true;

		case KEY_ALTF11:
		case KEY_RALTF11:
			if (GetCanLoseFocus())
				Global->CtrlObject->CmdLine()->ShowViewEditHistory();

			return true;

		default:
//      Этот кусок - на будущее (по аналогии с редактором :-)
//      if (Global->CtrlObject->Macro.IsExecuting() || !View.ProcessViewerInput(&ReadRec))
		{
			/* $ 22.03.2001 SVS
			   Это помогло от залипания :-)
			*/
			if (!Global->CtrlObject->Macro.IsExecuting())
				if (IsKeyBarVisible())
					m_windowKeyBar->Show();

			if (!m_windowKeyBar->ProcessKey(Key))
				return m_View->ProcessKey(Key);
		}
		return true;
	}
}


bool FileViewer::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	m_F3KeyOnly = false;
	if (!m_View->ProcessMouse(MouseEvent))
		if (!m_windowKeyBar->ProcessMouse(MouseEvent))
			return false;

	return true;
}


int FileViewer::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MScreensView);
	strName = m_View->GetFileName();
	return windowtype_viewer;
}


void FileViewer::ShowConsoleTitle()
{
	string strViewerTitleFormat = Global->Opt->strViewerTitleFormat.Get();
	replace_icase(strViewerTitleFormat, L"%Lng"sv, msg(lng::MInViewer));
	replace_icase(strViewerTitleFormat, L"%File"sv, PointToName(GetViewer()->strFileName));
	ConsoleTitle::SetFarTitle(strViewerTitleFormat);
	m_RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(string_view const Name, bool const DeleteFolder)
{
	m_DeleteOnClose = (DeleteFolder ? 1 : 2);
	m_View->SetTempViewName(Name, DeleteFolder);
}


FileViewer::~FileViewer()
{
	_OT(SysLog(L"[%p] ~FileViewer::FileViewer()",this));
}

void FileViewer::OnDestroy()
{
	_OT(SysLog(L"[%p] FileViewer::OnDestroy()",this));

	m_bClosing = true;

	if (!m_DisableHistory && (Global->CtrlObject->Cp()->ActivePanel() || m_Name != L"-"sv))
	{
		Global->CtrlObject->ViewHistory->AddToHistory(m_View->GetFileName(), HR_VIEWER);
	}
	m_View->OnDestroy();
}

bool FileViewer::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_VIEWER) != 0;
}

int FileViewer::ViewerControl(int Command, intptr_t Param1, void *Param2) const
{
	_VCTLLOG(CleverSysLog SL(L"FileViewer::ViewerControl()"));
	_VCTLLOG(SysLog(L"(Command=%s, Param2=[%d/0x%08X])",_VCTL_ToName(Command),(int)Param2,Param2));
	const auto result = m_View->ViewerControl(Command, Param1, Param2);
	if (result&&VCTL_GETINFO==Command)
	{
		const auto Info=static_cast<ViewerInfo*>(Param2);
		if (IsTitleBarVisible())
			Info->Options |= VOPT_SHOWTITLEBAR;
		if (IsKeyBarVisible())
			Info->Options |= VOPT_SHOWKEYBAR;
	}
	return result;
}

 string FileViewer::GetTitle() const
{
	return m_View->GetTitle();
}

long long FileViewer::GetViewFileSize() const
{
	return m_View->GetViewFileSize();
}

long long FileViewer::GetViewFilePos() const
{
	return m_View->GetViewFilePos();
}

void FileViewer::ShowStatus() const
{
	if (!IsTitleBarVisible())
		return;

	SetColor(COL_VIEWERSTATUS);
	GotoXY(m_Where.left, m_Where.top);

	auto StatusLine = format(FSTR(L"│{0}│{1:5.5}│{2:<10}│{3:.3} {4:<3}│{5:4}"),
		L"thd"[m_View->m_DisplayMode],
		ShortReadableCodepageName(m_View->m_Codepage),
		m_View->FileSize,
		msg(lng::MViewerStatusCol),
		m_View->LeftPos,
		str(m_View->LastPage? 100 : ToPercent(m_View->FilePos,m_View->FileSize)) + L'%'
	);

	// Explicitly signed types - it's too easy to screw it up on small console sizes otherwise
	const int ClockSize = Global->Opt->ViewerEditorClock && IsFullScreen()? static_cast<int>(Global->CurrentTime.size()) : 0;
	const int AvailableSpace = std::max(0, ObjWidth() - ClockSize - (ClockSize? 1 : 0));
	inplace::cut_right(StatusLine, AvailableSpace);
	const int NameWidth = std::max(0, AvailableSpace - static_cast<int>(StatusLine.size()));

	Text(fit_to_left(truncate_path(GetTitle(), NameWidth), NameWidth));
	Text(StatusLine);

	if (ClockSize)
	{
		Text(L'│'); // Separator before the clock
		ShowTime();
	}
}

void FileViewer::OnChangeFocus(bool focus)
{
	window::OnChangeFocus(focus);
	if (!m_bClosing)
	{
		Global->CtrlObject->Plugins->ProcessViewerEvent(focus? VE_GOTFOCUS : VE_KILLFOCUS, nullptr, m_View.get());
	}
}

void FileViewer::OnReload()
{
	ReadEvent();
}

void FileViewer::ReadEvent()
{
	Global->WindowManager->CallbackWindow([this]()
	{
		m_View->ReadEvent();
	});
}

Viewer* FileViewer::GetViewer()
{
	return m_View.get();
}

Viewer* FileViewer::GetById(int ID)
{
	return ID==GetId()?GetViewer():nullptr;
}

bool FileViewer::IsKeyBarVisible() const
{
	return Global->Opt->ViOpt.ShowKeyBar && ObjHeight() > 2;
}

bool FileViewer::IsTitleBarVisible() const
{
	return Global->Opt->ViOpt.ShowTitleBar && ObjHeight() > 1;
}

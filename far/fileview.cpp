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

#include "headers.hpp"
#pragma hdrstop

#include "fileview.hpp"
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
#include "language.hpp"
#include "exitcode.hpp"
#include "keybar.hpp"
#include "constitle.hpp"

FileViewer::FileViewer(private_tag, int DisableEdit, const wchar_t *Title):
	RedrawTitle(),
	F3KeyOnly(),
	m_bClosing(),
	FullScreen(true),
	DisableEdit(DisableEdit),
	DisableHistory(),
	SaveToSaveAs(),
	delete_on_close(),
	str_title(NullToEmpty(Title))
{
}

fileviewer_ptr FileViewer::create(const string& Name, int EnableSwitch, int DisableHistory, int DisableEdit,
                                  long long ViewStartPos,const wchar_t *PluginData, NamesList *ViewNamesList,bool ToSaveAs,
                                  uintptr_t aCodePage, const wchar_t *Title, int DeleteOnClose, window_ptr Update)
{
	const auto FileViewerPtr = std::make_shared<FileViewer>(private_tag(), DisableEdit, Title);
	FileViewerPtr->SetPosition(0, 0, ScrX, ScrY);
	FileViewerPtr->Init(Name, EnableSwitch, DisableHistory, ViewStartPos, PluginData, ViewNamesList, ToSaveAs, aCodePage, Update);

	if (DeleteOnClose)
	{
		FileViewerPtr->delete_on_close = DeleteOnClose == 1 ? 1 : 2;
		FileViewerPtr->SetTempViewName(Name, DeleteOnClose == 1);
	}

	return FileViewerPtr;
}


fileviewer_ptr FileViewer::create(const string& Name, int EnableSwitch, int DisableHistory,
                                  const wchar_t *Title, int X1,int Y1,int X2,int Y2,uintptr_t aCodePage)
{
	const auto FileViewerPtr = std::make_shared<FileViewer>(private_tag(), TRUE, Title);

	_OT(SysLog(L"[%p] FileViewer::FileViewer(II variant...)", this));

	if (X1 < 0)
		X1=0;

	if (X2 < 0 || X2 > ScrX)
		X2=ScrX;

	if (Y1 < 0)
		Y1=0;

	if (Y2 < 0 || Y2 > ScrY)
		Y2=ScrY;

	if (X1 >= X2)
	{
		X1=0;
		X2=ScrX;
	}

	if (Y1 >= Y2)
	{
		Y1=0;
		Y2=ScrY;
	}

	FileViewerPtr->SetPosition(X1, Y1, X2, Y2);
	FileViewerPtr->FullScreen = (!X1 && !Y1 && X2 == ScrX && Y2 == ScrY);
	FileViewerPtr->Init(Name, EnableSwitch, DisableHistory, -1, L"", nullptr, false, aCodePage);
	return FileViewerPtr;
}


void FileViewer::Init(const string& name,int EnableSwitch,int disableHistory,
	long long ViewStartPos,const wchar_t *PluginData,
	NamesList *ViewNamesList, bool ToSaveAs, uintptr_t aCodePage, window_ptr Update)
{
	m_View = std::make_unique<Viewer>(shared_from_this(), false, aCodePage);
	m_View->SetTitle(str_title);
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	RedrawTitle = FALSE;
	m_KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
	m_TitleBarVisible = Global->Opt->ViOpt.ShowTitleBar;
	SetMacroMode(MACROAREA_VIEWER);
	m_View->SetPluginData(PluginData);
	m_View->SetHostFileViewer(this);
	DisableHistory=disableHistory; ///
	m_Name = name;
	SetCanLoseFocus(EnableSwitch);
	SaveToSaveAs=ToSaveAs;
	InitKeyBar();
	m_windowKeyBar->SetPosition(m_X1, m_Y2, m_X2, m_Y2);

	if (ViewNamesList)
		m_View->SetNamesList(*ViewNamesList);

	if (!m_View->OpenFile(m_Name,TRUE)) // $ 04.07.2000 tran + add TRUE as 'warning' parameter
	{
		DisableHistory = TRUE;  // $ 26.03.2002 DJ - при неудаче открытия - не пишем мусор в историю
		// WindowManager->DeleteWindow(this); // ЗАЧЕМ? Вьювер то еще не помещен в очередь манагера!
		m_ExitCode=FALSE;
		return;
	}

	if (ViewStartPos != -1)
		m_View->SetFilePos(ViewStartPos);

	m_ExitCode=TRUE;

	if (Global->Opt->ViOpt.ShowKeyBar)
	{
		m_windowKeyBar->Show();
	}
	else
	{
		m_windowKeyBar->Hide();
	}

	ShowConsoleTitle();
	F3KeyOnly=true;

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
	m_windowKeyBar->SetLabels(Global->OnlyEditorViewerUsed ? MSingleViewF1 : MViewF1);

	if (DisableEdit)
		(*m_windowKeyBar)[KBL_MAIN][F6].clear();

	if (!GetCanLoseFocus())
	{
		(*m_windowKeyBar)[KBL_MAIN][F12].clear();
		(*m_windowKeyBar)[KBL_ALT][F11].clear();
	}

	m_windowKeyBar->SetCustomLabels(KBA_VIEWER);
	m_View->SetPosition(m_X1,m_Y1+(Global->Opt->ViOpt.ShowTitleBar?1:0),m_X2,m_Y2-(Global->Opt->ViOpt.ShowKeyBar?1:0));
	m_View->SetViewKeyBar(m_windowKeyBar.get());
}

void FileViewer::Show()
{
	if (FullScreen)
	{
		if (Global->Opt->ViOpt.ShowKeyBar)
		{
			m_windowKeyBar->SetPosition(0, ScrY, ScrX, ScrY);
			m_windowKeyBar->Redraw();
		}

		SetPosition(0,0,ScrX,ScrY-(Global->Opt->ViOpt.ShowKeyBar?1:0));
		m_View->SetPosition(0,(Global->Opt->ViOpt.ShowTitleBar?1:0),ScrX,ScrY-(Global->Opt->ViOpt.ShowKeyBar?1:0));
	}

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
		int PrevMode=Global->Opt->ViOpt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->ViOpt.ShowKeyBar = true;
				m_windowKeyBar->Show();
				Show();
				m_KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
				break;
			case 2:
				Global->Opt->ViOpt.ShowKeyBar = false;
				m_windowKeyBar->Hide();
				Show();
				m_KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
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

int FileViewer::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (RedrawTitle && (((unsigned int)LocalKey & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal((unsigned int)LocalKey & 0x00ffffff)))
		ShowConsoleTitle();

	if (LocalKey!=KEY_F3 && LocalKey!=KEY_IDLE)
		F3KeyOnly=false;

	switch (LocalKey)
	{
#if 0
			/* $ 30.05.2003 SVS
			   Фича :-) Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер
			   Пока закомментим
			*/
		case KEY_SHIFTF4:
		{
			if (!Global->Opt->OnlyEditorViewerUsed)
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Key);

			return TRUE;
		}
#endif
		/* $ 22.07.2000 tran
		   + выход по ctrl-f10 с установкой курсора на файл */
		case KEY_CTRLF10:
		case KEY_RCTRLF10:
		{
			if (Global->WindowManager->InModal())
			{
				return TRUE;
			}

			SCOPED_ACTION(SaveScreen);
			string strFileName;
			m_View->GetFileName(strFileName);
			Global->CtrlObject->Cp()->GoToFile(strFileName);
			RedrawTitle = TRUE;
			return TRUE;
		}
		// $ 15.07.2000 tran + CtrlB switch KeyBar
		case KEY_CTRLB:
		case KEY_RCTRLB:
			Global->Opt->ViOpt.ShowKeyBar=!Global->Opt->ViOpt.ShowKeyBar;

			if (Global->Opt->ViOpt.ShowKeyBar)
				m_windowKeyBar->Show();
			else
				m_windowKeyBar->Hide();

			Show();
			m_KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
			return TRUE;
		case KEY_CTRLSHIFTB:
		case KEY_RCTRLSHIFTB:
		{
			Global->Opt->ViOpt.ShowTitleBar=!Global->Opt->ViOpt.ShowTitleBar;
			m_TitleBarVisible = Global->Opt->ViOpt.ShowTitleBar;
			Show();
			return TRUE;
		}
		case KEY_CTRLO:
		case KEY_RCTRLO:
			if (Global->WindowManager->ShowBackground())
			{
				SetCursorType(false, 0);
				WaitKey();
				Global->WindowManager->RefreshWindow();
			}

			return TRUE;
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (F3KeyOnly)
				return TRUE;

		case KEY_ESC:
		case KEY_F10:
			Global->WindowManager->DeleteWindow();
			return TRUE;
		case KEY_F6:

			if (!DisableEdit)
			{
				const auto cp = m_View->m_Codepage;
				string strViewFileName;
				m_View->GetFileName(strViewFileName);
				os::fs::file Edit;
				while(!Edit.Open(strViewFileName, FILE_READ_DATA, FILE_SHARE_READ|(Global->Opt->EdOpt.EditOpenedForWrite?FILE_SHARE_WRITE:0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
				{
					Global->CatchError();
					if(!OperationFailed(strViewFileName, MEditTitle, MSG(MEditCannotOpen), false))
						continue;
					else
						return TRUE;
				}
				Edit.Close();
				long long FilePos=m_View->GetFilePos();
				DWORD flags = (GetCanLoseFocus()?FFILEEDIT_ENABLEF6:0)|(SaveToSaveAs?FFILEEDIT_SAVETOSAVEAS:0)|(DisableHistory?FFILEEDIT_DISABLEHISTORY:0);
				const auto ShellEditor = FileEditor::create(
					strViewFileName, cp, flags, -2,
					static_cast<int>(FilePos), // TODO: Editor StartChar should be long long
					str_title.empty() ? nullptr: &str_title,
					-1,-1, -1, -1, delete_on_close, shared_from_this());

				int load = ShellEditor->GetExitCode();
				if (!(load == XC_LOADING_INTERRUPTED || load == XC_OPEN_ERROR))
				{
					ShellEditor->SetEnableF6(true);
					/* $ 07.05.2001 DJ сохраняем NamesList */
					ShellEditor->SetNamesList(m_View->GetNamesList());

					// Если переключаемся в редактор, то удалять файл уже не нужно
					SetTempViewName(L"");
					SetExitCode(0);
				}
			}

			return TRUE;

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			// Работа с локальной копией ViewerOptions
			Global->Opt->LocalViewerConfig(m_View->ViOpt);

			if (Global->Opt->ViOpt.ShowKeyBar)
				m_windowKeyBar->Show();

			m_View->Show();
			return TRUE;
		case KEY_ALTF11:
		case KEY_RALTF11:
			if (GetCanLoseFocus())
				Global->CtrlObject->CmdLine()->ShowViewEditHistory();

			return TRUE;
		default:
//      Этот кусок - на будущее (по аналогии с редактором :-)
//      if (Global->CtrlObject->Macro.IsExecuting() || !View.ProcessViewerInput(&ReadRec))
		{
			/* $ 22.03.2001 SVS
			   Это помогло от залипания :-)
			*/
			if (!Global->CtrlObject->Macro.IsExecuting())
				if (Global->Opt->ViOpt.ShowKeyBar)
					m_windowKeyBar->Show();

			if (!m_windowKeyBar->ProcessKey(Key))
				return m_View->ProcessKey(Key);
		}
		return TRUE;
	}
}


int FileViewer::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	F3KeyOnly = false;
	if (!m_View->ProcessMouse(MouseEvent))
		if (!m_windowKeyBar->ProcessMouse(MouseEvent))
			return FALSE;

	return TRUE;
}


int FileViewer::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensView);
	m_View->GetFileName(strName);
	return windowtype_viewer;
}


void FileViewer::ShowConsoleTitle()
{
	string strViewerTitleFormat = Global->Opt->strViewerTitleFormat.Get();
	ReplaceStrings(strViewerTitleFormat, L"%Lng", MSG(MInViewer), true);
	ReplaceStrings(strViewerTitleFormat, L"%File", PointToName(GetViewer()->strFileName), true);
	ConsoleTitle::SetFarTitle(strViewerTitleFormat);
	RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(const string& Name, BOOL DeleteFolder)
{
	delete_on_close = (DeleteFolder ? 1 : 2);
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

	if (!DisableHistory && (Global->CtrlObject->Cp()->ActivePanel() || m_Name != L"-"))
	{
		string strFullFileName;
		m_View->GetFileName(strFullFileName);
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, HR_VIEWER);
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
	return m_View->ViewerControl(Command,Param1,Param2);
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

	string strName = GetTitle();
	int NameLength = ScrX+1 - 40;

	if (Global->Opt->ViewerEditorClock && IsFullScreen())
		NameLength -= 3 + static_cast<int>(Global->CurrentTime.size());

	NameLength = std::max(NameLength, 20);

	TruncPathStr(strName, NameLength);
	static constexpr wchar_t StatusFormat[] = L"%-*s %c %5u %13I64u %7.7s %-4I64d %3d%%";
	string strStatus = str_printf(
	    StatusFormat,
	    NameLength,
	    strName.data(),
	    L"thd"[m_View->m_DisplayMode],
	    m_View->m_Codepage,
	    m_View->FileSize,
	    MSG(MViewerStatusCol),
	    m_View->LeftPos,
	    (m_View->LastPage ? 100:ToPercent(m_View->FilePos,m_View->FileSize))
	);
	SetColor(COL_VIEWERSTATUS);
	GotoXY(m_X1,m_Y1);
	Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(m_View->Width+(m_View->ViOpt.ShowScrollbar?1:0))<<strStatus;

	if (Global->Opt->ViewerEditorClock && IsFullScreen())
		ShowTime();
}

void FileViewer::OnChangeFocus(bool focus)
{
	window::OnChangeFocus(focus);
	if (!m_bClosing)
	{
		Global->CtrlObject->Plugins->ProcessViewerEvent(focus? VE_GOTFOCUS : VE_KILLFOCUS, nullptr, m_View.get());
	}
}

void FileViewer::OnReload(void)
{
	ReadEvent();
}

void FileViewer::ReadEvent(void)
{
	Global->WindowManager->CallbackWindow([this]()
	{
		m_View->ReadEvent();
	});
}

Viewer* FileViewer::GetViewer(void)
{
	return m_View.get();
}

Viewer* FileViewer::GetById(int ID)
{
	return ID==GetId()?GetViewer():nullptr;
}

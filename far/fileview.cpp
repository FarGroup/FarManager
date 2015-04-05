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
#include "panel.hpp"
#include "history.hpp"
#include "manager.hpp"
#include "fileedit.hpp"
#include "cmdline.hpp"
#include "savescr.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "stddlg.hpp"
#include "macroopcode.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "exitcode.hpp"
#include "keybar.hpp"

FileViewer::FileViewer(int DisableEdit, const wchar_t *Title):
	FullScreen(true),
	DisableEdit(DisableEdit),
	delete_on_close(),
	str_title(NullToEmpty(Title))
{
}

fileviewer_ptr FileViewer::create(const string& Name, int EnableSwitch, int DisableHistory, int DisableEdit,
                                  __int64 ViewStartPos,const wchar_t *PluginData, NamesList *ViewNamesList,bool ToSaveAs,
                                  uintptr_t aCodePage, const wchar_t *Title, int DeleteOnClose, window_ptr Update)
{
	fileviewer_ptr FileViewerPtr(new FileViewer(DisableEdit, Title));

	if (DeleteOnClose)
	{
		FileViewerPtr->delete_on_close = DeleteOnClose == 1 ? 1 : 2;
		FileViewerPtr->SetTempViewName(Name, DeleteOnClose == 1);
	}
	FileViewerPtr->SetPosition(0, 0, ScrX, ScrY);
	FileViewerPtr->Init(Name, EnableSwitch, DisableHistory, ViewStartPos, PluginData, ViewNamesList, ToSaveAs, aCodePage, Update);
	return FileViewerPtr;
}


fileviewer_ptr FileViewer::create(const string& Name, int EnableSwitch, int DisableHistory,
                                  const wchar_t *Title, int X1,int Y1,int X2,int Y2,uintptr_t aCodePage)
{
	fileviewer_ptr FileViewerPtr(new FileViewer(TRUE, Title));

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
	__int64 ViewStartPos,const wchar_t *PluginData,
	NamesList *ViewNamesList, bool ToSaveAs, uintptr_t aCodePage, window_ptr Update)
{
	m_View = std::make_unique<Viewer>(shared_from_this(), false, aCodePage);
	GetView().SetTitle(str_title);
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	RedrawTitle = FALSE;
	m_KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
	m_TitleBarVisible = Global->Opt->ViOpt.ShowTitleBar;
	SetMacroMode(MACROAREA_VIEWER);
	GetView().SetPluginData(PluginData);
	GetView().SetHostFileViewer(this);
	DisableHistory=disableHistory; ///
	m_Name = name;
	SetCanLoseFocus(EnableSwitch);
	SaveToSaveAs=ToSaveAs;
	InitKeyBar();
	m_windowKeyBar->SetPosition(m_X1, m_Y2, m_X2, m_Y2);

	if (ViewNamesList)
		GetView().SetNamesList(*ViewNamesList);

	if (!GetView().OpenFile(m_Name,TRUE)) // $ 04.07.2000 tran + add TRUE as 'warning' parameter
	{
		DisableHistory = TRUE;  // $ 26.03.2002 DJ - при неудаче открытия - не пишем мусор в историю
		// WindowManager->DeleteWindow(this); // ЗАЧЕМ? Вьювер то еще не помещен в очередь манагера!
		m_ExitCode=FALSE;
		return;
	}

	if (ViewStartPos != -1)
		GetView().SetFilePos(ViewStartPos);

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
	GetView().SetPosition(m_X1,m_Y1+(Global->Opt->ViOpt.ShowTitleBar?1:0),m_X2,m_Y2-(Global->Opt->ViOpt.ShowKeyBar?1:0));
	GetView().SetViewKeyBar(m_windowKeyBar.get());
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
		GetView().SetPosition(0,(Global->Opt->ViOpt.ShowTitleBar?1:0),ScrX,ScrY-(Global->Opt->ViOpt.ShowKeyBar?1:0));
	}

	ScreenObjectWithShadow::Show();
	ShowStatus();
}


void FileViewer::DisplayObject()
{
	GetView().Show();
}

__int64 FileViewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Global->Opt->ViOpt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->ViOpt.ShowKeyBar=1;
				m_windowKeyBar->Show();
				Show();
				m_KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
				break;
			case 2:
				Global->Opt->ViOpt.ShowKeyBar=0;
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
	return GetView().VMProcess(OpCode,vParam,iParam);
}

int FileViewer::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey();
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
			if (GetView().isTemporary())
			{
				return TRUE;
			}

			SaveScreen Sc;
			string strFileName;
			GetView().GetFileName(strFileName);
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
				UINT cp=GetView().VM.CodePage;
				string strViewFileName;
				GetView().GetFileName(strViewFileName);
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
				__int64 FilePos=GetView().GetFilePos();
				DWORD flags = (GetCanLoseFocus()?FFILEEDIT_ENABLEF6:0)|(SaveToSaveAs?FFILEEDIT_SAVETOSAVEAS:0)|(DisableHistory?FFILEEDIT_DISABLEHISTORY:0);
				auto ShellEditor = FileEditor::create(
					strViewFileName, cp, flags, -2,
					static_cast<int>(FilePos), // TODO: Editor StartChar should be __int64
					str_title.empty() ? nullptr: &str_title,
					-1,-1, -1, -1, delete_on_close, shared_from_this());

				int load = ShellEditor->GetExitCode();
				if (!(load == XC_LOADING_INTERRUPTED || load == XC_OPEN_ERROR))
				{
					ShellEditor->SetEnableF6(true);
					/* $ 07.05.2001 DJ сохраняем NamesList */
					ShellEditor->SetNamesList(GetView().GetNamesList());

					// Если переключаемся в редактор, то удалять файл уже не нужно
					SetTempViewName(L"");
					SetExitCode(0);
				}
				ShowTime(2);
			}

			return TRUE;

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			// Работа с локальной копией ViewerOptions
			Global->Opt->LocalViewerConfig(GetView().ViOpt);

			if (Global->Opt->ViOpt.ShowKeyBar)
				m_windowKeyBar->Show();

			GetView().Show();
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
				return GetView().ProcessKey(Key);
		}
		return TRUE;
	}
}


int FileViewer::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	F3KeyOnly = false;
	if (!GetView().ProcessMouse(MouseEvent))
		if (!m_windowKeyBar->ProcessMouse(MouseEvent))
			return FALSE;

	return TRUE;
}


int FileViewer::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensView);
	GetView().GetFileName(strName);
	return windowtype_viewer;
}


void FileViewer::ShowConsoleTitle()
{
	GetView().ShowConsoleTitle();
	RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(const string& Name, BOOL DeleteFolder)
{
	delete_on_close = (DeleteFolder ? 1 : 2);
	GetView().SetTempViewName(Name, DeleteFolder);
}


FileViewer::~FileViewer()
{
	_OT(SysLog(L"[%p] ~FileViewer::FileViewer()",this));
}

void FileViewer::OnDestroy()
{
	_OT(SysLog(L"[%p] FileViewer::OnDestroy()",this));

	if (!DisableHistory && (Global->CtrlObject->Cp()->ActivePanel() || m_Name != L"-"))
	{
		string strFullFileName;
		GetView().GetFileName(strFullFileName);
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, HR_VIEWER);
	}
	GetView().OnDestroy();
}

bool FileViewer::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_VIEWER) != 0;
}

int FileViewer::ViewerControl(int Command, intptr_t Param1, void *Param2)
{
	_VCTLLOG(CleverSysLog SL(L"FileViewer::ViewerControl()"));
	_VCTLLOG(SysLog(L"(Command=%s, Param2=[%d/0x%08X])",_VCTL_ToName(Command),(int)Param2,Param2));
	return GetView().ViewerControl(Command,Param1,Param2);
}

 string FileViewer::GetTitle() const
{
	return GetView().GetTitle();
}

__int64 FileViewer::GetViewFileSize() const
{
	return GetView().GetViewFileSize();
}

__int64 FileViewer::GetViewFilePos() const
{
	return GetView().GetViewFilePos();
}

void FileViewer::ShowStatus()
{
	if (!IsTitleBarVisible())
		return;

	string strName = GetTitle();
	int NameLength = ScrX+1 - 40;

	if (Global->Opt->ViewerEditorClock && IsFullScreen())
		NameLength -= 3+5;

	NameLength = std::max(NameLength, 20);

	TruncPathStr(strName, NameLength);
	static const wchar_t lpwszStatusFormat[] = L"%-*s %c %5u %13I64u %7.7s %-4I64d %3d%%";
	string strStatus = str_printf(
	    lpwszStatusFormat,
	    NameLength,
	    strName.data(),
	    L"thd"[GetView().VM.Hex],
	    GetView().VM.CodePage,
	    GetView().FileSize,
	    MSG(MViewerStatusCol),
	    GetView().LeftPos,
	    (GetView().LastPage ? 100:ToPercent(GetView().FilePos,GetView().FileSize))
	);
	SetColor(COL_VIEWERSTATUS);
	GotoXY(m_X1,m_Y1);
	Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(GetView().Width+(GetView().ViOpt.ShowScrollbar?1:0))<<strStatus;

	if (Global->Opt->ViewerEditorClock && IsFullScreen())
		ShowTime(FALSE);
}

void FileViewer::OnChangeFocus(bool focus)
{
	window::OnChangeFocus(focus);
	int FCurViewerID=GetView().ViewerID;
	Global->CtrlObject->Plugins->ProcessViewerEvent(focus?VE_GOTFOCUS:VE_KILLFOCUS,nullptr,FCurViewerID);
}

void FileViewer::OnReload(void)
{
	ReadEvent();
}

void FileViewer::ReadEvent(void)
{
	Global->WindowManager->CallbackWindow([this]()
	{
		this->GetView().ReadEvent();
	});
}

Viewer* FileViewer::GetViewer(void)
{
	return &GetView();
}

Viewer* FileViewer::GetById(int ID)
{
	return ID==GetId()?GetViewer():nullptr;
}

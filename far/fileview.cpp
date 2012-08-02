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

#ifdef FAR_LUA
#include "macroopcode.hpp"
#endif

FileViewer::FileViewer(
	const wchar_t *Name,int EnableSwitch,int DisableHistory,
	int DisableEdit,__int64 ViewStartPos,const wchar_t *PluginData,
	NamesList *ViewNamesList,int ToSaveAs,UINT aCodePage,
	const wchar_t *Title, int DeleteOnClose)
 : View(false,aCodePage),
	FullScreen(true),
	DisableEdit(DisableEdit),
	delete_on_close(0)
{
	_OT(SysLog(L"[%p] FileViewer::FileViewer(I variant...)", this));
	str_title = (Title && *Title ? Title : L"");
	if (!str_title.IsEmpty())
		View.SetTitle(Title);
	if (DeleteOnClose)
	{
		delete_on_close = DeleteOnClose == 1 ? 1 : 2;
		SetTempViewName(Name, DeleteOnClose == 1 ? TRUE : FALSE);
	}
	SetPosition(0,0,ScrX,ScrY);
	Init(Name,EnableSwitch,DisableHistory,ViewStartPos,PluginData,ViewNamesList,ToSaveAs);
}


FileViewer::FileViewer(const wchar_t *Name,int EnableSwitch,int DisableHistory,
                       const wchar_t *Title, int X1,int Y1,int X2,int Y2,UINT aCodePage): View(false,aCodePage)
{
	_OT(SysLog(L"[%p] FileViewer::FileViewer(II variant...)", this));
	DisableEdit=TRUE;

	delete_on_close = 0;
	str_title = (Title && *Title ? Title : L"");

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

	SetPosition(X1,Y1,X2,Y2);
	FullScreen=(!X1 && !Y1 && X2==ScrX && Y2==ScrY);
	View.SetTitle(Title);
	Init(Name,EnableSwitch,DisableHistory,-1,L"",nullptr,FALSE);
}


void FileViewer::Init(const wchar_t *name,int EnableSwitch,int disableHistory,
	__int64 ViewStartPos,const wchar_t *PluginData,
	NamesList *ViewNamesList,int ToSaveAs)
{
	RedrawTitle = FALSE;
	ViewKeyBar.SetOwner(this);
	ViewKeyBar.SetPosition(X1,Y2,X2,Y2);
	KeyBarVisible = Opt.ViOpt.ShowKeyBar;
	TitleBarVisible = Opt.ViOpt.ShowTitleBar;
	int OldMacroMode=CtrlObject->Macro.GetMode();
	MacroMode = MACRO_VIEWER;
	CtrlObject->Macro.SetMode(MACRO_VIEWER);
	View.SetPluginData(PluginData);
	View.SetHostFileViewer(this);
	DisableHistory=disableHistory; ///
	strName = name;
	SetCanLoseFocus(EnableSwitch);
	SaveToSaveAs=ToSaveAs;
	InitKeyBar();

	if (!View.OpenFile(strName,TRUE)) // $ 04.07.2000 tran + add TRUE as 'warning' parameter
	{
		DisableHistory = TRUE;  // $ 26.03.2002 DJ - при неудаче открытия - не пишем мусор в историю
		// FrameManager->DeleteFrame(this); // ЗАЧЕМ? Вьювер то еще не помещен в очередь манагера!
		ExitCode=FALSE;
		CtrlObject->Macro.SetMode(OldMacroMode);
		return;
	}

	if (ViewStartPos!=-1)
		View.SetFilePos(ViewStartPos);

	if (ViewNamesList)
		View.SetNamesList(ViewNamesList);

	ExitCode=TRUE;
	ViewKeyBar.Show();

	if (!Opt.ViOpt.ShowKeyBar)
		ViewKeyBar.Hide0();

	ShowConsoleTitle();
	F3KeyOnly=true;

	if (EnableSwitch)
	{
		FrameManager->InsertFrame(this);
	}
	else
	{
		FrameManager->ExecuteFrame(this);
	}
}


void FileViewer::InitKeyBar()
{
	ViewKeyBar.SetAllGroup(KBL_MAIN,         Opt.OnlyEditorViewerUsed?MSingleViewF1:MViewF1, 12);
	ViewKeyBar.SetAllGroup(KBL_SHIFT,        Opt.OnlyEditorViewerUsed?MSingleViewShiftF1:MViewShiftF1, 12);
	ViewKeyBar.SetAllGroup(KBL_ALT,          Opt.OnlyEditorViewerUsed?MSingleViewAltF1:MViewAltF1, 12);
	ViewKeyBar.SetAllGroup(KBL_CTRL,         Opt.OnlyEditorViewerUsed?MSingleViewCtrlF1:MViewCtrlF1, 12);
	ViewKeyBar.SetAllGroup(KBL_CTRLSHIFT,    Opt.OnlyEditorViewerUsed?MSingleViewCtrlShiftF1:MViewCtrlShiftF1, 12);
	ViewKeyBar.SetAllGroup(KBL_CTRLALT,      Opt.OnlyEditorViewerUsed?MSingleViewCtrlAltF1:MViewCtrlAltF1, 12);
	ViewKeyBar.SetAllGroup(KBL_ALTSHIFT,     Opt.OnlyEditorViewerUsed?MSingleViewAltShiftF1:MViewAltShiftF1, 12);
	ViewKeyBar.SetAllGroup(KBL_CTRLALTSHIFT, Opt.OnlyEditorViewerUsed?MSingleViewCtrlAltShiftF1:MViewCtrlAltShiftF1, 12);

	if (DisableEdit)
		ViewKeyBar.Change(KBL_MAIN,L"",6-1);

	if (!GetCanLoseFocus())
		ViewKeyBar.Change(KBL_MAIN,L"",12-1);

	if (!GetCanLoseFocus())
		ViewKeyBar.Change(KBL_ALT,L"",11-1);

	ViewKeyBar.ReadRegGroup(L"Viewer",Opt.strLanguage);
	ViewKeyBar.SetAllRegGroup();
	SetKeyBar(&ViewKeyBar);
	View.SetPosition(X1,Y1+(Opt.ViOpt.ShowTitleBar?1:0),X2,Y2-(Opt.ViOpt.ShowKeyBar?1:0));
	View.SetViewKeyBar(&ViewKeyBar);
}

void FileViewer::Show()
{
	if (FullScreen)
	{
		if (Opt.ViOpt.ShowKeyBar)
		{
			ViewKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
			ViewKeyBar.Redraw();
		}

		SetPosition(0,0,ScrX,ScrY-(Opt.ViOpt.ShowKeyBar?1:0));
		View.SetPosition(0,(Opt.ViOpt.ShowTitleBar?1:0),ScrX,ScrY-(Opt.ViOpt.ShowKeyBar?1:0));
	}

	ScreenObject::Show();
	ShowStatus();
}


void FileViewer::DisplayObject()
{
	View.Show();
}

__int64 FileViewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Opt.ViOpt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Opt.ViOpt.ShowKeyBar=1;
				ViewKeyBar.Show();
				Show();
				KeyBarVisible = Opt.ViOpt.ShowKeyBar;
				break;
			case 2:
				Opt.ViOpt.ShowKeyBar=0;
				ViewKeyBar.Hide();
				Show();
				KeyBarVisible = Opt.ViOpt.ShowKeyBar;
				break;
			case 3:
				ProcessKey(KEY_CTRLB);
				break;
			default:
				PrevMode=0;
				break;
		}
		return PrevMode;
	}
	return View.VMProcess(OpCode,vParam,iParam);
}

int FileViewer::ProcessKey(int Key)
{
	if (RedrawTitle && (((unsigned int)Key & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal((unsigned int)Key & 0x00ffffff)))
		ShowConsoleTitle();

	if (Key!=KEY_F3 && Key!=KEY_IDLE)
		F3KeyOnly=false;

	switch (Key)
	{
#if 0
			/* $ 30.05.2003 SVS
			   Фича :-) Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер
			   Пока закомментим
			*/
		case KEY_SHIFTF4:
		{
			if (!Opt.OnlyEditorViewerUsed)
				CtrlObject->Cp()->ActivePanel->ProcessKey(Key);

			return TRUE;
		}
#endif
		/* $ 22.07.2000 tran
		   + выход по ctrl-f10 с установкой курсора на файл */
		case KEY_CTRLF10:
		case KEY_RCTRLF10:
		{
			if (View.isTemporary())
			{
				return TRUE;
			}

			SaveScreen Sc;
			string strFileName;
			View.GetFileName(strFileName);
			CtrlObject->Cp()->GoToFile(strFileName);
			RedrawTitle = TRUE;
			return (TRUE);
		}
		// $ 15.07.2000 tran + CtrlB switch KeyBar
		case KEY_CTRLB:
		case KEY_RCTRLB:
			Opt.ViOpt.ShowKeyBar=!Opt.ViOpt.ShowKeyBar;

			if (Opt.ViOpt.ShowKeyBar)
				ViewKeyBar.Show();
			else
				ViewKeyBar.Hide0(); // 0 mean - Don't purge saved screen

			Show();
			KeyBarVisible = Opt.ViOpt.ShowKeyBar;
			return (TRUE);
		case KEY_CTRLSHIFTB:
		case KEY_RCTRLSHIFTB:
		{
			Opt.ViOpt.ShowTitleBar=!Opt.ViOpt.ShowTitleBar;
			TitleBarVisible = Opt.ViOpt.ShowTitleBar;
			Show();
			return (TRUE);
		}
		case KEY_CTRLO:
		case KEY_RCTRLO:

			if (!Opt.OnlyEditorViewerUsed)
			{
				if (FrameManager->ShowBackground())
				{
					SetCursorType(FALSE,0);
					WaitKey();
					FrameManager->RefreshFrame();
				}
			}

			return TRUE;
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (F3KeyOnly)
				return TRUE;

		case KEY_ESC:
		case KEY_F10:
			FrameManager->DeleteFrame();
			return TRUE;
		case KEY_F6:

			if (!DisableEdit)
			{
				UINT cp=View.VM.CodePage;
				string strViewFileName;
				View.GetFileName(strViewFileName);
				File Edit;
				while(!Edit.Open(strViewFileName, FILE_READ_DATA, FILE_SHARE_READ|(Opt.EdOpt.EditOpenedForWrite?FILE_SHARE_WRITE:0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
				{
					if(!OperationFailed(strViewFileName, MEditTitle, MSG(MEditCannotOpen), false))
						continue;
					else
						return TRUE;
				}
				Edit.Close();
				__int64 FilePos=View.GetFilePos();
				DWORD flags = (GetCanLoseFocus()?FFILEEDIT_ENABLEF6:0)|(SaveToSaveAs?FFILEEDIT_SAVETOSAVEAS:0)|(DisableHistory?FFILEEDIT_DISABLEHISTORY:0);
				FileEditor *ShellEditor = new FileEditor(
					strViewFileName, cp, flags, -2,
					static_cast<int>(FilePos), // TODO: Editor StartChar should be __int64
					str_title.IsEmpty() ? nullptr: &str_title,
					-1,-1, -1, -1, delete_on_close );
				ShellEditor->SetEnableF6(TRUE);
				/* $ 07.05.2001 DJ сохраняем NamesList */
				ShellEditor->SetNamesList(View.GetNamesList());

				// Если переключаемся в редактор, то удалять файл уже не нужно
				SetTempViewName(L"");
				SetExitCode(0);

				FrameManager->DeleteFrame(this); // Insert уже есть внутри конструктора
				ShowTime(2);
			}

			return TRUE;

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			// Работа с локальной копией ViewerOptions
			ViewerConfig(View.ViOpt, true);

			if (Opt.ViOpt.ShowKeyBar)
				ViewKeyBar.Show();

			View.Show();
			return TRUE;
		case KEY_ALTF11:
		case KEY_RALTF11:
			if (GetCanLoseFocus())
				CtrlObject->CmdLine->ShowViewEditHistory();

			return TRUE;
		default:
//      Этот кусок - на будущее (по аналогии с редактором :-)
//      if (CtrlObject->Macro.IsExecuting() || !View.ProcessViewerInput(&ReadRec))
		{
			/* $ 22.03.2001 SVS
			   Это помогло от залипания :-)
			*/
			if (!CtrlObject->Macro.IsExecuting())
				if (Opt.ViOpt.ShowKeyBar)
					ViewKeyBar.Show();

			if (!ViewKeyBar.ProcessKey(Key))
				return(View.ProcessKey(Key));
		}
		return TRUE;
	}
}


int FileViewer::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	F3KeyOnly = false;
	if (!View.ProcessMouse(MouseEvent))
		if (!ViewKeyBar.ProcessMouse(MouseEvent))
			return FALSE;

	return TRUE;
}


int FileViewer::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensView);
	View.GetFileName(strName);
	return(MODALTYPE_VIEWER);
}


void FileViewer::ShowConsoleTitle()
{
	View.ShowConsoleTitle();
	RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(const wchar_t *Name, BOOL DeleteFolder)
{
	delete_on_close = (DeleteFolder ? 1 : 2);
	View.SetTempViewName(Name, DeleteFolder);
}


FileViewer::~FileViewer()
{
	_OT(SysLog(L"[%p] ~FileViewer::FileViewer()",this));
}

void FileViewer::OnDestroy()
{
	_OT(SysLog(L"[%p] FileViewer::OnDestroy()",this));

	if (!DisableHistory && (CtrlObject->Cp()->ActivePanel || StrCmp(strName, L"-")))
	{
		string strFullFileName;
		View.GetFileName(strFullFileName);
		CtrlObject->ViewHistory->AddToHistory(strFullFileName,0);
	}
}

int FileViewer::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_VIEWER;
}

int FileViewer::ViewerControl(int Command,void *Param)
{
	_VCTLLOG(CleverSysLog SL(L"FileViewer::ViewerControl()"));
	_VCTLLOG(SysLog(L"(Command=%s, Param=[%d/0x%08X])",_VCTL_ToName(Command),(int)Param,Param));
	return View.ViewerControl(Command,Param);
}

string &FileViewer::GetTitle(string &Title,int LenTitle,int TruncSize)
{
	return View.GetTitle(Title,LenTitle,TruncSize);
}

__int64 FileViewer::GetViewFileSize() const
{
	return View.GetViewFileSize();
}

__int64 FileViewer::GetViewFilePos() const
{
	return View.GetViewFilePos();
}

void FileViewer::ShowStatus()
{
	string strName;
	string strStatus;

	if (!IsTitleBarVisible())
		return;

	GetTitle(strName);
	int NameLength = ScrX+1 - 40;

	if (Opt.ViewerEditorClock && IsFullScreen())
		NameLength -= 3+5;

	NameLength = Max(NameLength, 20);

	TruncPathStr(strName, NameLength);
	const wchar_t *lpwszStatusFormat = L"%-*s %c %5u %13I64u %7.7s %-4I64d %3d%%";
	strStatus.Format(
	    lpwszStatusFormat,
	    NameLength,
	    strName.CPtr(),
		 L"thd"[View.VM.Hex],
	    View.VM.CodePage,
	    View.FileSize,
	    MSG(MViewerStatusCol),
	    View.LeftPos,
	    (View.LastPage ? 100:ToPercent64(View.FilePos,View.FileSize))
	);
	SetColor(COL_VIEWERSTATUS);
	GotoXY(X1,Y1);
	FS<<fmt::LeftAlign()<<fmt::ExactWidth(View.Width+(View.ViOpt.ShowScrollbar?1:0))<<strStatus;

	if (Opt.ViewerEditorClock && IsFullScreen())
		ShowTime(FALSE);
}

void FileViewer::OnChangeFocus(int focus)
{
	Frame::OnChangeFocus(focus);
	CtrlObject->Plugins->CurViewer=&View;
	int FCurViewerID=View.ViewerID;
	CtrlObject->Plugins->ProcessViewerEvent(focus?VE_GOTFOCUS:VE_KILLFOCUS,nullptr,FCurViewerID);
}

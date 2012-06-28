/*
filepanels.cpp

Файловые панели
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

#include "filepanels.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "ctrlobj.hpp"
#include "filelist.hpp"
#include "rdrwdsk.hpp"
#include "cmdline.hpp"
#include "treelist.hpp"
#include "qview.hpp"
#include "infolist.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "findfile.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "syslog.hpp"
#include "options.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "interf.hpp"

FilePanels::FilePanels():
	LastLeftFilePanel(0),
	LastRightFilePanel(0),
	LeftPanel(CreatePanel(Opt.LeftPanel.Type)),
	RightPanel(CreatePanel(Opt.RightPanel.Type)),
	ActivePanel(0),
	LastLeftType(0),
	LastRightType(0),
	LeftStateBeforeHide(0),
	RightStateBeforeHide(0)
{
	_OT(SysLog(L"[%p] FilePanels::FilePanels()", this));
	MacroMode = MACRO_SHELL;
	KeyBarVisible = Opt.ShowKeyBar;
//  SetKeyBar(&MainKeyBar);
//  _D(SysLog(L"MainKeyBar=0x%p",&MainKeyBar));
}

static void PrepareOptFolder(string &strSrc, int IsLocalPath_FarPath)
{
	if (strSrc.IsEmpty())
	{
		strSrc = g_strFarPath;
		DeleteEndSlash(strSrc);
	}
	else
	{
		apiExpandEnvironmentStrings(strSrc, strSrc);
	}

	if (!StrCmp(strSrc,L"/"))
	{
		strSrc = g_strFarPath;

		if (IsLocalPath_FarPath)
		{
			strSrc.SetLength(2);
			strSrc += L"\\";
		}
	}
	else
	{
		CheckShortcutFolder(&strSrc,FALSE,TRUE);
	}

	//ConvertNameToFull(strSrc,strSrc);
}

void FilePanels::Init(int DirCount)
{
	SetPanelPositions(FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
	                  FileList::IsModeFullScreen(Opt.RightPanel.ViewMode));
	LeftPanel->SetViewMode(Opt.LeftPanel.ViewMode);
	RightPanel->SetViewMode(Opt.RightPanel.ViewMode);
	LeftPanel->SetSortMode(Opt.LeftPanel.SortMode);
	RightPanel->SetSortMode(Opt.RightPanel.SortMode);
	LeftPanel->SetNumericSort(Opt.LeftPanel.NumericSort);
	RightPanel->SetNumericSort(Opt.RightPanel.NumericSort);
	LeftPanel->SetCaseSensitiveSort(Opt.LeftPanel.CaseSensitiveSort);
	RightPanel->SetCaseSensitiveSort(Opt.RightPanel.CaseSensitiveSort);
	LeftPanel->SetSortOrder(Opt.LeftPanel.SortOrder);
	RightPanel->SetSortOrder(Opt.RightPanel.SortOrder);
	LeftPanel->SetSortGroups(Opt.LeftPanel.SortGroups);
	RightPanel->SetSortGroups(Opt.RightPanel.SortGroups);
	LeftPanel->SetShowShortNamesMode(Opt.LeftPanel.ShowShortNames);
	RightPanel->SetShowShortNamesMode(Opt.RightPanel.ShowShortNames);
	LeftPanel->SetSelectedFirstMode(Opt.LeftSelectedFirst);
	RightPanel->SetSelectedFirstMode(Opt.RightSelectedFirst);
	LeftPanel->SetDirectoriesFirst(Opt.LeftPanel.DirectoriesFirst);
	RightPanel->SetDirectoriesFirst(Opt.RightPanel.DirectoriesFirst);
	SetCanLoseFocus(TRUE);
	Panel *PassivePanel=nullptr;
	int PassiveIsLeftFlag=TRUE;

	if (Opt.LeftPanel.Focus)
	{
		ActivePanel=LeftPanel;
		PassivePanel=RightPanel;
		PassiveIsLeftFlag=FALSE;
	}
	else
	{
		ActivePanel=RightPanel;
		PassivePanel=LeftPanel;
		PassiveIsLeftFlag=TRUE;
	}

	ActivePanel->SetFocus();
	// пытаемся избавится от зависания при запуске
	int IsLocalPath_FarPath = ParsePath(g_strFarPath)==PATH_DRIVELETTER;
	string strLeft = Opt.strLeftFolder.Get(), strRight = Opt.strRightFolder.Get();
	PrepareOptFolder(strLeft, IsLocalPath_FarPath);
	PrepareOptFolder(strRight, IsLocalPath_FarPath);
	Opt.strLeftFolder = strLeft;
	Opt.strRightFolder = strRight;

	if (Opt.AutoSaveSetup || !DirCount)
	{
		if (apiGetFileAttributes(Opt.strLeftFolder)!=INVALID_FILE_ATTRIBUTES)
			LeftPanel->InitCurDir(Opt.strLeftFolder);

		if (apiGetFileAttributes(Opt.strRightFolder)!=INVALID_FILE_ATTRIBUTES)
			RightPanel->InitCurDir(Opt.strRightFolder);
	}

	if (!Opt.AutoSaveSetup)
	{
		if (DirCount >= 1)
		{
			if (ActivePanel==RightPanel)
			{
				if (apiGetFileAttributes(Opt.strRightFolder)!=INVALID_FILE_ATTRIBUTES)
					RightPanel->InitCurDir(Opt.strRightFolder);
			}
			else
			{
				if (apiGetFileAttributes(Opt.strLeftFolder)!=INVALID_FILE_ATTRIBUTES)
					LeftPanel->InitCurDir(Opt.strLeftFolder);
			}

			if (DirCount == 2)
			{
				if (ActivePanel==LeftPanel)
				{
					if (apiGetFileAttributes(Opt.strRightFolder)!=INVALID_FILE_ATTRIBUTES)
						RightPanel->InitCurDir(Opt.strRightFolder);
				}
				else
				{
					if (apiGetFileAttributes(Opt.strLeftFolder)!=INVALID_FILE_ATTRIBUTES)
						LeftPanel->InitCurDir(Opt.strLeftFolder);
				}
			}
		}

		const string& PassiveFolder=PassiveIsLeftFlag?Opt.strLeftFolder:Opt.strRightFolder;

		if (DirCount < 2 && *PassiveFolder && (apiGetFileAttributes(PassiveFolder)!=INVALID_FILE_ATTRIBUTES))
		{
			PassivePanel->InitCurDir(PassiveFolder);
		}
	}

#if 1

	//! Вначале "показываем" пассивную панель
	if (PassiveIsLeftFlag)
	{
		if (Opt.LeftPanel.Visible)
		{
			LeftPanel->Show();
		}

		if (Opt.RightPanel.Visible)
		{
			RightPanel->Show();
		}
	}
	else
	{
		if (Opt.RightPanel.Visible)
		{
			RightPanel->Show();
		}

		if (Opt.LeftPanel.Visible)
		{
			LeftPanel->Show();
		}
	}

#endif

	// при понашенных панелях не забыть бы выставить корректно каталог в CmdLine
	if (!Opt.RightPanel.Visible && !Opt.LeftPanel.Visible)
	{
		CtrlObject->CmdLine->SetCurDir(PassiveIsLeftFlag?Opt.strRightFolder:Opt.strLeftFolder);
	}

	SetKeyBar(&MainKeyBar);
	MainKeyBar.SetOwner(this);
}

FilePanels::~FilePanels()
{
	_OT(SysLog(L"[%p] FilePanels::~FilePanels()", this));

	if (LastLeftFilePanel!=LeftPanel && LastLeftFilePanel!=RightPanel)
		DeletePanel(LastLeftFilePanel);

	if (LastRightFilePanel!=LeftPanel && LastRightFilePanel!=RightPanel)
		DeletePanel(LastRightFilePanel);

	DeletePanel(LeftPanel);
	LeftPanel=nullptr;
	DeletePanel(RightPanel);
	RightPanel=nullptr;
}

void FilePanels::SetPanelPositions(bool LeftFullScreen, bool RightFullScreen)
{
	if (Opt.WidthDecrement < -(ScrX/2-10))
		Opt.WidthDecrement=-(ScrX/2-10);

	if (Opt.WidthDecrement > (ScrX/2-10))
		Opt.WidthDecrement=(ScrX/2-10);

	Opt.LeftHeightDecrement=Max(0,Min(Opt.LeftHeightDecrement.Get(),ScrY-7));
	Opt.RightHeightDecrement=Max(0,Min(Opt.RightHeightDecrement.Get(),ScrY-7));

	if (LeftFullScreen)
	{
		LeftPanel->SetPosition(0,Opt.ShowMenuBar?1:0,ScrX,ScrY-1-(Opt.ShowKeyBar)-Opt.LeftHeightDecrement);
		LeftPanel->ViewSettings.Flags|=PVS_FULLSCREEN;
	}
	else
	{
		LeftPanel->SetPosition(0,Opt.ShowMenuBar?1:0,ScrX/2-Opt.WidthDecrement,ScrY-1-(Opt.ShowKeyBar)-Opt.LeftHeightDecrement);
	}

	if (RightFullScreen)
	{
		RightPanel->SetPosition(0,Opt.ShowMenuBar?1:0,ScrX,ScrY-1-(Opt.ShowKeyBar)-Opt.RightHeightDecrement);
		RightPanel->ViewSettings.Flags|=PVS_FULLSCREEN;
	}
	else
	{
		RightPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Opt.ShowMenuBar?1:0,ScrX,ScrY-1-(Opt.ShowKeyBar)-Opt.RightHeightDecrement);
	}
}

void FilePanels::SetScreenPosition()
{
	_OT(SysLog(L"[%p] FilePanels::SetScreenPosition() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
	CtrlObject->CmdLine->SetPosition(0,ScrY-(Opt.ShowKeyBar),ScrX-1,ScrY-(Opt.ShowKeyBar));
	TopMenuBar.SetPosition(0,0,ScrX,0);
	MainKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
	SetPanelPositions(LeftPanel->IsFullScreen(),RightPanel->IsFullScreen());
	SetPosition(0,0,ScrX,ScrY);
}

void FilePanels::RedrawKeyBar()
{
	ActivePanel->UpdateKeyBar();
	MainKeyBar.Redraw();
}


Panel* FilePanels::CreatePanel(int Type)
{
	Panel *pResult = nullptr;

	switch (Type)
	{
		case FILE_PANEL:
			pResult = new FileList;
			break;
		case TREE_PANEL:
			pResult = new TreeList;
			break;
		case QVIEW_PANEL:
			pResult = new QuickView;
			break;
		case INFO_PANEL:
			pResult = new InfoList;
			break;
	}

	if (pResult)
		pResult->SetOwner(this);

	return pResult;
}


void FilePanels::DeletePanel(Panel *Deleted)
{
	if (!Deleted)
		return;

	if (Deleted==LastLeftFilePanel)
		LastLeftFilePanel=nullptr;

	if (Deleted==LastRightFilePanel)
		LastRightFilePanel=nullptr;

	delete Deleted;
}

int FilePanels::SetAnhoterPanelFocus()
{
	int Ret=FALSE;

	if (ActivePanel==LeftPanel)
	{
		if (RightPanel->IsVisible())
		{
			RightPanel->SetFocus();
			Ret=TRUE;
		}
	}
	else
	{
		if (LeftPanel->IsVisible())
		{
			LeftPanel->SetFocus();
			Ret=TRUE;
		}
	}

	return Ret;
}


int FilePanels::SwapPanels()
{
	int Ret=FALSE; // это значит ни одна из панелей не видна

	if (LeftPanel->IsVisible() || RightPanel->IsVisible())
	{
		int XL1,YL1,XL2,YL2;
		int XR1,YR1,XR2,YR2;
		LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
		RightPanel->GetPosition(XR1,YR1,XR2,YR2);

		if (!LeftPanel->IsFullScreen() || !RightPanel->IsFullScreen())
		{
			Opt.WidthDecrement=-Opt.WidthDecrement;

			Opt.LeftHeightDecrement^=Opt.RightHeightDecrement;
			Opt.RightHeightDecrement=Opt.LeftHeightDecrement^Opt.RightHeightDecrement;
			Opt.LeftHeightDecrement^=Opt.RightHeightDecrement;

		}

		Panel *Swap;
		int SwapType;
		Swap=LeftPanel;
		LeftPanel=RightPanel;
		RightPanel=Swap;
		Swap=LastLeftFilePanel;
		LastLeftFilePanel=LastRightFilePanel;
		LastRightFilePanel=Swap;
		SwapType=LastLeftType;
		LastLeftType=LastRightType;
		LastRightType=SwapType;
		FileFilter::SwapFilter();
		Ret=TRUE;
	}
	SetScreenPosition();
	FrameManager->RefreshFrame();
	return Ret;
}

#ifdef FAR_LUA
#else
__int64 FilePanels::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Opt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Opt.ShowKeyBar=1;
				MainKeyBar.Show();
				KeyBarVisible = Opt.ShowKeyBar;
				SetScreenPosition();
				FrameManager->RefreshFrame();
				break;
			case 2:
				Opt.ShowKeyBar=0;
				MainKeyBar.Hide();
				KeyBarVisible = Opt.ShowKeyBar;
				SetScreenPosition();
				FrameManager->RefreshFrame();
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
	return ActivePanel->VMProcess(OpCode,vParam,iParam);
}
#endif

int FilePanels::ProcessKey(int Key)
{
	if (!Key)
		return TRUE;

	if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD4 || Key==KEY_CTRLNUMPAD6
		|| Key==KEY_RCTRLLEFT || Key==KEY_RCTRLRIGHT || Key==KEY_RCTRLNUMPAD4 || Key==KEY_RCTRLNUMPAD6
	        /* || Key==KEY_CTRLUP   || Key==KEY_CTRLDOWN || Key==KEY_CTRLNUMPAD8 || Key==KEY_CTRLNUMPAD2 */) &&
	        (CtrlObject->CmdLine->GetLength()>0 ||
	         (!LeftPanel->IsVisible() && !RightPanel->IsVisible())))
	{
		CtrlObject->CmdLine->ProcessKey(Key);
		return TRUE;
	}

	switch (Key)
	{
		case KEY_F1:
		{
			if (!ActivePanel->ProcessKey(KEY_F1))
			{
				Help Hlp(L"Contents");
			}

			return TRUE;
		}
		case KEY_TAB:
		{
			SetAnhoterPanelFocus();
			break;
		}
		case KEY_CTRLF1:
		case KEY_RCTRLF1:
		{
			if (LeftPanel->IsVisible())
			{
				LeftPanel->Hide();

				if (RightPanel->IsVisible())
					RightPanel->SetFocus();
			}
			else
			{
				if (!RightPanel->IsVisible())
					LeftPanel->SetFocus();

				LeftPanel->Show();
			}

			Redraw();
			break;
		}
		case KEY_CTRLF2:
		case KEY_RCTRLF2:
		{
			if (RightPanel->IsVisible())
			{
				RightPanel->Hide();

				if (LeftPanel->IsVisible())
					LeftPanel->SetFocus();
			}
			else
			{
				if (!LeftPanel->IsVisible())
					RightPanel->SetFocus();

				RightPanel->Show();
			}

			Redraw();
			break;
		}
		case KEY_CTRLB:
		case KEY_RCTRLB:
		{
			Opt.ShowKeyBar=!Opt.ShowKeyBar;
			KeyBarVisible = Opt.ShowKeyBar;

			if (!KeyBarVisible)
				MainKeyBar.Hide();

			SetScreenPosition();
			FrameManager->RefreshFrame();
			break;
		}
		case KEY_CTRLL: case KEY_RCTRLL:
		case KEY_CTRLQ: case KEY_RCTRLQ:
		case KEY_CTRLT: case KEY_RCTRLT:
		{
			if (ActivePanel->IsVisible())
			{
				Panel *AnotherPanel=GetAnotherPanel(ActivePanel);
				int NewType;

				if (Key==KEY_CTRLL || Key==KEY_RCTRLL)
					NewType=INFO_PANEL;
				else if (Key==KEY_CTRLQ || Key==KEY_RCTRLQ)
					NewType=QVIEW_PANEL;
				else
					NewType=TREE_PANEL;

				if (ActivePanel->GetType()==NewType)
					AnotherPanel=ActivePanel;

				if (!AnotherPanel->ProcessPluginEvent(FE_CLOSE,nullptr))
				{
					if (AnotherPanel->GetType()==NewType)
						/* $ 19.09.2000 IS
						  Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
						*/
						AnotherPanel=ChangePanel(AnotherPanel,FILE_PANEL,FALSE,FALSE);
					else
						AnotherPanel=ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

					/* $ 07.09.2001 VVM
					  ! При возврате из CTRL+Q, CTRL+L восстановим каталог, если активная панель - дерево. */
					if (ActivePanel->GetType() == TREE_PANEL)
					{
						string strCurDir;
						ActivePanel->GetCurDir(strCurDir);
						AnotherPanel->SetCurDir(strCurDir, TRUE);
						AnotherPanel->Update(0);
					}
					else
						AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					AnotherPanel->Show();
				}

				ActivePanel->SetFocus();
			}

			break;
		}
		case KEY_CTRLO:
		case KEY_RCTRLO:
		{
			{
				int LeftVisible=LeftPanel->IsVisible();
				int RightVisible=RightPanel->IsVisible();
				int HideState=!LeftVisible && !RightVisible;

				if (!HideState)
				{
					LeftStateBeforeHide=LeftVisible;
					RightStateBeforeHide=RightVisible;
					LeftPanel->Hide();
					RightPanel->Hide();
					FrameManager->RefreshFrame();
				}
				else
				{
					if (!LeftStateBeforeHide && !RightStateBeforeHide)
						LeftStateBeforeHide=RightStateBeforeHide=TRUE;

					if (LeftStateBeforeHide)
						LeftPanel->Show();

					if (RightStateBeforeHide)
						RightPanel->Show();

					if (!ActivePanel->IsVisible())
					{
						if (ActivePanel == RightPanel)
							LeftPanel->SetFocus();
						else
							RightPanel->SetFocus();
					}
				}
			}
			break;
		}
		case KEY_CTRLP:
		case KEY_RCTRLP:
		{
			if (ActivePanel->IsVisible())
			{
				Panel *AnotherPanel=GetAnotherPanel(ActivePanel);

				if (AnotherPanel->IsVisible())
					AnotherPanel->Hide();
				else
					AnotherPanel->Show();

				CtrlObject->CmdLine->Redraw();
			}

			FrameManager->RefreshFrame();
			break;
		}
		case KEY_CTRLI:
		case KEY_RCTRLI:
		{
			ActivePanel->EditFilter();
			return TRUE;
		}
		case KEY_CTRLU:
		case KEY_RCTRLU:
		{
			if (!LeftPanel->IsVisible() && !RightPanel->IsVisible())
				CtrlObject->CmdLine->ProcessKey(Key);
			else
				SwapPanels();

			break;
		}
		/* $ 08.04.2002 IS
		   При смене диска установим принудительно текущий каталог на активной
		   панели, т.к. система не знает ничего о том, что у Фара две панели, и
		   текущим для системы после смены диска может быть каталог и на пассивной
		   панели
		*/
		case KEY_ALTF1:
		case KEY_RALTF1:
		{
			LeftPanel->ChangeDisk();

			if (ActivePanel!=LeftPanel)
				ActivePanel->SetCurPath();

			break;
		}
		case KEY_ALTF2:
		case KEY_RALTF2:
		{
			RightPanel->ChangeDisk();

			if (ActivePanel!=RightPanel)
				ActivePanel->SetCurPath();

			break;
		}
		case KEY_ALTF7:
		case KEY_RALTF7:
		{
			{
				FindFiles FindFiles;
			}
			break;
		}
		case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP: case KEY_RCTRLNUMPAD8:
		{
			bool Set=false;
			if (Opt.LeftHeightDecrement<ScrY-7)
			{
				Opt.LeftHeightDecrement++;
				Set=true;
			}
			if (Opt.RightHeightDecrement<ScrY-7)
			{
				Opt.RightHeightDecrement++;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
		{
			bool Set=false;
			if (Opt.LeftHeightDecrement>0)
			{
				Opt.LeftHeightDecrement--;
				Set=true;
			}
			if (Opt.RightHeightDecrement>0)
			{
				Opt.RightHeightDecrement--;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}

		case KEY_CTRLSHIFTUP:  case KEY_CTRLSHIFTNUMPAD8:
		case KEY_RCTRLSHIFTUP: case KEY_RCTRLSHIFTNUMPAD8:
		{
			IntOption& HeightDecrement=(ActivePanel==LeftPanel)?Opt.LeftHeightDecrement:Opt.RightHeightDecrement;
			if (HeightDecrement<ScrY-7)
			{
				HeightDecrement++;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}
			break;
		}

		case KEY_CTRLSHIFTDOWN:  case KEY_CTRLSHIFTNUMPAD2:
		case KEY_RCTRLSHIFTDOWN: case KEY_RCTRLSHIFTNUMPAD2:
		{
			IntOption& HeightDecrement=(ActivePanel==LeftPanel)?Opt.LeftHeightDecrement:Opt.RightHeightDecrement;
			if (HeightDecrement>0)
			{
				HeightDecrement--;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}
			break;
		}

		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			if (Opt.WidthDecrement<ScrX/2-10)
			{
				Opt.WidthDecrement++;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
		{
			if (Opt.WidthDecrement>-(ScrX/2-10))
			{
				Opt.WidthDecrement--;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLCLEAR:
		case KEY_RCTRLCLEAR:
		{
			if (Opt.WidthDecrement)
			{
				Opt.WidthDecrement=0;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLALTCLEAR:
		case KEY_RCTRLRALTCLEAR:
		case KEY_CTRLRALTCLEAR:
		case KEY_RCTRLALTCLEAR:
		{
			bool Set=false;
			if (Opt.LeftHeightDecrement)
			{
				Opt.LeftHeightDecrement=0;
				Set=true;
			}
			if (Opt.RightHeightDecrement)
			{
				Opt.RightHeightDecrement=0;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_F9:
		{
			ShellOptions(0,nullptr);
			return TRUE;
		}
		case KEY_SHIFTF10:
		{
			ShellOptions(1,nullptr);
			return TRUE;
		}
		default:
		{
			if (Key >= KEY_CTRL0 && Key <= KEY_CTRL9)
				ChangePanelViewMode(ActivePanel,Key-KEY_CTRL0,TRUE);
			if (!ActivePanel->ProcessKey(Key))
				CtrlObject->CmdLine->ProcessKey(Key);

			break;
		}
	}

	return TRUE;
}

int FilePanels::ChangePanelViewMode(Panel *Current,int Mode,BOOL RefreshFrame)
{
	if (Current && Mode >= VIEW_0 && Mode <= VIEW_9)
	{
		Current->SetViewMode(Mode);
		Current=ChangePanelToFilled(Current,FILE_PANEL);
		Current->SetViewMode(Mode);
		// ВНИМАНИЕ! Костыль! Но Работает!
		SetScreenPosition();

		if (RefreshFrame)
			FrameManager->RefreshFrame();

		return TRUE;
	}

	return FALSE;
}

Panel* FilePanels::ChangePanelToFilled(Panel *Current,int NewType)
{
	if (Current->GetType()!=NewType && !Current->ProcessPluginEvent(FE_CLOSE,nullptr))
	{
		Current->Hide();
		Current=ChangePanel(Current,NewType,FALSE,FALSE);
		Current->Update(0);
		Current->Show();

		if (!GetAnotherPanel(Current)->GetFocus())
			Current->SetFocus();
	}

	return(Current);
}

Panel* FilePanels::GetAnotherPanel(Panel *Current)
{
	if (Current==LeftPanel)
		return(RightPanel);
	else
		return(LeftPanel);
}


Panel* FilePanels::ChangePanel(Panel *Current,int NewType,int CreateNew,int Force)
{
	Panel *NewPanel;
	SaveScreen *SaveScr=nullptr;
	// OldType не инициализировался...
	int OldType=Current->GetType(),X1,Y1,X2,Y2;
	int OldPanelMode=Current->GetMode();

	if (!Force && NewType==OldType && OldPanelMode==NORMAL_PANEL)
		return(Current);

	int UseLastPanel=0;
	int OldViewMode=Current->GetPrevViewMode();
	bool OldFullScreen=Current->IsFullScreen();
	int OldSortMode=Current->GetPrevSortMode();
	int OldSortOrder=Current->GetPrevSortOrder();
	int OldNumericSort=Current->GetPrevNumericSort();
	int OldCaseSensitiveSort=Current->GetPrevCaseSensitiveSort();
	int OldSortGroups=Current->GetSortGroups();
	int OldShowShortNames=Current->GetShowShortNamesMode();
	int OldFocus=Current->GetFocus();
	int OldSelectedFirst=Current->GetSelectedFirstMode();
	int OldDirectoriesFirst=Current->GetPrevDirectoriesFirst();
	int LeftPosition=(Current==LeftPanel);

	Panel *(&LastFilePanel)=LeftPosition ? LastLeftFilePanel:LastRightFilePanel;
	Current->GetPosition(X1,Y1,X2,Y2);
	int ChangePosition=((OldType==FILE_PANEL && NewType!=FILE_PANEL &&
	                    OldFullScreen) || (NewType==FILE_PANEL &&
	                                    ((OldFullScreen && !FileList::IsModeFullScreen(OldViewMode)) ||
	                                     (!OldFullScreen && FileList::IsModeFullScreen(OldViewMode)))));

	if (!ChangePosition)
	{
		SaveScr=Current->SaveScr;
		Current->SaveScr=nullptr;
	}

	if (OldType==FILE_PANEL && NewType!=FILE_PANEL)
	{
		delete Current->SaveScr;
		Current->SaveScr=nullptr;

		if (LastFilePanel!=Current)
		{
			DeletePanel(LastFilePanel);
			LastFilePanel=Current;
		}

		LastFilePanel->Hide();

		if (LastFilePanel->SaveScr)
		{
			LastFilePanel->SaveScr->Discard();
			delete LastFilePanel->SaveScr;
			LastFilePanel->SaveScr=nullptr;
		}
	}
	else
	{
		Current->Hide();
		DeletePanel(Current);

		if (OldType==FILE_PANEL && NewType==FILE_PANEL)
		{
			DeletePanel(LastFilePanel);
			LastFilePanel=nullptr;
		}
	}

	if (!CreateNew && NewType==FILE_PANEL && LastFilePanel)
	{
		int LastX1,LastY1,LastX2,LastY2;
		LastFilePanel->GetPosition(LastX1,LastY1,LastX2,LastY2);

		if (LastFilePanel->IsFullScreen())
			LastFilePanel->SetPosition(LastX1,Y1,LastX2,Y2);
		else
			LastFilePanel->SetPosition(X1,Y1,X2,Y2);

		NewPanel=LastFilePanel;

		if (!ChangePosition)
		{
			if ((NewPanel->IsFullScreen() && !OldFullScreen) ||
			        (!NewPanel->IsFullScreen() && OldFullScreen))
			{
				Panel *AnotherPanel=GetAnotherPanel(Current);

				if (SaveScr && AnotherPanel->IsVisible() &&
				        AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen())
					SaveScr->Discard();

				delete SaveScr;
			}
			else
				NewPanel->SaveScr=SaveScr;
		}

		if (!OldFocus && NewPanel->GetFocus())
			NewPanel->KillFocus();

		UseLastPanel=TRUE;
	}
	else
		NewPanel=CreatePanel(NewType);

	if (Current==ActivePanel)
		ActivePanel=NewPanel;

	if (LeftPosition)
	{
		LeftPanel=NewPanel;
		LastLeftType=OldType;
	}
	else
	{
		RightPanel=NewPanel;
		LastRightType=OldType;
	}

	if (!UseLastPanel)
	{
		if (ChangePosition)
		{
			if (LeftPosition)
			{
				NewPanel->SetPosition(0,Y1,ScrX/2-Opt.WidthDecrement,Y2);
				RightPanel->Redraw();
			}
			else
			{
				NewPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Y1,ScrX,Y2);
				LeftPanel->Redraw();
			}
		}
		else
		{
			NewPanel->SaveScr=SaveScr;
			NewPanel->SetPosition(X1,Y1,X2,Y2);
		}

		NewPanel->SetSortMode(OldSortMode);
		NewPanel->SetSortOrder(OldSortOrder);
		NewPanel->SetNumericSort(OldNumericSort);
		NewPanel->SetCaseSensitiveSort(OldCaseSensitiveSort);
		NewPanel->SetSortGroups(OldSortGroups);
		NewPanel->SetShowShortNamesMode(OldShowShortNames);
		NewPanel->SetPrevViewMode(OldViewMode);
		NewPanel->SetViewMode(OldViewMode);
		NewPanel->SetSelectedFirstMode(OldSelectedFirst);
		NewPanel->SetDirectoriesFirst(OldDirectoriesFirst);
	}

	return(NewPanel);
}

int  FilePanels::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensPanels);
	string strFullName, strShortName;

	switch (ActivePanel->GetType())
	{
		case TREE_PANEL:
		case QVIEW_PANEL:
		case FILE_PANEL:
		case INFO_PANEL:
			ActivePanel->GetCurName(strFullName, strShortName);
			ConvertNameToFull(strFullName, strFullName);
			break;
	}

	strName = strFullName;
	return(MODALTYPE_PANELS);
}

void FilePanels::OnChangeFocus(int f)
{
	_OT(SysLog(L"FilePanels::OnChangeFocus(%i)",f));

	/* $ 20.06.2001 tran
	   баг с отрисовкой при копировании и удалении
	   не учитывался LockRefreshCount */
	if (f)
	{
		/*$ 22.06.2001 SKV
		  + update панелей при получении фокуса
		*/
		CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
		ActivePanel->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
		/* $ 13.04.2002 KM
		  ! ??? Я не понял зачем здесь Redraw, если
		    Redraw вызывается следом во Frame::OnChangeFocus.
		*/
//    Redraw();
		ActivePanel->SetCurPath();
		Frame::OnChangeFocus(1);
	}
}

void FilePanels::DisplayObject()
{
//  if ( !Focus )
//      return;
	_OT(SysLog(L"[%p] FilePanels::Redraw() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
	CtrlObject->CmdLine->ShowBackground();

	if (Opt.ShowMenuBar)
		CtrlObject->TopMenuBar->Show();

	CtrlObject->CmdLine->Show();

	if (Opt.ShowKeyBar)
		MainKeyBar.Show();
	else if (MainKeyBar.IsVisible())
		MainKeyBar.Hide();

	KeyBarVisible=Opt.ShowKeyBar;
#if 1

	if (LeftPanel->IsVisible())
		LeftPanel->Show();

	if (RightPanel->IsVisible())
		RightPanel->Show();

#else
	Panel *PassivePanel=nullptr;
	int PassiveIsLeftFlag=TRUE;

	if (Opt.LeftPanel.Focus)
	{
		ActivePanel=LeftPanel;
		PassivePanel=RightPanel;
		PassiveIsLeftFlag=FALSE;
	}
	else
	{
		ActivePanel=RightPanel;
		PassivePanel=LeftPanel;
		PassiveIsLeftFlag=TRUE;
	}

	//! Вначале "показываем" пассивную панель
	if (PassiveIsLeftFlag)
	{
		if (Opt.LeftPanel.Visible)
		{
			LeftPanel->Show();
		}

		if (Opt.RightPanel.Visible)
		{
			RightPanel->Show();
		}
	}
	else
	{
		if (Opt.RightPanel.Visible)
		{
			RightPanel->Show();
		}

		if (Opt.LeftPanel.Visible)
		{
			LeftPanel->Show();
		}
	}

#endif
}

int  FilePanels::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!ActivePanel->ProcessMouse(MouseEvent))
		if (!GetAnotherPanel(ActivePanel)->ProcessMouse(MouseEvent))
			if (!MainKeyBar.ProcessMouse(MouseEvent))
				CtrlObject->CmdLine->ProcessMouse(MouseEvent);

	return TRUE;
}

void FilePanels::ShowConsoleTitle()
{
	if (ActivePanel)
		ActivePanel->SetTitle();
}

void FilePanels::ResizeConsole()
{
	Frame::ResizeConsole();
	CtrlObject->CmdLine->ResizeConsole();
	MainKeyBar.ResizeConsole();
	TopMenuBar.ResizeConsole();
	SetScreenPosition();
	_OT(SysLog(L"[%p] FilePanels::ResizeConsole() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
}

int FilePanels::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_PANEL;
}

void FilePanels::Refresh()
{
	/*$ 31.07.2001 SKV
	  Вызовем так, а не Frame::OnChangeFocus,
	  который из этого и позовётся.
	*/
	//Frame::OnChangeFocus(1);
	OnChangeFocus(1);
}

void FilePanels::GoToFile(const wchar_t *FileName)
{
	if (FirstSlash(FileName))
	{
		string ADir,PDir;
		Panel *PassivePanel = GetAnotherPanel(ActivePanel);
		int PassiveMode = PassivePanel->GetMode();

		if (PassiveMode == NORMAL_PANEL)
		{
			PassivePanel->GetCurDir(PDir);
			AddEndSlash(PDir);
		}

		int ActiveMode = ActivePanel->GetMode();

		if (ActiveMode==NORMAL_PANEL)
		{
			ActivePanel->GetCurDir(ADir);
			AddEndSlash(ADir);
		}

		string strNameFile = PointToName(FileName);
		string strNameDir = FileName;
		CutToSlash(strNameDir);
		/* $ 10.04.2001 IS
		     Не делаем SetCurDir, если нужный путь уже есть на открытых
		     панелях, тем самым добиваемся того, что выделение с элементов
		     панелей не сбрасывается.
		*/
		BOOL AExist=(ActiveMode==NORMAL_PANEL) && !StrCmpI(ADir,strNameDir);
		BOOL PExist=(PassiveMode==NORMAL_PANEL) && !StrCmpI(PDir,strNameDir);

		// если нужный путь есть на пассивной панели
		if (!AExist && PExist)
			ProcessKey(KEY_TAB);

		if (!AExist && !PExist)
			ActivePanel->SetCurDir(strNameDir,TRUE);

		ActivePanel->GoToFile(strNameFile);
		// всегда обновим заголовок панели, чтобы дать обратную связь, что
		// Ctrl-F10 обработан
		ActivePanel->SetTitle();
	}
}


int  FilePanels::GetMacroMode()
{
	switch (ActivePanel->GetType())
	{
		case TREE_PANEL:
			return MACRO_TREEPANEL;
		case QVIEW_PANEL:
			return MACRO_QVIEWPANEL;
		case INFO_PANEL:
			return MACRO_INFOPANEL;
		default:
			return MACRO_SHELL;
	}
}

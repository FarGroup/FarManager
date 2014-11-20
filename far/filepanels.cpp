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
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "interf.hpp"
#include "language.hpp"
#include "config.hpp"
#include "desktop.hpp"
#include "keybar.hpp"
#include "menubar.hpp"

FilePanels::FilePanels():
	LastLeftFilePanel(),
	LastRightFilePanel(),
	LeftPanel(),
	RightPanel(),
	LastLeftType(),
	LastRightType(),
	LeftStateBeforeHide(),
	RightStateBeforeHide(),
	m_ActivePanel()
{
}

filepanels_ptr FilePanels::create(bool CreatePanels, int DirCount)
{
	filepanels_ptr FilePanelsPtr(new FilePanels());

	FilePanelsPtr->m_windowKeyBar = std::make_unique<KeyBar>(FilePanelsPtr);
	FilePanelsPtr->SetMacroMode(MACROAREA_SHELL);
	FilePanelsPtr->m_KeyBarVisible = Global->Opt->ShowKeyBar;

	if (CreatePanels)
	{
		FilePanelsPtr->LeftPanel = FilePanelsPtr->CreatePanel(Global->Opt->LeftPanel.m_Type);
		FilePanelsPtr->RightPanel = FilePanelsPtr->CreatePanel(Global->Opt->RightPanel.m_Type);
		FilePanelsPtr->Init(DirCount);
	}
	return FilePanelsPtr;
}

static void PrepareOptFolder(string &strSrc, int IsLocalPath_FarPath)
{
	if (strSrc.empty())
	{
		strSrc = Global->g_strFarPath;
		DeleteEndSlash(strSrc);
	}
	else
	{
		strSrc = api::env::expand_strings(strSrc);
	}

	if (strSrc == L"/")
	{
		strSrc = Global->g_strFarPath;

		if (IsLocalPath_FarPath)
		{
			strSrc.resize(2);
			strSrc += L"\\";
		}
	}
	else
	{
		CheckShortcutFolder(strSrc,FALSE,TRUE);
	}

	//ConvertNameToFull(strSrc,strSrc);
}

void FilePanels::Init(int DirCount)
{
	SetPanelPositions(FileList::IsModeFullScreen(Global->Opt->LeftPanel.ViewMode),
	                  FileList::IsModeFullScreen(Global->Opt->RightPanel.ViewMode));
	LeftPanel->SetViewMode(Global->Opt->LeftPanel.ViewMode);
	RightPanel->SetViewMode(Global->Opt->RightPanel.ViewMode);

	if (Global->Opt->LeftPanel.SortMode < SORTMODE_COUNT)
		LeftPanel->SetSortMode(Global->Opt->LeftPanel.SortMode);

	if (Global->Opt->RightPanel.SortMode < SORTMODE_COUNT)
		RightPanel->SetSortMode(Global->Opt->RightPanel.SortMode);

	LeftPanel->SetNumericSort(Global->Opt->LeftPanel.NumericSort);
	RightPanel->SetNumericSort(Global->Opt->RightPanel.NumericSort);
	LeftPanel->SetCaseSensitiveSort(Global->Opt->LeftPanel.CaseSensitiveSort);
	RightPanel->SetCaseSensitiveSort(Global->Opt->RightPanel.CaseSensitiveSort);
	LeftPanel->SetSortOrder(Global->Opt->LeftPanel.ReverseSortOrder);
	RightPanel->SetSortOrder(Global->Opt->RightPanel.ReverseSortOrder);
	LeftPanel->SetSortGroups(Global->Opt->LeftPanel.SortGroups);
	RightPanel->SetSortGroups(Global->Opt->RightPanel.SortGroups);
	LeftPanel->SetShowShortNamesMode(Global->Opt->LeftPanel.ShowShortNames);
	RightPanel->SetShowShortNamesMode(Global->Opt->RightPanel.ShowShortNames);
	LeftPanel->SetSelectedFirstMode(Global->Opt->LeftPanel.SelectedFirst);
	RightPanel->SetSelectedFirstMode(Global->Opt->RightPanel.SelectedFirst);
	LeftPanel->SetDirectoriesFirst(Global->Opt->LeftPanel.DirectoriesFirst);
	RightPanel->SetDirectoriesFirst(Global->Opt->RightPanel.DirectoriesFirst);
	SetCanLoseFocus(TRUE);
	Panel *PassivePanel=nullptr;
	int PassiveIsLeftFlag=TRUE;

	if (Global->Opt->LeftFocus)
	{
		m_ActivePanel = LeftPanel;
		PassivePanel=RightPanel;
		PassiveIsLeftFlag=FALSE;
	}
	else
	{
		m_ActivePanel = RightPanel;
		PassivePanel=LeftPanel;
		PassiveIsLeftFlag=TRUE;
	}

	m_ActivePanel->SetFocus();
	// пытаемся избавится от зависания при запуске
	int IsLocalPath_FarPath = ParsePath(Global->g_strFarPath)==PATH_DRIVELETTER;
	string strLeft = Global->Opt->LeftPanel.Folder.Get(), strRight = Global->Opt->RightPanel.Folder.Get();
	PrepareOptFolder(strLeft, IsLocalPath_FarPath);
	PrepareOptFolder(strRight, IsLocalPath_FarPath);
	Global->Opt->LeftPanel.Folder = strLeft;
	Global->Opt->RightPanel.Folder = strRight;

	if (Global->Opt->AutoSaveSetup || !DirCount)
	{
		LeftPanel->InitCurDir(api::GetFileAttributes(Global->Opt->LeftPanel.Folder)!=INVALID_FILE_ATTRIBUTES? Global->Opt->LeftPanel.Folder.Get() : Global->g_strFarPath);
		RightPanel->InitCurDir(api::GetFileAttributes(Global->Opt->RightPanel.Folder)!=INVALID_FILE_ATTRIBUTES? Global->Opt->RightPanel.Folder.Get() : Global->g_strFarPath);
	}

	if (!Global->Opt->AutoSaveSetup)
	{
		if (DirCount >= 1)
		{
			if (m_ActivePanel == RightPanel)
			{
				RightPanel->InitCurDir(api::GetFileAttributes(Global->Opt->RightPanel.Folder)!=INVALID_FILE_ATTRIBUTES? Global->Opt->RightPanel.Folder.Get() : Global->g_strFarPath);
			}
			else
			{
				LeftPanel->InitCurDir(api::GetFileAttributes(Global->Opt->LeftPanel.Folder)!=INVALID_FILE_ATTRIBUTES? Global->Opt->LeftPanel.Folder.Get() : Global->g_strFarPath);
			}

			if (DirCount == 2)
			{
				if (m_ActivePanel == LeftPanel)
				{
					RightPanel->InitCurDir(api::GetFileAttributes(Global->Opt->RightPanel.Folder)!=INVALID_FILE_ATTRIBUTES? Global->Opt->RightPanel.Folder.Get() : Global->g_strFarPath);
				}
				else
				{
					LeftPanel->InitCurDir(api::GetFileAttributes(Global->Opt->LeftPanel.Folder)!=INVALID_FILE_ATTRIBUTES? Global->Opt->LeftPanel.Folder.Get() : Global->g_strFarPath);
				}
			}
		}

		const string& PassiveFolder=PassiveIsLeftFlag?Global->Opt->LeftPanel.Folder:Global->Opt->RightPanel.Folder;

		if (DirCount < 2 && !PassiveFolder.empty() && (api::GetFileAttributes(PassiveFolder)!=INVALID_FILE_ATTRIBUTES))
		{
			PassivePanel->InitCurDir(PassiveFolder);
		}
	}

	CmdLine = std::make_unique<CommandLine>(shared_from_this());
	TopMenuBar = std::make_unique<MenuBar>(shared_from_this());

#if 1

	//! Вначале "показываем" пассивную панель
	if (PassiveIsLeftFlag)
	{
		if (Global->Opt->LeftPanel.Visible)
		{
			LeftPanel->Show();
		}

		if (Global->Opt->RightPanel.Visible)
		{
			RightPanel->Show();
		}
	}
	else
	{
		if (Global->Opt->RightPanel.Visible)
		{
			RightPanel->Show();
		}

		if (Global->Opt->LeftPanel.Visible)
		{
			LeftPanel->Show();
		}
	}

#endif

	// при погашенных панелях не забыть бы выставить корректно каталог в CmdLine
	if (!Global->Opt->RightPanel.Visible && !Global->Opt->LeftPanel.Visible)
	{
		CmdLine->SetCurDir(PassiveIsLeftFlag?Global->Opt->RightPanel.Folder:Global->Opt->LeftPanel.Folder);
	}

	SetScreenPosition();
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
	if (Global->Opt->WidthDecrement < -(ScrX/2-10))
		Global->Opt->WidthDecrement=-(ScrX/2-10);

	if (Global->Opt->WidthDecrement > (ScrX/2-10))
		Global->Opt->WidthDecrement=(ScrX/2-10);

	Global->Opt->LeftHeightDecrement=std::max(0ll, std::min(Global->Opt->LeftHeightDecrement.Get(), ScrY-7ll));
	Global->Opt->RightHeightDecrement=std::max(0ll, std::min(Global->Opt->RightHeightDecrement.Get(), ScrY-7ll));

	if (LeftFullScreen)
	{
		LeftPanel->SetPosition(0,Global->Opt->ShowMenuBar?1:0,ScrX,ScrY-1-(Global->Opt->ShowKeyBar)-Global->Opt->LeftHeightDecrement);
		LeftPanel->SetFullScreen();
	}
	else
	{
		LeftPanel->SetPosition(0,Global->Opt->ShowMenuBar?1:0,ScrX/2-Global->Opt->WidthDecrement,ScrY-1-(Global->Opt->ShowKeyBar)-Global->Opt->LeftHeightDecrement);
	}

	if (RightFullScreen)
	{
		RightPanel->SetPosition(0,Global->Opt->ShowMenuBar?1:0,ScrX,ScrY-1-(Global->Opt->ShowKeyBar)-Global->Opt->RightHeightDecrement);
		RightPanel->SetFullScreen();
	}
	else
	{
		RightPanel->SetPosition(ScrX/2+1-Global->Opt->WidthDecrement,Global->Opt->ShowMenuBar?1:0,ScrX,ScrY-1-(Global->Opt->ShowKeyBar)-Global->Opt->RightHeightDecrement);
	}
}

void FilePanels::SetScreenPosition()
{
	_OT(SysLog(L"[%p] FilePanels::SetScreenPosition() {%d, %d - %d, %d}", this,m_X1,m_Y1,m_X2,m_Y2));
	CmdLine->SetPosition(0,ScrY-(Global->Opt->ShowKeyBar),ScrX-1,ScrY-(Global->Opt->ShowKeyBar));
	TopMenuBar->SetPosition(0, 0, ScrX, 0);
	m_windowKeyBar->SetPosition(0, ScrY, ScrX, ScrY);
	SetPanelPositions(LeftPanel->IsFullScreen(),RightPanel->IsFullScreen());
	SetPosition(0,0,ScrX,ScrY);
}

void FilePanels::RedrawKeyBar()
{
	m_ActivePanel->UpdateKeyBar();
	m_windowKeyBar->Redraw();
}


Panel* FilePanels::CreatePanel(int Type)
{
	Panel *pResult = nullptr;

	switch (Type)
	{
		case FILE_PANEL:
			pResult = new FileList(shared_from_this());
			break;
		case TREE_PANEL:
			pResult = new TreeList(shared_from_this());
			break;
		case QVIEW_PANEL:
			pResult = new QuickView(shared_from_this());
			break;
		case INFO_PANEL:
			pResult = new InfoList(shared_from_this());
			break;
	}

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

	Deleted->Destroy();
}

int FilePanels::SetAnhoterPanelFocus()
{
	int Ret=FALSE;

	if (m_ActivePanel == LeftPanel)
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
			Global->Opt->WidthDecrement=-Global->Opt->WidthDecrement;

			Global->Opt->LeftHeightDecrement^=Global->Opt->RightHeightDecrement;
			Global->Opt->RightHeightDecrement=Global->Opt->LeftHeightDecrement^Global->Opt->RightHeightDecrement;
			Global->Opt->LeftHeightDecrement^=Global->Opt->RightHeightDecrement;

		}

		using std::swap;
		swap(LeftPanel, RightPanel);
		swap(LastLeftFilePanel, LastRightFilePanel);
		swap(LastLeftType, LastRightType);
		FileFilter::SwapFilter();
		Ret=TRUE;
	}
	SetScreenPosition();
	Global->WindowManager->RefreshWindow();
	return Ret;
}

__int64 FilePanels::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Global->Opt->ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->ShowKeyBar=1;
				m_windowKeyBar->Show();
				m_KeyBarVisible = Global->Opt->ShowKeyBar;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
				break;
			case 2:
				Global->Opt->ShowKeyBar=0;
				m_windowKeyBar->Hide();
				m_KeyBarVisible = Global->Opt->ShowKeyBar;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
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
	return m_ActivePanel->VMProcess(OpCode, vParam, iParam);
}

int FilePanels::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey;
	if (!LocalKey)
		return TRUE;

	if ((LocalKey==KEY_CTRLLEFT || LocalKey==KEY_CTRLRIGHT || LocalKey==KEY_CTRLNUMPAD4 || LocalKey==KEY_CTRLNUMPAD6
		|| LocalKey==KEY_RCTRLLEFT || LocalKey==KEY_RCTRLRIGHT || LocalKey==KEY_RCTRLNUMPAD4 || LocalKey==KEY_RCTRLNUMPAD6
	        /* || LocalKey==KEY_CTRLUP   || LocalKey==KEY_CTRLDOWN || LocalKey==KEY_CTRLNUMPAD8 || LocalKey==KEY_CTRLNUMPAD2 */) &&
	        (CmdLine->GetLength()>0 ||
	         (!LeftPanel->IsVisible() && !RightPanel->IsVisible())))
	{
		CmdLine->ProcessKey(Key);
		return TRUE;
	}

	switch (LocalKey)
	{
		case KEY_F1:
		{
			if (!m_ActivePanel->ProcessKey(Manager::Key(KEY_F1)))
			{
				Help::create(L"Contents");
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
			Global->Opt->ShowKeyBar=!Global->Opt->ShowKeyBar;
			m_KeyBarVisible = Global->Opt->ShowKeyBar;

			if (!m_KeyBarVisible)
				m_windowKeyBar->Hide();

			SetScreenPosition();
			Global->WindowManager->RefreshWindow();
			break;
		}
		case KEY_CTRLL: case KEY_RCTRLL:
		case KEY_CTRLQ: case KEY_RCTRLQ:
		case KEY_CTRLT: case KEY_RCTRLT:
		{
			if (m_ActivePanel->IsVisible())
			{
				Panel *AnotherPanel = PassivePanel();
				int NewType;

				if (LocalKey==KEY_CTRLL || LocalKey==KEY_RCTRLL)
					NewType=INFO_PANEL;
				else if (LocalKey==KEY_CTRLQ || LocalKey==KEY_RCTRLQ)
					NewType=QVIEW_PANEL;
				else
					NewType=TREE_PANEL;

				if (m_ActivePanel->GetType() == NewType)
					AnotherPanel = m_ActivePanel;

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
					if (m_ActivePanel->GetType() == TREE_PANEL)
					{
						string strCurDir(m_ActivePanel->GetCurDir());
						AnotherPanel->SetCurDir(strCurDir, true);
						AnotherPanel->Update(0);
					}
					else
						AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					AnotherPanel->Show();
				}

				m_ActivePanel->SetFocus();
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
					Global->WindowManager->RefreshWindow();
				}
				else
				{
					if (!LeftStateBeforeHide && !RightStateBeforeHide)
						LeftStateBeforeHide=RightStateBeforeHide=TRUE;

					if (LeftStateBeforeHide)
						LeftPanel->Show();

					if (RightStateBeforeHide)
						RightPanel->Show();

					if (!m_ActivePanel->IsVisible())
					{
						if (m_ActivePanel == RightPanel)
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
			if (m_ActivePanel->IsVisible())
			{
				Panel *AnotherPanel = PassivePanel();

				if (AnotherPanel->IsVisible())
					AnotherPanel->Hide();
				else
					AnotherPanel->Show();

				CmdLine->Redraw();
			}

			Global->WindowManager->RefreshWindow();
			break;
		}
		case KEY_CTRLI:
		case KEY_RCTRLI:
		{
			m_ActivePanel->EditFilter();
			return TRUE;
		}
		case KEY_CTRLU:
		case KEY_RCTRLU:
		{
			if (!LeftPanel->IsVisible() && !RightPanel->IsVisible())
				CmdLine->ProcessKey(Key);
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

			if (ActivePanel() != LeftPanel)
				ActivePanel()->SetCurPath();

			break;
		}
		case KEY_ALTF2:
		case KEY_RALTF2:
		{
			RightPanel->ChangeDisk();

			if (ActivePanel() != RightPanel)
				ActivePanel()->SetCurPath();

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
			if (Global->Opt->LeftHeightDecrement<ScrY-7)
			{
				++Global->Opt->LeftHeightDecrement;
				Set=true;
			}
			if (Global->Opt->RightHeightDecrement<ScrY-7)
			{
				++Global->Opt->RightHeightDecrement;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
		{
			bool Set=false;
			if (Global->Opt->LeftHeightDecrement>0)
			{
				--Global->Opt->LeftHeightDecrement;
				Set=true;
			}
			if (Global->Opt->RightHeightDecrement>0)
			{
				--Global->Opt->RightHeightDecrement;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}

		case KEY_CTRLSHIFTUP:  case KEY_CTRLSHIFTNUMPAD8:
		case KEY_RCTRLSHIFTUP: case KEY_RCTRLSHIFTNUMPAD8:
		{
			IntOption& HeightDecrement = (m_ActivePanel == LeftPanel) ? Global->Opt->LeftHeightDecrement : Global->Opt->RightHeightDecrement;
			if (HeightDecrement<ScrY-7)
			{
				++HeightDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}
			break;
		}

		case KEY_CTRLSHIFTDOWN:  case KEY_CTRLSHIFTNUMPAD2:
		case KEY_RCTRLSHIFTDOWN: case KEY_RCTRLSHIFTNUMPAD2:
		{
			IntOption& HeightDecrement = (m_ActivePanel == LeftPanel) ? Global->Opt->LeftHeightDecrement : Global->Opt->RightHeightDecrement;
			if (HeightDecrement>0)
			{
				--HeightDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}
			break;
		}

		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			if (Global->Opt->WidthDecrement<ScrX/2-10)
			{
				++Global->Opt->WidthDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
		{
			if (Global->Opt->WidthDecrement>-(ScrX/2-10))
			{
				--Global->Opt->WidthDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLCLEAR:
		case KEY_RCTRLCLEAR:
		{
			if (Global->Opt->WidthDecrement)
			{
				Global->Opt->WidthDecrement=0;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLALTCLEAR:
		case KEY_RCTRLRALTCLEAR:
		case KEY_CTRLRALTCLEAR:
		case KEY_RCTRLALTCLEAR:
		{
			bool Set=false;
			if (Global->Opt->LeftHeightDecrement)
			{
				Global->Opt->LeftHeightDecrement=0;
				Set=true;
			}
			if (Global->Opt->RightHeightDecrement)
			{
				Global->Opt->RightHeightDecrement=0;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_F9:
		{
			Global->Opt->ShellOptions(false,nullptr);
			return TRUE;
		}
		case KEY_SHIFTF10:
		{
			Global->Opt->ShellOptions(true,nullptr);
			return TRUE;
		}
		default:
		{
			if (LocalKey >= KEY_CTRL0 && LocalKey <= KEY_CTRL9)
				ChangePanelViewMode(m_ActivePanel, LocalKey - KEY_CTRL0, TRUE);
			if (!m_ActivePanel->ProcessKey(Key))
				CmdLine->ProcessKey(Key);

			break;
		}
	}

	return TRUE;
}

int FilePanels::ChangePanelViewMode(Panel *Current, int Mode, BOOL RefreshWindow)
{
	if (Current && Mode >= VIEW_0 && Mode < (int)Global->Opt->ViewSettings.size())
	{
		Current->SetViewMode(Mode);
		Current=ChangePanelToFilled(Current,FILE_PANEL);
		Current->SetViewMode(Mode);
		// ВНИМАНИЕ! Костыль! Но Работает!
		SetScreenPosition();

		if (RefreshWindow)
			Global->WindowManager->RefreshWindow();

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

	return Current;
}

Panel* FilePanels::GetAnotherPanel(const Panel *Current)
{
	if (Current==LeftPanel)
		return RightPanel;
	else
		return LeftPanel;
}


Panel* FilePanels::ChangePanel(Panel *Current,int NewType,int CreateNew,int Force)
{
	Panel *NewPanel;
	std::unique_ptr<SaveScreen> TemporarySaveScr;
	// OldType не инициализировался...
	int OldType=Current->GetType(),X1,Y1,X2,Y2;
	int OldPanelMode=Current->GetMode();

	if (!Force && NewType==OldType && OldPanelMode==NORMAL_PANEL)
		return Current;

	int UseLastPanel=0;

	int OldViewMode=Current->GetPrevViewMode();
	bool OldFullScreen=Current->IsFullScreen();
	int OldSortMode=Current->GetPrevSortMode();
	bool OldSortOrder=Current->GetPrevSortOrder();
	bool OldNumericSort=Current->GetPrevNumericSort();
	bool OldCaseSensitiveSort=Current->GetPrevCaseSensitiveSort();
	bool OldSortGroups=Current->GetSortGroups();
	bool OldShowShortNames=Current->GetShowShortNamesMode();
	int OldFocus=Current->GetFocus();
	bool OldSelectedFirst=Current->GetSelectedFirstMode();
	bool OldDirectoriesFirst=Current->GetPrevDirectoriesFirst();
	bool LeftPosition=(Current==LeftPanel);

	Panel *(&LastFilePanel)=LeftPosition ? LastLeftFilePanel:LastRightFilePanel;
	Current->GetPosition(X1,Y1,X2,Y2);
	int ChangePosition=((OldType==FILE_PANEL && NewType!=FILE_PANEL &&
	                    OldFullScreen) || (NewType==FILE_PANEL &&
	                                    ((OldFullScreen && !FileList::IsModeFullScreen(OldViewMode)) ||
	                                     (!OldFullScreen && FileList::IsModeFullScreen(OldViewMode)))));

	if (!ChangePosition)
	{
		TemporarySaveScr = std::move(Current->SaveScr);
	}

	if (OldType==FILE_PANEL && NewType!=FILE_PANEL)
	{
		Current->SaveScr.reset();

		if (LastFilePanel!=Current)
		{
			DeletePanel(LastFilePanel);
			LastFilePanel=Current;
		}

		LastFilePanel->Hide();

		if (LastFilePanel->SaveScr)
		{
			LastFilePanel->SaveScr->Discard();
			LastFilePanel->SaveScr.reset();
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

				if (TemporarySaveScr && AnotherPanel->IsVisible() &&
				        AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen())
					TemporarySaveScr->Discard();
			}
			else
				NewPanel->SaveScr = std::move(TemporarySaveScr);
		}

		if (!OldFocus && NewPanel->GetFocus())
			NewPanel->KillFocus();

		UseLastPanel=TRUE;
	}
	else
	{
		NewPanel=CreatePanel(NewType);
	}
	if (Current == m_ActivePanel)
		m_ActivePanel = NewPanel;

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
				NewPanel->SetPosition(0,Y1,ScrX/2-Global->Opt->WidthDecrement,Y2);
				RightPanel->Redraw();
			}
			else
			{
				NewPanel->SetPosition(ScrX/2+1-Global->Opt->WidthDecrement,Y1,ScrX,Y2);
				LeftPanel->Redraw();
			}
		}
		else
		{
			NewPanel->SaveScr = std::move(SaveScr);
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

	return NewPanel;
}

int  FilePanels::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensPanels);
	string strFullName, strShortName;

	switch (m_ActivePanel->GetType())
	{
		case TREE_PANEL:
		case QVIEW_PANEL:
		case FILE_PANEL:
		case INFO_PANEL:
			m_ActivePanel->GetCurName(strFullName, strShortName);
			ConvertNameToFull(strFullName, strFullName);
			break;
	}

	strName = strFullName;
	return windowtype_panels;
}

void FilePanels::DisplayObject()
{
//  if ( !Focus )
//      return;
	_OT(SysLog(L"[%p] FilePanels::Redraw() {%d, %d - %d, %d}", this,m_X1,m_Y1,m_X2,m_Y2));
	Global->WindowManager->ShowBackground();

	if (Global->Opt->ShowMenuBar)
		TopMenuBar->Show();

	CmdLine->Show();

	if (Global->Opt->ShowKeyBar)
		m_windowKeyBar->Show();
	else if (m_windowKeyBar->IsVisible())
		m_windowKeyBar->Hide();

	m_KeyBarVisible=Global->Opt->ShowKeyBar;
#if 1

	if (LeftPanel->IsVisible())
		LeftPanel->Show();

	if (RightPanel->IsVisible())
		RightPanel->Show();

#else
	Panel *PassivePanel=nullptr;
	int PassiveIsLeftFlag=TRUE;

	if (Global->Opt->LeftPanel.Focus)
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
		if (Global->Opt->LeftPanel.Visible)
		{
			LeftPanel->Show();
		}

		if (Global->Opt->RightPanel.Visible)
		{
			RightPanel->Show();
		}
	}
	else
	{
		if (Global->Opt->RightPanel.Visible)
		{
			RightPanel->Show();
		}

		if (Global->Opt->LeftPanel.Visible)
		{
			LeftPanel->Show();
		}
	}

#endif
}

int  FilePanels::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!m_ActivePanel->ProcessMouse(MouseEvent))
		if (!PassivePanel()->ProcessMouse(MouseEvent))
			if (!m_windowKeyBar->ProcessMouse(MouseEvent))
				CmdLine->ProcessMouse(MouseEvent);

	return TRUE;
}

void FilePanels::ShowConsoleTitle()
{
	if (m_ActivePanel)
		m_ActivePanel->SetTitle();
}

void FilePanels::ResizeConsole()
{
	window::ResizeConsole();
	CmdLine->ResizeConsole();
	m_windowKeyBar->ResizeConsole();
	TopMenuBar->ResizeConsole();
	SetScreenPosition();
	_OT(SysLog(L"[%p] FilePanels::ResizeConsole() {%d, %d - %d, %d}", this,m_X1,m_Y1,m_X2,m_Y2));
}

bool FilePanels::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_PANEL) != 0;
}

void FilePanels::Refresh()
{
	window::Refresh();
	PassivePanel()->UpdateIfChanged(false);
	m_ActivePanel->UpdateIfChanged(false);
	m_ActivePanel->SetCurPath();
}

void FilePanels::GoToFile(const string& FileName)
{
	if (FirstSlash(FileName.data()))
	{
		string ADir,PDir;
		Panel *PassivePanel = this->PassivePanel();
		int PassiveMode = PassivePanel->GetMode();

		if (PassiveMode == NORMAL_PANEL)
		{
			PDir = PassivePanel->GetCurDir();
			AddEndSlash(PDir);
		}

		int ActiveMode = m_ActivePanel->GetMode();

		if (ActiveMode==NORMAL_PANEL)
		{
			ADir = m_ActivePanel->GetCurDir();
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
		BOOL AExist=(ActiveMode==NORMAL_PANEL) && !StrCmpI(ADir, strNameDir);
		BOOL PExist=(PassiveMode==NORMAL_PANEL) && !StrCmpI(PDir, strNameDir);

		// если нужный путь есть на пассивной панели
		if (!AExist && PExist)
			ProcessKey(Manager::Key(KEY_TAB));

		if (!AExist && !PExist)
			m_ActivePanel->SetCurDir(strNameDir, true);

		m_ActivePanel->GoToFile(strNameFile);
		// всегда обновим заголовок панели, чтобы дать обратную связь, что
		// Ctrl-F10 обработан
		m_ActivePanel->SetTitle();
	}
}


FARMACROAREA FilePanels::GetMacroMode() const
{
	switch (m_ActivePanel->GetType())
	{
		case TREE_PANEL:
			return MACROAREA_TREEPANEL;
		case QVIEW_PANEL:
			return MACROAREA_QVIEWPANEL;
		case INFO_PANEL:
			return MACROAREA_INFOPANEL;
		default:
			return MACROAREA_SHELL;
	}
}

Viewer* FilePanels::GetViewer(void)
{
	auto result=ActivePanel()->GetViewer();
	if (!result) result=PassivePanel()->GetViewer();
	return result;
}

Viewer* FilePanels::GetById(int ID)
{
	auto result=LeftPanel->GetById(ID);
	if (!result) result=RightPanel->GetById(ID);
	return result;
}

CommandLine* FilePanels::GetCmdLine(void)
{
	return CmdLine.get();
}

MenuBar* FilePanels::GetTopMenuBar(void)
{
	return TopMenuBar.get();
}

/*
filepanels.cpp

Файловые панели

*/

#include "headers.hpp"
#pragma hdrstop

#include "filepanels.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "lang.hpp"
#include "plugin.hpp"
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

FilePanels::FilePanels()
{
	_OT(SysLog("[%p] FilePanels::FilePanels()", this));
	LeftPanel=CreatePanel(Opt.LeftPanel.Type);
	RightPanel=CreatePanel(Opt.RightPanel.Type);
//  CmdLine=0;
	ActivePanel=0;
	LastLeftType=0;
	LastRightType=0;
//  HideState=0;
	LeftStateBeforeHide=0;
	RightStateBeforeHide=0;
	LastLeftFilePanel=0;
	LastRightFilePanel=0;
	MacroMode = MACRO_SHELL;
	/* $ 07.05.2001 DJ */
	KeyBarVisible = Opt.ShowKeyBar;
	/* DJ $ */
//  SetKeyBar(&MainKeyBar);
//  _D(SysLog("MainKeyBar=0x%p",&MainKeyBar));
}

static void PrepareOptFolder(char *Src,int SizeSrc,int IsLocalPath_FarPath)
{
	if (!*Src)
	{
		xstrncpy(Src,FarPath,SizeSrc);
		DeleteEndSlash(Src);
	}
	else
		ExpandEnvironmentStr(Src,Src,SizeSrc);

	if (!strcmp(Src,"/"))
	{
		xstrncpy(Src,FarPath,SizeSrc);

		if (IsLocalPath_FarPath)
		{
			Src[2]='\\';
			Src[3]=0;
		}
	}
	else
		CheckShortcutFolder(Src,SizeSrc,FALSE,TRUE);
}

void FilePanels::Init()
{
	SetPanelPositions(FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
	                  FileList::IsModeFullScreen(Opt.RightPanel.ViewMode));
	LeftPanel->SetViewMode(Opt.LeftPanel.ViewMode);
	RightPanel->SetViewMode(Opt.RightPanel.ViewMode);
	LeftPanel->SetSortMode(Opt.LeftPanel.SortMode);
	RightPanel->SetSortMode(Opt.RightPanel.SortMode);
	LeftPanel->SetNumericSort(Opt.LeftPanel.NumericSort);
	RightPanel->SetNumericSort(Opt.RightPanel.NumericSort);
	LeftPanel->SetSortOrder(Opt.LeftPanel.SortOrder);
	RightPanel->SetSortOrder(Opt.RightPanel.SortOrder);
	LeftPanel->SetSortGroups(Opt.LeftPanel.SortGroups);
	RightPanel->SetSortGroups(Opt.RightPanel.SortGroups);
	LeftPanel->SetShowShortNamesMode(Opt.LeftPanel.ShowShortNames);
	RightPanel->SetShowShortNamesMode(Opt.RightPanel.ShowShortNames);
	LeftPanel->SetSelectedFirstMode(Opt.LeftSelectedFirst);
	RightPanel->SetSelectedFirstMode(Opt.RightSelectedFirst);
	SetCanLoseFocus(TRUE);
	Panel *PassivePanel=NULL;
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
	int IsLocalPath_FarPath=IsLocalPath(FarPath);
	PrepareOptFolder(Opt.LeftFolder,sizeof(Opt.LeftFolder),IsLocalPath_FarPath);
	PrepareOptFolder(Opt.RightFolder,sizeof(Opt.RightFolder),IsLocalPath_FarPath);
	PrepareOptFolder(Opt.PassiveFolder,sizeof(Opt.PassiveFolder),IsLocalPath_FarPath);

	if (Opt.AutoSaveSetup || !Opt.SetupArgv)
	{
		if (GetFileAttributes(Opt.LeftFolder)!=INVALID_FILE_ATTRIBUTES)
			LeftPanel->InitCurDir(Opt.LeftFolder);

		if (GetFileAttributes(Opt.RightFolder)!=INVALID_FILE_ATTRIBUTES)
			RightPanel->InitCurDir(Opt.RightFolder);
	}

	if (!Opt.AutoSaveSetup)
	{
		if (Opt.SetupArgv >= 1)
		{
			if (ActivePanel==RightPanel)
			{
				if (GetFileAttributes(Opt.RightFolder)!=INVALID_FILE_ATTRIBUTES)
					RightPanel->InitCurDir(Opt.RightFolder);
			}
			else
			{
				if (GetFileAttributes(Opt.LeftFolder)!=INVALID_FILE_ATTRIBUTES)
					LeftPanel->InitCurDir(Opt.LeftFolder);
			}

			if (Opt.SetupArgv == 2)
			{
				if (ActivePanel==LeftPanel)
				{
					if (GetFileAttributes(Opt.RightFolder)!=INVALID_FILE_ATTRIBUTES)
						RightPanel->InitCurDir(Opt.RightFolder);
				}
				else
				{
					if (GetFileAttributes(Opt.LeftFolder)!=INVALID_FILE_ATTRIBUTES)
						LeftPanel->InitCurDir(Opt.LeftFolder);
				}
			}
		}

		if (Opt.SetupArgv < 2 && *Opt.PassiveFolder && (GetFileAttributes(Opt.PassiveFolder)!=INVALID_FILE_ATTRIBUTES))
		{
			PassivePanel->InitCurDir(Opt.PassiveFolder);
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
		CtrlObject->CmdLine->SetCurDir(PassiveIsLeftFlag?Opt.RightFolder:Opt.LeftFolder);
	}

	SetKeyBar(&MainKeyBar);
	MainKeyBar.SetOwner(this);
}

FilePanels::~FilePanels()
{
	_OT(SysLog("[%p] FilePanels::~FilePanels()", this));

	if (LastLeftFilePanel!=LeftPanel && LastLeftFilePanel!=RightPanel)
		DeletePanel(LastLeftFilePanel);

	if (LastRightFilePanel!=LeftPanel && LastRightFilePanel!=RightPanel)
		DeletePanel(LastRightFilePanel);

	DeletePanel(LeftPanel);
	LeftPanel=NULL;
	DeletePanel(RightPanel);
	RightPanel=NULL;
}

void FilePanels::SetPanelPositions(int LeftFullScreen,int RightFullScreen)
{
	if (Opt.WidthDecrement < -(ScrX/2-10))
		Opt.WidthDecrement=-(ScrX/2-10);

	if (Opt.WidthDecrement > (ScrX/2-10))
		Opt.WidthDecrement=(ScrX/2-10);

	if (Opt.HeightDecrement>ScrY-7)
		Opt.HeightDecrement=ScrY-7;

	if (Opt.HeightDecrement<0)
		Opt.HeightDecrement=0;

	if (LeftFullScreen)
	{
		LeftPanel->SetPosition(0,Opt.ShowMenuBar?1:0,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
		LeftPanel->ViewSettings.FullScreen=1;
	}
	else
	{
		LeftPanel->SetPosition(0,Opt.ShowMenuBar?1:0,ScrX/2-Opt.WidthDecrement,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
	}

	if (RightFullScreen)
	{
		RightPanel->SetPosition(0,Opt.ShowMenuBar?1:0,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
		RightPanel->ViewSettings.FullScreen=1;
	}
	else
	{
		RightPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Opt.ShowMenuBar?1:0,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
	}
}

void FilePanels::SetScreenPosition()
{
	_OT(SysLog("[%p] FilePanels::SetScreenPosition() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
//  RedrawDesktop Redraw;
	CtrlObject->CmdLine->SetPosition(0,ScrY-(Opt.ShowKeyBar!=0),ScrX,ScrY-(Opt.ShowKeyBar!=0));
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
	Panel *pResult = NULL;

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
	if (Deleted==NULL)
		return;

	/* $ 27.11.2001 DJ
	   не будем использовать указатель после того, как его удалили
	*/
	if (Deleted==LastLeftFilePanel)
		LastLeftFilePanel=NULL;

	if (Deleted==LastRightFilePanel)
		LastRightFilePanel=NULL;

	delete Deleted;
	/* DJ $ */
}

int FilePanels::SetAnhoterPanelFocus(void)
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


int FilePanels::SwapPanels(void)
{
	int Ret=FALSE; // это значит ни одна из панелей не видна

	if (LeftPanel->IsVisible() || RightPanel->IsVisible())
	{
		int XL1,YL1,XL2,YL2;
		int XR1,YR1,XR2,YR2;
		LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
		RightPanel->GetPosition(XR1,YR1,XR2,YR2);

		if (!LeftPanel->ViewSettings.FullScreen || !RightPanel->ViewSettings.FullScreen)
		{
			Opt.WidthDecrement=-Opt.WidthDecrement;

			if (!LeftPanel->ViewSettings.FullScreen)
			{
				LeftPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);

				if (LastLeftFilePanel)
					LastLeftFilePanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
			}

			if (!RightPanel->ViewSettings.FullScreen)
			{
				RightPanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);

				if (LastRightFilePanel)
					LastRightFilePanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
			}
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

	FrameManager->RefreshFrame();
	return Ret;
}

__int64 FilePanels::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return ActivePanel->VMProcess(OpCode,vParam,iParam);
}

int FilePanels::ProcessKey(int Key)
{
	if (!Key)
		return(TRUE);

	if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD4 || Key==KEY_CTRLNUMPAD6
	        /* || Key==KEY_CTRLUP   || Key==KEY_CTRLDOWN || Key==KEY_CTRLNUMPAD8 || Key==KEY_CTRLNUMPAD2 */) &&
	        (CtrlObject->CmdLine->GetLength()>0 ||
	         !LeftPanel->IsVisible() && !RightPanel->IsVisible()))
	{
		CtrlObject->CmdLine->ProcessKey(Key);
		return(TRUE);
	}

	switch (Key)
	{
		case KEY_F1:
		{
			if (!ActivePanel->ProcessKey(KEY_F1))
			{
				Help Hlp("Contents");
			}

			return(TRUE);
		}
		case KEY_TAB:
		{
			SetAnhoterPanelFocus();
			break;
		}
		case KEY_CTRLF1:
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
		{
			Opt.ShowKeyBar=!Opt.ShowKeyBar;
			/* $ 07.05.2001 DJ */
			KeyBarVisible = Opt.ShowKeyBar;

			/* DJ $ */
			if (!KeyBarVisible)
				MainKeyBar.Hide();

			SetScreenPosition();
			FrameManager->RefreshFrame();
			break;
		}
		case KEY_CTRLL:
		case KEY_CTRLQ:
		case KEY_CTRLT:
		{
			if (ActivePanel->IsVisible())
			{
				Panel *AnotherPanel=GetAnotherPanel(ActivePanel);
				int NewType;

				if (Key==KEY_CTRLL)
					NewType=INFO_PANEL;
				else if (Key==KEY_CTRLQ)
					NewType=QVIEW_PANEL;
				else
					NewType=TREE_PANEL;

				if (ActivePanel->GetType()==NewType)
					AnotherPanel=ActivePanel;

				if (!AnotherPanel->ProcessPluginEvent(FE_CLOSE,NULL))
				{
					if (AnotherPanel->GetType()==NewType)
						/* $ 19.09.2000 IS
						  Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
						*/
						AnotherPanel=ChangePanel(AnotherPanel,FILE_PANEL,FALSE,FALSE);
					/* IS % */
					else
						AnotherPanel=ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

					/* $ 07.09.2001 VVM
					  ! При возврате из CTRL+Q, CTRL+L восстановим каталог, если активная панель - дерево. */
					if (ActivePanel->GetType() == TREE_PANEL)
					{
						char CurDir[NM];
						ActivePanel->GetCurDir(CurDir);
						AnotherPanel->SetCurDir(CurDir, TRUE);
						AnotherPanel->Update(0);
					}
					else
						AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					/* VVM $ */
					AnotherPanel->Show();
				}

				ActivePanel->SetFocus();
			}

			break;
		}
		/* $ 19.09.2000 SVS
		   + Добавляем реакцию показа бакграунда в панелях на CtrlAltShift
		*/
		/* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */
		case KEY_CTRLO:
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
		{
			ActivePanel->EditFilter();
			return(TRUE);
		}
		case KEY_CTRLU:
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
		{
			LeftPanel->ChangeDisk();

			if (ActivePanel!=LeftPanel)
				ActivePanel->SetCurPath();

			break;
		}
		case KEY_ALTF2:
		{
			RightPanel->ChangeDisk();

			if (ActivePanel!=RightPanel)
				ActivePanel->SetCurPath();

			break;
		}
		/* IS $ */
		case KEY_ALTF7:
		{
			{
				FindFiles FindFiles;
			}
			break;
		}
		case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
		{
			if (Opt.HeightDecrement<ScrY-7)
			{
				Opt.HeightDecrement++;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
		{
			if (Opt.HeightDecrement>0)
			{
				Opt.HeightDecrement--;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLLEFT: case KEY_CTRLNUMPAD4:
		{
			if (Opt.WidthDecrement<ScrX/2-10)
			{
				Opt.WidthDecrement++;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLRIGHT: case KEY_CTRLNUMPAD6:
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
		{
			if (Opt.WidthDecrement!=0)
			{
				Opt.WidthDecrement=0;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_CTRLSHIFTCLEAR:
		{
			if (Opt.HeightDecrement!=0)
			{
				Opt.HeightDecrement=0;
				SetScreenPosition();
				FrameManager->RefreshFrame();
			}

			break;
		}
		case KEY_F9:
		{
			ShellOptions(0,NULL);
			return(TRUE);
		}
		case KEY_SHIFTF10:
		{
			ShellOptions(1,NULL);
			return(TRUE);
		}
		default:
		{
			if (Key >= KEY_CTRL0 && Key <= KEY_CTRL9)
				ChangePanelViewMode(ActivePanel,Key-KEY_CTRL0,TRUE);
			else if (!ActivePanel->ProcessKey(Key))
				CtrlObject->CmdLine->ProcessKey(Key);

			break;
		}
	}

	return(TRUE);
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
	if (Current->GetType()!=NewType && !Current->ProcessPluginEvent(FE_CLOSE,NULL))
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


/* $ 03.06.2001 IS
   + "Наследуем" состояние режима "Помеченные файлы вперед"
*/
Panel* FilePanels::ChangePanel(Panel *Current,int NewType,int CreateNew,int Force)
{
	Panel *NewPanel;
	SaveScreen *SaveScr=NULL;
	// OldType не инициализировался...
	int OldType=Current->GetType(),X1,Y1,X2,Y2;
	int OldViewMode,OldSortMode,OldSortOrder,OldSortGroups,OldSelectedFirst;
	int OldShowShortNames,OldPanelMode,LeftPosition,ChangePosition,OldNumericSort;
	int OldFullScreen,OldFocus,UseLastPanel=0;
	OldPanelMode=Current->GetMode();

	if (!Force && NewType==OldType && OldPanelMode==NORMAL_PANEL)
		return(Current);

	OldViewMode=Current->GetPrevViewMode();
	OldFullScreen=Current->IsFullScreen();
	OldSortMode=Current->GetPrevSortMode();
	OldSortOrder=Current->GetPrevSortOrder();
	OldNumericSort=Current->GetPrevNumericSort();
	OldSortGroups=Current->GetSortGroups();
	OldShowShortNames=Current->GetShowShortNamesMode();
	OldFocus=Current->GetFocus();
	OldSelectedFirst=Current->GetSelectedFirstMode();
	LeftPosition=(Current==LeftPanel);
	Panel *(&LastFilePanel)=LeftPosition ? LastLeftFilePanel:LastRightFilePanel;
	Current->GetPosition(X1,Y1,X2,Y2);
	ChangePosition=(OldType==FILE_PANEL && NewType!=FILE_PANEL &&
	                OldFullScreen || NewType==FILE_PANEL &&
	                (OldFullScreen && !FileList::IsModeFullScreen(OldViewMode) ||
	                 !OldFullScreen && FileList::IsModeFullScreen(OldViewMode)));

	if (!ChangePosition)
	{
		SaveScr=Current->SaveScr;
		Current->SaveScr=NULL;
	}

	if (OldType==FILE_PANEL && NewType!=FILE_PANEL)
	{
		delete Current->SaveScr;
		Current->SaveScr=NULL;

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
			LastFilePanel->SaveScr=NULL;
		}
	}
	else
	{
		Current->Hide();
		DeletePanel(Current);

		if (OldType==FILE_PANEL && NewType==FILE_PANEL)
		{
			DeletePanel(LastFilePanel);
			LastFilePanel=NULL;
		}
	}

	if (!CreateNew && NewType==FILE_PANEL && LastFilePanel!=NULL)
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
			if (NewPanel->IsFullScreen() && !OldFullScreen ||
			        !NewPanel->IsFullScreen() && OldFullScreen)
			{
				Panel *AnotherPanel=GetAnotherPanel(Current);

				if (SaveScr!=NULL && AnotherPanel->IsVisible() &&
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
		NewPanel->SetSortGroups(OldSortGroups);
		NewPanel->SetShowShortNamesMode(OldShowShortNames);
		NewPanel->SetPrevViewMode(OldViewMode);
		NewPanel->SetViewMode(OldViewMode);
		NewPanel->SetSelectedFirstMode(OldSelectedFirst);
	}

	return(NewPanel);
}
/* IS $ */

int  FilePanels::GetTypeAndName(char *Type,char *Name)
{
	if (Type)
		strcpy(Type,MSG(MScreensPanels));

	if (Name)
	{
		char FullName[NM], ShortName[NM];
		*FullName = *ShortName = 0;

		switch (ActivePanel->GetType())
		{
			case TREE_PANEL:
			case QVIEW_PANEL:
			case FILE_PANEL:
				/* $ 02.01.2002 IS а еще бывают информационные панели... */
			case INFO_PANEL:
				/* IS $ */
				ActivePanel->GetCurName(FullName, ShortName);
				ConvertNameToFull(FullName, FullName, sizeof(FullName));
				break;
		} /* case */

		strcpy(Name, FullName);
	}

	return(MODALTYPE_PANELS);
}

void FilePanels::OnChangeFocus(int f)
{
	_OT(SysLog("FilePanels::OnChangeFocus(%i)",f));

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
		/* SKV$*/
		/* $ 13.04.2002 KM
		  ! ??? Я не понял зачем здесь Redraw, если
		    Redraw вызывается следом во Frame::OnChangeFocus.
		//    Redraw();
		/* KM $ */
		Frame::OnChangeFocus(1);
	}
}

void FilePanels::DisplayObject()
{
//  if ( Focus==0 )
//      return;
	_OT(SysLog("[%p] FilePanels::Redraw() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
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
	Panel *PassivePanel=NULL;
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

	return(TRUE);
}

void FilePanels::ShowConsoleTitle()
{
	_MANAGERLOG(CleverSysLog Clev("FilePanels::ShowConsoleTitle()"));

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
	_OT(SysLog("[%p] FilePanels::ResizeConsole() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
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
	/* SKV$*/
}

/* $ 28.12.2001 DJ
   обработка Ctrl-F10 из вьюера и редактора */
/* $ 31.12.2001 VVM
   Не портим переданное имя файла... */
/* $ 26.03.2002 VVM
   Если пассивная панель - плагиновая, то не прыгаем на нее */
/* $ 17.09.2002 SKV
   Если активная панель - плагиновая, то в обяз делаем SetCurDir() :)
*/
void FilePanels::GoToFile(const char *FileName)
{
	if (strchr(FileName,'\\') || strchr(FileName,'/'))
	{
		char ADir[NM],PDir[NM];
		Panel *PassivePanel = GetAnotherPanel(ActivePanel);
		int PassiveMode = PassivePanel->GetMode();

		if (PassiveMode == NORMAL_PANEL)
		{
			PassivePanel->GetCurDir(PDir);
			AddEndSlash(PDir);
		}
		else
			PDir[0] = 0;

		int ActiveMode = ActivePanel->GetMode();

		if (ActiveMode==NORMAL_PANEL)
		{
			ActivePanel->GetCurDir(ADir);
			AddEndSlash(ADir);
		}
		else
		{
			ADir[0]=0;
		}

		char NameFile[NM], NameDir[NM];
		xstrncpy(NameDir, FileName,sizeof(NameDir)-1);
		char *NameTmp=PointToName(NameDir);
		xstrncpy(NameFile,NameTmp,sizeof(NameFile)-1);
		*NameTmp=0;
		/* $ 10.04.2001 IS
		     Не делаем SetCurDir, если нужный путь уже есть на открытых
		     панелях, тем самым добиваемся того, что выделение с элементов
		     панелей не сбрасывается.
		*/
		BOOL AExist=(ActiveMode==NORMAL_PANEL) && (LocalStricmp(ADir,NameDir)==0);
		BOOL PExist=(PassiveMode==NORMAL_PANEL) && (LocalStricmp(PDir,NameDir)==0);

		// если нужный путь есть на пассивной панели
		if (!AExist && PExist)
			ProcessKey(KEY_TAB);

		if (!AExist && !PExist)
			ActivePanel->SetCurDir(NameDir,TRUE);

		/* IS */
		ActivePanel->GoToFile(NameFile);
		// всегда обновим заголовок панели, чтобы дать обратную связь, что
		// Ctrl-F10 обработан
		ActivePanel->SetTitle();
	}
}

/* VVM $ */

/* $ 16.01.2002 OT
   переопределенный виртуальный метод от Frame
*/
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

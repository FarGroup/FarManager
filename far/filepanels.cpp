/*
filepanels.cpp

Файловые панели

*/

/* Revision: 1.22 13.08.2001 $ */

/*
Modify:
  13.08.2001 OT
    - исправление бага с появлением пропавших панелей при старте.
  31.07.2001 SKV
    ! Frame::OnChangeFocus(1)->OnChangeFocus(1)
  18.07.2001 OT
    VFMenu
  12.07.2001 OT
    - Не инициализировались панель Tree Info, QView
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  10.07.2001 SKV
    + keybar
  22.06.2001 SKV
    + update панелей при получении фокуса
  20.06.2001 tran
    - bug с отрисовкой при копировании
      смотреть в OnChangeFocus
  14.06.2001 OT
    ! "Бунт" ;-)
  03.06.2001 IS
    + ChangePanel: "Наследуем" состояние режима "Помеченные файлы вперед"
  31.05.2001 SVS
    ! Сносим лейбак по Alt-F6 для не NT
  30.05.2001 OT
    ! Перенос AltF9 в Manager::ProcessKey()
  21.05.2001 OT
    - Исправление поведения AltF9
  16.05.2001 DJ
    ! proof-of-concept
  16.05.2001 SVS
    ! _D() -> _OT()
  15.05.2001 OT
    ! NWZ -> NFZ
  11.05.2001 OT
    ! Отрисовка Background
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
   - чтоб кейбар обновлялся
  06.05.2001 DJ
   ! перетрях #include
  06.05.2001 ОТ
   ! Переименование Window в Frame :)
  05.05.2001 DJ
   + перетрях NWZ
  09.01.2001 tran
     Created
*/

#include "headers.hpp"
#pragma hdrstop

#include "filepanels.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
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
#include "filter.hpp"
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

void FilePanels::Init()
{
  SetPanelPositions(FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
                    FileList::IsModeFullScreen(Opt.RightPanel.ViewMode));
  LeftPanel->SetViewMode(Opt.LeftPanel.ViewMode);
  RightPanel->SetViewMode(Opt.RightPanel.ViewMode);
  LeftPanel->SetSortMode(Opt.LeftPanel.SortMode);
  RightPanel->SetSortMode(Opt.RightPanel.SortMode);
  LeftPanel->SetSortOrder(Opt.LeftPanel.SortOrder);
  RightPanel->SetSortOrder(Opt.RightPanel.SortOrder);
  LeftPanel->SetSortGroups(Opt.LeftPanel.SortGroups);
  RightPanel->SetSortGroups(Opt.RightPanel.SortGroups);
  LeftPanel->SetShowShortNamesMode(Opt.LeftPanel.ShowShortNames);
  RightPanel->SetShowShortNamesMode(Opt.RightPanel.ShowShortNames);
  LeftPanel->SetSelectedFirstMode(Opt.LeftSelectedFirst);
  RightPanel->SetSelectedFirstMode(Opt.RightSelectedFirst);
  SetCanLoseFocus(TRUE);

  if (Opt.LeftPanel.Focus)
    ActivePanel=LeftPanel;
  else
    ActivePanel=RightPanel;
  ActivePanel->SetFocus();
  if (Opt.AutoSaveSetup)
  {
    if (GetFileAttributes(Opt.LeftFolder)!=0xffffffff)
      LeftPanel->InitCurDir(Opt.LeftFolder);
    if (GetFileAttributes(Opt.RightFolder)!=0xffffffff)
      RightPanel->InitCurDir(Opt.RightFolder);
  }
  else
    if (*Opt.PassiveFolder)
    {
      Panel *PassivePanel=GetAnotherPanel(ActivePanel);
      if (GetFileAttributes(Opt.PassiveFolder)!=0xffffffff)
        PassivePanel->InitCurDir(Opt.PassiveFolder);
    }
  if (Opt.LeftPanel.Visible){
    LeftPanel->Show();
  }
  if (Opt.RightPanel.Visible){
    RightPanel->Show();
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

void FilePanels::DisplayObject()
{
  _OT(SysLog("[%p] FilePanels::DisplayObject()",this));
  Redraw();
}

void FilePanels::SetPanelPositions(int LeftFullScreen,int RightFullScreen)
{
  if (Opt.HeightDecrement>ScrY-7)
    Opt.HeightDecrement=ScrY-7;
  if (Opt.HeightDecrement<0)
    Opt.HeightDecrement=0;
  if (LeftFullScreen)
    LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  else
    LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX/2-Opt.WidthDecrement,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  if (RightFullScreen)
    RightPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  else
    RightPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  _OT(SysLog("[%p] FilePanels::SetPanelPositions() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
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
  char *FKeys[]={MSG(MF1),MSG(MF2),MSG(MF3),MSG(MF4),MSG(MF5),MSG(MF6),MSG(MF7),MSG(MF8),MSG(MF9),MSG(MF10),MSG(MF11),MSG(MF12)};
  char *FAltKeys[]={MSG(MAltF1),MSG(MAltF2),MSG(MAltF3),MSG(MAltF4),MSG(MAltF5),"",MSG(MAltF7),MSG(MAltF8),MSG(MAltF9),MSG(MAltF10),MSG(MAltF11),MSG(MAltF12)};
  char *FCtrlKeys[]={MSG(MCtrlF1),MSG(MCtrlF2),MSG(MCtrlF3),MSG(MCtrlF4),MSG(MCtrlF5),MSG(MCtrlF6),MSG(MCtrlF7),MSG(MCtrlF8),MSG(MCtrlF9),MSG(MCtrlF10),MSG(MCtrlF11),MSG(MCtrlF12)};
  char *FShiftKeys[]={MSG(MShiftF1),MSG(MShiftF2),MSG(MShiftF3),MSG(MShiftF4),MSG(MShiftF5),MSG(MShiftF6),MSG(MShiftF7),MSG(MShiftF8),MSG(MShiftF9),MSG(MShiftF10),MSG(MShiftF11),MSG(MShiftF12)};

  char *FAltShiftKeys[]={MSG(MAltShiftF1),MSG(MAltShiftF2),MSG(MAltShiftF3),MSG(MAltShiftF4),MSG(MAltShiftF5),MSG(MAltShiftF6),MSG(MAltShiftF7),MSG(MAltShiftF8),MSG(MAltShiftF9),MSG(MAltShiftF10),MSG(MAltShiftF11),MSG(MAltShiftF12)};
  char *FCtrlShiftKeys[]={MSG(MCtrlShiftF1),MSG(MCtrlShiftF2),MSG(MCtrlShiftF3),MSG(MCtrlShiftF4),MSG(MCtrlShiftF5),MSG(MCtrlShiftF6),MSG(MCtrlShiftF7),MSG(MCtrlShiftF8),MSG(MCtrlShiftF9),MSG(MCtrlShiftF10),MSG(MCtrlShiftF11),MSG(MCtrlShiftF12)};
  char *FCtrlAltKeys[]={MSG(MCtrlAltF1),MSG(MCtrlAltF2),MSG(MCtrlAltF3),MSG(MCtrlAltF4),MSG(MCtrlAltF5),MSG(MCtrlAltF6),MSG(MCtrlAltF7),MSG(MCtrlAltF8),MSG(MCtrlAltF9),MSG(MCtrlAltF10),MSG(MCtrlAltF11),MSG(MCtrlAltF12)};

  FAltKeys[6-1]=(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)?MSG(MAltF6):"";

  if (ActivePanel!=NULL && ActivePanel->GetMode()==PLUGIN_PANEL)
  {
    struct OpenPluginInfo Info;
    ActivePanel->GetOpenPluginInfo(&Info);
    if (Info.KeyBar!=NULL)
    {
      int I;
      for (I=0;I<sizeof(Info.KeyBar->Titles)/sizeof(Info.KeyBar->Titles[0]);I++)
        if (Info.KeyBar->Titles[I]!=NULL)
          FKeys[I]=Info.KeyBar->Titles[I];
      for (I=0;I<sizeof(Info.KeyBar->CtrlTitles)/sizeof(Info.KeyBar->CtrlTitles[0]);I++)
        if (Info.KeyBar->CtrlTitles[I]!=NULL)
          FCtrlKeys[I]=Info.KeyBar->CtrlTitles[I];
      for (I=0;I<sizeof(Info.KeyBar->AltTitles)/sizeof(Info.KeyBar->AltTitles[0]);I++)
        if (Info.KeyBar->AltTitles[I]!=NULL)
          FAltKeys[I]=Info.KeyBar->AltTitles[I];
      for (I=0;I<sizeof(Info.KeyBar->ShiftTitles)/sizeof(Info.KeyBar->ShiftTitles[0]);I++)
        if (Info.KeyBar->ShiftTitles[I]!=NULL)
          FShiftKeys[I]=Info.KeyBar->ShiftTitles[I];

      // Ага, мы ведь недаром увеличивали размер структуры ;-)
      if(Info.StructSize >= sizeof(struct OpenPluginInfo))
      {
        for (I=0;I<sizeof(Info.KeyBar->CtrlShiftTitles)/sizeof(Info.KeyBar->CtrlShiftTitles[0]);I++)
          if (Info.KeyBar->CtrlShiftTitles[I]!=NULL)
            FCtrlShiftKeys[I]=Info.KeyBar->CtrlShiftTitles[I];

        for (I=0;I<sizeof(Info.KeyBar->AltShiftTitles)/sizeof(Info.KeyBar->AltShiftTitles[0]);I++)
          if (Info.KeyBar->AltShiftTitles[I]!=NULL)
            FAltShiftKeys[I]=Info.KeyBar->AltShiftTitles[I];

        for (I=0;I<sizeof(Info.KeyBar->CtrlAltTitles)/sizeof(Info.KeyBar->CtrlAltTitles[0]);I++)
          if (Info.KeyBar->CtrlAltTitles[I]!=NULL)
            FCtrlAltKeys[I]=Info.KeyBar->CtrlAltTitles[I];
      }
    }
  }
  MainKeyBar.Set(FKeys,sizeof(FKeys)/sizeof(FKeys[0]));
  MainKeyBar.SetAlt(FAltKeys,sizeof(FAltKeys)/sizeof(FAltKeys[0]));
  MainKeyBar.SetCtrl(FCtrlKeys,sizeof(FCtrlKeys)/sizeof(FCtrlKeys[0]));
  MainKeyBar.SetShift(FShiftKeys,sizeof(FShiftKeys)/sizeof(FShiftKeys[0]));

  MainKeyBar.SetCtrlAlt(FCtrlAltKeys,sizeof(FCtrlAltKeys)/sizeof(FCtrlAltKeys[0]));
  MainKeyBar.SetCtrlShift(FCtrlShiftKeys,sizeof(FCtrlShiftKeys)/sizeof(FCtrlShiftKeys[0]));
  MainKeyBar.SetAltShift(FAltShiftKeys,sizeof(FAltShiftKeys)/sizeof(FAltShiftKeys[0]));

  MainKeyBar.Redraw();
}


Panel* FilePanels::CreatePanel(int Type)
{
  switch (Type)
  {
    case FILE_PANEL:
      return(new FileList);
    case TREE_PANEL:
      return(new TreeList);
    case QVIEW_PANEL:
      return(new QuickView);
    case INFO_PANEL:
      return(new InfoList);
  }
  return(NULL);
}


void FilePanels::DeletePanel(Panel *Deleted)
{
  if (Deleted==NULL)
    return;
  delete Deleted;
  if (Deleted==LastLeftFilePanel)
    LastLeftFilePanel=NULL;
  if (Deleted==LastRightFilePanel)
    LastRightFilePanel=NULL;
}

int  FilePanels::ProcessKey(int Key)
{
  if (!Key)
    return(TRUE);

  if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLRIGHT) && (CtrlObject->CmdLine->GetLength()>0 ||
      !LeftPanel->IsVisible() && !RightPanel->IsVisible()))
  {
    CtrlObject->CmdLine->ProcessKey(Key);
    return(TRUE);
  }

  switch(Key)
  {
    case KEY_TAB:
      if (ActivePanel==LeftPanel)
      {
        if (RightPanel->IsVisible())
          RightPanel->SetFocus();
      }
      else
        if (LeftPanel->IsVisible())
          LeftPanel->SetFocus();
      break;
    case KEY_CTRLF1:
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
    case KEY_F1:
      if (!ActivePanel->ProcessKey(KEY_F1))
      {
        Help Hlp ("Contents");
      }
      return(TRUE);
    case KEY_CTRLF2:
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
    case KEY_CTRLB:
      Opt.ShowKeyBar=!Opt.ShowKeyBar;
      /* $ 07.05.2001 DJ */
      KeyBarVisible = Opt.ShowKeyBar;
      /* DJ $ */
      if(!KeyBarVisible)
        MainKeyBar.Hide();
      SetScreenPosition();
      FrameManager->RefreshFrame();
      break;
    case KEY_CTRLL:
    case KEY_CTRLQ:
    case KEY_CTRLT:
      if (ActivePanel->IsVisible())
      {
        Panel *AnotherPanel=GetAnotherPanel(ActivePanel);
        int NewType;
        if (Key==KEY_CTRLL)
          NewType=INFO_PANEL;
        else
          if (Key==KEY_CTRLQ)
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

          AnotherPanel->Update(UPDATE_KEEP_SELECTION);
          AnotherPanel->Show();
        }
        ActivePanel->SetFocus();
      }
      break;
    /* $ 19.09.2000 SVS
       + Добавляем реакцию показа бакграунда в панелях на CtrlAltShift
    */
/* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */
    case KEY_CTRLO:
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
        }
      }
      break;
    case KEY_CTRLP:
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
    case KEY_CTRLI:
      ActivePanel->EditFilter();
      return(TRUE);
    case KEY_CTRLW:
      ShowProcessList();
      return(TRUE);
    case KEY_CTRLU:
      if (LeftPanel->IsVisible() || RightPanel->IsVisible())
      {
        int XL1,YL1,XL2,YL2;
        int XR1,YR1,XR2,YR2;
        int SwapType;
        Panel *Swap;
        LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
        RightPanel->GetPosition(XR1,YR1,XR2,YR2);
        if (XL2-XL1!=ScrX && XR2-XR1!=ScrX)
        {
          Opt.WidthDecrement=-Opt.WidthDecrement;
          LeftPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
          RightPanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
          Swap=LeftPanel;
          LeftPanel=RightPanel;
          RightPanel=Swap;
          if (LastLeftFilePanel)
            LastLeftFilePanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
          if (LastRightFilePanel)
            LastRightFilePanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
          Swap=LastLeftFilePanel;
          LastLeftFilePanel=LastRightFilePanel;
          LastRightFilePanel=Swap;
          SwapType=LastLeftType;
          LastLeftType=LastRightType;
          LastRightType=SwapType;
          PanelFilter::SwapFilter();
        }
      }
      FrameManager->RefreshFrame();
      break;
    case KEY_ALTF1:
      LeftPanel->ChangeDisk();
      break;
    case KEY_ALTF2:
      RightPanel->ChangeDisk();
      break;
    case KEY_ALTF7:
      {
        FindFiles FindFiles;
      }
      break;
    case KEY_CTRLUP:
      if (Opt.HeightDecrement<ScrY-7)
      {
        Opt.HeightDecrement++;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    case KEY_CTRLDOWN:
      if (Opt.HeightDecrement>0)
      {
        Opt.HeightDecrement--;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    case KEY_CTRLLEFT:
      if (Opt.WidthDecrement<ScrX/2-10)
      {
        Opt.WidthDecrement++;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    case KEY_CTRLRIGHT:
      if (Opt.WidthDecrement>-(ScrX/2-10))
      {
        Opt.WidthDecrement--;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    case KEY_CTRLCLEAR:
      if (Opt.WidthDecrement!=0)
      {
        Opt.WidthDecrement=0;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    case KEY_F9:
      ShellOptions(0,NULL);
      return(TRUE);
    case KEY_SHIFTF10:
      ShellOptions(1,NULL);
      return(TRUE);
    case KEY_CTRL1:
      ActivePanel->SetViewMode(VIEW_1);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_1);
      break;
    case KEY_CTRL2:
      ActivePanel->SetViewMode(VIEW_2);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_2);
      break;
    case KEY_CTRL3:
      ActivePanel->SetViewMode(VIEW_3);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_3);
      break;
    case KEY_CTRL4:
      ActivePanel->SetViewMode(VIEW_4);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_4);
      break;
    case KEY_CTRL5:
      ActivePanel->SetViewMode(VIEW_5);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_5);
      break;
    case KEY_CTRL6:
      ActivePanel->SetViewMode(VIEW_6);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_6);
      break;
    case KEY_CTRL7:
      ActivePanel->SetViewMode(VIEW_7);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_7);
      break;
    case KEY_CTRL8:
      ActivePanel->SetViewMode(VIEW_8);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_8);
      break;
    case KEY_CTRL9:
      ActivePanel->SetViewMode(VIEW_9);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_9);
      break;
    case KEY_CTRL0:
      ActivePanel->SetViewMode(VIEW_0);
      ChangePanelToFilled(ActivePanel,FILE_PANEL);
      ActivePanel->SetViewMode(VIEW_0);
      break;
    default:
      if (!ActivePanel->ProcessKey(Key))
        CtrlObject->CmdLine->ProcessKey(Key);
      break;
  }
  return(TRUE);
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
  int OldType,X1,Y1,X2,Y2;
  int OldViewMode,OldSortMode,OldSortOrder,OldSortGroups,OldSelectedFirst;
  int OldShowShortNames,OldPanelMode,LeftPosition,ChangePosition;
  int OldFullScreen,OldFocus,UseLastPanel=0;

  OldPanelMode=Current->GetMode();
  if (!Force && NewType==(OldType=Current->GetType()) && OldPanelMode==NORMAL_PANEL)
    return(Current);
  OldViewMode=Current->GetPrevViewMode();
  OldFullScreen=Current->IsFullScreen();

  OldSortMode=Current->GetPrevSortMode();
  OldSortOrder=Current->GetPrevSortOrder();
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
    /* $ 13.07.2000 SVS
       немного сократим код путем вызова функции класса CreatePanel(int Type)
    */
    NewPanel=CreatePanel(NewType);
    /* SVS $*/

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
  if ( Type )
      strcpy(Type,MSG(MScreensPanels));
  if ( Name)
      strcpy(Name,"");
  return(MODALTYPE_PANELS);
}

void FilePanels::OnChangeFocus(int f)
{
  _OT(SysLog("FilePanels::OnChangeFocus(%i)",f));
  /* $ 20.06.2001 tran
     баг с отрисовкой при копировании и удалении
     не учитывался LockRefreshCount */
  if ( f && LockRefreshCount==0) {
    /*$ 22.06.2001 SKV
      + update панелей при получении фокуса
    */
    LeftPanel->UpdateIfChanged(1);
    RightPanel->UpdateIfChanged(1);
    /* SKV$*/
    Redraw();
    Frame::OnChangeFocus(1);
  }
}

void FilePanels::Show()
{
    _OT(SysLog("[%p] FilePanels::Show() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
    Frame::Show();
}

void FilePanels::Redraw()
{
//    if ( Focus==0 )
//        return;
    _OT(SysLog("[%p] FilePanels::Redraw() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
    CtrlObject->CmdLine->ShowBackground();
    if (LeftPanel->IsVisible())
        LeftPanel->Show();
    if (RightPanel->IsVisible())
        RightPanel->Show();
    CtrlObject->CmdLine->Show();
    if (Opt.ShowKeyBar)
      MainKeyBar.Show();
    else
      if(MainKeyBar.IsVisible())
        MainKeyBar.Hide();
    KeyBarVisible=Opt.ShowKeyBar;
    if (Opt.ShowMenuBar)
      CtrlObject->TopMenuBar->Show();
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

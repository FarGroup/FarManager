/*
ctrlobj.cpp

Управление остальными объектами, раздача сообщений клавиатуры и мыши

*/

/* Revision: 1.02 11.07.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  29.06.2000 tran
    ! соощение о копирайте включается из copyright.inc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */


ControlObject::ControlObject()
{
  CtrlObject=this;
  ReadConfig();
  CmdHistory=new History("SavedHistory",&Opt.SaveHistory,FALSE,FALSE);
  FolderHistory=new History("SavedFolderHistory",&Opt.SaveFoldersHistory,FALSE,TRUE);
  ViewHistory=new History("SavedViewHistory",&Opt.SaveViewHistory,TRUE,TRUE);
  FolderHistory->SetAddMode(TRUE,2,TRUE);
  ViewHistory->SetAddMode(TRUE,1,TRUE);
  if (Opt.SaveHistory)
    CmdHistory->ReadHistory();
  if (Opt.SaveFoldersHistory)
    FolderHistory->ReadHistory();
  if (Opt.SaveViewHistory)
    ViewHistory->ReadHistory();
  EndLoop=0;
  LastLeftType=LastRightType=FILE_PANEL;
  LastLeftFilePanel=LastRightFilePanel=NULL;
  ActivePanel=NULL;
  RegVer=-1;
}


void ControlObject::Init()
{
  Plugins.LoadPlugins();
  TreeList::ClearCache(0);
  PanelFilter::InitFilter();

  SetColor(F_LIGHTGRAY|B_BLACK);
  GotoXY(0,ScrY-3);
  while (RegVer==-1)
    Sleep(0);
  ShowCopyright();
  GotoXY(0,ScrY-2);

  char TruncRegName[512];
  strcpy(TruncRegName,RegName);
  char *CountPtr=strstr(TruncRegName," - (");
  if (CountPtr!=NULL && isdigit(CountPtr[4]) && strchr(CountPtr+5,'/')!=NULL &&
      strchr(CountPtr+6,')')!=NULL)
    *CountPtr=0;
  if (RegVer)
    mprintf("%s: %s",MSG(MRegistered),TruncRegName);
  else
    Text(MShareware);

  LeftPanel=CreatePanel(Opt.LeftPanel.Type);
  RightPanel=CreatePanel(Opt.RightPanel.Type);

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

  MainKeyBar.SetOwner(this);
  RedrawKeyBar();
  SetScreenPositions();
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
  _beginthread(CheckVersion,0x10000,NULL);
  LeftPanel->Update(0);
  RightPanel->Update(0);

  if (Opt.LeftPanel.Visible)
    LeftPanel->Show();
  else
    LeftPanel->Hide();
  if (Opt.RightPanel.Visible)
    RightPanel->Show();
  else
    RightPanel->Hide();
  HideState=(!Opt.LeftPanel.Visible && !Opt.RightPanel.Visible);
  CmdLine.Redraw();
}


ControlObject::~ControlObject()
{
  ModalManager.CloseAll();
  if (ActivePanel!=NULL)
  {
    if (Opt.AutoSaveSetup)
      SaveConfig(0);
    if (ActivePanel->GetMode()!=PLUGIN_PANEL)
    {
      char CurDir[NM];
      ActivePanel->GetCurDir(CurDir);
      FolderHistory->AddToHistory(CurDir,NULL,0);
    }
  }
  if (Opt.SaveEditorPos)
    EditorPosCache.Save("Editor\\LastPositions");
  if (Opt.SaveViewerPos)
    ViewerPosCache.Save("Viewer\\LastPositions");
  if (LastLeftFilePanel!=LeftPanel && LastLeftFilePanel!=RightPanel)
    DeletePanel(LastLeftFilePanel);
  if (LastRightFilePanel!=LeftPanel && LastRightFilePanel!=RightPanel)
    DeletePanel(LastRightFilePanel);
  DeletePanel(LeftPanel);
  LeftPanel=NULL;
  DeletePanel(RightPanel);
  RightPanel=NULL;
  Plugins.SendExit();
  PanelFilter::CloseFilter();
  delete CmdHistory;
  delete FolderHistory;
  delete ViewHistory;
  Lang.Close();
  CtrlObject=NULL;
}


void ControlObject::SetPanelPositions(int LeftFullScreen,int RightFullScreen)
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
}


void ControlObject::SetScreenPositions()
{
  RedrawDesktop Redraw;
  CmdLine.Hide();

  CmdLine.SetPosition(0,ScrY-(Opt.ShowKeyBar!=0),ScrX,ScrY-(Opt.ShowKeyBar!=0));
  TopMenuBar.SetPosition(0,0,ScrX,0);
  MainKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
  SetPanelPositions(LeftPanel->IsFullScreen(),RightPanel->IsFullScreen());
}


void ControlObject::RedrawKeyBar()
{
  char *FKeys[]={MSG(MF1),MSG(MF2),MSG(MF3),MSG(MF4),MSG(MF5),MSG(MF6),MSG(MF7),MSG(MF8),MSG(MF9),MSG(MF10),MSG(MF11),MSG(MF12)};
  char *FAltKeys[]={MSG(MAltF1),MSG(MAltF2),MSG(MAltF3),MSG(MAltF4),MSG(MAltF5),MSG(MAltF6),MSG(MAltF7),MSG(MAltF8),MSG(MAltF9),MSG(MAltF10),MSG(MAltF11),MSG(MAltF12)};
  char *FCtrlKeys[]={MSG(MCtrlF1),MSG(MCtrlF2),MSG(MCtrlF3),MSG(MCtrlF4),MSG(MCtrlF5),MSG(MCtrlF6),MSG(MCtrlF7),MSG(MCtrlF8),MSG(MCtrlF9),MSG(MCtrlF10),MSG(MCtrlF11),MSG(MCtrlF12)};
  char *FShiftKeys[]={MSG(MShiftF1),MSG(MShiftF2),MSG(MShiftF3),MSG(MShiftF4),MSG(MShiftF5),MSG(MShiftF6),MSG(MShiftF7),MSG(MShiftF8),MSG(MShiftF9),MSG(MShiftF10),MSG(MShiftF11),MSG(MShiftF12)};
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
    }
  }
  MainKeyBar.Set(FKeys,sizeof(FKeys)/sizeof(FKeys[0]));
  MainKeyBar.SetAlt(FAltKeys,sizeof(FAltKeys)/sizeof(FAltKeys[0]));
  MainKeyBar.SetCtrl(FCtrlKeys,sizeof(FCtrlKeys)/sizeof(FCtrlKeys[0]));
  MainKeyBar.SetShift(FShiftKeys,sizeof(FShiftKeys)/sizeof(FShiftKeys[0]));
  MainKeyBar.Redraw();
}


Panel* ControlObject::CreatePanel(int Type)
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


void ControlObject::DeletePanel(Panel *Deleted)
{
  if (Deleted==NULL)
    return;
  delete Deleted;
  if (Deleted==LastLeftFilePanel)
    LastLeftFilePanel=NULL;
  if (Deleted==LastRightFilePanel)
    LastRightFilePanel=NULL;
}


int ControlObject::ProcessKey(int Key)
{
  if (!Key)
    return(TRUE);

  if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLRIGHT) && (CmdLine.GetLength()>0 ||
      !LeftPanel->IsVisible() && !RightPanel->IsVisible()))
  {
    CmdLine.ProcessKey(Key);
    return(TRUE);
  }

  switch(Key)
  {
    case KEY_CTRLTAB:
      ModalManager.NextModal(1);
      break;
    case KEY_CTRLSHIFTTAB:
      ModalManager.NextModal(-1);
      break;
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
      CmdLine.Show();
      break;
    case KEY_F1:
      if (!ActivePanel->ProcessKey(KEY_F1))
      {
        Help Hlp("Contents");
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
      CmdLine.Show();
      break;
    case KEY_CTRLB:
      Opt.ShowKeyBar=!Opt.ShowKeyBar;
      SetScreenPositions();
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
            if (AnotherPanel==LeftPanel)
              AnotherPanel=ChangePanel(AnotherPanel,LastLeftType,FALSE,FALSE);
            else
              AnotherPanel=ChangePanel(AnotherPanel,LastRightType,FALSE,FALSE);
          else
            AnotherPanel=ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

          AnotherPanel->Update(UPDATE_KEEP_SELECTION);
          AnotherPanel->Show();
        }
        ActivePanel->SetFocus();
      }
      break;
    case KEY_CTRLO:
      {
        int LeftVisible=LeftPanel->IsVisible();
        int RightVisible=RightPanel->IsVisible();
        if (!HideState || LeftVisible || RightVisible)
        {
          LeftStateBeforeHide=LeftVisible;
          RightStateBeforeHide=RightVisible;
          LeftPanel->Hide();
          RightPanel->Hide();
          HideState=TRUE;
        }
        else
        {
          if (!LeftStateBeforeHide && !RightStateBeforeHide)
            LeftStateBeforeHide=RightStateBeforeHide=TRUE;
          if (LeftStateBeforeHide)
            LeftPanel->Show();
          if (RightStateBeforeHide)
            RightPanel->Show();
          HideState=FALSE;
        }
      }
      CmdLine.Show();
      break;
    case KEY_CTRLP:
      if (ActivePanel->IsVisible())
      {
        Panel *AnotherPanel=GetAnotherPanel(ActivePanel);
        if (AnotherPanel->IsVisible())
          AnotherPanel->Hide();
        else
          AnotherPanel->Show();
        CmdLine.Redraw();
      }
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
        int XL1,YL1,XL2,YL2,VL;
        int XR1,YR1,XR2,YR2,VR;
        int SwapType;
        Panel *Swap;
        LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
        RightPanel->GetPosition(XR1,YR1,XR2,YR2);
        if (XL2-XL1!=ScrX && XR2-XR1!=ScrX)
        {
          VL=LeftPanel->IsVisible();
          VR=RightPanel->IsVisible();
          LeftPanel->Hide();
          RightPanel->Hide();
          Opt.WidthDecrement=-Opt.WidthDecrement;
          LeftPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
          RightPanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
          if (VL)
            LeftPanel->Show();
          if (VR)
            RightPanel->Show();
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
    case KEY_ALTF9:
      ChangeVideoMode(ScrY==24 ? 50:25);
      SetScreenPositions();
      break;
    case KEY_CTRLUP:
      if (Opt.HeightDecrement<ScrY-7)
      {
        Opt.HeightDecrement++;
        SetScreenPositions();
      }
      break;
    case KEY_CTRLDOWN:
      if (Opt.HeightDecrement>0)
      {
        Opt.HeightDecrement--;
        SetScreenPositions();
      }
      break;
    case KEY_CTRLLEFT:
      if (Opt.WidthDecrement<ScrX/2-10)
      {
        Opt.WidthDecrement++;
        SetScreenPositions();
      }
      break;
    case KEY_CTRLRIGHT:
      if (Opt.WidthDecrement>-(ScrX/2-10))
      {
        Opt.WidthDecrement--;
        SetScreenPositions();
      }
      break;
    case KEY_CTRLCLEAR:
      if (Opt.WidthDecrement!=0)
      {
        Opt.WidthDecrement=0;
        SetScreenPositions();
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
        CmdLine.ProcessKey(Key);
      break;
  }
  return(TRUE);
}


Panel* ControlObject::ChangePanelToFilled(Panel *Current,int NewType)
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


int ControlObject::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!ActivePanel->ProcessMouse(MouseEvent))
    if (!GetAnotherPanel(ActivePanel)->ProcessMouse(MouseEvent))
      if (!MainKeyBar.ProcessMouse(MouseEvent))
        CmdLine.ProcessMouse(MouseEvent);
  return(TRUE);
}


void ControlObject::EnterMainLoop()
{
  INPUT_RECORD rec;
  int Key;

  while (!EndLoop)
  {
    WaitInMainLoop=TRUE;
    Key=GetInputRecord(&rec);
    WaitInMainLoop=FALSE;
    if (EndLoop)
      break;
    MainKeyBar.RedrawIfChanged();
    if (rec.EventType==MOUSE_EVENT)
      ProcessMouse(&rec.Event.MouseEvent);
    else
      ProcessKey(Key);
    MainKeyBar.RedrawIfChanged();
  }
}


void ControlObject::ExitMainLoop(int Ask)
{
  if (!Ask || !Opt.Confirm.Exit || Message(0,2,MSG(MQuit),MSG(MAskQuit),MSG(MYes),MSG(MNo))==0)
    if (!LeftPanel->ProcessPluginEvent(FE_CLOSE,NULL) && !RightPanel->ProcessPluginEvent(FE_CLOSE,NULL))
      EndLoop=TRUE;
}


Panel* ControlObject::GetAnotherPanel(Panel *Current)
{
  if (Current==LeftPanel)
    return(RightPanel);
  else
    return(LeftPanel);
}


Panel* ControlObject::ChangePanel(Panel *Current,int NewType,int CreateNew,int Force)
{
  Panel *NewPanel;
  SaveScreen *SaveScr=NULL;
  int OldType,X1,Y1,X2,Y2;
  int OldViewMode,OldSortMode,OldSortOrder,OldSortGroups;
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
    switch(NewType)
    {
      case FILE_PANEL:
        NewPanel=new FileList;
        break;
      case TREE_PANEL:
        NewPanel=new TreeList;
        break;
      case QVIEW_PANEL:
        NewPanel=new QuickView;
        break;
      case INFO_PANEL:
        NewPanel=new InfoList;
        break;
    }

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
  }
  return(NewPanel);
}


void ControlObject::ShowCopyright()
{
/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "copyright.inc"
/* tran $ */
  char Str[256];
  strcpy(Str,Copyright);
  char Xor=17;
  for (int I=0;Str[I];I++)
  {
    Str[I]=(Str[I]&0x7f)^Xor;
    Xor^=Str[I];
  }
#ifdef BETA
  mprintf("Beta version %d.%02d.%d",BETA/1000,(BETA%1000)/10,BETA%10);
#else
  Text(Str);
#endif
}

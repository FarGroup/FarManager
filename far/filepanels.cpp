/*
filepanels.cpp

�������� ������

*/

/* Revision: 1.73 21.05.2006 $ */

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

static void PrepareOptFolderW(string &strSrc, int IsLocalPath_FarPath)
{
  if ( strSrc.IsEmpty() )
  {
    strSrc = g_strFarPath;
    DeleteEndSlashW(strSrc);
  }
  else
    apiExpandEnvironmentStrings(strSrc, strSrc);

  if(!wcscmp(strSrc,L"/"))
  {
    strSrc = g_strFarPath;

    wchar_t *lpwszSrc = strSrc.GetBuffer ();

    if(IsLocalPath_FarPath)
    {
      lpwszSrc[2]='\\';
      lpwszSrc[3]=0;
    }

    strSrc.ReleaseBuffer ();
  }
  else
    CheckShortcutFolderW(&strSrc,FALSE,TRUE);
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

  // �������� ��������� �� ��������� ��� �������
  int IsLocalPath_FarPath=IsLocalPathW(g_strFarPath);
  PrepareOptFolderW(Opt.strLeftFolder,IsLocalPath_FarPath);
  PrepareOptFolderW(Opt.strRightFolder,IsLocalPath_FarPath);
  PrepareOptFolderW(Opt.strPassiveFolder,IsLocalPath_FarPath);

  if (Opt.AutoSaveSetup || !Opt.SetupArgv)
  {
    if (GetFileAttributesW(Opt.strLeftFolder)!=0xffffffff)
      LeftPanel->InitCurDirW(Opt.strLeftFolder);
    if (GetFileAttributesW(Opt.strRightFolder)!=0xffffffff)
      RightPanel->InitCurDirW(Opt.strRightFolder);
  }

  if (!Opt.AutoSaveSetup)
  {
    if(Opt.SetupArgv >= 1)
    {
      if(ActivePanel==RightPanel)
      {
        if (GetFileAttributesW(Opt.strRightFolder)!=0xffffffff)
          RightPanel->InitCurDirW(Opt.strRightFolder);
      }
      else
      {
        if (GetFileAttributesW(Opt.strLeftFolder)!=0xffffffff)
          LeftPanel->InitCurDirW(Opt.strLeftFolder);
      }
      if(Opt.SetupArgv == 2)
      {
        if(ActivePanel==LeftPanel)
        {
          if (GetFileAttributesW(Opt.strRightFolder)!=0xffffffff)
            RightPanel->InitCurDirW(Opt.strRightFolder);
        }
        else
        {
          if (GetFileAttributesW(Opt.strLeftFolder)!=0xffffffff)
            LeftPanel->InitCurDirW(Opt.strLeftFolder);
        }
      }
    }
    if (Opt.SetupArgv < 2 && !Opt.strPassiveFolder.IsEmpty() && (GetFileAttributesW(Opt.strPassiveFolder)!=0xffffffff))
    {
      PassivePanel->InitCurDirW(Opt.strPassiveFolder);
    }
  }
#if 1
  //! ������� "����������" ��������� ������
  if(PassiveIsLeftFlag)
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
  // ��� ���������� ������� �� ������ �� ��������� ��������� ������� � CmdLine
  if (!Opt.RightPanel.Visible && !Opt.LeftPanel.Visible)
  {
    CtrlObject->CmdLine->SetCurDirW(PassiveIsLeftFlag?Opt.strRightFolder:Opt.strLeftFolder);
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
  if (LeftFullScreen){
    LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
    LeftPanel->ViewSettings.FullScreen=1;
  } else {
    LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX/2-Opt.WidthDecrement,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  }
  if (RightFullScreen) {
    RightPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
    RightPanel->ViewSettings.FullScreen=1;
  } else {
    RightPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
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
  if (ActivePanel->GetType()==FILE_PANEL)
  {
    wchar_t empty[] = L"";
    wchar_t *FKeys[]={UMSG(MF1),UMSG(MF2),UMSG(MF3),UMSG(MF4),UMSG(MF5),UMSG(MF6),UMSG(MF7),UMSG(MF8),UMSG(MF9),UMSG(MF10),UMSG(MF11),UMSG(MF12)};
    wchar_t *FAltKeys[]={UMSG(MAltF1),UMSG(MAltF2),UMSG(MAltF3),UMSG(MAltF4),UMSG(MAltF5),empty,UMSG(MAltF7),UMSG(MAltF8),UMSG(MAltF9),UMSG(MAltF10),UMSG(MAltF11),UMSG(MAltF12)};
    wchar_t *FCtrlKeys[]={UMSG(MCtrlF1),UMSG(MCtrlF2),UMSG(MCtrlF3),UMSG(MCtrlF4),UMSG(MCtrlF5),UMSG(MCtrlF6),UMSG(MCtrlF7),UMSG(MCtrlF8),UMSG(MCtrlF9),UMSG(MCtrlF10),UMSG(MCtrlF11),UMSG(MCtrlF12)};
    wchar_t *FShiftKeys[]={UMSG(MShiftF1),UMSG(MShiftF2),UMSG(MShiftF3),UMSG(MShiftF4),UMSG(MShiftF5),UMSG(MShiftF6),UMSG(MShiftF7),UMSG(MShiftF8),UMSG(MShiftF9),UMSG(MShiftF10),UMSG(MShiftF11),UMSG(MShiftF12)};

    wchar_t *FAltShiftKeys[]={UMSG(MAltShiftF1),UMSG(MAltShiftF2),UMSG(MAltShiftF3),UMSG(MAltShiftF4),UMSG(MAltShiftF5),UMSG(MAltShiftF6),UMSG(MAltShiftF7),UMSG(MAltShiftF8),UMSG(MAltShiftF9),UMSG(MAltShiftF10),UMSG(MAltShiftF11),UMSG(MAltShiftF12)};
    wchar_t *FCtrlShiftKeys[]={UMSG(MCtrlShiftF1),UMSG(MCtrlShiftF2),UMSG(MCtrlShiftF3),UMSG(MCtrlShiftF4),UMSG(MCtrlShiftF5),UMSG(MCtrlShiftF6),UMSG(MCtrlShiftF7),UMSG(MCtrlShiftF8),UMSG(MCtrlShiftF9),UMSG(MCtrlShiftF10),UMSG(MCtrlShiftF11),UMSG(MCtrlShiftF12)};
    wchar_t *FCtrlAltKeys[]={UMSG(MCtrlAltF1),UMSG(MCtrlAltF2),UMSG(MCtrlAltF3),UMSG(MCtrlAltF4),UMSG(MCtrlAltF5),UMSG(MCtrlAltF6),UMSG(MCtrlAltF7),UMSG(MCtrlAltF8),UMSG(MCtrlAltF9),UMSG(MCtrlAltF10),UMSG(MCtrlAltF11),UMSG(MCtrlAltF12)};

    FAltKeys[6-1]=(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)?UMSG(MAltF6):empty;

    if (ActivePanel!=NULL && ActivePanel->GetMode()==PLUGIN_PANEL)
    {
      struct OpenPluginInfoW Info;
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

        // ���, �� ���� ������� ����������� ������ ��������� ;-)
        if(Info.StructSize >= sizeof(struct OpenPluginInfoW))
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
  }
  else // ��� �� �������� ������... �������� ��������� ������
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

  if ( pResult )
    pResult->SetOwner (this);

  return pResult;
}


void FilePanels::DeletePanel(Panel *Deleted)
{
  if (Deleted==NULL)
    return;
  /* $ 27.11.2001 DJ
     �� ����� ������������ ��������� ����� ����, ��� ��� �������
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
  int Ret=FALSE; // ��� ������ �� ���� �� ������� �� �����

  if (LeftPanel->IsVisible() || RightPanel->IsVisible())
  {
    int XL1,YL1,XL2,YL2;
    int XR1,YR1,XR2,YR2;

    LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
    RightPanel->GetPosition(XR1,YR1,XR2,YR2);
    if (!LeftPanel->ViewSettings.FullScreen || !RightPanel->ViewSettings.FullScreen)
    {
      Opt.WidthDecrement=-Opt.WidthDecrement;
      if (!LeftPanel->ViewSettings.FullScreen){
        LeftPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
        if (LastLeftFilePanel)
          LastLeftFilePanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
      }
      if(!RightPanel->ViewSettings.FullScreen){
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
    PanelFilter::SwapFilter();

    Ret=TRUE;
  }
  FrameManager->RefreshFrame();
  return Ret;
}

int  FilePanels::ProcessKey(int Key)
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

  switch(Key)
  {
    case MCODE_C_EOF:
    case MCODE_C_BOF:
    case MCODE_C_SELECTED:
    case MCODE_V_ITEMCOUNT:
    case MCODE_V_CURPOS:
      return ActivePanel->ProcessKey(Key);
  }

  switch(Key)
  {
    case KEY_F1:
    {
      if (!ActivePanel->ProcessKey(KEY_F1))
      {
        Help Hlp (L"Contents");
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
      if(!KeyBarVisible)
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
            ��������� ������� �� ctrl-l|q|t ������ �������� �������� ������
          */
            AnotherPanel=ChangePanel(AnotherPanel,FILE_PANEL,FALSE,FALSE);
          /* IS % */
          else
            AnotherPanel=ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

          /* $ 07.09.2001 VVM
            ! ��� �������� �� CTRL+Q, CTRL+L ����������� �������, ���� �������� ������ - ������. */
          if (ActivePanel->GetType() == TREE_PANEL)
          {
            string strCurDir;
            ActivePanel->GetCurDirW(strCurDir);
            AnotherPanel->SetCurDirW(strCurDir, TRUE);
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
       + ��������� ������� ������ ���������� � ������� �� CtrlAltShift
    */
/* $ KEY_CTRLALTSHIFTPRESS ������� � manager OT */
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
            if(ActivePanel == RightPanel)
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
      SwapPanels();
      break;
    }

    /* $ 08.04.2002 IS
       ��� ����� ����� ��������� ������������� ������� ������� �� ��������
       ������, �.�. ������� �� ����� ������ � ���, ��� � ���� ��� ������, �
       ������� ��� ������� ����� ����� ����� ����� ���� ������� � �� ���������
       ������
    */
    case KEY_ALTF1:
    {
      LeftPanel->ChangeDisk();
      if(ActivePanel!=LeftPanel)
        ActivePanel->SetCurPath();
      break;
    }

    case KEY_ALTF2:
    {
      RightPanel->ChangeDisk();
      if(ActivePanel!=RightPanel)
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
      if(Key >= KEY_CTRL0 && Key <= KEY_CTRL9)
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
  if(Current && Mode >= VIEW_0 && Mode <= VIEW_9)
  {
    Current->SetViewMode(Mode);
    Current=ChangePanelToFilled(Current,FILE_PANEL);
    Current->SetViewMode(Mode);
    // ��������! �������! �� ��������!
    SetScreenPosition();
    if(RefreshFrame)
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
   + "���������" ��������� ������ "���������� ����� ������"
*/
Panel* FilePanels::ChangePanel(Panel *Current,int NewType,int CreateNew,int Force)
{
  Panel *NewPanel;
  SaveScreen *SaveScr=NULL;
  // OldType �� �����������������...
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
    /* $ 13.07.2000 SVS
       ������� �������� ��� ����� ������ ������� ������ CreatePanel(int Type)
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

int  FilePanels::GetTypeAndName(string &strType, string &strName)
{
  strType = UMSG(MScreensPanels);

  string strFullName, strShortName;

  switch (ActivePanel->GetType())
  {
    case TREE_PANEL:
    case QVIEW_PANEL:
    case FILE_PANEL:
    case INFO_PANEL:
        ActivePanel->GetCurNameW(strFullName, strShortName);
        ConvertNameToFullW(strFullName, strFullName);
        break;
  }

  strName = strFullName;

  return(MODALTYPE_PANELS);
}

void FilePanels::OnChangeFocus(int f)
{
  _OT(SysLog("FilePanels::OnChangeFocus(%i)",f));
  /* $ 20.06.2001 tran
     ��� � ���������� ��� ����������� � ��������
     �� ���������� LockRefreshCount */
  if ( f ) {
    /*$ 22.06.2001 SKV
      + update ������� ��� ��������� ������
    */
    CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
    ActivePanel->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
    /* SKV$*/
    /* $ 13.04.2002 KM
      ! ??? � �� ����� ����� ����� Redraw, ����
        Redraw ���������� ������ �� Frame::OnChangeFocus.
//    Redraw();
    /* KM $ */
    Frame::OnChangeFocus(1);
  }
}

void FilePanels::DisplayObject ()
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
  else
    if(MainKeyBar.IsVisible())
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

  //! ������� "����������" ��������� ������
  if(PassiveIsLeftFlag)
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
    ������� ���, � �� Frame::OnChangeFocus,
    ������� �� ����� � ��������.
  */
  //Frame::OnChangeFocus(1);
  OnChangeFocus(1);
  /* SKV$*/
}

void FilePanels::GoToFileW (const wchar_t *FileName)
{
  if(wcschr(FileName,'\\') || wcschr(FileName,'/'))
  {
    string ADir,PDir;
    Panel *PassivePanel = GetAnotherPanel(ActivePanel);
    int PassiveMode = PassivePanel->GetMode();
    if (PassiveMode == NORMAL_PANEL)
    {
      PassivePanel->GetCurDirW(PDir);
      AddEndSlashW (PDir);
    }

    int ActiveMode = ActivePanel->GetMode();
    if(ActiveMode==NORMAL_PANEL)
    {
      ActivePanel->GetCurDirW(ADir);
      AddEndSlashW (ADir);
    }

    string strNameFile = PointToNameW (FileName);
    string strNameDir = FileName;

    CutToSlashW (strNameDir);

    /* $ 10.04.2001 IS
         �� ������ SetCurDir, ���� ������ ���� ��� ���� �� ��������
         �������, ��� ����� ���������� ����, ��� ��������� � ���������
         ������� �� ������������.
    */
    BOOL AExist=(ActiveMode==NORMAL_PANEL) && (LocalStricmpW(ADir,strNameDir)==0);
    BOOL PExist=(PassiveMode==NORMAL_PANEL) && (LocalStricmpW(PDir,strNameDir)==0);
    // ���� ������ ���� ���� �� ��������� ������
    if (!AExist && PExist)
      ProcessKey(KEY_TAB);
    if (!AExist && !PExist)
      ActivePanel->SetCurDirW(strNameDir,TRUE);
    /* IS */
    ActivePanel->GoToFileW(strNameFile);
    // ������ ������� ��������� ������, ����� ���� �������� �����, ���
    // Ctrl-F10 ���������
    ActivePanel->SetTitle();
  }
}


/* VVM $ */

/* $ 16.01.2002 OT
   ���������������� ����������� ����� �� Frame
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

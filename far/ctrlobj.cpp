/*
ctrlobj.cpp

Управление остальными объектами, раздача сообщений клавиатуры и мыши

*/

/* Revision: 1.21 06.05.2001 $ */

/*
Modify:
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 VVM
    + KeyBar тоже умеет обрабатывать клавиши.
  22.04.2001 SVS
    ! Загрузка плагнов - после создания ВСЕХ основных объектов
  02.04.2001 VVM
    + Обработка Opt.FlagPosixSemantics
  28.02.2001 IS
    ! Т.е. CmdLine теперь указатель, то произведем замену
      "CmdLine." на "CmdLine->" и собственно создадим/удалим ее в конструкторе
      и деструкторе CtrlObject.
  09.02.2001 IS
    + восстановим состояние опций "помеченное вперед"
  09.01.2001 SVS
    + Учтем правило Opt.ShiftsKeyRules (WaitInFastFind)
  29.12.2000 IS
    + Проверяем при выходе, сохранены ли все измененные файлы. Если нет, то
      не выходим из фара.
  15.12.2000 SVS
    ! Метод ShowCopyright - public static & параметр Flags.
      Если Flags&1, то использовать printf вместо внутренних функций
  25.11.2000 SVS
    ! Copyright в 2 строки
  27.09.2000 SVS
    ! Ctrl-Alt-Shift - реагируем, если надо.
  19.09.2000 IS
    ! Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
  19.09.2000 SVS
    + Opt.PanelCtrlAltShiftRule задает поведение Ctrl-Alt-Shift для панелей.
  19.09.2000 SVS
    + Добавляем реакцию показа бакграунда в панелях на CtrlAltShift
  07.09.2000 tran 1.05
    + Current File
  15.07.2000 tran
    + а я код раздуваю :) вводя новый метод Redraw
  13.07.2000 SVS
    ! Некоторые коррекция по сокращению кода ;-)
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  29.06.2000 tran
    ! соощение о копирайте включается из copyright.inc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
  FPanels=0;
  CtrlObject=this;
  ReadConfig();
  /* $ 28.02.2001 IS
       Создадим обязательно только после того, как прочитали настройки
  */
  CmdLine=new CommandLine;
  /* IS $ */
  CmdHistory=new History("SavedHistory",&Opt.SaveHistory,FALSE,FALSE);
  FolderHistory=new History("SavedFolderHistory",&Opt.SaveFoldersHistory,FALSE,TRUE);
  ViewHistory=new History("SavedViewHistory",&Opt.SaveViewHistory,TRUE,TRUE);
  FolderHistory->SetAddMode(TRUE,2,TRUE);
  ViewHistory->SetAddMode(TRUE,Opt.FlagPosixSemantics?1:2,TRUE);
  if (Opt.SaveHistory)
    CmdHistory->ReadHistory();
  if (Opt.SaveFoldersHistory)
    FolderHistory->ReadHistory();
  if (Opt.SaveViewHistory)
    ViewHistory->ReadHistory();
///  LastLeftType=LastRightType=FILE_PANEL;
///  LastLeftFilePanel=LastRightFilePanel=NULL;
///  Cp()->ActivePanel=NULL;
  RegVer=-1;
}


void ControlObject::Init()
{
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

/*///
  Cp()->LeftPanel=CreatePanel(Opt.LeftPanel.Type);
  Cp()->RightPanel=CreatePanel(Opt.RightPanel.Type);

  SetPanelPositions(FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
                    FileList::IsModeFullScreen(Opt.RightPanel.ViewMode));
  Cp()->LeftPanel->SetViewMode(Opt.LeftPanel.ViewMode);
  Cp()->RightPanel->SetViewMode(Opt.RightPanel.ViewMode);
  Cp()->LeftPanel->SetSortMode(Opt.LeftPanel.SortMode);
  Cp()->RightPanel->SetSortMode(Opt.RightPanel.SortMode);
  Cp()->LeftPanel->SetSortOrder(Opt.LeftPanel.SortOrder);
  Cp()->RightPanel->SetSortOrder(Opt.RightPanel.SortOrder);
  Cp()->LeftPanel->SetSortGroups(Opt.LeftPanel.SortGroups);
  Cp()->RightPanel->SetSortGroups(Opt.RightPanel.SortGroups);
  Cp()->LeftPanel->SetShowShortNamesMode(Opt.LeftPanel.ShowShortNames);
  Cp()->RightPanel->SetShowShortNamesMode(Opt.RightPanel.ShowShortNames);

  MainKeyBar->SetOwner(this);
  RedrawKeyBar();
  SetScreenPositions();
  if (Opt.LeftPanel.Focus)
    Cp()->ActivePanel=Cp()->LeftPanel;
  else
    Cp()->ActivePanel=Cp()->RightPanel;
  Cp()->ActivePanel->SetFocus();
  if (Opt.AutoSaveSetup)
  {
    if (GetFileAttributes(Opt.LeftFolder)!=0xffffffff)
      Cp()->LeftPanel->InitCurDir(Opt.LeftFolder);
    if (GetFileAttributes(Opt.RightFolder)!=0xffffffff)
      Cp()->RightPanel->InitCurDir(Opt.RightFolder);
  }
  else
    if (*Opt.PassiveFolder)
    {
      Panel *PassivePanel=GetAnotherPanel(Cp()->ActivePanel);
      if (GetFileAttributes(Opt.PassiveFolder)!=0xffffffff)
        PassivePanel->InitCurDir(Opt.PassiveFolder);
    }
*///
///
  FPanels=new FilePanels();
  FPanels->Init();
  this->MainKeyBar=&(FPanels->MainKeyBar);
  this->TopMenuBar=&(FPanels->TopMenuBar);
  FPanels->CmdLine=ControlObject::CmdLine;
  FPanels->SetScreenPositions();
///
_beginthread(CheckVersion,0x10000,NULL);
  Cp()->LeftPanel->Update(0);
  Cp()->RightPanel->Update(0);

  /* $ 07.09.2000 tran
    + Config//Current File */
  if (Opt.AutoSaveSetup)
  {
      Cp()->LeftPanel->GoToFile(Opt.LeftCurFile);
      Cp()->RightPanel->GoToFile(Opt.RightCurFile);
  }
  /* tran 07.09.2000 $ */

  if (Opt.LeftPanel.Visible)
    Cp()->LeftPanel->Show();
  else
    Cp()->LeftPanel->Hide();
  if (Opt.RightPanel.Visible)
    Cp()->RightPanel->Show();
  else
    Cp()->RightPanel->Hide();
  Cp()->HideState=(!Opt.LeftPanel.Visible && !Opt.RightPanel.Visible);
  CmdLine->Redraw();

  ModalManager.AddFrame(FPanels);
  Plugins.LoadPlugins();
}


ControlObject::~ControlObject()
{
///  ModalManager.CloseAll();
  if (Cp()->ActivePanel!=NULL)
  {
    if (Opt.AutoSaveSetup)
      SaveConfig(0);
    if (Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
    {
      char CurDir[NM];
      Cp()->ActivePanel->GetCurDir(CurDir);
      FolderHistory->AddToHistory(CurDir,NULL,0);
    }
  }
  if (Opt.SaveEditorPos)
    EditorPosCache.Save("Editor\\LastPositions");
  if (Opt.SaveViewerPos)
    ViewerPosCache.Save("Viewer\\LastPositions");
///
  ModalManager.CloseAll();
  FPanels=NULL;
///
/*///
  if (LastLeftFilePanel!=Cp()->LeftPanel && LastLeftFilePanel!=Cp()->RightPanel)
    DeletePanel(LastLeftFilePanel);
  if (LastRightFilePanel!=Cp()->LeftPanel && LastRightFilePanel!=Cp()->RightPanel)
    DeletePanel(LastRightFilePanel);
  DeletePanel(Cp()->LeftPanel);
  Cp()->LeftPanel=NULL;
  DeletePanel(Cp()->RightPanel);
  Cp()->RightPanel=NULL;
*///
  Plugins.SendExit();
  PanelFilter::CloseFilter();
  delete CmdHistory;
  delete FolderHistory;
  delete ViewHistory;
  delete CmdLine;
  Lang.Close();
  CtrlObject=NULL;
}

/*///
void ControlObject::SetPanelPositions(int LeftFullScreen,int RightFullScreen)
{
  if (Opt.HeightDecrement>ScrY-7)
    Opt.HeightDecrement=ScrY-7;
  if (Opt.HeightDecrement<0)
    Opt.HeightDecrement=0;
  if (LeftFullScreen)
    Cp()->LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  else
    Cp()->LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX/2-Opt.WidthDecrement,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  if (RightFullScreen)
    Cp()->RightPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  else
    Cp()->RightPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
}
*///
/*///
void ControlObject::SetScreenPositions()
{
  RedrawDesktop Redraw;
  CmdLine->Hide();

  CmdLine->SetPosition(0,ScrY-(Opt.ShowKeyBar!=0),ScrX,ScrY-(Opt.ShowKeyBar!=0));
  TopMenuBar.SetPosition(0,0,ScrX,0);
  MainKeyBar->SetPosition(0,ScrY,ScrX,ScrY);
  SetPanelPositions(Cp()->LeftPanel->IsFullScreen(),Cp()->RightPanel->IsFullScreen());
}
*///

/* $ 02.08.2000 SVS
   Новые индикаторы!
*/
#if 0
void ControlObject::RedrawKeyBar()
{
  /* $ 30.04.2001 DJ
     использование UpdateKeyBar() и SetAllGroup()
  */
  if (!ActivePanel || !ActivePanel->UpdateKeyBar())
  {
    MainKeyBar.SetAllGroup (KBL_MAIN, MF1, 12);
    MainKeyBar.SetAllGroup (KBL_ALT, MAltF1, 12);
    MainKeyBar.SetAllGroup (KBL_CTRL, MCtrlF1, 12);
    MainKeyBar.SetAllGroup (KBL_SHIFT, MShiftF1, 12);
    MainKeyBar.SetAllGroup (KBL_ALTSHIFT, MAltShiftF1, 12);
    MainKeyBar.SetAllGroup (KBL_CTRLSHIFT, MCtrlShiftF1, 12);
    MainKeyBar.SetAllGroup (KBL_CTRLALT, MCtrlAltF1, 12);
  }
  /* DJ $ */

  MainKeyBar->Redraw();
}
#endif

/* SVS $ */

/*///
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
*///

#if 0
int ControlObject::ProcessKey(int Key)
{
  int KeyProcessed; ///
  if (!Key)
    return(TRUE);
///
  SysLog(1,"ControlObject::ProcessKey(), Key=%i, call ModalManager->ProcessKey",Key);
  KeyProcessed=ModalManager.ProcessKey(Key);
  SysLog("ControlObject::ProcessKey(), KeyProcessed=%i",KeyProcessed);

  if ( KeyProcessed==TRUE )
  {
    SysLog(-1);
    return (TRUE);
  }
///

/*///
  if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLRIGHT) && (CmdLine->GetLength()>0 ||
      !Cp()->LeftPanel->IsVisible() && !Cp()->RightPanel->IsVisible()))
  {
    CmdLine->ProcessKey(Key);
    return(TRUE);
  }
*///

  switch(Key)
  {
    case KEY_TAB:
      if (Cp()->ActivePanel==Cp()->LeftPanel)
      {
        if (Cp()->RightPanel->IsVisible())
          Cp()->RightPanel->SetFocus();
      }
      else
        if (Cp()->LeftPanel->IsVisible())
          Cp()->LeftPanel->SetFocus();
      break;
    case KEY_CTRLF1:
      if (Cp()->LeftPanel->IsVisible())
      {
        Cp()->LeftPanel->Hide();
        if (Cp()->RightPanel->IsVisible())
          Cp()->RightPanel->SetFocus();
      }
      else
      {
        if (!Cp()->RightPanel->IsVisible())
          Cp()->LeftPanel->SetFocus();
        Cp()->LeftPanel->Show();
      }
      CmdLine->Show();
      break;
    case KEY_F1:
      if (!Cp()->ActivePanel->ProcessKey(KEY_F1))
      {
        Help Hlp("Contents");
      }
      return(TRUE);
    case KEY_CTRLF2:
      if (Cp()->RightPanel->IsVisible())
      {
        Cp()->RightPanel->Hide();
        if (Cp()->LeftPanel->IsVisible())
          Cp()->LeftPanel->SetFocus();
      }
      else
      {
        if (!Cp()->LeftPanel->IsVisible())
          Cp()->RightPanel->SetFocus();
        Cp()->RightPanel->Show();
      }
      CmdLine->Show();
      break;
/***/
    case KEY_CTRLB:
      Opt.ShowKeyBar=!Opt.ShowKeyBar;
      Cp()->SetScreenPositions();
      break;
    case KEY_CTRLL:
    case KEY_CTRLQ:
    case KEY_CTRLT:
      if (Cp()->ActivePanel->IsVisible())
      {
        Panel *AnotherPanel=Cp()->GetAnotherPanel(Cp()->ActivePanel);
        int NewType;
        if (Key==KEY_CTRLL)
          NewType=INFO_PANEL;
        else
          if (Key==KEY_CTRLQ)
            NewType=QVIEW_PANEL;
          else
            NewType=TREE_PANEL;
        if (Cp()->ActivePanel->GetType()==NewType)
          AnotherPanel=Cp()->ActivePanel;
        if (!AnotherPanel->ProcessPluginEvent(FE_CLOSE,NULL))
        {
          if (AnotherPanel->GetType()==NewType)
          /* $ 19.09.2000 IS
            Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
          */
            AnotherPanel=Cp()->ChangePanel(AnotherPanel,FILE_PANEL,FALSE,FALSE);
          /* IS % */
          else
            AnotherPanel=Cp()->ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

          AnotherPanel->Update(UPDATE_KEEP_SELECTION);
          AnotherPanel->Show();
        }
        Cp()->ActivePanel->SetFocus();
      }
      break;
    /* $ 19.09.2000 SVS
       + Добавляем реакцию показа бакграунда в панелях на CtrlAltShift
    */
    case KEY_CTRLALTSHIFTPRESS:
    {
      if(Opt.AllCtrlAltShiftRule & CASR_PANEL)
      {
        /* $ 19.09.2000 SVS
         + Opt.PanelCtrlAltShiftRule задает поведение
           Ctrl-Alt-Shift для панелей.
        */
        int LeftVisible=Cp()->LeftPanel->IsVisible();
        int RightVisible=Cp()->RightPanel->IsVisible();
        int CmdLineVisible=CmdLine->IsVisible();
        int KeyBarVisible=MainKeyBar->IsVisible();
        Cp()->LeftPanel->Hide();
        Cp()->RightPanel->Hide();
        if(!Opt.PanelCtrlAltShiftRule)
          CmdLine->Show();
        if(Opt.PanelCtrlAltShiftRule == 2)
          MainKeyBar->Hide();
        if(Opt.PanelCtrlAltShiftRule)
          CmdLine->Hide();
        WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:-1);
        if (LeftVisible)      Cp()->LeftPanel->Show();
        if (RightVisible)     Cp()->RightPanel->Show();
        if (CmdLineVisible)   CmdLine->Show();
        if (KeyBarVisible)    MainKeyBar->Show();
        /* SVS $ */
      }
      break;
    }
    /* SVS $ */

    case KEY_CTRLO:
      {
        int LeftVisible=Cp()->LeftPanel->IsVisible();
        int RightVisible=Cp()->RightPanel->IsVisible();
        if (!Cp()->HideState || LeftVisible || RightVisible)
        {
          Cp()->LeftStateBeforeHide=LeftVisible;
          Cp()->RightStateBeforeHide=RightVisible;
          Cp()->LeftPanel->Hide();
          Cp()->RightPanel->Hide();
          Cp()->HideState=TRUE;
        }
        else
        {
          if (!Cp()->LeftStateBeforeHide && !Cp()->RightStateBeforeHide)
            Cp()->LeftStateBeforeHide=Cp()->RightStateBeforeHide=TRUE;
          if (Cp()->LeftStateBeforeHide)
            Cp()->LeftPanel->Show();
          if (Cp()->RightStateBeforeHide)
            Cp()->RightPanel->Show();
          Cp()->HideState=FALSE;
        }
      }
      CmdLine->Show();
      break;
    case KEY_CTRLP:
      if (Cp()->ActivePanel->IsVisible())
      {
        Panel *AnotherPanel=Cp()->GetAnotherPanel(Cp()->ActivePanel);
        if (AnotherPanel->IsVisible())
          AnotherPanel->Hide();
        else
          AnotherPanel->Show();
        CmdLine->Redraw();
      }
      break;
    case KEY_CTRLI:
      Cp()->ActivePanel->EditFilter();
      return(TRUE);
    case KEY_CTRLW:
      ShowProcessList();
      return(TRUE);
    case KEY_CTRLU:
      if (Cp()->LeftPanel->IsVisible() || Cp()->RightPanel->IsVisible())
      {
        int XL1,YL1,XL2,YL2,VL;
        int XR1,YR1,XR2,YR2,VR;
        int SwapType;
        Panel *Swap;
        Cp()->LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
        Cp()->RightPanel->GetPosition(XR1,YR1,XR2,YR2);
        if (XL2-XL1!=ScrX && XR2-XR1!=ScrX)
        {
          VL=Cp()->LeftPanel->IsVisible();
          VR=Cp()->RightPanel->IsVisible();
          Cp()->LeftPanel->Hide();
          Cp()->RightPanel->Hide();
          Opt.WidthDecrement=-Opt.WidthDecrement;
          Cp()->LeftPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
          Cp()->RightPanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
          if (VL)
            Cp()->LeftPanel->Show();
          if (VR)
            Cp()->RightPanel->Show();
          Swap=Cp()->LeftPanel;
          Cp()->LeftPanel=Cp()->RightPanel;
          Cp()->RightPanel=Swap;
          if (Cp()->LastLeftFilePanel)
            Cp()->LastLeftFilePanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
          if (Cp()->LastRightFilePanel)
            Cp()->LastRightFilePanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
          Swap=Cp()->LastLeftFilePanel;
          Cp()->LastLeftFilePanel=Cp()->LastRightFilePanel;
          Cp()->LastRightFilePanel=Swap;
          SwapType=Cp()->LastLeftType;
          Cp()->LastLeftType=Cp()->LastRightType;
          Cp()->LastRightType=SwapType;
          PanelFilter::SwapFilter();
        }
      }
      break;
    case KEY_ALTF1:
      Cp()->LeftPanel->ChangeDisk();
      break;
    case KEY_ALTF2:
      Cp()->RightPanel->ChangeDisk();
      break;
    case KEY_ALTF7:
      {
        FindFiles FindFiles;
      }
      break;
    case KEY_ALTF9:
      ChangeVideoMode(ScrY==24 ? 50:25);
      Cp()->SetScreenPositions();
      break;
    case KEY_CTRLUP:
      if (Opt.HeightDecrement<ScrY-7)
      {
        Opt.HeightDecrement++;
        Cp()->SetScreenPositions();
      }
      break;
    case KEY_CTRLDOWN:
      if (Opt.HeightDecrement>0)
      {
        Opt.HeightDecrement--;
        Cp()->SetScreenPositions();
      }
      break;
    case KEY_CTRLLEFT:
      if (Opt.WidthDecrement<ScrX/2-10)
      {
        Opt.WidthDecrement++;
        Cp()->SetScreenPositions();
      }
      break;
    case KEY_CTRLRIGHT:
      if (Opt.WidthDecrement>-(ScrX/2-10))
      {
        Opt.WidthDecrement--;
        Cp()->SetScreenPositions();
      }
      break;
    case KEY_CTRLCLEAR:
      if (Opt.WidthDecrement!=0)
      {
        Opt.WidthDecrement=0;
        Cp()->SetScreenPositions();
      }
      break;
    case KEY_F9:
      ShellOptions(0,NULL);
      return(TRUE);
    case KEY_SHIFTF10:
      ShellOptions(1,NULL);
      return(TRUE);
    case KEY_CTRL1:
      Cp()->ActivePanel->SetViewMode(VIEW_1);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_1);
      break;
    case KEY_CTRL2:
      Cp()->ActivePanel->SetViewMode(VIEW_2);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_2);
      break;
    case KEY_CTRL3:
      Cp()->ActivePanel->SetViewMode(VIEW_3);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_3);
      break;
    case KEY_CTRL4:
      Cp()->ActivePanel->SetViewMode(VIEW_4);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_4);
      break;
    case KEY_CTRL5:
      Cp()->ActivePanel->SetViewMode(VIEW_5);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_5);
      break;
    case KEY_CTRL6:
      Cp()->ActivePanel->SetViewMode(VIEW_6);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_6);
      break;
    case KEY_CTRL7:
      Cp()->ActivePanel->SetViewMode(VIEW_7);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_7);
      break;
    case KEY_CTRL8:
      Cp()->ActivePanel->SetViewMode(VIEW_8);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_8);
      break;
    case KEY_CTRL9:
      Cp()->ActivePanel->SetViewMode(VIEW_9);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_9);
      break;
    case KEY_CTRL0:
      Cp()->ActivePanel->SetViewMode(VIEW_0);
      Cp()->ChangePanelToFilled(Cp()->ActivePanel,FILE_PANEL);
      Cp()->ActivePanel->SetViewMode(VIEW_0);
      break;
/***/
    default:
      if (!Cp()->ActivePanel->ProcessKey(Key) &&
           !MainKeyBar->ProcessKey(Key))
        CmdLine->ProcessKey(Key);
      break;
  }
  return(TRUE);
}

#endif

/*///
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
*///

#if 0
int ControlObject::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
/*///
  if (!Cp()->ActivePanel->ProcessMouse(MouseEvent))
    if (!GetAnotherPanel(Cp()->ActivePanel)->ProcessMouse(MouseEvent))
      if (!MainKeyBar->ProcessMouse(MouseEvent))
        CmdLine->ProcessMouse(MouseEvent);
*///
  ModalManager.ProcessMouse(MouseEvent);
  return(TRUE);
}

#endif

/*///
Panel* ControlObject::GetAnotherPanel(Panel *Current)
{
  if (Current==Cp()->LeftPanel)
    return(Cp()->RightPanel);
  else
    return(Cp()->LeftPanel);
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

  LeftPosition=(Current==Cp()->LeftPanel);
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
    * /
    NewPanel=CreatePanel(NewType);
    /* SVS $* /

  if (Current==Cp()->ActivePanel)
    Cp()->ActivePanel=NewPanel;
  if (LeftPosition)
  {
    Cp()->LeftPanel=NewPanel;
    LastLeftType=OldType;
  }
  else
  {
    Cp()->RightPanel=NewPanel;
    LastRightType=OldType;
  }
  if (!UseLastPanel)
  {
    if (ChangePosition)
    {
      if (LeftPosition)
      {
        NewPanel->SetPosition(0,Y1,ScrX/2-Opt.WidthDecrement,Y2);
        Cp()->RightPanel->Redraw();
      }
      else
      {
        NewPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Y1,ScrX,Y2);
        Cp()->LeftPanel->Redraw();
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
*///

/* $ 25.11.2000 SVS
   Copyright в 2 строки
*/
/* $ 15.12.2000 SVS
 Метод ShowCopyright - public static & параметр Flags.
*/
void ControlObject::ShowCopyright(DWORD Flags)
{
/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "copyright.inc"
/* tran $ */
  char Str[256];
  char *Line2=NULL;
  strcpy(Str,Copyright);
  char Xor=17;
  for (int I=0;Str[I];I++)
  {
    Str[I]=(Str[I]&0x7f)^Xor;
    Xor^=Str[I];
    if(Str[I] == '\n')
    {
      Line2=&Str[I+1];
      Str[I]='\0';
    }
  }
  if(Flags&1)
  {
    fprintf(stderr,"%s\n%s\n",Str,Line2);
  }
  else
  {
#ifdef BETA
    mprintf("Beta version %d.%02d.%d",BETA/1000,(BETA%1000)/10,BETA%10);
#else
    if(Line2)
    {
      GotoXY(0,ScrY-4);
      Text(Str);
      GotoXY(0,ScrY-3);
      Text(Line2);
    }
    else
      Text(Str);
#endif
  }
}
/* SVS $ */
/* SVS $ */


/* $ 15.07.2000 tran
   + этот метод просто перерисовывает панели, ком.строку и кейбар */
/*///
void ControlObject::Redraw()
{
    if (Cp()->LeftPanel->IsVisible())
        Cp()->LeftPanel->Show();
    if (Cp()->RightPanel->IsVisible())
        Cp()->RightPanel->Show();
    CmdLine->Show();
    MainKeyBar->Redraw();
}
*///
/* tran 15.07.2000 $ */

FilePanels* ControlObject::Cp()
{
    if ( FPanels==0 )
    {
        Message(MSG_WARNING,1,MSG(MError),"CtrlObject::Cp(), FPanels==0",MSG(MOk));
    }
    return FPanels;
}

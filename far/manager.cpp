/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.08 05.05.2001 $ */

/*
Modify:
  05.05.2001 DJ
    + перетрях NWZ
  04.05.2001 OT
    + Неверно формировалось меню плагинов по F11 (NWZ)
      Изменился PluginSet::CommandsMenu()
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  29.12.2000 IS
    + Метод ExitAll - аналог CloseAll, но разрешает продолжение полноценной
      работы в фаре, если пользователь продолжил редактировать файл.
      Возвращает TRUE, если все закрыли и можно выходить из фара.
  28.07.2000 tran 1.04
    + косметика при выводе списка окон -
      измененные файлы в редакторе маркируются "*"
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  28.06.2000 tran
    - NT Console resize
      add class member ActiveModal
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

Manager::Manager()
{
  WindowList=NULL;
  WindowCount=WindowPos=WindowListSize=0;

  *NextName=0;

  CurrentWindow=NULL;
  DestroyedWindow = NULL;
  EndLoop = FALSE;
}

Manager::~Manager()
{
  if (WindowList)
    free(WindowList);
}


/* $ 29.12.2000 IS
  Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
  если пользователь продолжил редактировать файл.
  Возвращает TRUE, если все закрыли и можно выходить из фара.
*/
BOOL Manager::ExitAll()
{
/*  int I, J;
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->ClearDone();
    CurModal->Show();
    CurModal->ProcessKey(KEY_ESC);
    if (!CurModal->Done())
    {
      CurModal->ShowConsoleTitle();
      ModalPos=I;
      NextModal(0);
      return FALSE;
    }
    else
    {
     delete CurModal;
     if(ModalCount>1) for (J=I+1;J<ModalCount;J++) ModalList[J-1]=ModalList[J];
     ModalCount--;
     I--;
    }
  }  */
  return TRUE;
}
/* IS $ */

void Manager::CloseAll()
{
#if 0
  int I;
  EnableSwitch=FALSE;
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->ClearDone();
    // ~ CurModal->SetEnableSwitch(FALSE);
    CurModal->Show();
    CurModal->ProcessKey(KEY_ESC);
    if (!CurModal->Done())
    {
      //CurModal->ShowConsoleTitle();
      //CurModal->Process();
      ActiveModal=CurModal;
      return ; //(FALSE);
    }
    delete CurModal;
  }
  /* $ 13.07.2000 SVS
     Здесь было "delete ModalList;", но перераспределение массива ссылок
     идет через realloc...
  */
  free(ModalList);
  /* SVS $ */
  ModalList=NULL;
  ModalCount=ModalPos=0;
#endif
}

BOOL Manager::IsAnyWindowModified(int Activate)
{
  for (int I=0;I<WindowCount;I++)
    if (WindowList[I]->IsFileModified())
    {
      if (Activate)
      {
        WindowPos=I;
        NextWindow(0);
      }
      return(TRUE);
    }
  return(FALSE);
}


void Manager::AddWindow(Window *NewWindow)
{
  SysLog(1,"Manager::AddWindow(), NewWindow=0x%p, Type=%s",NewWindow,NewWindow->GetTypeName());

  if (WindowListSize < WindowCount+1)
  {
    SysLog("Manager::AddWindow(), realloc list");
    WindowList=(Window **)realloc(WindowList,sizeof(*WindowList)*(WindowCount+1));
    WindowListSize++;
  }
  WindowPos=WindowCount;
  WindowList[WindowCount]=NewWindow;
  WindowCount++;

//  NewModal->Hide();

  NextWindow(0);

  SysLog("Manager::AddWindow(), end.");
  SysLog(-1);
  WaitInMainLoop=IsPanelsActive();
}

void Manager::DestroyWindow(Window *Killed)
{
    int i,j;
    SysLog(1,"Manager::DestroyWindow(), Killed=0x%p, '%s'",Killed,Killed->GetTypeName());
    for ( i=0; i<WindowCount; i++ )
    {
        if ( WindowList[i]==Killed )
        {
            SysLog("Manager::DestroyWindow(), found at i=%i,WindowPos=%i delete and shrink list",i,WindowPos);
            Killed->OnDestroy();
            for ( j=i+1; j<WindowCount; j++ )
                WindowList[j-1]=WindowList[j];
            if ( WindowPos>=i )
                WindowPos--;
            WindowCount--;
            SysLog("Manager::DestroyWindow(), new WindowCount=%i, WindowPos=%i",WindowCount,WindowPos);
            break;
        }
    }
    if ( CurrentWindow==Killed )
    {
        if ( WindowCount )
        {
          SetCurrentWindow (WindowList[WindowPos]);
          SysLog("Manager::DestroyWindow(), Killed==CurrentWindow, set new Current to 0x%p, '%s'",CurrentWindow,
            CurrentWindow->GetTypeName());
        }
        else
        {
            CurrentWindow=0;
            SysLog("Manager::DestroyWindow(), Killed==CurrentWindow, set new Current to 0");
        }
    }
    SysLog("Manager::DestroyWindow() end.");
    SysLog(-1);
    DestroyedWindow = Killed;
}

int Manager::ExecuteModal (Window &ModalWindow)
{
  AddWindow (&ModalWindow);
  ModalWindow.Show();
  DestroyedWindow = NULL;
  while (DestroyedWindow != &ModalWindow)
    ProcessMainLoop();
  int exitCode = ModalWindow.GetExitCode();
  DestroyedWindow = NULL;
  return exitCode;
}

void Manager::NextWindow(int Increment)
{
  SysLog(1,"Manager::NextWindow(), WindowPos=%i, Increment=%i, WindowCount=%i",WindowPos,Increment,WindowCount);
  if (WindowCount>0)
  {
    WindowPos+=Increment;
    if (WindowPos<0)
      WindowPos=WindowCount-1;
  }
  if (WindowPos>=WindowCount)
    WindowPos=0;
  SysLog("Manager::NextWindow(), new WindowPos=%i",WindowPos);
  Window *CurWindow=WindowList[WindowPos];

  if (CurrentWindow)
    CurrentWindow->OnChangeFocus(0);

  SetCurrentWindow (CurWindow);

  SysLog("Manager::NextWindow(), set CurrentWindow=0x%p, %s",CurrentWindow,CurrentWindow->GetTypeName());

  if (CurWindow->GetType()==MODALTYPE_EDITOR)
    UpdateRequired=TRUE;

/*    int ExitCode=CurModal->GetExitCode();
    if (ExitCode<=0)
    {
      delete CurModal;
      for (int I=ModalPos+1;I<ModalCount;I++)
        ModalList[I-1]=ModalList[I];
      ModalCount--;
      if (*NextName)
      {
        ActivateNextWindow();
        return;
      }
    }
    else
    {
      if (ExitCode==2)
        SelectModal();
      else
        if (ExitCode==3)
        {
          if (--ModalPos<0)
            ModalPos=ModalCount;
        }
        else
          ModalPos++;
      CurModal->Hide();
    }
*/
//  } // while
/*  CtrlObject->Cp()->ActivePanel->SetTitle();
  if (UpdateRequired)
  {
    CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    UpdateRequired=TRUE;
  }
  CtrlObject->Cp()->LeftPanel->Redraw();
  CtrlObject->Cp()->RightPanel->Redraw();
  CtrlObject->CmdLine.Redraw();
  CtrlObject->MainKeyBar.Redraw();*/
  SysLog(-1);
}


void Manager::SetCurrentWindow (Window *NewCurWindow)
{
  CurrentWindow = NewCurWindow;
  CurrentWindow->ShowConsoleTitle();
  CurrentWindow->OnChangeFocus(1);
  CtrlObject->Macro.SetMode(CurrentWindow->MacroMode);
}


void Manager::SelectWindow()
{
  int ExitCode;
  {
    struct MenuItem ModalMenuItem;
    memset(&ModalMenuItem,0,sizeof(ModalMenuItem));
    VMenu ModalMenu(MSG(MScreensTitle),NULL,0,ScrY-4);
    ModalMenu.SetHelp("ScrSwitch");
    ModalMenu.SetFlags(MENU_WRAPMODE);
    ModalMenu.SetPosition(-1,-1,0,0);

//    sprintf(ModalMenuItem.Name,"&0. %-30s",MSG(MScreensPanels));
//    ModalMenuItem.Selected=(ModalPos==ModalCount);
//    ModalMenu.AddItem(&ModalMenuItem);

    for (int I=0;I<WindowCount;I++)
    {
      char Type[200],Name[NM],NumText[100];
      WindowList[I]->GetTypeAndName(Type,Name);
      if (I<10)
        sprintf(NumText,"&%d. ",I);
      else
        strcpy(NumText,"&   ");
      /* $ 28.07.2000 tran
         файл усекает по ширине экрана */
      TruncPathStr(Name,ScrX-40);
      /*  добавляется "*" если файл изменен */
      sprintf(ModalMenuItem.Name,"%s%-20s %c %s",NumText,Type,(WindowList[I]->IsFileModified()?'*':' '),Name);
      /* tran 28.07.2000 $ */
      ModalMenuItem.Selected=(I==WindowPos);
      ModalMenu.AddItem(&ModalMenuItem);
    }
    ModalMenu.Process();
    ExitCode=ModalMenu.GetExitCode();
  }
  if (ExitCode>=0)
  {
    NextWindow(ExitCode-WindowPos);
  }
}


void Manager::GetWindowTypesCount(int &Viewers,int &Editors)
{
  Viewers=Editors=0;
  for (int I=0;I<WindowCount;I++)
  {
    switch(WindowList[I]->GetType())
    {
      case MODALTYPE_VIEWER:
        Viewers++;
        break;
      case MODALTYPE_EDITOR:
        Editors++;
        break;
    }
  }
}

int  Manager::GetWindowCountByType(int Type)
{
  int ret=0;
  for (int I=0;I<WindowCount;I++)
  {
    if (WindowList[I]->GetType()==Type)
      ret++;
  }
  return ret;
}

void Manager::SetWindowPos(int NewPos)
{
  SysLog("Manager::SetWindowPos(), NewPos=%i",NewPos);
  WindowPos=NewPos;
}

int  Manager::FindWindowByFile(int ModalType,char *FileName)
{
  for (int I=0;I<WindowCount;I++)
  {
    char Type[200],Name[NM];
    if (WindowList[I]->GetTypeAndName(Type,Name)==ModalType)
      if (LocalStricmp(Name,FileName)==0)
        return(I);
  }
  return(-1);
}


void Manager::ShowBackground()
{
  if (!RegVer)
  {
    Message(MSG_WARNING,1,MSG(MWarning),MSG(MRegOnly),MSG(MOk));
    return;
  }

  SaveScreen SaveScr;
/*  int I;
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->Hide();
  }
  {
    RedrawDesktop Redraw;
    CtrlObject->Cp()->CmdLine.Hide();
    SetCursorType(FALSE,10);
    WaitKey();
    CtrlObject->Cp()->CmdLine.Show();
  }
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->SavePrevScreen();
  } */
}

void Manager::SetNextWindow(int Viewer,char *Name,long Pos)
{
  NextViewer=Viewer;
  strcpy(NextName,Name);
  NextPos=Pos;
}


void Manager::ActivateNextWindow()
{
 // int es;
  if (*NextName)
  {
    Window *NewWindow;
    char NewName[NM];
    strcpy(NewName,NextName);
    *NextName=0;
    if (NextViewer)
      NewWindow=new FileViewer(NewName,TRUE,FALSE,FALSE,NextPos);
    else
      NewWindow=new FileEditor(NewName,FALSE,TRUE,-2,NextPos,FALSE);
    AddWindow(NewWindow);
  }
}

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  while (!EndLoop)
  {
    ProcessMainLoop();
    if (DestroyedWindow)
    {
      delete DestroyedWindow;
      DestroyedWindow = NULL;
    }
  }
}


void Manager::ProcessMainLoop()
{
  WaitInMainLoop=IsPanelsActive();

  WaitInFastFind++;
  int Key=GetInputRecord(&LastInputRecord);
  WaitInFastFind--;
  WaitInMainLoop=FALSE;
  if (EndLoop)
    return;
///    MainKeyBar->RedrawIfChanged();
  if (LastInputRecord.EventType==MOUSE_EVENT)
    ProcessMouse(&LastInputRecord.Event.MouseEvent);
  else
    ProcessKey(Key);
///    MainKeyBar->RedrawIfChanged();
}

void Manager::ExitMainLoop(int Ask)
{
  if (!Ask || !Opt.Confirm.Exit || Message(0,2,MSG(MQuit),MSG(MAskQuit),MSG(MYes),MSG(MNo))==0)
   /* $ 29.12.2000 IS
      + Проверяем, сохранены ли все измененные файлы. Если нет, то не выходим
        из фара.
   */
   if(ExitAll())
   /* IS $ */
    if (!CtrlObject->Cp()->LeftPanel->ProcessPluginEvent(FE_CLOSE,NULL) && !CtrlObject->Cp()->RightPanel->ProcessPluginEvent(FE_CLOSE,NULL))
      EndLoop=TRUE;
}

int  Manager::ProcessKey(int Key)
{
    int ret=FALSE;
    BOOL es;
    char kn[32];
    KeyToText(Key,kn);
    SysLog(1,"Manager::ProcessKey(), key=%i, '%s'",Key,kn);

    if ( CurrentWindow)
    {
      es=CurrentWindow->GetEnableSwitch();
      SysLog("Manager::ProcessKey(), es=%i, to CurrentWindow 0x%p, '%s'",es,CurrentWindow, CurrentWindow->GetTypeName());;
      switch(Key)
        {
            case KEY_F11:
                PluginsMenu();
                SysLog(-1);
                return TRUE;
            case KEY_F12:
                if ( es )
                    SelectWindow();
                SysLog(-1);
                return TRUE;
            case KEY_CTRLTAB:
                if ( es )
                    NextWindow(1);
                SysLog(-1);
                return TRUE;
            case KEY_CTRLSHIFTTAB:
                if ( es )
                    NextWindow(-1);
                SysLog(-1);
                return TRUE;
        }
        SysLog("Manager::ProcessKey(), to CurrentWindow 0x%p, '%s'",CurrentWindow, CurrentWindow->GetTypeName());;
        CurrentWindow->UpdateKeyBar();
        // сохраняем, потому что внутри ProcessKey
        // может быть вызван AddModal и
        // CurrentModal будет изменен.
        Window *cw=CurrentWindow;
        ret=CurrentWindow->ProcessKey(Key);
        if ( ret )
        {
            // а так проверяем код выхода у того, кого надо
            if ( cw->GetExitCode()==XC_QUIT )
                DestroyWindow(cw);
        }
    }
    SysLog("Manager::ProcessKey() ret=%i",ret);
    SysLog(-1);
    return ret;
}

int  Manager::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
    int ret=FALSE;
//    SysLog(1,"Manager::ProcessMouse()");
    if ( CurrentWindow)
        ret=CurrentWindow->ProcessMouse(MouseEvent);
//    SysLog("Manager::ProcessMouse() ret=%i",ret);
    SysLog(-1);
    return ret;
}

void Manager::PluginsMenu()
{
  SysLog(1);
 // ╧юьхэ ыё  т√чют ъюььрэфё ╨рчюсЁрЄ№ё 
///    CtrlObject->Plugins.CommandsMenu(CurrentModal->GetTypeAndName(0,0),0,0);
  int curType = CurrentWindow->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
    CtrlObject->Plugins.CommandsMenu(curType,0,0);
  SysLog(-1);
}

BOOL Manager::IsPanelsActive()
{
    if (CurrentWindow->GetTypeAndName(0,0)==MODALTYPE_PANELS )
        return TRUE;
    return FALSE;
}

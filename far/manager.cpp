/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.10 06.05.2001 $ */

/*
Modify:
  07.05.2001 ОТ
    - Баг с порядком индекса текущего фрейма FramePos при удалении 
      какого-нибудь из списка :)
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
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
  FrameList=NULL;
  FrameCount=FramePos=FrameListSize=0;

  *NextName=0;

  CurrentFrame=NULL;
  DestroyedFrame = NULL;
  EndLoop = FALSE;

}

Manager::~Manager()
{
  if (FrameList)
    free(FrameList);
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

BOOL Manager::IsAnyFrameModified(int Activate)
{
  for (int I=0;I<FrameCount;I++)
    if (FrameList[I]->IsFileModified())
    {
      if (Activate)
      {
        FramePos=I;
        NextFrame(0);
      }
      return(TRUE);
    }
  return(FALSE);
}


void Manager::AddFrame(Frame *NewFrame)
{
  SysLog(1,"Manager::AddFrame(), NewFrame=0x%p, Type=%s",NewFrame,NewFrame->GetTypeName());

  if (FrameListSize < FrameCount+1)
  {
    SysLog("Manager::AddFrame(), realloc list");
    FrameList=(Frame **)realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
    FrameListSize++;
  }
  FramePos=FrameCount;
  FrameList[FrameCount]=NewFrame;
  FrameCount++;

//  NewModal->Hide();

  NextFrame(0);

  SysLog("Manager::AddFrame(), end.");
  SysLog(-1);
  WaitInMainLoop=IsPanelsActive();
}

void Manager::DestroyFrame(Frame *Killed)
{
    int i,j;
    SysLog(1,"Manager::DestroyFrame(), Killed=0x%p, '%s'",Killed,Killed->GetTypeName());
    for ( i=0; i<FrameCount; i++ )
    {
        if ( FrameList[i]==Killed )
        {
            SysLog("Manager::DestroyFrame(), found at i=%i,FramePos=%i delete and shrink list",i,FramePos);
            Killed->OnDestroy();
            for ( j=i+1; j<FrameCount; j++ )
                FrameList[j-1]=FrameList[j];
            FrameCount--;
            if ( FramePos>=FrameCount )
                FramePos=0;
            SysLog("Manager::DestroyFrame(), new FrameCount=%i, FramePos=%i",FrameCount,FramePos);
            break;
        }
    }
    if ( CurrentFrame==Killed )
    {
        if ( FrameCount )
        {
          SetCurrentFrame (FrameList[FramePos]);
          SysLog("Manager::DestroyFrame(), Killed==CurrentFrame, set new Current to 0x%p, '%s'",CurrentFrame,
            CurrentFrame->GetTypeName());
        }
        else
        {
            CurrentFrame=0;
            SysLog("Manager::DestroyFrame(), Killed==CurrentFrame, set new Current to 0");
        }
    }
    SysLog("Manager::DestroyFrame() end.");
    SysLog(-1);
    DestroyedFrame = Killed;
}

int Manager::ExecuteModal (Frame &ModalFrame)
{
  AddFrame (&ModalFrame);
  ModalFrame.Show();
  DestroyedFrame = NULL;
  while (DestroyedFrame != &ModalFrame)
    ProcessMainLoop();
  int exitCode = ModalFrame.GetExitCode();
  DestroyedFrame = NULL;
  return exitCode;
}

void Manager::NextFrame(int Increment)
{
  SysLog(1,"Manager::NextFrame(), FramePos=%i, Increment=%i, FrameCount=%i",FramePos,Increment,FrameCount);
  if (FrameCount>0)
  {
    FramePos+=Increment;
    if (FramePos<0)
      FramePos=FrameCount-1;
  }
  if (FramePos>=FrameCount)
    FramePos=0;
  SysLog("Manager::NextFrame(), new FramePos=%i",FramePos);
  Frame *CurFrame=FrameList[FramePos];

  if (CurrentFrame)
    CurrentFrame->OnChangeFocus(0);

  SetCurrentFrame (CurFrame);

  SysLog("Manager::NextFrame(), set CurrentFrame=0x%p, %s",CurrentFrame,CurrentFrame->GetTypeName());

  if (CurFrame->GetType()==MODALTYPE_EDITOR)
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
        ActivateNextFrame();
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


void Manager::SetCurrentFrame (Frame *NewCurFrame)
{
  CurrentFrame = NewCurFrame;
  CurrentFrame->ShowConsoleTitle();
  CurrentFrame->OnChangeFocus(1);
  CtrlObject->Macro.SetMode(CurrentFrame->MacroMode);
}


void Manager::SelectFrame()
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

    for (int I=0;I<FrameCount;I++)
    {
      char Type[200],Name[NM],NumText[100];
      FrameList[I]->GetTypeAndName(Type,Name);
      if (I<10)
        sprintf(NumText,"&%d. ",I);
      else
        strcpy(NumText,"&   ");
      /* $ 28.07.2000 tran
         файл усекает по ширине экрана */
      TruncPathStr(Name,ScrX-40);
      /*  добавляется "*" если файл изменен */
      sprintf(ModalMenuItem.Name,"%s%-20s %c %s",NumText,Type,(FrameList[I]->IsFileModified()?'*':' '),Name);
      /* tran 28.07.2000 $ */
      ModalMenuItem.Selected=(I==FramePos);
      ModalMenu.AddItem(&ModalMenuItem);
    }
    ModalMenu.Process();
    ExitCode=ModalMenu.GetExitCode();
  }
  if (ExitCode>=0)
  {
    NextFrame(ExitCode-FramePos);
  }
}


void Manager::GetFrameTypesCount(int &Viewers,int &Editors)
{
  Viewers=Editors=0;
  for (int I=0;I<FrameCount;I++)
  {
    switch(FrameList[I]->GetType())
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

int  Manager::GetFrameCountByType(int Type)
{
  int ret=0;
  for (int I=0;I<FrameCount;I++)
  {
    if (FrameList[I]->GetType()==Type)
      ret++;
  }
  return ret;
}

void Manager::SetFramePos(int NewPos)
{
  SysLog("Manager::SetFramePos(), NewPos=%i",NewPos);
  FramePos=NewPos;
}

int  Manager::FindFrameByFile(int ModalType,char *FileName)
{
  for (int I=0;I<FrameCount;I++)
  {
    char Type[200],Name[NM];
    if (FrameList[I]->GetTypeAndName(Type,Name)==ModalType)
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

void Manager::SetNextFrame(int Viewer,char *Name,long Pos)
{
  NextViewer=Viewer;
  strcpy(NextName,Name);
  NextPos=Pos;
}


void Manager::ActivateNextFrame()
{
 // int es;
  if (*NextName)
  {
    Frame *NewFrame;
    char NewName[NM];
    strcpy(NewName,NextName);
    *NextName=0;
    if (NextViewer)
      NewFrame=new FileViewer(NewName,TRUE,FALSE,FALSE,NextPos);
    else
      NewFrame=new FileEditor(NewName,FALSE,TRUE,-2,NextPos,FALSE);
    AddFrame(NewFrame);
  }
}

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  while (!EndLoop)
  {
    ProcessMainLoop();
    if (DestroyedFrame)
    {
      delete DestroyedFrame;
      DestroyedFrame = NULL;
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

    if ( CurrentFrame)
    {
      es=CurrentFrame->GetEnableSwitch();
      SysLog("Manager::ProcessKey(), es=%i, to CurrentFrame 0x%p, '%s'",es,CurrentFrame, CurrentFrame->GetTypeName());;
      switch(Key)
        {
            case KEY_F11:
                PluginsMenu();
                SysLog(-1);
                return TRUE;
            case KEY_F12:
                if ( es )
                    SelectFrame();
                SysLog(-1);
                return TRUE;
            case KEY_CTRLTAB:
                if ( es )
                    NextFrame(1);
                SysLog(-1);
                return TRUE;
            case KEY_CTRLSHIFTTAB:
                if ( es )
                    NextFrame(-1);
                SysLog(-1);
                return TRUE;
        }
        SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName());;
        CurrentFrame->UpdateKeyBar();
        // сохраняем, потому что внутри ProcessKey
        // может быть вызван AddModal и
        // CurrentModal будет изменен.
        Frame *cw=CurrentFrame;
        ret=CurrentFrame->ProcessKey(Key);
        if ( ret )
        {
            // а так проверяем код выхода у того, кого надо
            if ( cw->GetExitCode()==XC_QUIT )
                DestroyFrame(cw);
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
    if ( CurrentFrame)
        ret=CurrentFrame->ProcessMouse(MouseEvent);
//    SysLog("Manager::ProcessMouse() ret=%i",ret);
    SysLog(-1);
    return ret;
}

void Manager::PluginsMenu()
{
  SysLog(1);
 // Поменялся вызов коммандс Разобраться
///    CtrlObject->Plugins.CommandsMenu(CurrentModal->GetTypeAndName(0,0),0,0);
  int curType = CurrentFrame->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
    CtrlObject->Plugins.CommandsMenu(curType,0,0);
  SysLog(-1);
}

BOOL Manager::IsPanelsActive()
{
    if (CurrentFrame->GetTypeAndName(0,0)==MODALTYPE_PANELS )
        return TRUE;
    return FALSE;
}

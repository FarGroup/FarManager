/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.13 07.05.2001 $ */

/*
Modify:
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
    ! приведены в порядок CloseAll() и ExitAll()
  06.05.2001 DJ
    ! перетрях #include
    + ReplaceCurrentFrame()
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

#include "manager.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "frame.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"

Manager::Manager()
{
  FrameList=NULL;
  FrameCount=FramePos=FrameListSize=0;

  CurrentFrame=NULL;
  DestroyedFrame = NULL;
  FrameToReplace = NULL;
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
  // FrameList [0] - это панели, и им KEY_ESC посылать не нужно и бесполезно
  for (int I=FrameCount-1; I>=1; I--)
  {
    Frame *CurFrame=FrameList[I];
    CurFrame->Show();
    CurFrame->ProcessKey(KEY_ESC);
    if (CurFrame->GetExitCode() != XC_QUIT)
    {
      FramePos = I;
      SetCurrentFrame (CurFrame);
      return FALSE;
    }
    else
    {
     delete CurFrame;
     FrameCount--;
    }
  }
  return TRUE;
}
/* IS $ */

void Manager::CloseAll()
{
  for (int I=0;I<FrameCount;I++)
  {
    Frame *CurFrame=FrameList[I];
    CurFrame->SetCanLoseFocus(FALSE);
    CurFrame->Show();
    CurFrame->ProcessKey(KEY_ESC);
    delete CurFrame;
  }
  /* $ 13.07.2000 SVS
     Здесь было "delete ModalList;", но перераспределение массива ссылок
     идет через realloc...
  */
  free(FrameList);
  /* SVS $ */
  FrameList=NULL;
  FrameCount=FramePos=0;
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
  _D(SysLog(1,"Manager::AddFrame(), NewFrame=0x%p, Type=%s",NewFrame,NewFrame->GetTypeName()));

  if (FrameListSize < FrameCount+1)
  {
    _D(SysLog("Manager::AddFrame(), realloc list"));
    FrameList=(Frame **)realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
    FrameListSize++;
  }
  FramePos=FrameCount;
  FrameList[FrameCount]=NewFrame;
  FrameCount++;

//  NewModal->Hide();

  NextFrame(0);

  _D(SysLog("Manager::AddFrame(), end."));
  _D(SysLog(-1));
  WaitInMainLoop=IsPanelsActive();
}

void Manager::DestroyFrame(Frame *Killed)
{
    int i,j;
    _D(SysLog(1,"Manager::DestroyFrame(), Killed=0x%p, '%s'",Killed,Killed->GetTypeName()));
    for ( i=0; i<FrameCount; i++ )
    {
        if ( FrameList[i]==Killed )
        {
            _D(SysLog("Manager::DestroyFrame(), found at i=%i,FramePos=%i delete and shrink list",i,FramePos));
            Killed->OnDestroy();
            for ( j=i+1; j<FrameCount; j++ )
                FrameList[j-1]=FrameList[j];
            FrameCount--;
            if ( FramePos>=FrameCount )
                FramePos=0;
            _D(SysLog("Manager::DestroyFrame(), new FrameCount=%i, FramePos=%i",FrameCount,FramePos));
            break;
        }
    }
    if ( CurrentFrame==Killed )
    {
        if ( FrameCount )
        {
          SetCurrentFrame (FrameList[FramePos]);
          _D(SysLog("Manager::DestroyFrame(), Killed==CurrentFrame, set new Current to 0x%p, '%s'",CurrentFrame,CurrentFrame->GetTypeName()));
        }
        else
        {
            CurrentFrame=0;
            _D(SysLog("Manager::DestroyFrame(), Killed==CurrentFrame, set new Current to 0"));
        }
    }
    _D(SysLog("Manager::DestroyFrame() end."));
    _D(SysLog(-1));
    DestroyedFrame = Killed;
}

/* $ 06.05.2001 DJ
   заменить текущий фрейм на указанный (используется для обработки F6)
*/

void Manager::ReplaceCurrentFrame (Frame *NewFrame)
{
  DestroyedFrame = FrameList [FramePos];
  DestroyedFrame->OnChangeFocus (0);
  FrameToReplace = NewFrame;
}

/* DJ $ */

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
  _D(SysLog(1,"Manager::NextFrame(), FramePos=%i, Increment=%i, FrameCount=%i",FramePos,Increment,FrameCount));
  if (FrameCount>0)
  {
    FramePos+=Increment;
    if (FramePos<0)
      FramePos=FrameCount-1;
  }
  if (FramePos>=FrameCount)
    FramePos=0;
  _D(SysLog("Manager::NextFrame(), new FramePos=%i",FramePos));
  Frame *CurFrame=FrameList[FramePos];

  if (CurrentFrame)
    CurrentFrame->OnChangeFocus(0);

  SetCurrentFrame (CurFrame);

  _D(SysLog("Manager::NextFrame(), set CurrentFrame=0x%p, %s",CurrentFrame,CurrentFrame->GetTypeName()));

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
  _D(SysLog(-1));
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
  _D(SysLog("Manager::SetFramePos(), NewPos=%i",NewPos));
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

/* $ 06.05.2001 DJ
   активирует фрейм с указанным номером
*/

void Manager::ActivateFrameByPos (int NewPos)
{
  SetFramePos(FramePos);
  NextFrame(0);
}
/* DJ $ */


void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  while (!EndLoop)
  {
    ProcessMainLoop();
    if (DestroyedFrame)
    {
      DestroyedFrame->OnDestroy();
      delete DestroyedFrame;
      DestroyedFrame = NULL;
    }
    if (FrameToReplace)
    {
      FrameList [FramePos] = FrameToReplace;
      SetCurrentFrame (FrameToReplace);
      FrameToReplace = NULL;
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
    char kn[32];
    KeyToText(Key,kn);
    _D(SysLog(1,"Manager::ProcessKey(), key=%i, '%s'",Key,kn));

    if ( CurrentFrame)
    {
      _D(SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName()));
      switch(Key)
        {
            case KEY_F11:
                PluginsMenu();
                _D(SysLog(-1));
                return TRUE;
            case KEY_F12:
                if (CurrentFrame->GetCanLoseFocus())
                    SelectFrame();
                _D(SysLog(-1));
                return TRUE;
            case KEY_CTRLTAB:
                if (CurrentFrame->GetCanLoseFocus())
                    NextFrame(1);
                _D(SysLog(-1));
                return TRUE;
            case KEY_CTRLSHIFTTAB:
                if (CurrentFrame->GetCanLoseFocus())
                    NextFrame(-1);
                _D(SysLog(-1));
                return TRUE;
        }
        _D(SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName()));
        CurrentFrame->UpdateKeyBar();
        // сохраняем, потому что внутри ProcessKey
        // может быть вызван AddModal и
        // CurrentModal будет изменен.
        Frame *cw=CurrentFrame;
        ret=CurrentFrame->ProcessKey(Key);
        if ( ret )
        {
          // а так проверяем код выхода у того, кого надо
          if (cw->GetExitCode()==XC_QUIT && !FrameToReplace)
            DestroyFrame(cw);
        }
    }
    _D(SysLog("Manager::ProcessKey() ret=%i",ret));
    _D(SysLog(-1));
    return ret;
}

int  Manager::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
    int ret=FALSE;
//    _D(SysLog(1,"Manager::ProcessMouse()"));
    if ( CurrentFrame)
        ret=CurrentFrame->ProcessMouse(MouseEvent);
//    _D(SysLog("Manager::ProcessMouse() ret=%i",ret));
    _D(SysLog(-1));
    return ret;
}

void Manager::PluginsMenu()
{
  _D(SysLog(1));
 // Поменялся вызов коммандс Разобраться
///    CtrlObject->Plugins.CommandsMenu(CurrentModal->GetTypeAndName(0,0),0,0);
  int curType = CurrentFrame->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
    CtrlObject->Plugins.CommandsMenu(curType,0,0);
  _D(SysLog(-1));
}

BOOL Manager::IsPanelsActive()
{
    if (CurrentFrame->GetTypeAndName(0,0)==MODALTYPE_PANELS )
        return TRUE;
    return FALSE;
}

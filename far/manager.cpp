/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.17 12.05.2001 $ */

/*
Modify:
  12.05.2001 DJ
    ! Убран ModalFrame.Show() в Manager::ExecuteModal()
  12.05.2001 DJ
    ! FrameManager оторван от CtrlObject
    ! объединены ExecuteModal() и ExecuteModalPtr() (и о чем я думал, когда
      делал две функции?)
    ! ReplaceCurrentFrame() заменена на более универсальную ReplaceFrame()
      (с подачи ОТ)
  11.05.2001 OT
    ! Отрисовка Background
  10.05.2001 DJ
    + SwitchToPanels()
    * GetFrameTypesCount() не учитывает фрейм, который мы собрались удалять
    + ModalStack
    - всякие перетряхи логики DestroyFrame() и иже с ними
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
#include "cmdline.hpp"
#include "ctrlobj.hpp"

Manager *FrameManager;

Manager::Manager()
{
  FrameList=NULL;
  FrameCount=FramePos=FrameListSize=0;
  ModalStack=NULL;
  ModalStackCount = ModalStackSize = 0;

  CurrentFrame=NULL;
  DestroyedFrame = NULL;
  FrameToReplace = NULL;
  EndLoop = FALSE;

}

Manager::~Manager()
{
  if (FrameList)
    free(FrameList);
  if (ModalStack)
    free(ModalStack);
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
  _D(SysLog(1,"Manager::DestroyFrame(), Killed=0x%p, '%s'",Killed,Killed->GetTypeName()));
  if (!FrameToReplace)
    for (int i=0; i<FrameCount; i++ )
    {
        if ( FrameList[i]==Killed )
        {
            _D(SysLog("Manager::DestroyFrame(), found at i=%i,FramePos=%i delete and shrink list",i,FramePos));
            for (int j=i+1; j<FrameCount; j++ )
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
    if (FrameToReplace)
    {
      FrameList [FramePos] = FrameToReplace;
      SetCurrentFrame (FrameToReplace);
    }
    else if (ModalStackCount > 0)
      SetCurrentFrame (ModalStack [--ModalStackCount]);
    else
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
  }
  _D(SysLog("Manager::DestroyFrame() end."));
  _D(SysLog(-1));
  DestroyedFrame = Killed;
}

void Manager::DeleteDestroyedFrame()
{
  DestroyedFrame->OnDestroy();
  delete DestroyedFrame;
  DestroyedFrame = NULL;
}

/* $ 06.05.2001 DJ
   заменить текущий фрейм на указанный (используется для обработки F6)
*/

/* $ 12.05.2001 DJ
   заменена на более общую функцию (с подачи ОТ)
*/

void Manager::ReplaceFrame (Frame *OldFrame, Frame *NewFrame)
{
  DestroyedFrame = OldFrame;
  DestroyedFrame->OnChangeFocus (0);
  if (OldFrame == CurrentFrame)
    FrameToReplace = NewFrame;
  else
    for (int i=0; i<FrameCount; i++)
      if (FrameList [i] == OldFrame)
        FrameList [i] = NewFrame;
}

/* DJ $ */
/* DJ $ */


/* $ 10.05.2001 DJ
   поддерживает ReplaceFrame (F6)
*/

int Manager::ExecuteModal (Frame &ModalFrame)
{
  ModalSaveState();
  AddFrame (&ModalFrame);
  DestroyedFrame = NULL;
  Frame *CurModalFrame = &ModalFrame;
  while (DestroyedFrame != CurModalFrame)
  {
    ProcessMainLoop();
    if (FrameToReplace)
    {
      CurModalFrame = FrameToReplace;
      if (DestroyedFrame == &ModalFrame)
        DestroyedFrame = NULL;
      else
        DeleteDestroyedFrame();
      continue;
    }
  }
  DestroyedFrame = NULL;
  return CurModalFrame->GetExitCode();
}

/* DJ */

void Manager::ModalSaveState()
{
  if (ModalStackCount == ModalStackSize)
    ModalStack = (Frame **) realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
  ModalStack [ModalStackCount++] = CurrentFrame;
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
  _D(SysLog(-1));
}


void Manager::SetCurrentFrame (Frame *NewCurFrame)
{
  CurrentFrame = NewCurFrame;
  CurrentFrame->ShowConsoleTitle();
  CurrentFrame->OnChangeFocus(1);
  CtrlObject->Macro.SetMode(CurrentFrame->GetMacroMode());
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
    /* $ 10.05.2001 DJ
       не учитываем фрейм, который собираемся удалять
    */
    if (FrameList[I] == DestroyedFrame || FrameList [I]->GetExitCode() == XC_QUIT)
      continue;
    /* DJ $ */
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
  CtrlObject->CmdLine->ShowBackground();
}

/* $ 06.05.2001 DJ
   активирует фрейм с указанным номером
*/
void Manager::ActivateFrameByPos (int NewPos)
{
  SetFramePos(NewPos);
  NextFrame(0);
}
/* DJ $ */

/* $ 10.05.2001 DJ
   переключается на панели (фрейм с номером 0)
*/

void Manager::SwitchToPanels()
{
  ActivateFrameByPos (0);
}

/* DJ $ */

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  while (!EndLoop)
  {
    ProcessMainLoop();
    if (DestroyedFrame)
      DeleteDestroyedFrame();
  }
}


void Manager::ProcessMainLoop()
{
  FrameToReplace = NULL;
  WaitInMainLoop=IsPanelsActive();

  WaitInFastFind++;
  int Key=GetInputRecord(&LastInputRecord);
  WaitInFastFind--;
  WaitInMainLoop=FALSE;
  if (EndLoop)
    return;
  if (LastInputRecord.EventType==MOUSE_EVENT)
    ProcessMouse(&LastInputRecord.Event.MouseEvent);
  else
    ProcessKey(Key);
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
    _D(char kn[32]);
    _D(KeyToText(Key,kn));
//    _D(SysLog(1,"Manager::ProcessKey(), key=%i, '%s'",Key,kn));

    if ( CurrentFrame)
    {
//      _D(SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName()));
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
//        _D(SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName()));
        CurrentFrame->UpdateKeyBar();
        // сохраняем, потому что внутри ProcessKey
        // может быть вызван AddModal и
        // CurrentModal будет изменен.
        Frame *cw=CurrentFrame;
        ret=CurrentFrame->ProcessKey(Key);
        if ( ret )
        {
          // а так проверяем код выхода у того, кого надо
          if (cw->GetExitCode()==XC_QUIT)
            DestroyFrame(cw);
        }
    }
//    _D(SysLog("Manager::ProcessKey() ret=%i",ret));
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
  int curType = CurrentFrame->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
    CtrlObject->Plugins.CommandsMenu(curType,0,0);
  _D(SysLog(-1));
}

BOOL Manager::IsPanelsActive()
{
  return CurrentFrame->GetType() == MODALTYPE_PANELS;
}

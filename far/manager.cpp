/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.20 16.05.2001 $ */

/*
Modify:
  16.05.2001 DJ
    ! возвращение ExecuteModal()
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    - Борьба с F4 -> ReloadAgain
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
  ModalStackSize = ModalStackCount = 0;
  EndLoop = FALSE;

  CurrentFrame  = NULL;
  InsertedFrame = NULL;
  DeletedFrame  = NULL;
  ActivatedFrame= NULL;
  DeactivatedFrame=NULL;
  ModalizedFrame=NULL;

  DisableDelete = FALSE;
}

Manager::~Manager()
{
  if (FrameList)
    free(FrameList);
  if (ModalStack)
    free (ModalStack);
}


/* $ 29.12.2000 IS
  Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
  если пользователь продолжил редактировать файл.
  Возвращает TRUE, если все закрыли и можно выходить из фара.
*/
BOOL Manager::ExitAll()
{
/*
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
*/
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
        ActivateFrame(I);
        Commit();
      }
      return(TRUE);
    }

  return(FALSE);
}

void Manager::InsertFrame(Frame *Inserted, int Index)
{
  if (Index==-1)
    Index=FramePos;
  InsertedFrame=Inserted;
}
/*
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
*/


void Manager::DeleteFrame(Frame *Deleted)
{
  if (!Deleted){
    DeletedFrame=CurrentFrame;
  } else {
    DeletedFrame=Deleted;
  }
}

void Manager::DeleteFrame(int Index)
{
  DeleteFrame(this->operator[](Index));
}

/*
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
  DeletedFrame = Killed;
}
*/

/*
void Manager::DeleteDeletedFrame()
{
  DeletedFrame->OnDestroy();
  delete DeletedFrame;
  DeletedFrame = NULL;
}
*/

/* $ 06.05.2001 DJ
   заменить текущий фрейм на указанный (используется для обработки F6)
*/

/* $ 12.05.2001 DJ
   заменена на более общую функцию (с подачи ОТ)
*/

void Manager::ReplaceFrame (Frame *Inserted, Frame *Deleted)
{
/*
  if (!OldFrame){
    OldFrame=CurrentFrame;
  }
  DeletedFrame = OldFrame;
  DeletedFrame->OnChangeFocus (0);
  if (OldFrame == CurrentFrame){
    FrameToReplace = NewFrame;
  } else {
    for (int i=0; i<FrameCount; i++){
      if (FrameList [i] == OldFrame){
        FrameToReplace = FrameList [i] = NewFrame;
        break;
      }
    }
  }
*/
}

/* DJ $ */
/* DJ $ */

void Manager::ReplaceFrame (Frame *NewFrame, int FramePos)
{
  if (FramePos<0||FramePos>=FrameCount){
    return;
  }
  ReplaceFrame(NewFrame,this->FrameList[FramePos]);
}


/* $ 10.05.2001 DJ
   поддерживает ReplaceFrame (F6)
*/

void Manager::ModalizeFrame (Frame *Modalized, int Mode)
{
  ModalizedFrame=Modalized;
}

int Manager::ExecuteModal (Frame &ModalFrame)
{
  _DJ(SysLog ("Executing modal %08x", &ModalFrame));
  ModalSaveState();
  InsertFrame (&ModalFrame);
  Commit();
  DeletedFrame = NULL;
  Frame *CurModalFrame = &ModalFrame;
  while (1)
  {
    ProcessMainLoop();
    if (DeletedFrame)
    {
      if (InsertedFrame)
        _DJ (SysLog ("Replaced %08x with %08x", DeletedFrame, InsertedFrame));
      else
        _DJ (SysLog ("Deleted %08x", DeletedFrame));
    }
    if (DeletedFrame == &ModalFrame)
      DisableDelete = TRUE;
    if (DeletedFrame && InsertedFrame)
      CurModalFrame = InsertedFrame;
    if (DeletedFrame == CurModalFrame && !InsertedFrame)
      break;
    Commit();
  }
  ActivatedFrame = ModalStack [--ModalStackCount];
  _DJ (SysLog ("Activating %08x", ActivatedFrame));
  Commit();
  return ModalFrame.GetExitCode();
}

/* DJ $ */

void Manager::ModalSaveState()
{
  if (ModalStackCount == ModalStackSize)
    ModalStack = (Frame **) realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
  _DJ(SysLog ("ModalSaveState: CurrentFrame=%08x", CurrentFrame));
  ModalStack [ModalStackCount++] = CurrentFrame;
}

/*
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
*/


/*
void Manager::SetCurrentFrame (int FrameIndex)
{
  CurrentFrame=this->operator[](FrameIndex);
  if (!CurrentFrame){
    FramePos=FrameIndex;
    CurrentFrame->ShowConsoleTitle();
    CurrentFrame->OnChangeFocus(1);
    CtrlObject->Macro.SetMode(CurrentFrame->GetMacroMode());
  } else {
    SetCurrentFrame (FrameList[0]);
  }
}

void Manager::SetCurrentFrame (Frame *NewCurFrame)
{
  FramePos=this->operator[](NewCurFrame);
  if (FramePos!=-1){
    CurrentFrame = NewCurFrame;
    CurrentFrame->ShowConsoleTitle();
    CurrentFrame->OnChangeFocus(1);
    CtrlObject->Macro.SetMode(CurrentFrame->GetMacroMode());
  } else {
    SetCurrentFrame(0);
  }
}
*/


void Manager::FrameMenu()
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
//    NextFrame(ExitCode-FramePos);
    ActivateFrame (ExitCode);
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
    if (FrameList[I] == DeletedFrame || FrameList [I]->GetExitCode() == XC_QUIT)
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

/*$ 11.05.2001 OT Теперь можно искать файл не только по полному имени, но и отдельно - путь, отдельно имя */
int  Manager::FindFrameByFile(int ModalType,char *FileName,char *Dir)
{
  char bufFileName[NM];
  char *FullFileName=FileName;
  if (Dir){
    strcpy(bufFileName,Dir);
    AddEndSlash(bufFileName);
    strcat(bufFileName,FileName);
    FullFileName=bufFileName;
  }
  for (int I=0;I<FrameCount;I++)
  {
    char Type[200],Name[NM];
    if (FrameList[I]->GetTypeAndName(Type,Name)==ModalType)
      if (LocalStricmp(Name,FullFileName)==0)
        return(I);
  }
  return(-1);
}
/* 11.05.2001 OT $*/

void Manager::ShowBackground()
{
  if (!RegVer)
  {
    Message(MSG_WARNING,1,MSG(MWarning),MSG(MRegOnly),MSG(MOk));
    return;
  }
  CtrlObject->CmdLine->ShowBackground();
}


void Manager::ActivateFrame(int Index)
{

  ActivatedFrame=this->operator[](Index);
}

/* $ 10.05.2001 DJ
   переключается на панели (фрейм с номером 0)
*/

void Manager::SwitchToPanels()
{
  ActivateFrame (0);
}

/* DJ $ */

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  while (!EndLoop)
  {
    ProcessMainLoop();
    Commit();
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
        FrameMenu();
      _D(SysLog(-1));
      return TRUE;
    case KEY_CTRLTAB:
      if (CurrentFrame->GetCanLoseFocus())
        DeactivateFrame(CurrentFrame,1);
//        NextFrame(1);
        _D(SysLog(-1));
      return TRUE;
    case KEY_CTRLSHIFTTAB:
      if (CurrentFrame->GetCanLoseFocus())
        DeactivateFrame(CurrentFrame,-1);
      _D(SysLog(-1));
      return TRUE;
    }
    CurrentFrame->UpdateKeyBar();
    CurrentFrame->ProcessKey(Key);
  }
//  _D(SysLog("Manager::ProcessKey() ret=%i",ret));
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
  return CurrentFrame?CurrentFrame->GetType() == MODALTYPE_PANELS:FALSE;
}

Frame *Manager::operator[](int Index)
{
  if (Index<0||Index>=FrameCount){
    return NULL;
  }
  return FrameList[Index];
}

int Manager::operator[](Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<FrameCount;i++){
    if (Frame==FrameList[i]){
      Result=i;
      break;
    }
  }
  return Result;
}

void Manager::DeactivateFrame (Frame *Deactivated,int Direction)
{
  FramePos+=Direction;
  if (Direction>0){
    if (FramePos>=FrameCount){
      FramePos=0;
    }
  } else {
    if (FramePos<0) {
      FramePos=FrameCount-1;
    }
  }
  ActivateFrame(FramePos);
}


BOOL Manager::Commit()
{
  int Result = false;
  if (DeletedFrame && (InsertedFrame||ModalizedFrame)){
    UpdateCommit();
    DeletedFrame = NULL;
    InsertedFrame = NULL;
    ModalizedFrame = NULL;
    Result=true;
  } else if (DeletedFrame){
    DeleteCommit();
    DeletedFrame = NULL;
    Result=true;
  } else if (InsertedFrame||ModalizedFrame){
    InsertCommit();
    InsertedFrame =
    ModalizedFrame = NULL;
    Result=true;
  } else if(ActivatedFrame||DeactivatedFrame){
    ActivateCommit();
    DeactivatedFrame=NULL;
    ActivatedFrame=NULL;
    Result=true;
  }
  if (Result){
    Result=Commit();
  }
  return Result;
}

void Manager::StartupMainloop()
{

}

void Manager::ActivateCommit()
{
  if (CurrentFrame==ActivatedFrame){
    return;
  }
  int FrameIndex=this->operator[](ActivatedFrame);
  if (-1!=FrameIndex){
    FramePos=FrameIndex;
    int ModalTopIndex=ActivatedFrame->ModalCount();
    if (ModalTopIndex>0){
      ActivatedFrame->OnChangeFocus(TRUE);
      for (int i=0;i<ModalTopIndex-1;i++){
        Frame *iModal=(*ActivatedFrame)[i];
        iModal->OnChangeFocus(TRUE);
      }
      CurrentFrame=(*ActivatedFrame)[ModalTopIndex-1];
    } else {
      CurrentFrame=ActivatedFrame;
    }
  } else {
    Frame *FoundFrame=NULL;
    for (int i=0;i<FrameCount;i++){
      Frame *iFrame=FrameList[i];
      int TopModalIndex=(*iFrame)[ActivatedFrame];
      if((TopModalIndex>=0)&&(TopModalIndex==iFrame->ModalCount()-1)){
        FoundFrame=ActivatedFrame;
        break;
      }
    }
    if (FoundFrame){
      CurrentFrame=FoundFrame;
    }
  }
  CurrentFrame->ShowConsoleTitle();
  CurrentFrame->OnChangeFocus(1);
  CtrlObject->Macro.SetMode(CurrentFrame->GetMacroMode());

}

void Manager::UpdateCommit()
{
  int FrameIndex=this->operator[](DeletedFrame);
  if (-1!=FrameIndex){
    for (int i=0; i<FrameCount; i++){
      if (FrameList [i] == DeletedFrame){
        FrameList [i] = InsertedFrame;
        break;
      }
    }
    ActivatedFrame=InsertedFrame;
    DeletedFrame->OnDestroy();
    if (!DisableDelete)
      delete DeletedFrame;
    DisableDelete = TRUE;
  } else {
    Frame *iFrame=NULL;
    for (int i=0;i<FrameCount;i++){
      iFrame=FrameList[i];
      int ModalDeletedIndex=(*iFrame)[DeletedFrame];
      if(ModalDeletedIndex>=0){
        if (ModalDeletedIndex>0){
          int iModalCount=iFrame->ModalCount();
          for (int i=iModalCount-1;i>ModalDeletedIndex;i--){
            iFrame->Pop();
          }
        } else {
          iFrame->DestroyAllModal();
        }
        iFrame->Push(ModalizedFrame);
        break;
      }
    }
    ActivatedFrame = (*iFrame)[iFrame->ModalCount()-1];
  }
}

void Manager::DeleteCommit()
{
  if (!DeletedFrame){
    return;
  }
  int FrameIndex=this->operator[](DeletedFrame);
  if (-1!=FrameIndex){
    DeletedFrame->DestroyAllModal();
    for (int i=0; i<FrameCount; i++ ){
      if ( FrameList[i]==DeletedFrame){
        for (int j=i+1; j<FrameCount; j++ )
          FrameList[j-1]=FrameList[j];
        FrameCount--;
        if ( FramePos>=FrameCount ){
          FramePos=0;
        }
        DeletedFrame->OnDestroy();
        break;
      }
    }
    if (!DisableDelete)
      delete DeletedFrame;
    DisableDelete = FALSE;
    if (!ActivatedFrame)
      ActivatedFrame=FrameList[FramePos];
  } else {
    for (int i=0;i<FrameCount;i++){
      Frame *iFrame=FrameList[i];
      int ModalDeletedIndex=(*iFrame)[DeletedFrame];
      if(ModalDeletedIndex>=0){
        if (ModalDeletedIndex>0){
          int iModalCount=iFrame->ModalCount();
          for (int j=iModalCount;j>ModalDeletedIndex;j--){
            iFrame->Pop();
          }
          ActivatedFrame = (*iFrame)[iFrame->ModalCount()-1];
        } else {
          ActivatedFrame = iFrame;
          iFrame->DestroyAllModal();
        }
        break;
      }
    }
  }
}

void Manager::InsertCommit()
{
  if (ModalizedFrame){
    int FrameIndex=this->operator[](CurrentFrame);
    if (FrameIndex==-1) {
      for (int i=0;i<FrameCount;i++){
        Frame *iFrame=FrameList[i];
        if ((*iFrame)[CurrentFrame]!=-1){
          iFrame->Push(ModalizedFrame);
          break;
        }
      }
    } else {
      CurrentFrame->Push(ModalizedFrame);
    }
    ActivatedFrame=ModalizedFrame;
  } else if (InsertedFrame){
    if (FrameListSize <= FrameCount)
    {
      FrameList=(Frame **)realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
      FrameListSize++;
    }
    FrameList[FrameCount]=InsertedFrame;
    if (!ActivatedFrame){
      ActivatedFrame=InsertedFrame;
    }
    FrameCount++;
  }
}

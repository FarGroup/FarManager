/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.06 29.04.2001 $ */

/*
Modify:
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
  ModalList=NULL;
  ActiveList=NULL;
  ModalCount=ModalPos=ModalSizeList=0;
  ActiveListCount=0;

  *NextName=0;

  ActiveModal=0;
  CurrentModal=0;
}

Manager::~Manager()
{
    if ( ModalList )
        free(ModalList);
    if ( ActiveList )
        free(ActiveList);
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
}

BOOL Manager::IsAnyModalModified(int Activate)
{
  for (int I=0;I<ModalCount;I++)
    if (ModalList[I]->IsFileModified())
    {
      if (Activate)
      {
        ModalPos=I;
        NextModal(0);
      }
      return(TRUE);
    }
  return(FALSE);
}


void Manager::AddModal(Modal *NewModal)
{
  SysLog(1,"Manager::AddModal(), NewModal=0x%p, Type=%s",NewModal,NewModal->GetTypeName());

  if ( ModalSizeList<ModalCount+1 )
  {
      SysLog("Manager::AddModal(), realloc list");
      ModalList=(Modal **)realloc(ModalList,sizeof(*ModalList)*(ModalCount+1));
      ModalSizeList++;
  }
  ModalPos=ModalCount;
  ModalList[ModalCount]=NewModal;
  ModalCount++;

//  NewModal->Hide();

  NextModal(0);

  SysLog("Manager::AddModal(), end.");
  SysLog(-1);
  WaitInMainLoop=IsPanelsActive();
}

void Manager::ExecuteModal(Modal *Modal)
{
    INPUT_RECORD rec;
    int Key;
    int EndLoop=0;
    int es;

    SysLog(1,"Manager::ExecuteModal Modal=0x%p, '%s', set ActiveModal",Modal,Modal->GetTypeName());
    PushActive();
    ActiveModal=Modal;

    es=EnableSwitch;
    EnableSwitch=FALSE;

    SysLog("Manager::ExecModal(), begin");

    while (!EndLoop)
    {
        WaitInMainLoop=IsPanelsActive();
        Key=GetInputRecord(&rec);
        WaitInMainLoop=FALSE;
        if (EndLoop)
        break;
        if (rec.EventType==MOUSE_EVENT)
            Modal->ProcessMouse(&rec.Event.MouseEvent);
        else
        {
//            if (Key!=KEY_NONE)
                Modal->UpdateKeyBar();
            Modal->ProcessKey(Key);
        }
        if ( Modal->GetExitCode()!=XC_WORKING )
            EndLoop=1;
    }
    EnableSwitch=es;
    SysLog("Manager::ExecModal(), end.");
    SysLog(-1);
}

void Manager::DestroyModal(Modal *Killed)
{
    int i,j;
    SysLog(1,"Manager::DestroyModal(), Killed=0x%p, '%s'",Killed,Killed->GetTypeName());
    if ( Killed==ActiveModal )
    {
        SysLog("Manager::DestroyModal(), it's ActiveModal, so delete Killed and PopActive");
        delete Killed;
        PopActive();
        SysLog(-1);
        return ;
    }
    for ( i=0; i<ModalCount; i++ )
    {
        if ( ModalList[i]==Killed )
        {
            SysLog("Manager::DestroyModal(), found at i=%i,ModalPos=%i delete and shrink list",i,ModalPos);
            Killed->OnDestroy();
            delete Killed;
            for ( j=i+1; j<ModalCount; j++ )
                ModalList[j-1]=ModalList[j];
            if ( ModalPos>=i )
                ModalPos--;
            ModalCount--;
            SysLog("Manager::DestroyModal(), new ModalCount=%i, ModalPos=%i",ModalCount,ModalPos);
            break;
        }
    }
    if ( CurrentModal==Killed )
    {
        if ( ModalCount )
        {
            CurrentModal=ModalList[ModalPos];
            SysLog("Manager::DestroyModal(), Killed==CurrentModal, set new Current to 0x%p, '%s'",CurrentModal,
                CurrentModal->GetTypeName());
        }
        else
        {
            CurrentModal=0;
            SysLog("Manager::DestroyModal(), Killed==CurrentModal, set new Current to 0");
        }
    }
    SysLog("Manager::DestroyModal() end.");
    SysLog(-1);

}

void Manager::NextModal(int Increment)
{
  SysLog(1,"Manager::NextModal(), ModalPos=%i, Increment=%i, ModalCount=%i",ModalPos,Increment,ModalCount);
  if (ModalCount>0)
  {
    ModalPos+=Increment;
    if (ModalPos<0)
      ModalPos=ModalCount-1;
  }
  if (ModalPos>=ModalCount)
    ModalPos=0;
  SysLog("Manager::NextModal(), new ModalPos=%i",ModalPos);
  Modal *CurModal=ModalList[ModalPos];

  if ( CurrentModal )
      CurrentModal->OnChangeFocus(0);

  CurrentModal=CurModal;
  CurrentModal->ShowConsoleTitle();
  CurrentModal->OnChangeFocus(1);
  CtrlObject->Macro.SetMode(CurrentModal->MacroMode);

  SysLog("Manager::NextModal(), set CurrentModal=0x%p, %s",CurrentModal,CurrentModal->GetTypeName());

  char Type[200],Name[NM];
  if (CurModal->GetTypeAndName(Type,Name)==MODALTYPE_EDITOR)
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


void Manager::SelectModal()
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

    for (int I=0;I<ModalCount;I++)
    {
      char Type[200],Name[NM],NumText[100];
      ModalList[I]->GetTypeAndName(Type,Name);
      if (I<9)
        sprintf(NumText,"&%d. ",I+1);
      else
        strcpy(NumText,"&   ");
      /* $ 28.07.2000 tran
         файл усекает по ширине экрана */
      TruncPathStr(Name,ScrX-40);
      /*  добавляется "*" если файл изменен */
      sprintf(ModalMenuItem.Name,"%s%-20s %c %s",NumText,Type,(ModalList[I]->IsFileModified()?'*':' '),Name);
      /* tran 28.07.2000 $ */
      ModalMenuItem.Selected=(I==ModalPos);
      ModalMenu.AddItem(&ModalMenuItem);
    }
    ModalMenu.Process();
    ExitCode=ModalMenu.GetExitCode();
  }
  if (ExitCode>=0)
  {
    NextModal(ExitCode-ModalPos);
  }
}


void Manager::GetModalTypesCount(int &Viewers,int &Editors)
{
  Viewers=Editors=0;
  for (int I=0;I<ModalCount;I++)
  {
    char Type[200],Name[NM];
    switch(ModalList[I]->GetTypeAndName(Type,Name))
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

int  Manager::GetModalCountByType(int Type)
{
  int ret;
  for (int I=0;I<ModalCount;I++)
  {
    char type[200],Name[NM];
    if (ModalList[I]->GetTypeAndName(type,Name)==Type)
        ret++;
  }
  return ret;
}

void Manager::SetModalPos(int NewPos)
{
  SysLog("Manager::SetModalPos(), NewPos=%i",NewPos);
  ModalPos=NewPos;
}

int  Manager::FindModalByFile(int ModalType,char *FileName)
{
  for (int I=0;I<ModalCount;I++)
  {
    char Type[200],Name[NM];
    if (ModalList[I]->GetTypeAndName(Type,Name)==ModalType)
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
    Modal *NewModal;
    char NewName[NM];
    strcpy(NewName,NextName);
    *NextName=0;
    if (NextViewer)
      NewModal=new FileViewer(NewName,TRUE,FALSE,FALSE,NextPos);
    else
      NewModal=new FileEditor(NewName,FALSE,TRUE,-2,NextPos,FALSE);
    AddModal(NewModal);
  }
}

int  Manager::ProcessKey(int Key)
{
    int ret=FALSE;
    BOOL es;
    char kn[32];
    Modal *cm;
    KeyToText(Key,kn);
    SysLog(1,"Manager::ProcessKey(), key=%i, '%s'",Key,kn);

    if ( ActiveModal )
    {
        SysLog("Manager::ProcessKey(), to ActiveModal 0x%p, '%s'",ActiveModal, ActiveModal->GetTypeName());;
        ActiveModal->UpdateKeyBar();
        ret=ActiveModal->ProcessKey(Key);
    }
    else if ( CurrentModal)
    {
        es=CurrentModal->GetEnableSwitch();
        SysLog("Manager::ProcessKey(), es=%i, to CurrentModal 0x%p, '%s'",es,CurrentModal, CurrentModal->GetTypeName());;
        switch(Key)
        {
            case KEY_F11:
                PluginsMenu();
                SysLog(-1);
                return TRUE;
            case KEY_F12:
                if ( es )
                    SelectModal();
                SysLog(-1);
                return TRUE;
            case KEY_CTRLTAB:
                if ( es )
                    NextModal(1);
                SysLog(-1);
                return TRUE;
            case KEY_CTRLSHIFTTAB:
                if ( es )
                    NextModal(-1);
                SysLog(-1);
                return TRUE;
        }
        SysLog("Manager::ProcessKey(), to CurrentModal 0x%p, '%s'",CurrentModal, CurrentModal->GetTypeName());;
        if (Key!=KEY_NONE)
            CurrentModal->UpdateKeyBar();
        // сохраняем, потому что внутри ProcessKey
        // может быть вызван AddModal и
        // CurrentModal будет изменен.
        cm=CurrentModal;
        ret=CurrentModal->ProcessKey(Key);
        if ( ret )
        {
            // а так проверяем код выхода у того, кого надо
            if ( cm->GetExitCode()==XC_QUIT )
                DestroyModal(cm);
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
    if ( ActiveModal )
        ret=ActiveModal->ProcessMouse(MouseEvent);
    else if ( CurrentModal)
        ret=CurrentModal->ProcessMouse(MouseEvent);
//    SysLog("Manager::ProcessMouse() ret=%i",ret);
    SysLog(-1);
    return ret;
}

void Manager::PushActive()
{
    ActiveList=(Modal **)realloc(ActiveList,sizeof(*ActiveList)*(ActiveListCount+1));
    ActiveList[ActiveListCount]=ActiveModal;
    ActiveListCount++;
    SysLog("Manager::PushActive()");
}

void Manager::PopActive()
{
    if ( ActiveListCount==0 )
    {
        ActiveModal=0;
        return ;
    }
    ActiveModal=ActiveList[ActiveListCount-1];
    ActiveList=(Modal **)realloc(ActiveList,sizeof(*ActiveList)*(ActiveListCount-1));
    ActiveListCount--;
    SysLog("Manager::PopActive(), Active=0x%p, '%s'",ActiveModal,ActiveModal->GetTypeName());
}

void Manager::PluginsMenu()
{
    SysLog(1);
    if ( ActiveModal )
    {
        SysLog("Manager:PluginsMenu(), ActiveModal!=0, return");
        SysLog(-1);
        return;
    }
/// ╧юьхэ ыё  т√чют ъюььрэфё ╨рчюсЁрЄ№ё 
    CtrlObject->Plugins.CommandsMenu(CurrentModal->GetTypeAndName(0,0),0,0);
    SysLog(-1);
}

BOOL Manager::IsPanelsActive()
{
    if ( ActiveModal==0 && CurrentModal->GetTypeAndName(0,0)==MODALTYPE_PANELS )
        return TRUE;
    return FALSE;
}


/*
manager.cpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.04 28.07.2000 $ */

/*
Modify:
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
  ModalCount=ModalPos=0;
  UpdateRequired=FALSE;
  *NextName=0;
  /* $ 28.06.2000 tran
     clear it */
  ActiveModal=0;
  /* tran $ */
}


void Manager::CloseAll()
{
  int I;
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->ClearDone();
    CurModal->SetEnableSwitch(FALSE);
    CurModal->Show();
    CurModal->ProcessKey(KEY_ESC);
    if (!CurModal->Done())
    {
      CurModal->ShowConsoleTitle();
      CurModal->Process();
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
  int ExitCode=NewModal->GetExitCode();
  if (ExitCode<=0)
  {
    delete NewModal;
    CtrlObject->ActivePanel->SetTitle();
    CtrlObject->LeftPanel->Redraw();
    ActivateNextWindow();
    ModalPos=ModalCount;
    return;
  }
  ModalList=(Modal **)realloc(ModalList,sizeof(*ModalList)*(ModalCount+1));
  ModalPos=ModalCount;
  ModalList[ModalCount]=NewModal;
  ModalCount++;
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
  NewModal->Hide();
  NextModal(0);
}


void Manager::NextModal(int Increment)
{
  if (ModalCount>0)
  {
    ModalPos+=Increment;
    if (ModalPos<0)
      ModalPos=ModalCount-1;
  }
  while (ModalCount>0)
  {
    if (ModalPos==ModalCount)
      break;
    if (ModalPos>ModalCount)
      ModalPos=0;
    Modal *CurModal=ModalList[ModalPos];
    CurModal->ClearDone();
    CurModal->ShowConsoleTitle();
    CurModal->Process();

    char Type[200],Name[NM];
    if (CurModal->GetTypeAndName(Type,Name)==MODALTYPE_EDITOR)
      UpdateRequired=TRUE;

    int ExitCode=CurModal->GetExitCode();
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
  }
  CtrlObject->ActivePanel->SetTitle();
  if (UpdateRequired)
  {
    CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    UpdateRequired=TRUE;
  }
  CtrlObject->LeftPanel->Redraw();
  CtrlObject->RightPanel->Redraw();
  CtrlObject->CmdLine.Redraw();
  CtrlObject->MainKeyBar.Redraw();
}


void Manager::SelectModal()
{
  int ExitCode;
  {
    struct MenuItem ModalMenuItem;
    ModalMenuItem.Checked=ModalMenuItem.Separator=*ModalMenuItem.UserData=ModalMenuItem.UserDataSize=0;
    VMenu ModalMenu(MSG(MScreensTitle),NULL,0,ScrY-4);
    ModalMenu.SetHelp("ScrSwitch");
    ModalMenu.SetFlags(MENU_WRAPMODE);
    ModalMenu.SetPosition(-1,-1,0,0);

    sprintf(ModalMenuItem.Name,"&0. %-30s",MSG(MScreensPanels));
    ModalMenuItem.Selected=(ModalPos==ModalCount);
    ModalMenu.AddItem(&ModalMenuItem);

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
      TruncStr(Name,ScrX-40);
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
    if (ExitCode==0)
      ModalPos=ModalCount;
    else
      ModalPos=ExitCode-1;
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


void Manager::SetModalPos(int NewPos)
{
  ModalPos=NewPos;
}


int Manager::FindModalByFile(int ModalType,char *FileName)
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
  int I;
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->Hide();
  }
  {
    RedrawDesktop Redraw;
    CtrlObject->CmdLine.Hide();
    SetCursorType(FALSE,10);
    WaitKey();
    CtrlObject->CmdLine.Show();
  }
  for (I=0;I<ModalCount;I++)
  {
    Modal *CurModal=ModalList[I];
    CurModal->SavePrevScreen();
  }
}


void Manager::SetNextWindow(int Viewer,char *Name,long Pos)
{
  NextViewer=Viewer;
  strcpy(NextName,Name);
  NextPos=Pos;
}


void Manager::ActivateNextWindow()
{
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

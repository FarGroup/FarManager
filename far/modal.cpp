/*
modal.cpp

Parent class для модальных объектов

*/

/* Revision: 1.02 11.07.2000 $ */

/*
Modify:
  11.07.2000 tran
    - trap if no lang files found
  29.06.2000 tran
    - (NT Console resize bug)
      adding virtual method SetScreenPosition
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

Modal::Modal()
{
  ExitCode=-1;
  WriteKey=-1;
  EndLoop=0;
  *HelpTopic=0;
  ModalKeyBar=NULL;
  EnableSwitch=FALSE;
}


void Modal::Process()
{
  Show();
  /* $ 29.06.2000 tran
     установка ActiveModal в CtrlObject (NT Console resize) */
  /* $ 11.07.2000 tran
     just add checking for CtrlObject */
  if ( CtrlObject )
     CtrlObject->ModalManager.ActiveModal=this;
  /* tran $ */
  /* tran 11.07.2000 $ */

  while (!Done())
  {
    ReadInput();
    ProcessInput();
  }
  GetDialogObjectsData();
  /* $ 29.06.2000 tran
     установка ActiveModal в CtrlObject (NT Console resize) */
  if ( CtrlObject )
     CtrlObject->ModalManager.ActiveModal=0;
  /* tran $ */
}


int Modal::ReadInput()
{
  if (WriteKey>=0)
  {
    ReadKey=WriteKey;
    WriteKey=-1;
  }
  else
    ReadKey=GetInputRecord(&ReadRec);
  if (ModalKeyBar!=NULL)
    ModalKeyBar->RedrawIfChanged();
  return(ReadKey);
}


void Modal::WriteInput(int Key)
{
  WriteKey=Key;
}


void Modal::ProcessInput()
{
  if (ReadRec.EventType==MOUSE_EVENT)
    ProcessMouse(&ReadRec.Event.MouseEvent);
  else
    ProcessKey(ReadKey);
}


int Modal::Done()
{
  return(EndLoop);
}


void Modal::ClearDone()
{
  EndLoop=0;
}


int Modal::GetExitCode()
{
  return(ExitCode);
}


void Modal::SetExitCode(int Code)
{
  ExitCode=Code;
  EndLoop=TRUE;
}


void Modal::SetHelp(char *Topic)
{
  strcpy(HelpTopic,Topic);
}


void Modal::ShowHelp()
{
  if (*HelpTopic)
    Help Hlp(HelpTopic);
}


void Modal::SetKeyBar(KeyBar *ModalKeyBar)
{
  Modal::ModalKeyBar=ModalKeyBar;
}

/* $ 29.06.2000 tran
   adding virtual method for processing NT Console resize
*/

void Modal::SetScreenPosition()
{
}
/* tran $ */


/*
modal.cpp

Parent class для модальных объектов

*/

/* Revision: 1.01 29.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  29.06.2000 tran
    - (NT Console resize bug)
      adding virtual method SetScreenPosition
*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif

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
  CtrlObject->ModalManager.ActiveModal=this;
  /* tran $ */
  while (!Done())
  {
    ReadInput();
    ProcessInput();
  }
  GetDialogObjectsData();
  /* $ 29.06.2000 tran
     установка ActiveModal в CtrlObject (NT Console resize) */
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


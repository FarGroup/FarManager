/*
modal.cpp

Parent class для модальных объектов

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif

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
  while (!Done())
  {
    ReadInput();
    ProcessInput();
  }
  GetDialogObjectsData();
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


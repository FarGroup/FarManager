/*
modal.cpp

Parent class для модальных объектов

*/

/* Revision: 1.07 16.05.2001 $ */

/*
Modify:
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  06.05.2001 DJ
    ! перетрях #include
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
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

#include "modal.hpp"
#include "fn.hpp"
#include "help.hpp"

Modal::Modal()
{
  ExitCode=-1;
  WriteKey=-1;
  EndLoop=0;
  *HelpTopic=0;
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
    Help Hlp (HelpTopic);
}

/* $ 29.06.2000 tran
   adding virtual method for processing NT Console resize
*/

void Modal::SetScreenPosition()
{
}
/* tran $ */

/*
modal.cpp

Parent class для модальных объектов

*/

/* Revision: 1.12 25.04.2002 $ */

/*
Modify:
  25.04.2002 IS
    ! внедрение const
  30.03.2002 OT
    - После исправления бага №314 (патч 1250) отвалилось закрытие
      фара по кресту.
  22.03.2002 SVS
    - strcpy - Fuck!
  20.02.2002 OT
    - BugZ#314 - Shift-Enter на папке меняет путь заголовок окна
  14.06.2001 OT
    ! "Бунт" ;-)
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

#include "global.hpp"
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
  if (CloseFARMenu){
    SetExitCode(TRUE);
  }
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


void Modal::SetHelp(const char *Topic)
{
  strncpy(HelpTopic,Topic,sizeof(HelpTopic)-1);
}


void Modal::ShowHelp()
{
  if (*HelpTopic)
    Help Hlp (HelpTopic);
}

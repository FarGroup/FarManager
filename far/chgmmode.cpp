/*
chgmmode.cpp

class ChangeMacroMode

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "chgmmode.hpp"
#include "global.hpp"

ChangeMacroMode::ChangeMacroMode(int NewMode)
{
  if (CtrlObject!=NULL)
  {
    PrevMacroMode=CtrlObject->Macro.GetMode();
    CtrlObject->Macro.SetMode(NewMode);
  }
  else
    PrevMacroMode=MACRO_SHELL;
}


ChangeMacroMode::~ChangeMacroMode()
{
  if (CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);
}

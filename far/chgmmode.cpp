/*
chgmmode.cpp

class ChangeMacroMode

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
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

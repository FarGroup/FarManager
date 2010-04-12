/*
chgmmode.cpp

class ChangeMacroMode

*/

#include "headers.hpp"
#pragma hdrstop

#include "chgmmode.hpp"
#include "ctrlobj.hpp"

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

/*
rdrwdsk.cpp

class RedrawDesktop

*/

/* Revision: 1.02 29.04.2001 $ */

/*
Modify:
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
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

RedrawDesktop::RedrawDesktop()
{
  LeftVisible=CtrlObject->Cp()->LeftPanel->IsVisible();
  RightVisible=CtrlObject->Cp()->RightPanel->IsVisible();
  CtrlObject->Cp()->LeftPanel->Hide();
  CtrlObject->Cp()->RightPanel->Hide();
  CtrlObject->MainKeyBar->Hide();
  CtrlObject->TopMenuBar->Hide();
}


RedrawDesktop::~RedrawDesktop()
{
  if (Opt.ShowKeyBar)
    CtrlObject->MainKeyBar->Show();
  if (Opt.ShowMenuBar)
    CtrlObject->TopMenuBar->Show();
  CtrlObject->CmdLine->Show();
  int RightType=CtrlObject->Cp()->RightPanel->GetType();
  if (RightVisible && RightType!=QVIEW_PANEL)
    CtrlObject->Cp()->RightPanel->Show();
  if (LeftVisible)
    CtrlObject->Cp()->LeftPanel->Show();
  if (RightVisible && RightType==QVIEW_PANEL)
    CtrlObject->Cp()->RightPanel->Show();
}

/*
rdrwdsk.cpp

class RedrawDesktop

*/

/* Revision: 1.01 28.02.2001 $ */

/*
Modify:
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
  LeftVisible=CtrlObject->LeftPanel->IsVisible();
  RightVisible=CtrlObject->RightPanel->IsVisible();
  CtrlObject->LeftPanel->Hide();
  CtrlObject->RightPanel->Hide();
  CtrlObject->MainKeyBar.Hide();
  CtrlObject->TopMenuBar.Hide();
}


RedrawDesktop::~RedrawDesktop()
{
  if (Opt.ShowKeyBar)
    CtrlObject->MainKeyBar.Show();
  if (Opt.ShowMenuBar)
    CtrlObject->TopMenuBar.Show();
  CtrlObject->CmdLine->Show();
  int RightType=CtrlObject->RightPanel->GetType();
  if (RightVisible && RightType!=QVIEW_PANEL)
    CtrlObject->RightPanel->Show();
  if (LeftVisible)
    CtrlObject->LeftPanel->Show();
  if (RightVisible && RightType==QVIEW_PANEL)
    CtrlObject->RightPanel->Show();
}

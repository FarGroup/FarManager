/*
rdrwdsk.cpp

class RedrawDesktop

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
  CtrlObject->CmdLine.Show();
  int RightType=CtrlObject->RightPanel->GetType();
  if (RightVisible && RightType!=QVIEW_PANEL)
    CtrlObject->RightPanel->Show();
  if (LeftVisible)
    CtrlObject->LeftPanel->Show();
  if (RightVisible && RightType==QVIEW_PANEL)
    CtrlObject->RightPanel->Show();
}

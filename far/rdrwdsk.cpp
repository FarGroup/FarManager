/*
rdrwdsk.cpp

class RedrawDesktop

*/

/* Revision: 1.09 07.12.2001 $ */

/*
Modify:
  07.12.2001 SVS
    + В RedrawDesktop перенесена логика гашения и восстановления доски.
    - бага - кейбар и полоса меню гасились, а переменные Opt.ShowKeyBar и
      Opt.ShowMenuBar не вставлялись в ноль, от чего иногда был "момент
      истины" - глюки с прорисовкой после Alt-F9.
  12.11.2001 SVS
    ! немного уточнения с расширением сервиса
  30.05.2001 OT
    - Не прорисовывалась командная строка при сохранении/восстановлении буфера
  21.05.2001 OT
    ! Про "Ресайзинг буфера консоли"
  11.05.2001 OT
    ! Отрисовка Background
  06.05.2001 DJ
    ! перетрях #include
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

#include "manager.hpp"
#include "keys.hpp"
#include "rdrwdsk.hpp"
#include "global.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

RedrawDesktop::RedrawDesktop(BOOL IsHidden)
{
  CtrlObject->CmdLine->ShowBackground();
  CtrlObject->CmdLine->Show();
  LeftVisible=CtrlObject->Cp()->LeftPanel->IsVisible();
  RightVisible=CtrlObject->Cp()->RightPanel->IsVisible();
  KeyBarVisible=Opt.ShowKeyBar;//CtrlObject->MainKeyBar->IsVisible();
  TopMenuBarVisible=Opt.ShowMenuBar;//CtrlObject->TopMenuBar->IsVisible();
  if(IsHidden)
  {
    CtrlObject->Cp()->LeftPanel->CloseFile();
    CtrlObject->Cp()->RightPanel->CloseFile();
    CtrlObject->Cp()->LeftPanel->Hide();
    CtrlObject->Cp()->RightPanel->Hide();
    CtrlObject->MainKeyBar->Hide();
    CtrlObject->TopMenuBar->Hide();
    Opt.ShowKeyBar=0;
    Opt.ShowMenuBar=0;
  }
}


RedrawDesktop::~RedrawDesktop()
{
  Opt.ShowKeyBar=KeyBarVisible;
  Opt.ShowMenuBar=TopMenuBarVisible;

  CtrlObject->CmdLine->SaveBackground();
  CtrlObject->CmdLine->Show();

  if (KeyBarVisible)
    CtrlObject->MainKeyBar->Show();
  if (TopMenuBarVisible)
    CtrlObject->TopMenuBar->Show();

  int RightType=CtrlObject->Cp()->RightPanel->GetType();
  if (RightVisible && RightType!=QVIEW_PANEL)
    //CtrlObject->Cp()->RightPanel->Show();
    CtrlObject->Cp()->RightPanel->SetVisible(TRUE);
  if (LeftVisible)
    // CtrlObject->Cp()->LeftPanel->Show();
    CtrlObject->Cp()->LeftPanel->SetVisible(TRUE);
  if (RightVisible && RightType==QVIEW_PANEL)
    // CtrlObject->Cp()->RightPanel->Show();
    CtrlObject->Cp()->RightPanel->SetVisible(TRUE);

  // Временное решение!
  // Иначе траблы при пересчете...
  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
}

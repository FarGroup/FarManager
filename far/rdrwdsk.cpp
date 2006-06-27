/*
rdrwdsk.cpp

class RedrawDesktop

*/

/* Revision: 1.10 04.03.2002 $ */

/*
Modify:
  04.03.2002 SVS
    ! ����� ���� � �����������...
  07.12.2001 SVS
    + � RedrawDesktop ���������� ������ ������� � �������������� �����.
    - ���� - ������ � ������ ���� ��������, � ���������� Opt.ShowKeyBar �
      Opt.ShowMenuBar �� ����������� � ����, �� ���� ������ ��� "������
      ������" - ����� � ����������� ����� Alt-F9.
  12.11.2001 SVS
    ! ������� ��������� � ����������� �������
  30.05.2001 OT
    - �� ��������������� ��������� ������ ��� ����������/�������������� ������
  21.05.2001 OT
    ! ��� "��������� ������ �������"
  11.05.2001 OT
    ! ��������� Background
  06.05.2001 DJ
    ! �������� #include
  29.04.2001 ��
    + ��������� NWZ �� ����������
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
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
    // ��������! �������!
    // �������� �����������, � ����������� ��!
    if(CtrlObject->Cp()->ActivePanel == CtrlObject->Cp()->LeftPanel)
    {
      CtrlObject->Cp()->LeftPanel->Hide();
      CtrlObject->Cp()->RightPanel->Hide();
    }
    else
    {
      CtrlObject->Cp()->RightPanel->Hide();
      CtrlObject->Cp()->LeftPanel->Hide();
    }
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

  // ��������� �������!
  // ����� ������ ��� ���������...
  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
}

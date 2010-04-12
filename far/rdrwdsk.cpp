/*
rdrwdsk.cpp

class RedrawDesktop

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

	if (IsHidden)
	{
		CtrlObject->Cp()->LeftPanel->CloseFile();
		CtrlObject->Cp()->RightPanel->CloseFile();

		// ВНИМАНИЕ! КОСТЫЛЬ!
		// соблюдем очередность, в зависимости от!
		if (CtrlObject->Cp()->ActivePanel == CtrlObject->Cp()->LeftPanel)
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

	// Временное решение!
	// Иначе траблы при пересчете...
	FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
}

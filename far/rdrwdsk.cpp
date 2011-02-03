/*
rdrwdsk.cpp

class RedrawDesktop
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "manager.hpp"
#include "keys.hpp"
#include "rdrwdsk.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "config.hpp"

RedrawDesktop::RedrawDesktop(BOOL IsHidden):
	LeftVisible(CtrlObject->Cp()->LeftPanel->IsVisible()),
	RightVisible(CtrlObject->Cp()->RightPanel->IsVisible()),
	ClockVisible(Opt.Clock!=0)
{
	CtrlObject->CmdLine->ShowBackground();
	CtrlObject->CmdLine->Show();

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
		Opt.Clock=FALSE;
		CtrlObject->MainKeyBar->Hide();
		CtrlObject->TopMenuBar->Hide();
	}
}


RedrawDesktop::~RedrawDesktop()
{
	CtrlObject->CmdLine->SaveBackground();
	CtrlObject->CmdLine->Show();

	if (Opt.ShowKeyBar)
		CtrlObject->MainKeyBar->Show();

	Opt.Clock=ClockVisible;

	if (Opt.ShowMenuBar)
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

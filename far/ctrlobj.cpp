/*
ctrlobj.cpp

Управление остальными объектами, раздача сообщений клавиатуры и мыши
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

#include "ctrlobj.hpp"
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "history.hpp"
#include "filefilter.hpp"
#include "filepanels.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "poscache.hpp"
#include "plugins.hpp"
#include "desktop.hpp"
#include "menubar.hpp"
#include "colormix.hpp"

ControlObject::ControlObject()
{
	_OT(SysLog(L"[%p] ControlObject::ControlObject()", this));

	SetColor(COL_COMMANDLINEUSERSCREEN);
	GotoXY(0, ScrY - 3);
	ShowCopyright();
	GotoXY(0, ScrY - 2);
	MoveCursor(0, ScrY - 1);

	Desktop = desktop::create();
	Global->WindowManager->InsertWindow(Desktop);
	Desktop->FillFromBuffer();

	HiFiles = std::make_unique<HighlightFiles>();
	Plugins = std::make_unique<PluginManager>();

	CmdHistory = std::make_unique<History>(HISTORYTYPE_CMD, string(), Global->Opt->SaveHistory);

	FolderHistory = std::make_unique<History>(HISTORYTYPE_FOLDER, string(), Global->Opt->SaveFoldersHistory);
	FolderHistory->SetAddMode(true, 2, true);

	ViewHistory = std::make_unique<History>(HISTORYTYPE_VIEW, string(), Global->Opt->SaveViewHistory);
	ViewHistory->SetAddMode(true,Global->Opt->FlagPosixSemantics?1:2,true);

	FileFilter::InitFilter();
}


void ControlObject::Init(int DirCount)
{
	FPanels = FilePanels::create(true, DirCount);

	Global->WindowManager->InsertWindow(FPanels); // before PluginCommit()

	const auto strOldTitle = Console().GetTitle();
	Global->WindowManager->PluginCommit();
	Plugins->LoadPlugins();
	Console().SetTitle(strOldTitle);

	FPanels->LeftPanel()->Update(0);
	FPanels->RightPanel()->Update(0);
	FPanels->LeftPanel()->GoToFile(Global->Opt->LeftPanel.CurFile);
	FPanels->RightPanel()->GoToFile(Global->Opt->RightPanel.CurFile);

	FarChDir(FPanels->ActivePanel()->GetCurDir());

	Macro.LoadMacros(true);
	FPanels->LeftPanel()->SetCustomSortMode(Global->Opt->LeftPanel.SortMode, true);
	FPanels->RightPanel()->SetCustomSortMode(Global->Opt->RightPanel.SortMode, true);
	Global->WindowManager->SwitchToPanels();  // otherwise panels are empty
	/*
		FarChDir(StartCurDir);
	*/
//  _SVS(SysLog(L"ActivePanel->GetCurDir='%s'",StartCurDir));
//  _SVS(char PPP[NM];Cp()->GetAnotherPanel(Cp()->ActivePanel)->GetCurDir(PPP);SysLog(L"AnotherPanel->GetCurDir='%s'",PPP));
}

void ControlObject::CreateDummyFilePanels()
{
	FPanels = FilePanels::create(false, 0);
}

ControlObject::~ControlObject()
{
	if (Global->CriticalInternalError)
		return;

	_OT(SysLog(L"[%p] ControlObject::~ControlObject()", this));

	if (Cp() && Cp()->ActivePanel())
	{
		if (Global->Opt->AutoSaveSetup)
			Global->Opt->Save(false);

		if (Cp()->ActivePanel()->GetMode() != panel_mode::PLUGIN_PANEL)
		{
			FolderHistory->AddToHistory(Cp()->ActivePanel()->GetCurDir());
		}
	}

	Global->WindowManager->CloseAll();
	FPanels=nullptr;
	FileFilter::CloseFilter();
	History::CompactHistory();
	FilePositionCache::CompactHistory();
}


void ControlObject::ShowCopyright(DWORD Flags)
{
	if (Flags&1)
	{
		string strOut(Global->Version());
		strOut.append(EOL_STR).append(Global->Copyright()).append(EOL_STR);
		Console().Write(strOut);
		Console().Commit();
	}
	else
	{
		COORD Size, CursorPosition;
		Console().GetSize(Size);
		Console().GetCursorPosition(CursorPosition);
		int FreeSpace=Size.Y-CursorPosition.Y-1;

		if (FreeSpace<5)
			ScrollScreen(5-FreeSpace);

		GotoXY(0,ScrY-4);
		Text(Global->Version());
		GotoXY(0,ScrY-3);
		Text(Global->Copyright());
	}
}

FilePanels* ControlObject::Cp()
{
	return FPanels.get();
}

window_ptr ControlObject::Panels(void)
{
	return FPanels;
}

CommandLine* ControlObject::CmdLine(void)
{
	return FPanels->GetCmdLine();
}

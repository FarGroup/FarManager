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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "ctrlobj.hpp"

// Internal:
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "history.hpp"
#include "filefilter.hpp"
#include "filefilterparams.hpp"
#include "panel.hpp"
#include "filepanels.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "poscache.hpp"
#include "plugins.hpp"
#include "scrbuf.hpp"
#include "global.hpp"
#include "farversion.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

ControlObject::ControlObject()
{
	Global->WindowManager->InitDesktop();

	filters::InitFilters();

	HiFiles = std::make_unique<highlight::configuration>();
	Plugins = std::make_unique<PluginManager>();

	CmdHistory = std::make_unique<History>(HISTORYTYPE_CMD, string_view{}, Global->Opt->SaveHistory, false);
	FolderHistory = std::make_unique<History>(HISTORYTYPE_FOLDER, string_view{}, Global->Opt->SaveFoldersHistory, true);
	ViewHistory = std::make_unique<History>(HISTORYTYPE_VIEW, string_view{}, Global->Opt->SaveViewHistory, true);
}


void ControlObject::Init()
{
	FPanels = FilePanels::create(true);

	Global->WindowManager->InsertWindow(FPanels); // before PluginCommit()

	const auto strOldTitle = Global->ScrBuf->GetTitle();
	Global->WindowManager->PluginCommit();
	Plugins->LoadPlugins();
	Global->ScrBuf->SetTitle(strOldTitle);
	Macro.LoadMacros(true);

	FPanels->LeftPanel()->Update(0);
	FPanels->RightPanel()->Update(0);
	FPanels->LeftPanel()->GoToFile(Global->Opt->LeftPanel.CurFile);
	FPanels->RightPanel()->GoToFile(Global->Opt->RightPanel.CurFile);

	FarChDir(FPanels->ActivePanel()->GetCurDir());

	// BUGBUG
	FPanels->LeftPanel()->SetCustomSortMode(static_cast<panel_sort>(Global->Opt->LeftPanel.SortMode.Get()), sort_order::keep, false);
	FPanels->RightPanel()->SetCustomSortMode(static_cast<panel_sort>(Global->Opt->RightPanel.SortMode.Get()), sort_order::keep, false);

	Global->WindowManager->SwitchToPanels();  // otherwise panels are empty
}

void ControlObject::CreateDummyFilePanels()
{
	FPanels = FilePanels::create(false);
}

ControlObject::~ControlObject() = default;

void ControlObject::close()
{
	if (Global->CriticalInternalError)
	{
		Global->WindowManager->CloseAll();
		return;
	}

	// dummy_panel indicates /v or /e mode
	if (FPanels && FPanels->ActivePanel() && !std::dynamic_pointer_cast<dummy_panel>(FPanels->ActivePanel()))
	{
		if (Global->Opt->AutoSaveSetup)
			Global->Opt->Save(false);

		if (FPanels->ActivePanel()->GetMode() != panel_mode::PLUGIN_PANEL)
			FolderHistory->AddToHistory(FPanels->ActivePanel()->GetCurDir());
	}

	Global->WindowManager->CloseAll();
	History::CompactHistory();
	FilePositionCache::CompactHistory();

	FPanels.reset();
	Plugins->UnloadPlugins();
}

FilePanels* ControlObject::Cp() const
{
	return FPanels.get();
}

window_ptr ControlObject::Panels() const
{
	return FPanels;
}

CommandLine* ControlObject::CmdLine() const
{
	return FPanels->GetCmdLine();
}

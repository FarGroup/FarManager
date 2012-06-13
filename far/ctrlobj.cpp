/*
ctrlobj.cpp

”правление остальными объектами, раздача сообщений клавиатуры и мыши
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
#include "treelist.hpp"
#include "filefilter.hpp"
#include "filepanels.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "fileowner.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "shortcuts.hpp"
#include "poscache.hpp"

ControlObject *CtrlObject;

ControlObject::ControlObject():
	FPanels(0),
	CmdLine(0),
	MainKeyBar(nullptr),
	TopMenuBar(nullptr)
{
	_OT(SysLog(L"[%p] ControlObject::ControlObject()", this));
	CtrlObject=this;

	HiFiles = new HighlightFiles;
	FolderShortcuts = new Shortcuts();
	FrameManager = new Manager;
	Plugins = new PluginManager;

	CmdHistory=new History(HISTORYTYPE_CMD,nullptr, Opt.SaveHistory, false);
	FolderHistory=new History(HISTORYTYPE_FOLDER, nullptr, Opt.SaveFoldersHistory, true);
	ViewHistory=new History(HISTORYTYPE_VIEW, nullptr, Opt.SaveViewHistory, true);
	FolderHistory->SetAddMode(true,2,true);
	ViewHistory->SetAddMode(true,Opt.FlagPosixSemantics?1:2,true);
}


void ControlObject::Init(int DirCount)
{
	TreeList::ClearCache(0);
	FileFilter::InitFilter();
	SetColor(COL_COMMANDLINEUSERSCREEN);
	GotoXY(0,ScrY-3);
	ShowCopyright();
	GotoXY(0,ScrY-2);
	MoveCursor(0,ScrY-1);
	FPanels=new FilePanels();
	CmdLine=new CommandLine();
	CmdLine->SaveBackground(0,0,ScrX,ScrY);
	this->MainKeyBar=&(FPanels->MainKeyBar);
	this->TopMenuBar=&(FPanels->TopMenuBar);
	FPanels->Init(DirCount);
	FPanels->SetScreenPosition();

	if (Opt.ShowMenuBar)
		this->TopMenuBar->Show();

//  FPanels->Redraw();
	CmdLine->Show();

	if (Opt.ShowKeyBar)
		this->MainKeyBar->Show();

	Cp()->LeftPanel->Update(0);
	Cp()->RightPanel->Update(0);

	if (Opt.AutoSaveSetup)
	{
		Cp()->LeftPanel->GoToFile(Opt.strLeftCurFile);
		Cp()->RightPanel->GoToFile(Opt.strRightCurFile);
	}

	FrameManager->InsertFrame(FPanels);
	string strStartCurDir;
	Cp()->ActivePanel->GetCurDir(strStartCurDir);
	FarChDir(strStartCurDir);
	Cp()->ActivePanel->SetFocus();
	{
		string strOldTitle;
		Console.GetTitle(strOldTitle);
		FrameManager->PluginCommit();
		Plugins->LoadPlugins();
		Console.SetTitle(strOldTitle);
	}
	Macro.LoadMacros();
	/*
		FarChDir(StartCurDir);
	*/
//  _SVS(SysLog(L"ActivePanel->GetCurDir='%s'",StartCurDir));
//  _SVS(char PPP[NM];Cp()->GetAnotherPanel(Cp()->ActivePanel)->GetCurDir(PPP);SysLog(L"AnotherPanel->GetCurDir='%s'",PPP));
}

void ControlObject::CreateFilePanels()
{
	FPanels=new FilePanels();
}

ControlObject::~ControlObject()
{
	if (CriticalInternalError)
		return;

	_OT(SysLog(L"[%p] ControlObject::~ControlObject()", this));

	if (Cp()&&Cp()->ActivePanel)
	{
		if (Opt.AutoSaveSetup)
			SaveConfig(0);

		if (Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
		{
			string strCurDir;
			Cp()->ActivePanel->GetCurDir(strCurDir);
			FolderHistory->AddToHistory(strCurDir);
		}
	}

	TreeList::FlushCache();
	SIDCacheFlush();
	Lang.Close();
	FrameManager->CloseAll();
	FPanels=nullptr;
	FileFilter::CloseFilter();
	History::CompactHistory();
	FilePositionCache::CompactHistory();

	delete CmdLine;
	delete ViewHistory;
	delete FolderHistory;
	delete CmdHistory;
	delete FrameManager;
	delete Plugins;
	delete FolderShortcuts;
	delete HiFiles;

	CtrlObject=nullptr;
}


void ControlObject::ShowCopyright(DWORD Flags)
{
	if (Flags&1)
	{
		string strOut(Version);
		strOut.Append(L"\n").Append(Copyright).Append(L"\n");
		Console.Write(strOut);
		Console.Commit();
	}
	else
	{
		COORD Size, CursorPosition;
		Console.GetSize(Size);
		Console.GetCursorPosition(CursorPosition);
		int FreeSpace=Size.Y-CursorPosition.Y-1;

		if (FreeSpace<5)
			ScrollScreen(5-FreeSpace);

		GotoXY(0,ScrY-4);
		Text(Version);
		GotoXY(0,ScrY-3);
		Text(Copyright);
	}
}

FilePanels* ControlObject::Cp()
{
	return FPanels;
}

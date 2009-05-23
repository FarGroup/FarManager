/*
ctrlobj.cpp

”правление остальными объектами, раздача сообщений клавиатуры и мыши
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "poscache.hpp"
#include "history.hpp"
#include "treelist.hpp"
#include "filefilter.hpp"
#include "filepanels.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "fileowner.hpp"

ControlObject *CtrlObject;

ControlObject::ControlObject()
{
  _OT(SysLog(L"[%p] ControlObject::ControlObject()", this));
  FPanels=0;
  CtrlObject=this;

  HiFiles = new HighlightFiles;
  ViewerPosCache = new FilePositionCache(FPOSCACHE_64);
  EditorPosCache = new FilePositionCache(FPOSCACHE_32);
  FrameManager = new Manager;

  ReadConfig();

  CmdLine=new CommandLine;

  CmdHistory=new History(HISTORYTYPE_CMD,Opt.HistoryCount,L"SavedHistory",&Opt.SaveHistory,false);
  FolderHistory=new History(HISTORYTYPE_FOLDER,Opt.FoldersHistoryCount,L"SavedFolderHistory",&Opt.SaveFoldersHistory,true);
  ViewHistory=new History(HISTORYTYPE_VIEW,Opt.ViewHistoryCount,L"SavedViewHistory",&Opt.SaveViewHistory,true);

  FolderHistory->SetAddMode(true,2,true);
  ViewHistory->SetAddMode(true,Opt.FlagPosixSemantics?1:2,true);
  if (Opt.SaveHistory)
    CmdHistory->ReadHistory();
  if (Opt.SaveFoldersHistory)
    FolderHistory->ReadHistory();
  if (Opt.SaveViewHistory)
    ViewHistory->ReadHistory();
}


void ControlObject::Init()
{
  TreeList::ClearCache(0);
  FileFilter::InitFilter();

  SetColor(COL_COMMANDLINEUSERSCREEN);
  GotoXY(0,ScrY-3);
  ShowCopyright();
  GotoXY(0,ScrY-2);

  MoveCursor(0,ScrY-1);
  CmdLine->SaveBackground(0,0,ScrX,ScrY);

  FPanels=new FilePanels();
  this->MainKeyBar=&(FPanels->MainKeyBar);
  this->TopMenuBar=&(FPanels->TopMenuBar);

  FPanels->Init();
  FPanels->SetScreenPosition();

  if (Opt.ShowMenuBar)
    this->TopMenuBar->Show();
//  FPanels->Redraw();
  CmdLine->Show();
  if(Opt.ShowKeyBar)
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
    apiGetConsoleTitle (strOldTitle);
    FrameManager->PluginCommit();
    Plugins.LoadPlugins();
    SetConsoleTitleW(strOldTitle);
  }
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
  if(CriticalInternalError)
    return;

  _OT(SysLog(L"[%p] ControlObject::~ControlObject()", this));
  if (Cp()&&Cp()->ActivePanel!=NULL)
  {
    if (Opt.AutoSaveSetup)
      SaveConfig(0);
    if (Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
    {
      string strCurDir;
      Cp()->ActivePanel->GetCurDir(strCurDir);
      FolderHistory->AddToHistory(strCurDir,NULL,0);
    }
  }

  FrameManager->CloseAll();
  FPanels=NULL;

  FileFilter::CloseFilter();
  delete CmdHistory;
  delete FolderHistory;
  delete ViewHistory;
  delete CmdLine;
  delete HiFiles;

  if (Opt.ViOpt.SaveViewerPos)
    ViewerPosCache->Save(L"Viewer\\LastPositions");
  delete ViewerPosCache;

  if (Opt.EdOpt.SavePos)
    EditorPosCache->Save(L"Editor\\LastPositions");

  delete EditorPosCache;
  delete FrameManager;

  TreeList::FlushCache();

  SIDCacheFlush();
  Lang.Close();
  CtrlObject=NULL;
}


void ControlObject::ShowCopyright(DWORD Flags)
{
  char *Str=xf_strdup(Copyright);
  char *Line2=NULL;
  char Xor=17;
  for (int I=0;Str[I];I++)
  {
    Str[I]=(Str[I]&0x7f)^Xor;
    Xor^=Str[I];
    if(Str[I] == '\n')
    {
      Line2=&Str[I+1];
      Str[I]='\0';
    }
  }

	string strStr(Str, CP_OEMCP); //BUGBUG
	string strLine(Line2, CP_OEMCP);  //BUGBUG

  xf_free (Str);

  if(Flags&1)
  {
		fwprintf(stderr,L"%s\n%s\n",(const wchar_t*)strStr,(const wchar_t*)strLine);
  }
  else
  {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConOut,&csbi);
    int FreeSpace=csbi.dwSize.Y-csbi.dwCursorPosition.Y-1;
		int LineCount=4+(strLine.IsEmpty()?0:1);
    if(FreeSpace<LineCount)
      ScrollScreen(LineCount-FreeSpace);
		if(!strLine.IsEmpty())
    {
      GotoXY(0,ScrY-4);
      Text(strStr);
      GotoXY(0,ScrY-3);
      Text(strLine);
    }
    else
      Text(strStr);
  }
}

FilePanels* ControlObject::Cp()
{
  return FPanels;
}

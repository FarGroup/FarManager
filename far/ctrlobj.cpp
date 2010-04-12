/*
ctrlobj.cpp

Управление остальными объектами, раздача сообщений клавиатуры и мыши

*/

#include "headers.hpp"
#pragma hdrstop

#include "ctrlobj.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "poscache.hpp"
#include "history.hpp"
#include "treelist.hpp"
#include "filefilter.hpp"
#include "filepanels.hpp"

ControlObject *CtrlObject;

ControlObject::ControlObject()
{
	_OT(SysLog("[%p] ControlObject::ControlObject()", this));
	FPanels=0;
	CtrlObject=this;
	/* $ 06.05.2001 DJ
	   создаем динамически (для уменьшения dependencies)
	*/
	HiFiles = new HighlightFiles;
	ViewerPosCache = new FilePositionCache(FPOSCACHE_64);
	EditorPosCache = new FilePositionCache(FPOSCACHE_32);
	FrameManager = new Manager;
	ReadConfig();
	/* $ 28.02.2001 IS
	     Создадим обязательно только после того, как прочитали настройки
	*/
	CmdLine=new CommandLine;
	CmdHistory=new History(HISTORYTYPE_CMD,Opt.HistoryCount,"SavedHistory",&Opt.SaveHistory,false);
	FolderHistory=new History(HISTORYTYPE_FOLDER,Opt.FoldersHistoryCount,"SavedFolderHistory",&Opt.SaveFoldersHistory,true);
	ViewHistory=new History(HISTORYTYPE_VIEW,Opt.ViewHistoryCount,"SavedViewHistory",&Opt.SaveViewHistory,true);
	FolderHistory->SetAddMode(TRUE,2,TRUE);
	ViewHistory->SetAddMode(TRUE,Opt.FlagPosixSemantics?1:2,TRUE);

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

	while (RegVer==-1)
		Sleep(0);

	ShowCopyright();
	GotoXY(0,ScrY-2);
	char TruncRegName[512];
	xstrncpy(TruncRegName,RegName,sizeof(TruncRegName)-1);
	char *CountPtr=strstr(TruncRegName," - (");

	if (CountPtr!=NULL && isdigit(CountPtr[4]) && strchr(CountPtr+5,'/')!=NULL &&
	        strchr(CountPtr+6,')')!=NULL)
		*CountPtr=0;

	if (RegVer)
		mprintf("%s: %s",MSG(MRegistered),TruncRegName);
	else
		Text(MShareware);

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

	if (Opt.ShowKeyBar)
		this->MainKeyBar->Show();

	RegistrationBugs=FALSE;
#ifdef _DEBUGEXC

	if (CheckRegistration)
#endif
		if (_beginthread(CheckVersion,0x10000,NULL) == -1)
		{
			RegistrationBugs=TRUE;
			CheckVersion(NULL);
		}

	if (Cp()->LeftPanel->GetType() != TREE_PANEL)
		Cp()->LeftPanel->Update(0);

	if (Cp()->RightPanel->GetType() != TREE_PANEL)
		Cp()->RightPanel->Update(0);

	/* $ 07.09.2000 tran
	  + Config//Current File */
	if (Opt.AutoSaveSetup)
	{
		Cp()->LeftPanel->GoToFile(Opt.LeftCurFile);
		Cp()->RightPanel->GoToFile(Opt.RightCurFile);
	}

	FrameManager->InsertFrame(FPanels);
	char StartCurDir[2048];
	Cp()->ActivePanel->GetCurDir(StartCurDir);
	FarChDir(StartCurDir, TRUE);
	Cp()->ActivePanel->SetFocus();
	{
		char OldTitle[512];
		GetConsoleTitle(OldTitle,sizeof(OldTitle));
		FrameManager->PluginCommit();
		Plugins.LoadPlugins();
		SetConsoleTitle(OldTitle);
	}
	/*
	  FarChDir(StartCurDir, TRUE);
	*/
//  _SVS(SysLog("ActivePanel->GetCurDir='%s'",StartCurDir));
//  _SVS(char PPP[NM];Cp()->GetAnotherPanel(Cp()->ActivePanel)->GetCurDir(PPP);SysLog("AnotherPanel->GetCurDir='%s'",PPP));
}

void ControlObject::CreateFilePanels()
{
	FPanels=new FilePanels();
}

ControlObject::~ControlObject()
{
	if (CriticalInternalError)
		return;

	_OT(SysLog("[%p] ControlObject::~ControlObject()", this));

	if (Cp()&&Cp()->ActivePanel!=NULL)
	{
		if (Opt.AutoSaveSetup)
			SaveConfig(0);

		if (Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
		{
			char CurDir[NM];
			Cp()->ActivePanel->GetCurDir(CurDir);
			FolderHistory->AddToHistory(CurDir,NULL,0);
		}
	}

	FrameManager->CloseAll();
	FPanels=NULL;
	Plugins.SendExit();
	FileFilter::CloseFilter();
	delete CmdHistory;
	delete FolderHistory;
	delete ViewHistory;
	delete CmdLine;
	delete HiFiles;

	if (Opt.ViOpt.SaveViewerPos)
		ViewerPosCache->Save("Viewer\\LastPositions");

	delete ViewerPosCache;

	if (Opt.EdOpt.SavePos)
		EditorPosCache->Save("Editor\\LastPositions");

	delete EditorPosCache;
	delete FrameManager;
	TreeList::FlushCache();
	SIDCacheFlush();
	Lang.Close();
	CtrlObject=NULL;
}


/* $ 25.11.2000 SVS
   Copyright в 2 строки
*/
/* $ 15.12.2000 SVS
 Метод ShowCopyright - public static & параметр Flags.
*/
void ControlObject::ShowCopyright(DWORD Flags)
{
	char *Str=xf_strdup(Copyright);
	char *Line2=NULL;
	char Xor=17;

	for (int I=0; Str[I]; I++)
	{
		Str[I]=(Str[I]&0x7f)^Xor;
		Xor^=Str[I];

		if (Str[I] == '\n')
		{
			Line2=&Str[I+1];
			Str[I]='\0';
		}
	}

	if (Flags&1)
	{
		fprintf(stderr,"%s\n%s\n",Str,Line2);
	}
	else
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConOut,&csbi);
		int FreeSpace=csbi.dwSize.Y-csbi.dwCursorPosition.Y-1;
		int LineCount=4+(Line2?1:0);

		if (FreeSpace<LineCount)
			ScrollScreen(LineCount-FreeSpace);

		if (Line2)
		{
			GotoXY(0,ScrY-4);
			Text(Str);
			GotoXY(0,ScrY-3);
			Text(Line2);
		}
		else
			Text(Str);
	}

	xf_free(Str);
}


FilePanels* ControlObject::Cp()
{
	return FPanels;
}

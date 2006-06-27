/*
ctrlobj.cpp

���������� ���������� ���������, ������� ��������� ���������� � ����

*/

/* Revision: 1.62 21.05.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "ctrlobj.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "grpsort.hpp"
#include "poscache.hpp"
#include "history.hpp"
#include "treelist.hpp"
#include "filter.hpp"
#include "filepanels.hpp"

ControlObject *CtrlObject;

ControlObject::ControlObject()
{
  _OT(SysLog("[%p] ControlObject::ControlObject()", this));
  FPanels=0;
  CtrlObject=this;
  /* $ 06.05.2001 DJ
     ������� ����������� (��� ���������� dependencies)
  */
  HiFiles = new HighlightFiles;
  GrpSort = new GroupSort;
  ViewerPosCache = new FilePositionCache(FPOSCACHE_64);
  EditorPosCache = new FilePositionCache(FPOSCACHE_32);
  FrameManager = new Manager;
  /* DJ $ */
  ReadConfig();
  /* $ 28.02.2001 IS
       �������� ����������� ������ ����� ����, ��� ��������� ���������
  */
  CmdLine=new CommandLine;
  /* IS $ */

  CmdHistory=new HistoryW(HISTORYTYPE_CMD,Opt.HistoryCount,L"SavedHistory",&Opt.SaveHistory,FALSE,FALSE);
  FolderHistory=new HistoryW(HISTORYTYPE_FOLDER,Opt.FoldersHistoryCount,L"SavedFolderHistory",&Opt.SaveFoldersHistory,FALSE,TRUE);
  ViewHistory=new HistoryW(HISTORYTYPE_VIEW,Opt.ViewHistoryCount,L"SavedViewHistory",&Opt.SaveViewHistory,TRUE,TRUE);

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
  PanelFilter::InitFilter();

  SetColor(F_LIGHTGRAY|B_BLACK);
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
    TextW(MShareware);

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

  RegistrationBugs=FALSE;
#ifdef _DEBUGEXC
  if(CheckRegistration)
#endif
    if(_beginthread(CheckVersion,0x10000,NULL) == -1)
    {
      RegistrationBugs=TRUE;
      CheckVersion(NULL);
    }

  Cp()->LeftPanel->Update(0);
  Cp()->RightPanel->Update(0);
  /* $ 07.09.2000 tran
    + Config//Current File */
  if (Opt.AutoSaveSetup)
  {
      Cp()->LeftPanel->GoToFileW(Opt.strLeftCurFile);
      Cp()->RightPanel->GoToFileW(Opt.strRightCurFile);
  }
  /* tran 07.09.2000 $ */

  FrameManager->InsertFrame(FPanels);

  string strStartCurDir;
  Cp()->ActivePanel->GetCurDirW(strStartCurDir);
  FarChDirW(strStartCurDir, TRUE);
  Cp()->ActivePanel->SetFocus();

  {
    string strOldTitle;
    apiGetConsoleTitle (strOldTitle);
    FrameManager->PluginCommit();
    Plugins.LoadPlugins();
    SetConsoleTitleW(strOldTitle);
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
  if(CriticalInternalError)
    return;

  _OT(SysLog("[%p] ControlObject::~ControlObject()", this));
  if (Cp()&&Cp()->ActivePanel!=NULL)
  {
    if (Opt.AutoSaveSetup)
      SaveConfig(0);
    if (Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
    {
      string strCurDir;
      Cp()->ActivePanel->GetCurDirW(strCurDir);
      FolderHistory->AddToHistory(strCurDir,NULL,0);
    }
  }

  FrameManager->CloseAll();
  FPanels=NULL;

  Plugins.SendExit();
  PanelFilter::CloseFilter();
  delete CmdHistory;
  delete FolderHistory;
  delete ViewHistory;
  delete CmdLine;
  /* $ 06.05.2001 DJ
     ������� ��, ��� ������� �����������
  */
  delete HiFiles;
  delete GrpSort;

  if (Opt.ViOpt.SaveViewerPos)
    ViewerPosCache->Save(L"Viewer\\LastPositions");
  delete ViewerPosCache;

  if (Opt.EdOpt.SavePos)
    EditorPosCache->Save(L"Editor\\LastPositions");

  delete EditorPosCache;
  delete FrameManager;
  /* DJ $ */

  SIDCacheFlush();
  Lang.Close();
  CtrlObject=NULL;
}


/* $ 25.11.2000 SVS
   Copyright � 2 ������
*/
/* $ 15.12.2000 SVS
 ����� ShowCopyright - public static & �������� Flags.
*/
void ControlObject::ShowCopyright(DWORD Flags)
{
  char Str[256];
  char *Line2=NULL;
  strcpy(Str,Copyright);
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
  if(Flags&1)
  {
    fprintf(stderr,"%s\n%s\n",Str,Line2);
  }
  else
  {
#ifdef BETA
    mprintf("Beta version %d.%02d.%d",BETA/1000,(BETA%1000)/10,BETA%10);
#else
    ScrollScreen(2+Line2?1:0);
    if(Line2)
    {
      GotoXY(0,ScrY-4);
      Text(Str);
      GotoXY(0,ScrY-3);
      Text(Line2);
    }
    else
      Text(Str);
#endif
  }
}
/* SVS $ */
/* SVS $ */


FilePanels* ControlObject::Cp()
{
  return FPanels;
}

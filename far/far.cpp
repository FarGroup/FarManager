#include "const.hpp"
#include "plugin.hpp"
#include "classes.hpp"
#include "fn.hpp"
#include "lang.hpp"

#include "global.cpp"
#include "int64.cpp"
#include "namelist.cpp"
#include "iswind.cpp"
#include "history.cpp"
#include "checkver.cpp"
#include "scrbuf.cpp"
#include "macro.cpp"
#include "plugins.cpp"
#include "manager.cpp"
#include "poscache.cpp"
#include "grabber.cpp"
#include "lang.cpp"
#include "palette.cpp"
#include "dizlist.cpp"
#include "grpsort.cpp"
#include "print.cpp"
#include "gettable.cpp"
#include "foldtree.cpp"
#include "ffolders.cpp"
#include "help.cpp"
#include "config.cpp"
#include "registry.cpp"
#include "setcolor.cpp"
#include "ctrlobj.cpp"
#include "scrobj.cpp"
#include "panel.cpp"
#include "scantree.cpp"
#include "cmdline.cpp"
#include "edit.cpp"
#include "modal.cpp"
#include "hmenu.cpp"
#include "vmenu.cpp"
#include "keybar.cpp"
#include "menubar.cpp"
#include "interf.cpp"
#include "savescr.cpp"
#include "local.cpp"
#include "mix.cpp"
#include "message.cpp"
#include "mkdir.cpp"
#include "delete.cpp"
#include "setattr.cpp"
#include "options.cpp"
#include "scrsaver.cpp"
#include "fileview.cpp"
#include "fileedit.cpp"
#include "smallobj.cpp"

static void ConvertOldSettings();
static void SetHighlighting();
static void CopyGlobalSettings();

int _cdecl main(int Argc, char *Argv[])
{
  char EditName[NM],ViewName[NM],DestName[NM];
  int StartLine=-1,StartChar=-1,RegOpt=FALSE;
  *EditName=*ViewName=*DestName=0;
  CmdMode=FALSE;

  strcpy(Opt.RegRoot,"Software\\Far");

  for (int I=1;I<Argc;I++)
    if ((Argv[I][0]=='/' || Argv[I][0]=='-') && Argv[I][1])
      switch(toupper(Argv[I][1]))
      {
        case 'A':
          switch (toupper(Argv[I][2]))
          {
            case 0:
              Opt.CleanAscii=TRUE;
              break;
            case 'G':
              Opt.NoGraphics=TRUE;
              break;
          }
          break;
        case 'E':
          if (isdigit(Argv[I][2]))
          {
            StartLine=atoi(&Argv[I][2]);
            char *ChPtr=strchr(&Argv[I][2],':');
            if (ChPtr!=NULL)
              StartChar=atoi(ChPtr+1);
          }
          if (I+1<Argc)
          {
            CharToOem(Argv[I+1],EditName);
            I++;
          }
          break;
        case 'V':
          if (I+1<Argc)
          {
            CharToOem(Argv[I+1],ViewName);
            I++;
          }
          break;
        case 'R':
          RegOpt=TRUE;
          break;
        case 'I':
          Opt.SmallIcon=TRUE;
          break;
        case 'U':
          if (I+1<Argc)
          {
            strcat(Opt.RegRoot,"\\Users\\");
            strcat(Opt.RegRoot,Argv[I+1]);
            CopyGlobalSettings();
            I++;
          }
          break;
      }
    else
      CharToOem(Argv[I],DestName);

  WaitForInputIdle(GetCurrentProcess(),0);
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));

  set_new_handler(0);

  SetFileApisToOEM();
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
  LocalUpperInit();
  GetModuleFileName(NULL,FarPath,sizeof(FarPath));
  *PointToName(FarPath)=0;
  InitDetectWindowedMode();
  InitConsole();
  GetRegKey("Language","Main",Opt.Language,"English",sizeof(Opt.Language));
  if (!Lang.Init(FarPath))
  {
    Message(MSG_WARNING,1,"Error","Cannot load language data","Ok");
    exit(0);
  }
  ConvertOldSettings();
  SetHighlighting();
  {
    ChangePriority ChPriority(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? THREAD_PRIORITY_ABOVE_NORMAL:THREAD_PRIORITY_NORMAL);
    ControlObject CtrlObj;
    if (*EditName || *ViewName)
    {
      Panel *DummyPanel=new Panel;
      CmdMode=TRUE;
      CtrlObj.LeftPanel=CtrlObj.RightPanel=CtrlObj.ActivePanel=DummyPanel;
      CtrlObj.Plugins.LoadPlugins();
      if (*EditName)
        FileEditor ShellEditor(EditName,TRUE,FALSE,StartLine,StartChar);
      if (*ViewName)
        FileViewer ShellViewer(ViewName,FALSE);
      delete DummyPanel;
      CtrlObj.LeftPanel=CtrlObj.RightPanel=CtrlObj.ActivePanel=NULL;
    }
    else
    {
      if (RegOpt)
        Register();
      static struct RegInfo Reg;
      _beginthread(CheckReg,0x10000,&Reg);
      while (!Reg.Done)
        Sleep(0);
      CtrlObj.Init();
      if (*DestName)
      {
        LockScreen LockScr;
        char Path[NM];
        strcpy(Path,DestName);
        *PointToName(Path)=0;
        Panel *ActivePanel=CtrlObject->ActivePanel;
        if (*Path)
          ActivePanel->SetCurDir(Path,TRUE);
        strcpy(Path,PointToName(DestName));
        if (*Path)
        {
          if (ActivePanel->GoToFile(Path))
            ActivePanel->ProcessKey(KEY_CTRLPGDN);
        }
        CtrlObject->LeftPanel->Redraw();
        CtrlObject->RightPanel->Redraw();
      }
      CtrlObj.EnterMainLoop();
    }
  }
  SetConsoleTitle(OldTitle);
  CloseConsole();
  RestoreIcons();
  return(0);
}


void ConvertOldSettings()
{
}


void SetHighlighting()
{
  if (CheckRegKey("Highlight"))
    return;
  SetRegKey("Highlight\\Group0","Mask","*.*");
  SetRegKey("Highlight\\Group0","IncludeAttributes",2);
  SetRegKey("Highlight\\Group0","NormalColor",19);
  SetRegKey("Highlight\\Group0","CursorColor",56);
  SetRegKey("Highlight\\Group1","Mask","*.*");
  SetRegKey("Highlight\\Group1","IncludeAttributes",4);
  SetRegKey("Highlight\\Group1","NormalColor",19);
  SetRegKey("Highlight\\Group1","CursorColor",56);
  SetRegKey("Highlight\\Group2","Mask","*.*");
  SetRegKey("Highlight\\Group2","IncludeAttributes",16);
  SetRegKey("Highlight\\Group2","NormalColor",31);
  SetRegKey("Highlight\\Group2","CursorColor",63);
  SetRegKey("Highlight\\Group3","Mask","*.exe,*.com,*.bat,*.cmd");
  SetRegKey("Highlight\\Group3","NormalColor",26);
  SetRegKey("Highlight\\Group3","CursorColor",58);
  SetRegKey("Highlight\\Group4","Mask","*.rar,*.arj,*.zip,*.lzh");
  SetRegKey("Highlight\\Group4","NormalColor",29);
  SetRegKey("Highlight\\Group4","CursorColor",61);
  SetRegKey("Highlight\\Group5","Mask","*.bak,*.tmp");
  SetRegKey("Highlight\\Group5","NormalColor",22);
  SetRegKey("Highlight\\Group5","CursorColor",54);
}


void CopyGlobalSettings()
{
  if (CheckRegKey(""))
    return;
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  CopyKeyTree("Software\\Far",Opt.RegRoot,"Software\\Far\\Users\0");
  SetRegRootKey(HKEY_CURRENT_USER);
  CopyKeyTree("Software\\Far",Opt.RegRoot,"Software\\Far\\Users\0Software\\Far\\PluginsCache\0");
}

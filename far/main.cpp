/*
far.cpp

Функция main.

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif
#ifndef __STDIO_H
#include <stdio.h>
#endif
#ifndef __TIME_H
#include <time.h>
#endif
#ifndef __PROCESS_H
#include <process.h>
#endif
#ifndef __NEW_H
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif

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

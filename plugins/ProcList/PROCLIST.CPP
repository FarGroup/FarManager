#define STRICT

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include "proclng.hpp"
#include "d:\lang\bc5\far\plugin.hpp"
#include "proclist.hpp"

#include "plist95.cpp"
#include "plistnt.cpp"
#include "pclass.cpp"
#include "pmix.cpp"
#include "preg.cpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  OSVERSIONINFO WinVer;
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
  NT=(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT);
  ::Info=*Info;
  sprintf(PluginRootKey,"%s\\Plist",Info->RootKey);
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  HANDLE hPlugin=new Plist;
  return(hPlugin==NULL ? INVALID_HANDLE_VALUE:hPlugin);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (Plist *)hPlugin;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  Plist *Panel=(Plist *)hPlugin;
  return(Panel->GetFindData(pPanelItem,pItemsNumber,OpMode));
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  Plist *Panel=(Plist *)hPlugin;
  Panel->FreeFindData(PanelItem,ItemsNumber);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MPlistPanel);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  Plist *Panel=(Plist *)hPlugin;
  Panel->GetOpenPluginInfo(Info);
}


int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  Plist *Panel=(Plist *)hPlugin;
  return(Panel->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode));
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int OpMode)
{
  Plist *Panel=(Plist *)hPlugin;
  return(Panel->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  Plist *Panel=(Plist *)hPlugin;
  return(Panel->ProcessEvent(Event,Param));
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  Plist *Panel=(Plist *)hPlugin;
  return(Panel->ProcessKey(Key,ControlState));
}

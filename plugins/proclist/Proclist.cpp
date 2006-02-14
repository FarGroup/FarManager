#include "proclist.hpp"
#include "proclng.hpp"

#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>

_Opt Opt;

PluginStartupInfo Info;
FarStandardFunctions FSF;
int NT, W2K;
char PluginRootKey[80];

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  OSVERSIONINFO WinVer;
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
  NT = (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT);
  W2K = NT && (WinVer.dwMajorVersion > 4);
  ::Info = *Info;
  FSF = *Info->FSF;
  ::Info.FSF = &FSF;
  FSF.sprintf(PluginRootKey,"%s\\Plist",Info->RootKey);
  Opt.Read();
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  Opt.Read();
  Plist* hPlugin = new Plist();

  if(OpenFrom==OPEN_COMMANDLINE && (*(short*)Item==0x5c5c||*(short*)Item==0x2f2f)) {
      if(!hPlugin->Connect((char*)Item)) {
          delete hPlugin;
          hPlugin = (Plist*)INVALID_HANDLE_VALUE;
      }
  }
  return hPlugin;
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (Plist *)hPlugin;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **ppPanelItem,int *pItemsNumber,int OpMode)
{
  Plist *Panel=(Plist *)hPlugin;
  return Panel->GetFindData(*ppPanelItem,*pItemsNumber,OpMode);
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
  if(Opt.AddToPluginsMenu)
  {
    static char *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(MPlistPanel);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  }

  static char *DiskMenuStrings[1];
  static int DiskMenuNumbers[1];
  DiskMenuStrings[0]=GetMsg(MPlistPanel);
  DiskMenuNumbers[0]=Opt.DisksMenuDigit;
  Info->DiskMenuNumbers=DiskMenuNumbers;
  Info->DiskMenuStrings=DiskMenuStrings;
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;

  static char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MPlistPanel);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
  Info->CommandPrefix = "plist";
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  Plist *Panel=(Plist *)hPlugin;
  Panel->GetOpenPluginInfo(Info);
}


int WINAPI _export GetFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  return ((Plist *)hPlugin)->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode);
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,
                   int ItemsNumber,int OpMode)
{
  return ((Plist *)hPlugin)->DeleteFiles(PanelItem,ItemsNumber,OpMode);
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  return ((Plist *)hPlugin)->ProcessEvent(Event,Param);
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  return ((Plist *)hPlugin)->ProcessKey(Key,ControlState);
}

int WINAPI _export Configure(int ItemNumber)
{
  return Config();
}
int WINAPI _export Compare (HANDLE hPlugin, const struct PluginPanelItem *Item1, const struct PluginPanelItem *Item2, unsigned int Mode)
{
   return ((Plist *)hPlugin)->Compare(Item1, Item2, Mode);
}

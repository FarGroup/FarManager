#include <windows.h>
#include <stdio.h>
#include <dos.h>
#include "netlng.hpp"
#include "d:\lang\bc5\far\plugin.hpp"
#include "network.hpp"

#include "netclass.cpp"
#include "netmix.cpp"
#include "netreg.cpp"
#include "netcfg.cpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  sprintf(PluginRootKey,"%s\\Network",Info->RootKey);
  Opt.AddToDisksMenu=GetRegKey(HKEY_CURRENT_USER,"","AddToDisksMenu",1);
  Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,"","DisksMenuDigit",3);
  Opt.SkipTopLevel=GetRegKey(HKEY_CURRENT_USER,"","SkipTopLevel",0);
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  HANDLE hPlugin=new NetBrowser;
  if (hPlugin==NULL)
    return(INVALID_HANDLE_VALUE);
  return(hPlugin);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (NetBrowser *)hPlugin;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->GetFindData(pPanelItem,pItemsNumber,OpMode));
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  Browser->FreeFindData(PanelItem,ItemsNumber);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  static char *DiskMenuStrings[1];
  DiskMenuStrings[0]=GetMsg(MDiskMenuString);
  static int DiskMenuNumbers[1];
  Info->DiskMenuStrings=DiskMenuStrings;
  DiskMenuNumbers[0]=Opt.DisksMenuDigit;
  Info->DiskMenuNumbers=DiskMenuNumbers;
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MNetMenu);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  static char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MNetMenu);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  Browser->GetOpenPluginInfo(Info);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->SetDirectory(Dir,OpMode));
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->ProcessKey(Key,ControlState));
}


int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}


void WINAPI _export ExitFAR()
{
  NetBrowser::DeleteCommonRootResources();
}


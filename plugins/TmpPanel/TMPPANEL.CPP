#include <windows.h>
#include <stdio.h>
#include <dos.h>
#include "tmplng.hpp"
#include "d:\lang\bc5\far\plugin.hpp"
#include "tmppanel.hpp"

#include "tmpclass.cpp"
#include "tmpmix.cpp"
#include "tmpreg.cpp"
#include "tmpcfg.cpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  sprintf(PluginRootKey,"%s\\TmpPanel",Info->RootKey);
  Opt.AddToDisksMenu=GetRegKey(HKEY_CURRENT_USER,"","AddToDisksMenu",1);
  Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,"","DisksMenuDigit",1);
  Opt.CommonPanel=GetRegKey(HKEY_CURRENT_USER,"","CommonPanel",1);
  CommonTmpPanelItem=new PluginPanelItem;
  CommonTmpItemsNumber=0;
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  HANDLE hPlugin=new TmpPanel;
  return(hPlugin==NULL ? INVALID_HANDLE_VALUE:hPlugin);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (TmpPanel *)hPlugin;
}


void WINAPI _export ExitFAR()
{
  for (int I=0;I<CommonTmpItemsNumber;I++)
    if (CommonTmpPanelItem[I].Owner)
      delete CommonTmpPanelItem[I].Owner;
  delete CommonTmpPanelItem;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->GetFindData(pPanelItem,pItemsNumber,OpMode));
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
  PluginMenuStrings[0]=GetMsg(MTempPanel);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  static char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MTempPanel);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  Panel->GetOpenPluginInfo(Info);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->SetDirectory(Dir,OpMode));
}


int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->PutFiles(PanelItem,ItemsNumber,Move,OpMode));
}


int WINAPI _export SetFindList(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->SetFindList(PanelItem,ItemsNumber));
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->ProcessEvent(Event,Param));
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->ProcessKey(Key,ControlState));
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

#include <windows.h>
#include "d:\lang\bc5\far\plugin.hpp"
#include "filelng.hpp"
#include "filecase.hpp"
#include "filemix.cpp"
#include "filecvt.cpp"
#include "filereg.cpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  strcpy(PluginRootKey,Info->RootKey);
  strcat(PluginRootKey,"\\CaseConvertion");
  Opt.ConvertMode=GetRegKey(HKEY_CURRENT_USER,"","ConvertMode",0);
  Opt.SkipMixedCase=GetRegKey(HKEY_CURRENT_USER,"","SkipMixedCase",1);
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  CaseConvertion();
  return(INVALID_HANDLE_VALUE);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  Info->DiskMenuStringsNumber=0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MFileCase);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}




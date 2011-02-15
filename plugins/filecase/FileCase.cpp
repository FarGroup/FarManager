#include "plugin.hpp"
#include "CRT/crt.hpp"
#include "FileLng.hpp"
#include "FileCase.hpp"
#include "pluginreg/pluginreg.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

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
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

#include "FileMix.cpp"
#include "filecvt.cpp"
#include "ProcessName.cpp"

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI ExitFARW()
{
	free(PluginRootKey);
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  PluginRootKey = (wchar_t *)malloc(lstrlen(Info->RootKey)*sizeof(wchar_t) + sizeof(L"\\CaseConvertion"));
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,L"\\CaseConvertion");

  Opt.ConvertMode=GetRegKey(L"",L"ConvertMode",0);
  Opt.ConvertModeExt=GetRegKey(L"",L"ConvertModeExt",0);
  Opt.SkipMixedCase=GetRegKey(L"",L"SkipMixedCase",1);
  Opt.ProcessSubDir=GetRegKey(L"",L"ProcessSubDir",0);
  Opt.ProcessDir=GetRegKey(_T(""),L"ProcessDir",0);
  GetRegKey(L"",L"WordDiv",Opt.WordDiv,L" _",ARRAYSIZE(Opt.WordDiv));
  Opt.WordDivLen=lstrlen(Opt.WordDiv);
}


HANDLE WINAPI OpenPluginW(int OpenFrom, const GUID* Guid, INT_PTR Item)
{
  CaseConvertion();
  return INVALID_HANDLE_VALUE;
}


void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  static const wchar_t *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MFileCase);
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

#include <plugin.hpp>
#include <CRT/crt.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "FileLng.hpp"
#include "FileCase.hpp"
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

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

struct Options
{
  int ConvertMode;
  int ConvertModeExt;
  int SkipMixedCase;
  int ProcessSubDir;
  int ProcessDir;
  wchar_t WordDiv[512];
  int WordDivLen;
} Opt;

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

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  PluginSettings settings(MainGuid, ::Info.SettingsControl);
  Opt.ConvertMode=settings.Get(0,L"ConvertMode",0);
  Opt.ConvertModeExt=settings.Get(0,L"ConvertModeExt",0);
  Opt.SkipMixedCase=settings.Get(0,L"SkipMixedCase",1);
  Opt.ProcessSubDir=settings.Get(0,L"ProcessSubDir",0);
  Opt.ProcessDir=settings.Get(0,L"ProcessDir",0);
  settings.Get(0,L"WordDiv",Opt.WordDiv,ARRAYSIZE(Opt.WordDiv),L" _");
  Opt.WordDivLen=lstrlen(Opt.WordDiv);
}


HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
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

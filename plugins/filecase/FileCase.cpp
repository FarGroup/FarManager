#include <windows.h>
#define _FAR_NO_NAMELESS_UNIONS
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "filelng.hpp"
#include "filecase.hpp"

#if defined(__GNUC__)

#include "CRT/crt.hpp"

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

#include "filemix.cpp"
#include "filecvt.cpp"
#include "filereg.cpp"
#include "ProcessName.cpp"

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  IsOldFar=TRUE;
  if(Info->StructSize >= (int)sizeof(struct PluginStartupInfo))
  {
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    IsOldFar=FALSE;

    lstrcpy(PluginRootKey,Info->RootKey);
    lstrcat(PluginRootKey,"\\CaseConvertion");

    Opt.ConvertMode=GetRegKey(HKEY_CURRENT_USER,"","ConvertMode",0);
    Opt.ConvertModeExt=GetRegKey(HKEY_CURRENT_USER,"","ConvertModeExt",0);
    Opt.SkipMixedCase=GetRegKey(HKEY_CURRENT_USER,"","SkipMixedCase",1);
    Opt.ProcessSubDir=GetRegKey(HKEY_CURRENT_USER,"","ProcessSubDir",0);
    Opt.ProcessDir=GetRegKey(HKEY_CURRENT_USER,"","ProcessDir",0);
    GetRegKey(HKEY_CURRENT_USER,"","WordDiv",Opt.WordDiv," _",sizeof(Opt.WordDiv));
    Opt.WordDivLen=lstrlen(Opt.WordDiv);
  }
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,INT_PTR Item)
{
  if(!IsOldFar)
    CaseConvertion();
  return(INVALID_HANDLE_VALUE);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFar)
  {
    Info->StructSize=sizeof(*Info);
    static char *PluginMenuStrings[1];
    PluginMenuStrings[0]=(char*)GetMsg(MFileCase);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  }
}

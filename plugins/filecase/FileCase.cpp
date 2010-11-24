#include <windows.h>
#define _FAR_NO_NAMELESS_UNIONS
#include "CRT/crt.hpp"
#include "plugin.hpp"
#include "FileLng.hpp"
#include "FileCase.hpp"

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
#include "FileReg.cpp"
#include "ProcessName.cpp"

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}


void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;

	lstrcpy(PluginRootKey,Info->RootKey);
	lstrcat(PluginRootKey,_T("\\CaseConvertion"));

	Opt.ConvertMode=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("ConvertMode"),0);
	Opt.ConvertModeExt=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("ConvertModeExt"),0);
	Opt.SkipMixedCase=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("SkipMixedCase"),1);
	Opt.ProcessSubDir=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("ProcessSubDir"),0);
	Opt.ProcessDir=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("ProcessDir"),0);
	GetRegKey(HKEY_CURRENT_USER,_T(""),_T("WordDiv"),Opt.WordDiv,_T(" _"),ARRAYSIZE(Opt.WordDiv));
	Opt.WordDivLen=lstrlen(Opt.WordDiv);
}


HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  CaseConvertion();
  return(INVALID_HANDLE_VALUE);
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	static TCHAR *PluginMenuStrings[1];
	PluginMenuStrings[0]=(TCHAR*)GetMsg(MFileCase);
	Info->PluginMenuStrings=PluginMenuStrings;
	Info->PluginMenuStringsNumber=ARRAYSIZE(PluginMenuStrings);
}

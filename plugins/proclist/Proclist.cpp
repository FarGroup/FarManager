#include "proclist.hpp"
#include "proclng.hpp"

#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>

#ifndef UNICODE
#define EXP_NAME(p) _export p
#else
#define EXP_NAME(p) _export p ## W
#endif

_Opt Opt;
ui64Table *_ui64Table;

PluginStartupInfo Info;
FarStandardFunctions FSF;
int NT, W2K;
TCHAR PluginRootKey[80];

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HMODULE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HMODULE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  if(dwReason == DLL_PROCESS_ATTACH)
      DisableThreadLibraryCalls(hDll);
  return TRUE;
}

#ifndef __GNUC__
#pragma comment(linker, "/ENTRY:DllMainCRTStartup")
// for ulink
#pragma comment(linker, "/alternatename:DllMainCRTStartup=_DllMainCRTStartup@12")
// for ms-link BUG
#ifndef _M_AMD64
void __cdecl main(void) {}
#endif
#endif

//-----------------------------------------------------------------------------
static LONG WINAPI fNtQueryInformationProcess(
                              HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryInformationProcess pNtQueryInformationProcess = fNtQueryInformationProcess;

static LONG WINAPI fNtQueryInformationThread(HANDLE, ULONG, PVOID, DWORD, DWORD*)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryInformationThread pNtQueryInformationThread = fNtQueryInformationThread;

static LONG WINAPI fNtQueryObject(HANDLE, DWORD, VOID*, DWORD, VOID*)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryObject pNtQueryObject = fNtQueryObject;

static LONG WINAPI fNtQuerySystemInformation(DWORD, VOID*, DWORD, ULONG*)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQuerySystemInformation pNtQuerySystemInformation = fNtQuerySystemInformation;

static LONG WINAPI fNtQueryInformationFile(HANDLE, PVOID, PVOID, DWORD, DWORD)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryInformationFile pNtQueryInformationFile = fNtQueryInformationFile;


static BOOL WINAPI fIsWow64Process(HANDLE, PBOOL) { return FALSE; }
PIsWow64Process pIsWow64Process = fIsWow64Process;


static DWORD WINAPI fGetGuiResources(HANDLE, DWORD) { return 0; }
PGetGuiResources pGetGuiResources = fGetGuiResources;


static BOOL WINAPI fIsValidSid(PSID) { return FALSE; }
PIsValidSid pIsValidSid = fIsValidSid;

static PSID_IDENTIFIER_AUTHORITY WINAPI fGetSidIdentifierAuthority(PSID)
{ return NULL; }
PGetSidIdentifierAuthority pGetSidIdentifierAuthority = fGetSidIdentifierAuthority;

static PUCHAR WINAPI fGetSidSubAuthorityCount(PSID) { return NULL; }
PGetSidSubAuthorityCount pGetSidSubAuthorityCount = fGetSidSubAuthorityCount;

static PDWORD WINAPI fGetSidSubAuthority(PSID, DWORD) { return NULL; }
PGetSidSubAuthority pGetSidSubAuthority = fGetSidSubAuthority;

static BOOL WINAPI fLookupAccountName(LPCTSTR,LPCTSTR,PSID,LPDWORD,LPTSTR,LPDWORD,PSID_NAME_USE)
{ return FALSE; }
PLookupAccountName pLookupAccountName = fLookupAccountName;


static HRESULT WINAPI fCoInitializeSecurity(
              PSECURITY_DESCRIPTOR, LONG, SOLE_AUTHENTICATION_SERVICE*, void*,
              DWORD, DWORD, SOLE_AUTHENTICATION_LIST*, DWORD, void*)
{ return E_FAIL; }
PCoInitializeSecurity pCoInitializeSecurity = fCoInitializeSecurity;

#ifndef UNICODE
#define FUNC_AW_SUFFIX  "A"
#else
#define FUNC_AW_SUFFIX  "W"
#endif
static void dynamic_bind(void)
{
  static BOOL Inited;
  if (!Inited) {
    HMODULE h;
    FARPROC f;

    if ((h = GetModuleHandle(_T("ntdll"))) != NULL) {
      if ((f = GetProcAddress(h, "NtQueryInformationProcess")) != NULL)
          *(FARPROC*)&pNtQueryInformationProcess = f;
      if ((f = GetProcAddress(h, "NtQueryInformationThread")) != NULL)
          *(FARPROC*)&pNtQueryInformationThread = f;
      if ((f = GetProcAddress(h, "NtQueryObject")) != NULL)
          *(FARPROC*)&pNtQueryObject = f;
      if ((f = GetProcAddress(h, "NtQuerySystemInformation")) != NULL)
          *(FARPROC*)&pNtQuerySystemInformation = f;
      if ((f = GetProcAddress(h, "NtQueryInformationFile")) != NULL)
          *(FARPROC*)&pNtQueryInformationFile = f;
    }
    if ((h = GetModuleHandle(_T("kernel32"))) != NULL) {
      if ((f = GetProcAddress(h, "IsWow64Process")) != NULL)
          *(FARPROC*)&pIsWow64Process = f;
    }
    if ((h = GetModuleHandle(_T("advapi32"))) != NULL) {
      if ((f = GetProcAddress(h, "IsValidSid")) != NULL)
          *(FARPROC*)&pIsValidSid = f;
      if ((f = GetProcAddress(h, "GetSidIdentifierAuthority")) != NULL)
          *(FARPROC*)&pGetSidIdentifierAuthority = f;
      if ((f = GetProcAddress(h, "GetSidSubAuthorityCount")) != NULL)
          *(FARPROC*)&pGetSidSubAuthorityCount = f;
      if ((f = GetProcAddress(h, "GetSidSubAuthority")) != NULL)
          *(FARPROC*)&pGetSidSubAuthority = f;
      if ((f = GetProcAddress(h, "LookupAccountName" FUNC_AW_SUFFIX)) != NULL)
          *(FARPROC*)&pLookupAccountName = f;
    }
    if ((h = GetModuleHandle(_T("user32"))) != NULL) {
      if ((f = GetProcAddress(h, "GetGuiResources")) != NULL)
          *(FARPROC*)&pGetGuiResources = f;
    }
    if ((h = GetModuleHandle(_T("ole32"))) != NULL) {
      if ((f = GetProcAddress(h, "CoInitializeSecurity")) != NULL)
          *(FARPROC*)&pCoInitializeSecurity = f;
    }
    Inited = TRUE;
  }
}
#undef FUNC_AW_SUFFIX

//----------------------------------------------------------------------------

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  dynamic_bind();
  OSVERSIONINFO WinVer;
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
  NT = (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT);
  W2K = NT && (WinVer.dwMajorVersion > 4);
  ::Info = *Info;
  FSF = *Info->FSF;
  ::Info.FSF = &FSF;
  FSF.sprintf(PluginRootKey,_T("%s\\Plist"),Info->RootKey);
  _ui64Table = new ui64Table;
  Opt.Read();
}


void WINAPI EXP_NAME(ExitFAR)()
{
  delete _ui64Table;
}


HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  Opt.Read();
  Plist* hPlugin = new Plist();

  if(OpenFrom==OPEN_COMMANDLINE && (NORM_M_PREFIX(Item) || REV_M_PREFIX(Item)))
  {
      if(!hPlugin->Connect((TCHAR*)Item)) {
          delete hPlugin;
          hPlugin = (Plist*)INVALID_HANDLE_VALUE;
      }
  }
  return hPlugin;
}


void WINAPI EXP_NAME(ClosePlugin)(HANDLE hPlugin)
{
  delete (Plist *)hPlugin;
}


int WINAPI EXP_NAME(GetFindData)(HANDLE hPlugin,struct PluginPanelItem **ppPanelItem,int *pItemsNumber,int OpMode)
{
  Plist *Panel=(Plist *)hPlugin;
  return Panel->GetFindData(*ppPanelItem,*pItemsNumber,OpMode);
}


void WINAPI EXP_NAME(FreeFindData)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  Plist *Panel=(Plist *)hPlugin;
  Panel->FreeFindData(PanelItem,ItemsNumber);
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  if(Opt.AddToPluginsMenu)
  {
    static TCHAR *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(MPlistPanel);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=ArraySize(PluginMenuStrings);
  }

  static TCHAR *DiskMenuStrings[1];
  static int DiskMenuNumbers[1];
  DiskMenuStrings[0]=GetMsg(MPlistPanel);
  DiskMenuNumbers[0]=Opt.DisksMenuDigit;
  Info->DiskMenuNumbers=DiskMenuNumbers;
  Info->DiskMenuStrings=DiskMenuStrings;
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;

  static TCHAR *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MPlistPanel);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=ArraySize(PluginCfgStrings);
  Info->CommandPrefix = _T("plist");
}


void WINAPI EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  Plist *Panel=(Plist *)hPlugin;
  Panel->GetOpenPluginInfo(Info);
}


int WINAPI EXP_NAME(GetFiles)(HANDLE hPlugin,PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,WCONST TCHAR *DestPath,int OpMode)
{
  return ((Plist *)hPlugin)->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode);
}


int WINAPI EXP_NAME(DeleteFiles)(HANDLE hPlugin,PluginPanelItem *PanelItem,
                   int ItemsNumber,int OpMode)
{
  return ((Plist *)hPlugin)->DeleteFiles(PanelItem,ItemsNumber,OpMode);
}


int WINAPI EXP_NAME(ProcessEvent)(HANDLE hPlugin,int Event,void *Param)
{
  return ((Plist *)hPlugin)->ProcessEvent(Event,Param);
}


int WINAPI EXP_NAME(ProcessKey)(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  return ((Plist *)hPlugin)->ProcessKey(Key,ControlState);
}

int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  return Config();
}
int WINAPI EXP_NAME(Compare)(HANDLE hPlugin, const struct PluginPanelItem *Item1, const struct PluginPanelItem *Item2, unsigned int Mode)
{
   return ((Plist *)hPlugin)->Compare(Item1, Item2, Mode);
}

#include "proclist.hpp"
#include "proclng.hpp"

#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>

_Opt Opt;
ui64Table *_ui64Table;

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

#define FUNC_AW_SUFFIX  "A"
static void dynamic_bind(void)
{
  static BOOL Inited;
  if (!Inited) {
    HMODULE h;
    FARPROC f;

    if ((h = GetModuleHandle("ntdll")) != NULL) {
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
    if ((h = GetModuleHandle("kernel32")) != NULL) {
      if ((f = GetProcAddress(h, "IsWow64Process")) != NULL)
          *(FARPROC*)&pIsWow64Process = f;
    }
    if ((h = GetModuleHandle("advapi32")) != NULL) {
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
    if ((h = GetModuleHandle("user32")) != NULL) {
      if ((f = GetProcAddress(h, "GetGuiResources")) != NULL)
          *(FARPROC*)&pGetGuiResources = f;
    }
    if ((h = GetModuleHandle("ole32")) != NULL) {
      if ((f = GetProcAddress(h, "CoInitializeSecurity")) != NULL)
          *(FARPROC*)&pCoInitializeSecurity = f;
    }
    Inited = TRUE;
  }
}
#undef FUNC_AW_SUFFIX

//----------------------------------------------------------------------------

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
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
  FSF.sprintf(PluginRootKey,"%s\\Plist",Info->RootKey);
  _ui64Table = new ui64Table;
  Opt.Read();
}


void WINAPI _export ExitFAR()
{
  delete _ui64Table;
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,INT_PTR Item)
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

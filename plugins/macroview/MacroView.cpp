#define _FAR_NO_NAMELESS_UNIONS
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "CRT/crt.hpp"

#include "macrolng.hpp"
#include "macroview.hpp"
#include "farcolor.hpp"
#include "farkeys.hpp"
#include "macromix.cpp"
#include "strclass.cpp"
#include "regclass.cpp"
#include "config.cpp"
#include "macrodiff.cpp"

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

int WINAPI _export GetMinFarVersion()
{
  return MAKEFARVERSION(1,70,1810);
}

#if defined(__BORLANDC__)
  #pragma argsused
#endif
void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  hInstance=GetModuleHandle(NULL);
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  vi.dwOSVersionInfoSize=sizeof(vi);
  GetVersionEx(&vi);

  wsprintf(PluginRootKey,"%s\\%s",Info->RootKey,Module_KEY);
  CheckFirstBackSlash(PluginRootKey,TRUE);
  const char *ptr=strstr(Info->RootKey,Plugins_KEY);
  if (ptr)
    lstrcpyn(FarKey,Info->RootKey,(int)(ptr-Info->RootKey)+1);
  else
    lstrcpyn(FarKey,Default_KEY,sizeof(FarKey));

  CheckFirstBackSlash(FarKey,TRUE);

  ptr=strstr(FarKey,Users_KEY);
  if (ptr)
  {
    ptr=strrchr(FarKey,'\\');
    if (ptr)
      lstrcpyn(FarUserName,ptr+1,lstrlen(ptr+1)+1);
    else
      *FarUserName=0;
  }
  else
    *FarUserName=0;

  lstrcpyn(FarUsersKey,Users_KEY,sizeof(FarUsersKey));
  CheckFirstBackSlash(FarUsersKey,TRUE);
  wsprintf(KeyMacros,"%s\\%s",FarKey,KeyMacros_KEY);
  CheckFirstBackSlash(KeyMacros,TRUE);
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  ::OpenFrom=OpenFrom;

  // создаем экземпл€р класса дл€ работы с реестром
  if (!Reg)
    Reg=new TReg;
  Reg->SetRootKey(HKEY_CURRENT_USER);

  // создаем экземпл€р Macro browser'а
  if (!Macro)
    Macro=new TMacroView;

  switch (Item)
  {
    case 0:
      Macro->MacroList();
  }
  return INVALID_HANDLE_VALUE;
}


#if defined(__BORLANDC__)
  #pragma argsused
#endif
void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  if (Reg)
    delete Reg;
  Reg=NULL;
  if (Macro)
    delete Macro;
  Macro=NULL;
}


void WINAPI _export ExitFAR()
{
  if (Reg)
    delete Reg;
  Reg=NULL;
  if (Macro)
    delete Macro;
  Macro=NULL;
}


int WINAPI _export Configure(int ItemNumber)
{
  // создаем экземпл€р класса дл€ работы с реестром
  if (!Reg)
    Reg=new TReg;
  Reg->SetRootKey(HKEY_CURRENT_USER);

  // создаем экземпл€р Macro browser'а
  if (!Macro)
    Macro=new TMacroView;

  switch(ItemNumber)
  {
    case 0:
      return(Macro->Configure());
  }
  return FALSE;
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_VIEWER;
  Info->DiskMenuStrings=NULL;
  Info->DiskMenuNumbers=NULL;
  Info->DiskMenuStringsNumber=0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MMacroMenu);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  static char *PluginConfigStrings[1];
  PluginConfigStrings[0]=GetMsg(MMacroMenu);
  Info->PluginConfigStrings=PluginConfigStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginConfigStrings)/sizeof(PluginConfigStrings[0]);
}

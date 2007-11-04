#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "macrolng.hpp"
#include "macroview.hpp"
#include "farcolor.hpp"
#include "farkeys.hpp"
#include "macromix.cpp"
#include "strclass.cpp"
#include "regclass.cpp"
#include "config.cpp"
#include "macrodiff.cpp"


static int NewFar=FALSE;

void IllegalFarVersion()
{
  char S[256];
  char *WrongFar[]=
  {
    GetMsg(MMacroError),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };
  
  WORD Major=(MINVERSION>>48)&0xFFFF;
  WORD Minor=(MINVERSION>>32)&0xFFFF;
  WORD Beta=(MINVERSION>>16)&0xFFFF;
  WORD Build=MINVERSION&0xFFFF;
  wsprintf(S,GetMsg(MMacroWrongFar),Major,Minor,Beta,Build);
  Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,WrongFar,
    sizeof(WrongFar)/sizeof(WrongFar[0]),1);
  return;
}

int WINAPI _export GetMinFarVersion()
{
  NewFar=TRUE;
  return MAKEFARVERSION(1,70,1810);
}


#if defined(__BORLANDC__)
  #pragma argsused
#endif
void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  hInstance=GetModuleHandle(NULL);
  ::Info=*Info;
  if (NewFar)
  {
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
  }

  vi.dwOSVersionInfoSize=sizeof(vi);
  GetVersionEx(&vi);
  FarVersion=GetFarVersion();

  wsprintf(PluginRootKey,"%s\\%s",Info->RootKey,Module_KEY);
  CheckFirstBackSlash(PluginRootKey,TRUE);
  char *ptr=strstr(Info->RootKey,Plugins_KEY);
  if (ptr)
    lstrcpyn(FarKey,Info->RootKey,ptr-Info->RootKey+1);
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

  if (!NewFar)
    IllegalFarVersion();
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
      return(Macro->Config());
  }
  return FALSE;
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if (NewFar)
  {
    if (FarVersion<MINVERSION) // верси€ FAR 1.70.5.1810
      return;
  }
  else
    return;

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


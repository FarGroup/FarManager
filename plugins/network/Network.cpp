#include <windows.h>
#include <lm.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>


//#define NETWORK_LOGGING
#include "Memory.cpp"
#include "netlng.hpp"
#include "plugin.hpp"
#include "network.hpp"
#include "netclass.cpp"
#include "netmix.cpp"
#include "netreg.cpp"
#include "netcfg.cpp"
#include "NetNT.cpp"


BOOL IsOldFAR;

static void InitializeNetFunction(void)
{
  static BOOL Init=FALSE;
  if(Init) return;

  if(!hMpr32 && !(hMpr32 = GetModuleHandle("Mpr")))
    hMpr32 = LoadLibrary("Mpr");

  if(!FWNetGetResourceInformation)
    FWNetGetResourceInformation=(PWNetGetResourceInformation )GetProcAddress(hMpr32,"WNetGetResourceInformationA");

  if(!FWNetGetResourceParent)
    FWNetGetResourceParent=(PWNetGetResourceParent)GetProcAddress(hMpr32,"WNetGetResourceParentA");

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if(!hNetApi && !(hNetApi = GetModuleHandle("netapi32")))
      hNetApi = LoadLibrary("netapi32");

    if(!FNetApiBufferFree)
      FNetApiBufferFree=(PNetApiBufferFree)GetProcAddress(hNetApi,"NetApiBufferFree");

    if(!FNetShareEnum)
      FNetShareEnum=(PNetShareEnum)GetProcAddress(hNetApi,"NetShareEnum");

	if(!FNetDfsGetInfo)
	  FNetDfsGetInfo=(PNetDfsGetInfo)GetProcAddress(hNetApi, "NetDfsGetInfo");

    UsedNetFunctions=FWNetGetResourceInformation &&
      FNetShareEnum &&
      FNetApiBufferFree &&
      FWNetGetResourceParent;
  }
  else {
    if (!hSvrApi && !(hSvrApi = GetModuleHandle ("svrapi")))
      hSvrApi = LoadLibrary ("svrapi");

    if (!FNetShareEnum95)
      FNetShareEnum95 = (PNetShareEnum95) GetProcAddress (hSvrApi, "NetShareEnum");

    UsedNetFunctions=FWNetGetResourceInformation &&
      FNetShareEnum95 &&
      FWNetGetResourceParent;
  }

  Init=TRUE;
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);

  ::Info=*Info;
  IsOldFAR=TRUE;

  if(Info->StructSize >= sizeof(struct PluginStartupInfo))
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    strcpy (FarRootKey, Info->RootKey);
    char *pBkSlash = strrchr (FarRootKey, '\\');
    if (pBkSlash)
       *pBkSlash = '\0';

    FSF.sprintf(PluginRootKey,"%s\\Network",Info->RootKey);
    Opt.AddToDisksMenu=GetRegKey(HKEY_CURRENT_USER,"",StrAddToDisksMenu,1);
    Opt.AddToPluginsMenu=GetRegKey(HKEY_CURRENT_USER,"",StrAddToPluginsMenu,1);
    Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,"",StrDisksMenuDigit,3);
    Opt.NTGetHideShare=GetRegKey(HKEY_CURRENT_USER,"",StrNTHiddenShare,0);
    Opt.LocalNetwork=GetRegKey (HKEY_CURRENT_USER, "", StrLocalNetwork, TRUE);
    Opt.DisconnectMode=GetRegKey (HKEY_CURRENT_USER, "", StrDisconnectMode, FALSE);
    Opt.HiddenSharesAsHidden=GetRegKey (HKEY_CURRENT_USER, "", StrHiddenSharesAsHidden, TRUE);

    NetResourceList::InitNetResource (CommonCurResource);
  }
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  if(!IsOldFAR)
  {
    InitializeNetFunction();

    HANDLE hPlugin=new NetBrowser;
    if (hPlugin==NULL)
      return(INVALID_HANDLE_VALUE);
    NetBrowser *Browser=(NetBrowser *)hPlugin;

    if(OpenFrom==OPEN_COMMANDLINE)
    {
      char Path[NM]="\\\\";
      int I=0;
      char *cmd=(char *)Item;
      if(strlen(FSF.Trim(cmd)))
      {
        if (cmd [0] == '/')
          cmd [0] = '\\';
        if (cmd [1] == '/')
          cmd [1] = '\\';
        if(cmd[0] == '\\' && cmd[1] != '\\')
          I=1;
        else if(cmd[0] != '\\' && cmd[1] != '\\')
          I=2;
        OemToChar (cmd, Path+I);

        FSF.Unquote(Path);
        Browser->SetOpenFromCommandLine (Path);
      }
    }
    /* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
       mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
       in later versions. Please DON'T use it in your plugins. */
    else if (OpenFrom == 7)
    {
      if (!Browser->SetOpenFromFilePanel ((char *) Item))
      {
        // we don't support upwards browsing from NetWare shares -
        // it doesn't work correctly
        delete Browser;
        return INVALID_HANDLE_VALUE;
      }
    }
    else {
      if (IsFirstRun && Opt.LocalNetwork)
        Browser->GotoLocalNetwork();
    }
    IsFirstRun = FALSE;

    return(hPlugin);
  }
  return(INVALID_HANDLE_VALUE);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  if(!IsOldFAR)
  {
    delete (NetBrowser *)hPlugin;
  }
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->GetFindData(pPanelItem,pItemsNumber,OpMode));
  return FALSE;
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    Browser->FreeFindData(PanelItem,ItemsNumber);
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
  if(Opt.AddToPluginsMenu)
  {
    static char *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(MNetMenu);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  }
  static char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MNetMenu);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
  Info->CommandPrefix="net";
  /* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
     mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
     in later versions. Please DON'T use it in your plugins. */
  Info->Reserved = 0x5774654E;
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    Browser->GetOpenPluginInfo(Info);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->SetDirectory(Dir,OpMode));

  return(FALSE);
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                               int ItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->DeleteFiles(PanelItem,ItemsNumber,OpMode));
  return(FALSE);
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->ProcessKey(Key,ControlState));
  return(FALSE);
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if (!IsOldFAR)
    return Browser->ProcessEvent (Event, Param);
  return(FALSE);
}


int WINAPI _export Configure(int ItemNumber)
{
  if(!IsOldFAR)
    switch(ItemNumber)
    {
      case 0:
        return(Config());
    }
  return(FALSE);
}


void WINAPI _export ExitFAR()
{
  if(!IsOldFAR)
  {
    CommonRootResources.Clear();
    NetResourceList::DeleteNetResource (CommonCurResource);
  }
  if(hMpr32)
    FreeLibrary(hMpr32);
  if(hNetApi)
    FreeLibrary(hNetApi);
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1080);
}

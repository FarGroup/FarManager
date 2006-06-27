#include "NetCommon.hpp"
#include "NetReg.hpp"
#include "NetCfg.hpp"
#include "NetFavorites.hpp"

struct Options Opt;

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
NETRESOURCE CommonCurResource;
NETRESOURCE *PCommonCurResource = NULL;

OSVERSIONINFO WinVer;

char PluginRootKey[80];
char FarRootKey [NM];

BOOL IsOldFAR;

HMODULE hMpr32=NULL;
HMODULE hNetApi=NULL;
HMODULE hSvrApi=NULL;

BOOL IsFirstRun=TRUE;

PWNetGetResourceInformation FWNetGetResourceInformation=NULL;
PNetApiBufferFree FNetApiBufferFree=NULL;
PNetShareEnum FNetShareEnum=NULL;
PNetShareEnum95 FNetShareEnum95 = NULL;
PWNetGetResourceParent FWNetGetResourceParent=NULL;
PNetDfsGetInfo FNetDfsGetInfo;
BOOL UsedNetFunctions=FALSE;

void InitializeNetFunction(void)
{
  static BOOL Init=FALSE;
  if(Init) return;

  if(!hMpr32 && 0==(hMpr32 = GetModuleHandle("Mpr")))
    hMpr32 = LoadLibrary("Mpr");

  if(!FWNetGetResourceInformation)
    FWNetGetResourceInformation=(PWNetGetResourceInformation )GetProcAddress(hMpr32,"WNetGetResourceInformationA");

  if(!FWNetGetResourceParent)
    FWNetGetResourceParent=(PWNetGetResourceParent)GetProcAddress(hMpr32,"WNetGetResourceParentA");

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if(!hNetApi && 0==(hNetApi = GetModuleHandle("netapi32")))
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
    if (!hSvrApi && 0==(hSvrApi = GetModuleHandle ("svrapi")))
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

  if((size_t)Info->StructSize >= sizeof(struct PluginStartupInfo))
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    lstrcpy (FarRootKey, Info->RootKey);
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
    Opt.FullPathShares=GetRegKey (HKEY_CURRENT_USER, "", StrFullPathShares, TRUE);
    Opt.FavoritesFlags=GetRegKey (HKEY_CURRENT_USER, "", StrFavoritesFlags, FAVORITES_DEFAULTS);
    Opt.NoRootDoublePoint=GetRegKey (HKEY_CURRENT_USER, "", StrNoRootDoublePoint, FALSE);
    Opt.NavigateToDomains=GetRegKey (HKEY_CURRENT_USER, "", StrNavigateToDomains, FALSE);

    NetResourceList::InitNetResource (CommonCurResource);
  }
}

void DeinitializeNetFunctions(void)
{
    if(hMpr32)
        FreeLibrary(hMpr32);
    if(hNetApi)
        FreeLibrary(hNetApi);
}

int WINAPI Configure(int ItemNumber)
{
  if(!IsOldFAR)
    switch(ItemNumber)
    {
      case 0:
        return(Config());
    }
  return(FALSE);
}

int WINAPI GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1821);
}

BOOL DlgCreateFolder(char* lpBuffer, int nBufferSize)
{
//  struct InitDialogItem InitItems[]=
//  {
//    /* 0 */ { DI_DOUBLEBOX, 3, 1, 72, 8, 0, 0,                  0,                0,"Create folder" },
//    /* 1 */ { DI_TEXT,      5, 2,  0, 0, 0, 0,                  0,                0,"Folder Name" },
//    /* 2 */ { DI_EDIT,      5, 3, 70, 0, 1, (DWORD)"NewFolder", DIF_HISTORY ,     0,"" },
//    /* 3 */ { DI_TEXT,      0, 4,  0, 6, 0, 0,                  DIF_SEPARATOR,    0,"" },
//    /* 4 */ { DI_CHECKBOX,  5, 5, 70, 5, 0, 0,                  DIF_DISABLE,      0,"Process Multiple Names" },
//    /* 5 */ { DI_TEXT,      0, 6,  0, 6, 0, 0,                  DIF_SEPARATOR,    0,"" },
//    /* 6 */ { DI_BUTTON,    0, 7,  0, 0, 0, 0,                  DIF_CENTERGROUP,  1,(char*)MOk },
//    /* 7 */ { DI_BUTTON,    0, 7,  0, 0, 0, 0,                  DIF_CENTERGROUP,  0,(char*)MCancel }
//  };
//  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
//  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
//  BOOL res = 6 == Info.Dialog (Info.ModuleNumber, -1, -1, DialogItems [0].X2+4, 10,
//    "CreateFolder", DialogItems, sizeof (DialogItems)/sizeof (DialogItems [0]));
//  if(res && lpBuffer && nBufferSize)
//  {
//    lstrcpyn(lpBuffer, DialogItems[2].Data, nBufferSize);
//  }
  BOOL res = InputBox(
    "Make Folder",
    "Folder name:",
    "NewFolder",
    NULL,
    lpBuffer,
    nBufferSize,
    NULL,
    FIB_BUTTONS
    );
  return res;
}

/* NO NEED THIS
char* NextToken(char *szSource, char *szToken, int nBuff)
{
  if(!szSource||!szToken)
    return NULL;
  lstrcpyn(szToken, szSource, nBuff);
  return szSource + lstrlen(szSource);
}
*/

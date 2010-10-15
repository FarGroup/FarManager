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

TCHAR *PluginRootKey;
TCHAR FarRootKey [MAX_PATH];

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

  if(!hMpr32 && 0==(hMpr32 = GetModuleHandle(_T("Mpr"))))
    hMpr32 = LoadLibrary(_T("Mpr"));

#ifndef UNICODE
#define _SUFF "A"
#else
#define _SUFF "W"
#endif
  if(!FWNetGetResourceInformation)
    FWNetGetResourceInformation=(PWNetGetResourceInformation)GetProcAddress(hMpr32,"WNetGetResourceInformation" _SUFF);

  if(!FWNetGetResourceParent)
    FWNetGetResourceParent=(PWNetGetResourceParent)GetProcAddress(hMpr32,"WNetGetResourceParent" _SUFF);
#undef _SUFF

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if(!hNetApi && 0==(hNetApi = GetModuleHandle(_T("netapi32"))))
      hNetApi = LoadLibrary(_T("netapi32"));

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
    if (!hSvrApi && 0==(hSvrApi = GetModuleHandle (_T("svrapi"))))
      hSvrApi = LoadLibrary (_T("svrapi"));

    if (!FNetShareEnum95)
      FNetShareEnum95 = (PNetShareEnum95) GetProcAddress (hSvrApi, "NetShareEnum");

    UsedNetFunctions=FWNetGetResourceInformation &&
      FNetShareEnum95 &&
      FWNetGetResourceParent;
  }

  Init=TRUE;
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);

  ::Info=*Info;

  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  lstrcpy (FarRootKey, Info->RootKey);
  TCHAR *pBkSlash = _tcsrchr (FarRootKey, _T('\\'));
  if (pBkSlash)
    *pBkSlash = _T('\0');

  PluginRootKey = (TCHAR *)malloc(lstrlen(Info->RootKey)*sizeof(TCHAR) + sizeof(_T("\\Network")));
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\Network"));

  Opt.AddToDisksMenu=GetRegKey(HKEY_CURRENT_USER,_T(""),StrAddToDisksMenu,1);
  Opt.AddToPluginsMenu=GetRegKey(HKEY_CURRENT_USER,_T(""),StrAddToPluginsMenu,1);
#ifndef UNICODE
  Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,_T(""),StrDisksMenuDigit,3);
#endif
  Opt.NTGetHideShare=GetRegKey(HKEY_CURRENT_USER,_T(""),StrNTHiddenShare,0);
  Opt.ShowPrinters=GetRegKey(HKEY_CURRENT_USER,_T(""),StrShowPrinters,0);
  Opt.LocalNetwork=GetRegKey (HKEY_CURRENT_USER, _T(""), StrLocalNetwork, TRUE);
  Opt.DisconnectMode=GetRegKey (HKEY_CURRENT_USER, _T(""), StrDisconnectMode, FALSE);
  Opt.HiddenSharesAsHidden=GetRegKey (HKEY_CURRENT_USER, _T(""), StrHiddenSharesAsHidden, TRUE);
  Opt.FullPathShares=GetRegKey (HKEY_CURRENT_USER, _T(""), StrFullPathShares, TRUE);
  Opt.FavoritesFlags=GetRegKey (HKEY_CURRENT_USER, _T(""), StrFavoritesFlags, FAVORITES_DEFAULTS);
  Opt.NoRootDoublePoint=GetRegKey (HKEY_CURRENT_USER, _T(""), StrNoRootDoublePoint, FALSE);
  Opt.NavigateToDomains=GetRegKey (HKEY_CURRENT_USER, _T(""), StrNavigateToDomains, FALSE);

  CommonRootResources = new NetResourceList;
  NetResourceList::InitNetResource (CommonCurResource);
}

void DeinitializeNetFunctions(void)
{
    if(hMpr32)
        FreeLibrary(hMpr32);
    if(hNetApi)
        FreeLibrary(hNetApi);
}

int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}

BOOL DlgCreateFolder(TCHAR* lpBuffer, int nBufferSize)
{
//  struct InitDialogItem InitItems[]=
//  {
//    /* 0 */ { DI_DOUBLEBOX, 3, 1, 72, 8, 0, 0,                      0,                0,_T("Create folder") },
//    /* 1 */ { DI_TEXT,      5, 2,  0, 0, 0, 0,                      0,                0,_T("Folder Name") },
//    /* 2 */ { DI_EDIT,      5, 3, 70, 0, 1, (DWORD)_T("NewFolder"), DIF_HISTORY ,     0,_T("") },
//    /* 3 */ { DI_TEXT,      0, 4,  0, 6, 0, 0,                      DIF_SEPARATOR,    0,_T("") },
//    /* 4 */ { DI_CHECKBOX,  5, 5, 70, 5, 0, 0,                      DIF_DISABLE,      0,_T("Process Multiple Names") },
//    /* 5 */ { DI_TEXT,      0, 6,  0, 6, 0, 0,                      DIF_SEPARATOR,    0,_T("") },
//    /* 6 */ { DI_BUTTON,    0, 7,  0, 0, 0, 0,                      DIF_CENTERGROUP,  1,(TCHAR*)MOk },
//    /* 7 */ { DI_BUTTON,    0, 7,  0, 0, 0, 0,                      DIF_CENTERGROUP,  0,(TCHAR*)MCancel }
//  };
//  struct FarDialogItem DialogItems[ArraySize(InitItems)];
//  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
//  BOOL res = 6 == Info.Dialog (Info.ModuleNumber, -1, -1, DialogItems [0].X2+4, 10,
//    _T("CreateFolder"), DialogItems, ArraySize(DialogItems));
//  if(res && lpBuffer && nBufferSize)
//  {
//    lstrcpyn(lpBuffer, DialogItems[2].Data, nBufferSize);
//  }
  BOOL res = InputBox(
    _T("Make Folder"),
    _T("Folder name:"),
    _T("NewFolder"),
    NULL,
    lpBuffer,
    nBufferSize,
    NULL,
    FIB_BUTTONS
    );
  return res;
}

/* NO NEED THIS
TCHAR* NextToken(TCHAR *szSource, TCHAR *szToken, int nBuff)
{
  if(!szSource||!szToken)
    return NULL;
  lstrcpyn(szToken, szSource, nBuff);
  return szSource + lstrlen(szSource);
}
*/

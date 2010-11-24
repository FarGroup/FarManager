#include "NetCfg.hpp"
#include "NetCommon.hpp"
#include "NetFavorites.hpp"
#include "NetReg.hpp"

const TCHAR *StrAddToDisksMenu=_T("AddToDisksMenu");
const TCHAR *StrAddToPluginsMenu=_T("AddToPluginsMenu");
#ifndef UNICODE
const TCHAR *StrDisksMenuDigit=_T("DisksMenuDigit");
#endif
const TCHAR *StrHelpNetBrowse=_T("Contents");
const TCHAR *StrNTHiddenShare=_T("NTHiddenShare");
const TCHAR *StrShowPrinters=_T("ShowPrinters");
const TCHAR *StrLocalNetwork=_T("LocalNetwork");
const TCHAR *StrDisconnectMode=_T("DisconnectMode");
const TCHAR *StrRemoveConnection=_T("RemoveConnection");
const TCHAR *StrHiddenSharesAsHidden=_T("HiddenSharesAsHidden");
const TCHAR *StrFullPathShares=_T("FullPathShares");
const TCHAR *StrFavoritesFlags=_T("FavoritesFlags");
const TCHAR *StrNoRootDoublePoint=_T("NoRootDoublePoint");
const TCHAR *StrNavigateToDomains=_T("NavigateToDomains");
const TCHAR *StrPanelMode=_T("PanelMode");

int Config()
{
  struct InitDialogItem InitItems[]={
  /*  0 */{DI_DOUBLEBOX,3,1,72,17,0,0,0,0,(TCHAR *)MConfigTitle},
  /*  1 */{DI_CHECKBOX,5,2,0,0,0,0,0,0,(TCHAR *)MConfigAddToDisksMenu},
  /*  2 */{DI_FIXEDIT,7,3,7,3,1,0,0
#ifdef UNICODE
                                   |DIF_HIDDEN
#endif
                                              ,0,_T("")},
  /*  3 */{DI_TEXT,9,3,0,0,0,0,0
#ifdef UNICODE
                                |DIF_HIDDEN
#endif
                                           ,0,(TCHAR *)MConfigDisksMenuDigit},
  /*  4 */{DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)MConfigAddToPluginMenu},
  /*  5 */{DI_CHECKBOX,5,5,0,0,0,0,0,0,(TCHAR *)MNoRootDoublePoint},
  /*  6 */{DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /*  7 */{DI_CHECKBOX,5,7,0,0,0,0,0,0,(TCHAR *)MConfigLocalNetwork},
  /*  8 */{DI_CHECKBOX,5,8,0,0,0,0,0,0,(TCHAR *)MNTGetHideShare},
  /*  9 */{DI_CHECKBOX,5,9,0,0,0,0,0,0,(TCHAR *)MConfigShowPrinters},
  /* 10 */{DI_TEXT,5,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 11 */{DI_CHECKBOX,5,11,0,0,0,0,0,0,(TCHAR *)MFullPathShares},
  /* 12 */{DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERGROUP,0,(TCHAR *)MFavorites},
  /* 13 */{DI_CHECKBOX,5,13,0,0,0,0,0,0,(TCHAR *)MUpbrowseToFavorites},
  /* 14 */{DI_CHECKBOX,5,14,0,0,0,0,0,0,(TCHAR *)MCheckResource},
  /* 15 */{DI_TEXT,5,15,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 16 */{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MOk},
  /* 17 */{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel}
  };

  struct FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
  InitDialogItems(InitItems,DialogItems,ARRAYSIZE(InitItems));
  DialogItems[1].Selected=Opt.AddToDisksMenu;
  DialogItems[4].Selected=Opt.AddToPluginsMenu;
  DialogItems[5].Selected=!Opt.NoRootDoublePoint;
#ifndef UNICODE
  if (Opt.DisksMenuDigit)
    FSF.sprintf((TCHAR *)DialogItems[2].Data,_T("%d"),Opt.DisksMenuDigit);
#endif
  DialogItems[7].Selected=Opt.LocalNetwork;
  DialogItems[8].Selected=Opt.NTGetHideShare;
  DialogItems[9].Selected=Opt.ShowPrinters;
  DialogItems[11].Selected=Opt.FullPathShares;

  DialogItems[13].Selected=Opt.FavoritesFlags & FAVORITES_UPBROWSE_TO_FAVORITES ? TRUE : FALSE;
  DialogItems[14].Selected=Opt.FavoritesFlags & FAVORITES_CHECK_RESOURCES       ? TRUE : FALSE;

  int ret=FALSE;

#ifndef UNICODE
  int ExitCode=Info.Dialog(Info.ModuleNumber, -1, -1, 76, 19,_T("Config"),
                           DialogItems, ARRAYSIZE(DialogItems));
#else
  HANDLE hDlg=Info.DialogInit(Info.ModuleNumber, -1, -1, 76, 19,_T("Config"),
                           DialogItems, ARRAYSIZE(DialogItems),0,0,NULL,0);
  if (hDlg == INVALID_HANDLE_VALUE)
    return ret;
  int ExitCode=Info.DialogRun(hDlg);
#endif

  if (ExitCode==16)
  {
    Opt.AddToDisksMenu=GetCheck(1);
    Opt.AddToPluginsMenu=GetCheck(4);
    Opt.NoRootDoublePoint=!GetCheck(5);
#ifndef UNICODE
    Opt.DisksMenuDigit=FSF.atoi(GetDataPtr(2));
#endif
    Opt.LocalNetwork=GetCheck(7);
    Opt.NTGetHideShare=GetCheck(8);
    Opt.ShowPrinters=GetCheck(9);
    Opt.FullPathShares=GetCheck(11);

    if(GetCheck(13))
      Opt.FavoritesFlags |= FAVORITES_UPBROWSE_TO_FAVORITES;
    else
      Opt.FavoritesFlags &= ~FAVORITES_UPBROWSE_TO_FAVORITES;

    if(GetCheck(14))
      Opt.FavoritesFlags |= FAVORITES_CHECK_RESOURCES;
    else
      Opt.FavoritesFlags &= ~FAVORITES_CHECK_RESOURCES;

    SetRegKey(HKEY_CURRENT_USER,_T(""),StrAddToDisksMenu,Opt.AddToDisksMenu);
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrAddToPluginsMenu,Opt.AddToPluginsMenu);
#ifndef UNICODE
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrDisksMenuDigit,Opt.DisksMenuDigit);
#endif
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrLocalNetwork,Opt.LocalNetwork);
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrNTHiddenShare,Opt.NTGetHideShare);
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrShowPrinters,Opt.ShowPrinters);
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrFullPathShares,Opt.FullPathShares);
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrFavoritesFlags,Opt.FavoritesFlags);
    SetRegKey(HKEY_CURRENT_USER,_T(""),StrNoRootDoublePoint,Opt.NoRootDoublePoint);
    ret=TRUE;
  }
#ifdef UNICODE
  Info.DialogFree(hDlg);
#endif
  return ret;
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0|
    PF_FULLCMDLINE|
  0;
  static TCHAR *DiskMenuStrings[1];
  DiskMenuStrings[0]=GetMsg(MDiskMenuString);
  Info->DiskMenuStrings=DiskMenuStrings;
#ifndef UNICODE
  static int DiskMenuNumbers[1];
  DiskMenuNumbers[0]=Opt.DisksMenuDigit;
  Info->DiskMenuNumbers=DiskMenuNumbers;
#endif
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;
  if(Opt.AddToPluginsMenu)
  {
    static TCHAR *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(MNetMenu);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=ARRAYSIZE(PluginMenuStrings);
  }
  static TCHAR *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MNetMenu);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=ARRAYSIZE(PluginCfgStrings);
  Info->CommandPrefix=_T("net:netg");
  /* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
     mechanism supported ONLY in FAR 1.70 beta 6. It will NOT be supported
     in later versions. Please DON'T use it in your plugins. */
  Info->Reserved = 0x5774654E;
}

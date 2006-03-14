#include "NetCfg.hpp"
#include "NetCommon.hpp"
#include "NetFavorites.hpp"
#include "NetReg.hpp"

const char *StrAddToDisksMenu="AddToDisksMenu";
const char *StrAddToPluginsMenu="AddToPluginsMenu";
const char *StrDisksMenuDigit="DisksMenuDigit";
const char *StrHelpNetBrowse="Contents";
const char *StrNTHiddenShare="NTHiddenShare";
const char *StrLocalNetwork="LocalNetwork";
const char *StrDisconnectMode="DisconnectMode";
const char *StrRemoveConnection="RemoveConnection";
const char *StrHiddenSharesAsHidden="HiddenSharesAsHidden";
const char *StrFullPathShares="FullPathShares";
const char *StrFavoritesFlags="FavoritesFlags";
const char *StrNoRootDoublePoint="NoRootDoublePoint";
const char *StrPanelMode="PanelMode";

int Config()
{
  struct InitDialogItem InitItems[]={
  /*  0 */{DI_DOUBLEBOX,3,1,72,16,0,0,0,0,(char *)MConfigTitle},
  /*  1 */{DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)MConfigAddToDisksMenu},
  /*  2 */{DI_FIXEDIT,7,3,7,3,1,0,0,0,""},
  /*  3 */{DI_TEXT,9,3,0,0,0,0,0,0,(char *)MConfigDisksMenuDigit},
  /*  4 */{DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigAddToPluginMenu},
  /*  5 */{DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MNoRootDoublePoint},
  /*  6 */{DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /*  7 */{DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MConfigLocalNetwork},
  /*  8 */{DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MNTGetHideShare},
  /*  9 */{DI_TEXT,5,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 10 */{DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MFullPathShares},
  /* 11 */{DI_TEXT,5,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERGROUP,0,(char *)MFavorites},
  /* 12 */{DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MUpbrowseToFavorites},
  /* 13 */{DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MCheckResource},
  /* 14 */{DI_TEXT,5,14,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 15 */{DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk},
  /* 16 */{DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel}
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=Opt.AddToDisksMenu;
  DialogItems[4].Selected=Opt.AddToPluginsMenu;
  DialogItems[5].Selected=!Opt.NoRootDoublePoint;
  if (Opt.DisksMenuDigit)
    FSF.sprintf(DialogItems[2].Data,"%d",Opt.DisksMenuDigit);
  DialogItems[7].Selected=Opt.LocalNetwork;
  DialogItems[8].Selected=Opt.NTGetHideShare;
  DialogItems[10].Selected=Opt.FullPathShares;

  DialogItems[12].Selected=Opt.FavoritesFlags & FAVORITES_UPBROWSE_TO_FAVORITES ? TRUE : FALSE;
  DialogItems[13].Selected=Opt.FavoritesFlags & FAVORITES_CHECK_RESOURCES       ? TRUE : FALSE;

  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,18,"Config",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=15)
    return(FALSE);
  Opt.AddToDisksMenu=DialogItems[1].Selected;
  Opt.AddToPluginsMenu=DialogItems[4].Selected;
  Opt.NoRootDoublePoint=!DialogItems[5].Selected;
  Opt.DisksMenuDigit=FSF.atoi(DialogItems[2].Data);
  Opt.LocalNetwork=DialogItems[7].Selected;
  Opt.NTGetHideShare=DialogItems[8].Selected;
  Opt.FullPathShares=DialogItems[10].Selected;

  if(DialogItems[12].Selected)
    Opt.FavoritesFlags |= FAVORITES_UPBROWSE_TO_FAVORITES;
  else
    Opt.FavoritesFlags &= ~FAVORITES_UPBROWSE_TO_FAVORITES;

  if(DialogItems[13].Selected)
    Opt.FavoritesFlags |= FAVORITES_CHECK_RESOURCES;
  else
    Opt.FavoritesFlags &= ~FAVORITES_CHECK_RESOURCES;

  SetRegKey(HKEY_CURRENT_USER,"",StrAddToDisksMenu,Opt.AddToDisksMenu);
  SetRegKey(HKEY_CURRENT_USER,"",StrAddToPluginsMenu,Opt.AddToPluginsMenu);
  SetRegKey(HKEY_CURRENT_USER,"",StrDisksMenuDigit,Opt.DisksMenuDigit);
  SetRegKey(HKEY_CURRENT_USER,"",StrLocalNetwork,Opt.LocalNetwork);
  SetRegKey(HKEY_CURRENT_USER,"",StrNTHiddenShare,Opt.NTGetHideShare);
  SetRegKey(HKEY_CURRENT_USER,"",StrFullPathShares,Opt.FullPathShares);
  SetRegKey(HKEY_CURRENT_USER,"",StrFavoritesFlags,Opt.FavoritesFlags);
  SetRegKey(HKEY_CURRENT_USER,"",StrNoRootDoublePoint,Opt.NoRootDoublePoint);
  return(TRUE);
}

void WINAPI GetPluginInfo(struct PluginInfo *Info)
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
     mechanism supported ONLY in FAR 1.70 beta 6. It will NOT be supported
     in later versions. Please DON'T use it in your plugins. */
  Info->Reserved = 0x5774654E;
}

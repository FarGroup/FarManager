#include "NetCfg.hpp"
#include "NetCommon.hpp"
#include "NetFavorites.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

const TCHAR *StrAddToDisksMenu=_T("AddToDisksMenu");
const TCHAR *StrAddToPluginsMenu=_T("AddToPluginsMenu");
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
  PluginDialogBuilder Builder(Info, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");
  Builder.AddCheckbox(MConfigAddToDisksMenu, &Opt.AddToDisksMenu);
  Builder.AddCheckbox(MConfigAddToPluginMenu, &Opt.AddToPluginsMenu);
  Builder.AddCheckbox(MNoRootDoublePoint, &Opt.RootDoublePoint);
  Builder.AddSeparator();
  Builder.AddCheckbox(MConfigLocalNetwork, &Opt.LocalNetwork);
  Builder.AddCheckbox(MNTGetHideShare, &Opt.NTGetHideShare);
  Builder.AddCheckbox(MConfigShowPrinters, &Opt.ShowPrinters);
  Builder.AddSeparator();
  Builder.AddCheckbox(MFullPathShares, &Opt.FullPathShares);
  Builder.AddSeparator(MFavorites);
  Builder.AddCheckbox(MUpbrowseToFavorites, &Opt.FavoritesFlags, FAVORITES_UPBROWSE_TO_FAVORITES);
  Builder.AddCheckbox(MCheckResource, &Opt.FavoritesFlags, FAVORITES_CHECK_RESOURCES);
  Builder.AddOKCancel(MOk, MCancel);

  if (Builder.ShowDialog())
  {
    PluginSettings settings(MainGuid, Info.SettingsControl);
    settings.Set(0,StrAddToDisksMenu,Opt.AddToDisksMenu);
    settings.Set(0,StrAddToPluginsMenu,Opt.AddToPluginsMenu);
    settings.Set(0,StrLocalNetwork,Opt.LocalNetwork);
    settings.Set(0,StrNTHiddenShare,Opt.NTGetHideShare);
    settings.Set(0,StrShowPrinters,Opt.ShowPrinters);
    settings.Set(0,StrFullPathShares,Opt.FullPathShares);
    settings.Set(0,StrFavoritesFlags,Opt.FavoritesFlags);
    settings.Set(0,StrNoRootDoublePoint,Opt.RootDoublePoint);
    return TRUE;
  }

  return FALSE;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_FULLCMDLINE;

  static const wchar_t *PluginMenuStrings[1];
  static const wchar_t *DiskMenuStrings[1];

  if (Opt.AddToDisksMenu)
  {
    DiskMenuStrings[0]=GetMsg(MDiskMenuString);
    Info->DiskMenu.Guids=&MenuGuid;
    Info->DiskMenu.Strings=DiskMenuStrings;
    Info->DiskMenu.Count=ARRAYSIZE(DiskMenuStrings);
  }

  PluginMenuStrings[0]=GetMsg(MNetMenu);

  if(Opt.AddToPluginsMenu)
  {
    Info->PluginMenu.Guids=&MenuGuid;
    Info->PluginMenu.Strings=PluginMenuStrings;
    Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
  }

  Info->PluginConfig.Guids=&MenuGuid;
  Info->PluginConfig.Strings=PluginMenuStrings;
  Info->PluginConfig.Count=ARRAYSIZE(PluginMenuStrings);

  Info->CommandPrefix=L"net:netg";
}

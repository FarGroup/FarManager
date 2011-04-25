#include "NetCfg.hpp"
#include "NetCommon.hpp"
#include "NetFavorites.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

const wchar_t *StrAddToDisksMenu=L"AddToDisksMenu";
const wchar_t *StrAddToPluginsMenu=L"AddToPluginsMenu";
const wchar_t *StrHelpNetBrowse=L"Contents";
const wchar_t *StrNTHiddenShare=L"NTHiddenShare";
const wchar_t *StrShowPrinters=L"ShowPrinters";
const wchar_t *StrLocalNetwork=L"LocalNetwork";
const wchar_t *StrDisconnectMode=L"DisconnectMode";
const wchar_t *StrRemoveConnection=L"RemoveConnection";
const wchar_t *StrHiddenSharesAsHidden=L"HiddenSharesAsHidden";
const wchar_t *StrFullPathShares=L"FullPathShares";
const wchar_t *StrFavoritesFlags=L"FavoritesFlags";
const wchar_t *StrNoRootDoublePoint=L"NoRootDoublePoint";
const wchar_t *StrNavigateToDomains=L"NavigateToDomains";
const wchar_t *StrPanelMode=L"PanelMode";

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

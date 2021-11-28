#include "NetCfg.hpp"
#include "Network.hpp"
#include "NetCommon.hpp"
#include "NetFavorites.hpp"
#include "NetLng.hpp"
#include "guid.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

const wchar_t* StrAddToDisksMenu = L"AddToDisksMenu";
const wchar_t* StrAddToPluginsMenu = L"AddToPluginsMenu";
const wchar_t* StrHelpNetBrowse = L"Contents";
const wchar_t* StrHiddenShares = L"HiddenShares";
const wchar_t* StrShowPrinters = L"ShowPrinters";
const wchar_t* StrLocalNetwork = L"LocalNetwork";
const wchar_t* StrDisconnectMode = L"DisconnectMode";
const wchar_t* StrRemoveConnection = L"RemoveConnection";
const wchar_t* StrHiddenSharesAsHidden = L"HiddenSharesAsHidden";
const wchar_t* StrFullPathShares = L"FullPathShares";
const wchar_t* StrFavoritesFlags = L"FavoritesFlags";
const wchar_t* StrNoRootDoublePoint = L"NoRootDoublePoint";
const wchar_t* StrNavigateToDomains = L"NavigateToDomains";
const wchar_t* StrPanelMode = L"PanelMode";

int Config()
{
	PluginDialogBuilder Builder(PsInfo, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");
	Builder.AddCheckbox(MConfigAddToDisksMenu, &Opt.AddToDisksMenu);
	Builder.AddCheckbox(MConfigAddToPluginMenu, &Opt.AddToPluginsMenu);
	Builder.AddCheckbox(MNoRootDoublePoint, &Opt.RootDoublePoint);
	Builder.AddCheckbox(MConfigLocalNetwork, &Opt.LocalNetwork);
	Builder.AddCheckbox(MConfigShowPrinters, &Opt.ShowPrinters);
	Builder.AddSeparator(MConfigShares);
	Builder.AddCheckbox(MConfigSharesFullPath, &Opt.FullPathShares);

	Builder.StartSingleBox(MConfigHiddenShares);
	int HiddenSharesState = Opt.HiddenShares? (Opt.HiddenSharesAsHidden? 1 : 2) : 0;
	int HiddenSharesMsgs[] = {
		MConfigHiddenSharesNeverShow, MConfigHiddenSharesMakeHidden, MConfigHiddenSharesAlwaysShow
	};
	Builder.AddRadioButtons(&HiddenSharesState, 3, HiddenSharesMsgs);
	Builder.EndSingleBox();

	Builder.AddSeparator(MFavorites);
	Builder.AddCheckbox(MUpbrowseToFavorites, &Opt.FavoritesFlags, FAVORITES_UPBROWSE_TO_FAVORITES);
	// TODO restore ?
	//	Builder.AddCheckbox(MCheckResource, &Opt.FavoritesFlags, FAVORITES_CHECK_RESOURCES);
	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog())
	{
		switch (HiddenSharesState)
		{
		case 0: // never show
			Opt.HiddenShares = FALSE;
			Opt.HiddenSharesAsHidden = FALSE;
			break;
		case 1: // make hidden
			Opt.HiddenShares = TRUE;
			Opt.HiddenSharesAsHidden = TRUE;
			break;
		case 2: // always show
			Opt.HiddenShares = TRUE;
			Opt.HiddenSharesAsHidden = FALSE;
			break;
		}

		Opt.Write();

		return TRUE;
	}

	return FALSE;
}

void Options::Read()
{
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	Opt.AddToDisksMenu = settings.Get(0, StrAddToDisksMenu, 1);
	Opt.AddToPluginsMenu = settings.Get(0, StrAddToPluginsMenu, 1);
	Opt.LocalNetwork = settings.Get(0, StrLocalNetwork, TRUE);
	Opt.HiddenShares = settings.Get(0, StrHiddenShares, 1);
	Opt.ShowPrinters = settings.Get(0, StrShowPrinters, 0);
	Opt.FullPathShares = settings.Get(0, StrFullPathShares, TRUE);
	Opt.FavoritesFlags = settings.Get(0, StrFavoritesFlags, static_cast<int>(FAVORITES_DEFAULTS));
	Opt.RootDoublePoint = settings.Get(0, StrNoRootDoublePoint, TRUE);
	Opt.DisconnectMode = settings.Get(0, StrDisconnectMode, FALSE);
	Opt.HiddenSharesAsHidden = settings.Get(0, StrHiddenSharesAsHidden, TRUE);
	Opt.NavigateToDomains = settings.Get(0, StrNavigateToDomains, FALSE);
}

void Options::Write()
{
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	settings.Set(0, StrAddToDisksMenu, Opt.AddToDisksMenu);
	settings.Set(0, StrAddToPluginsMenu, Opt.AddToPluginsMenu);
	settings.Set(0, StrLocalNetwork, Opt.LocalNetwork);
	settings.Set(0, StrHiddenShares, Opt.HiddenShares);
	settings.Set(0, StrShowPrinters, Opt.ShowPrinters);
	settings.Set(0, StrFullPathShares, Opt.FullPathShares);
	settings.Set(0, StrFavoritesFlags, Opt.FavoritesFlags);
	settings.Set(0, StrNoRootDoublePoint, Opt.RootDoublePoint);
	settings.Set(0, StrHiddenSharesAsHidden, Opt.HiddenSharesAsHidden);
}

__int64 GetSetting(FARSETTINGS_SUBFOLDERS Root, const wchar_t* Name)
{
	__int64 result = 0;
	FarSettingsCreate settings = {sizeof(FarSettingsCreate), FarGuid, INVALID_HANDLE_VALUE};
	HANDLE Settings = PsInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings)?
		                  settings.Handle :
		                  nullptr;
	if (Settings)
	{
		FarSettingsItem item = {sizeof(FarSettingsItem), static_cast<size_t>(Root), Name, FST_UNKNOWN, {}};
		if (PsInfo.SettingsControl(Settings, SCTL_GET, 0, &item) && FST_QWORD == item.Type)
		{
			result = item.Number;
		}
		PsInfo.SettingsControl(Settings, SCTL_FREE, 0, {});
	}
	return result;
}

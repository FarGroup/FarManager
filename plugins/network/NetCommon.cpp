#include "NetCommon.hpp"
#include "NetCfg.hpp"
#include "NetFavorites.hpp"
#include <PluginSettings.hpp>

struct Options Opt;

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
NETRESOURCE CommonCurResource;
NETRESOURCE *PCommonCurResource = NULL;

HMODULE hMpr32=NULL;
HMODULE hNetApi=NULL;
HMODULE hSvrApi=NULL;

BOOL IsFirstRun=TRUE;

PWNetGetResourceInformation FWNetGetResourceInformation=NULL;
PNetApiBufferFree FNetApiBufferFree=NULL;
PNetShareEnum FNetShareEnum=NULL;
PWNetGetResourceParent FWNetGetResourceParent=NULL;
PNetDfsGetInfo FNetDfsGetInfo;
BOOL UsedNetFunctions=FALSE;

void InitializeNetFunction(void)
{
	static BOOL Init=FALSE;

	if (Init) return;

	if (!hMpr32 && 0==(hMpr32 = GetModuleHandle(L"Mpr")))
		hMpr32 = LoadLibrary(L"Mpr");

	if (!FWNetGetResourceInformation)
		FWNetGetResourceInformation=(PWNetGetResourceInformation)GetProcAddress(hMpr32,"WNetGetResourceInformationW");

	if (!FWNetGetResourceParent)
		FWNetGetResourceParent=(PWNetGetResourceParent)GetProcAddress(hMpr32,"WNetGetResourceParentW");

	if (!hNetApi && 0==(hNetApi = GetModuleHandle(L"netapi32")))
		hNetApi = LoadLibrary(L"netapi32");

	if (!FNetApiBufferFree)
		FNetApiBufferFree=(PNetApiBufferFree)GetProcAddress(hNetApi,"NetApiBufferFree");

	if (!FNetShareEnum)
		FNetShareEnum=(PNetShareEnum)GetProcAddress(hNetApi,"NetShareEnum");

	if (!FNetDfsGetInfo)
		FNetDfsGetInfo=(PNetDfsGetInfo)GetProcAddress(hNetApi, "NetDfsGetInfo");

	UsedNetFunctions=FWNetGetResourceInformation &&
	                 FNetShareEnum &&
	                 FNetApiBufferFree &&
	                 FWNetGetResourceParent;
	Init=TRUE;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;
	PluginSettings settings(MainGuid, ::Info.SettingsControl);
	Opt.AddToDisksMenu = settings.Get(0,StrAddToDisksMenu,1);
	Opt.AddToPluginsMenu = settings.Get(0,StrAddToPluginsMenu,1);
	Opt.LocalNetwork = settings.Get(0,StrLocalNetwork,TRUE);
	Opt.NTGetHideShare = settings.Get(0,StrNTHiddenShare,0);
	Opt.ShowPrinters = settings.Get(0,StrShowPrinters,0);
	Opt.FullPathShares = settings.Get(0,StrFullPathShares,TRUE);
	Opt.FavoritesFlags = settings.Get(0,StrFavoritesFlags,int(FAVORITES_DEFAULTS));
	Opt.RootDoublePoint = settings.Get(0,StrNoRootDoublePoint,TRUE);
	Opt.DisconnectMode = settings.Get(0,StrDisconnectMode, FALSE);
	Opt.HiddenSharesAsHidden = settings.Get(0,StrHiddenSharesAsHidden, TRUE);
	Opt.NavigateToDomains = settings.Get(0,StrNavigateToDomains, FALSE);
	CommonRootResources = new NetResourceList;
	NetResourceList::InitNetResource(CommonCurResource);
}

void DeinitializeNetFunctions(void)
{
	if (hMpr32)
		FreeLibrary(hMpr32);

	if (hNetApi)
		FreeLibrary(hNetApi);
}

int WINAPI ConfigureW(const ConfigureInfo* Info)
{
	return Config();
}

BOOL DlgCreateFolder(wchar_t* lpBuffer, int nBufferSize)
{
	BOOL res = Info.InputBox(&MainGuid,
	                         L"Make Folder",
	                         L"Folder name:",
	                         L"NewFolder",
	                         NULL,
	                         lpBuffer,
	                         nBufferSize,
	                         NULL,
	                         FIB_BUTTONS
	                        );
	return res;
}

/* NO NEED THIS
wchar_t* NextToken(wchar_t *szSource, wchar_t *szToken, int nBuff)
{
  if(!szSource||!szToken)
    return NULL;
  lstrcpyn(szToken, szSource, nBuff);
  return szSource + lstrlen(szSource);
}
*/

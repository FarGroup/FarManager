#include "Network.hpp"
#include "version.hpp"
#include "NetCfg.hpp"
#include "NetFavorites.hpp"
#include "NetLng.hpp"
#include "NetCommon.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

PluginStartupInfo PsInfo;
FarStandardFunctions FSF;
BOOL IsFirstRun = TRUE;

void WINAPI GetGlobalInfoW(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;
	Info->Version = PLUGIN_VERSION;
	Info->Guid = MainGuid;
	Info->Title = PLUGIN_NAME;
	Info->Description = PLUGIN_DESC;
	Info->Author = PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo* Info)
{
	PsInfo = *Info;
	FSF = *PsInfo.FSF;
	PsInfo.FSF = &FSF;

	Opt.Read();
	CommonRootResources = new NetResourceList{};
}

void WINAPI GetPluginInfoW(PluginInfo* Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = PF_FULLCMDLINE;

	if (Opt.AddToDisksMenu)
	{
		static const wchar_t* DiskMenuStrings[1];
		DiskMenuStrings[0] = GetMsg(MDiskMenuString);
		Info->DiskMenu.Guids = &MenuGuid;
		Info->DiskMenu.Strings = DiskMenuStrings;
		Info->DiskMenu.Count = std::size(DiskMenuStrings);
	}

	static const wchar_t* PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MNetMenu);

	if (Opt.AddToPluginsMenu)
	{
		Info->PluginMenu.Guids = &MenuGuid;
		Info->PluginMenu.Strings = PluginMenuStrings;
		Info->PluginMenu.Count = std::size(PluginMenuStrings);
	}

	Info->PluginConfig.Guids = &MenuGuid;
	Info->PluginConfig.Strings = PluginMenuStrings;
	Info->PluginConfig.Count = std::size(PluginMenuStrings);
	Info->CommandPrefix = L"net:netg";
}


HANDLE WINAPI OpenW(const OpenInfo* Info)
{
	auto hPlugin = std::make_unique<NetBrowser>();

	if (Info->OpenFrom == OPEN_COMMANDLINE)
	{
		auto cmd = const_cast<wchar_t*>(reinterpret_cast<OpenCommandLineInfo*>(Info->Data)->CommandLine); //BUGBUG
		auto result = hPlugin->SetOpenFromCommandLine(cmd);
		if (!result)
			return nullptr;
	}
	else if (Info->OpenFrom == OPEN_FILEPANEL)
	{
		if (!hPlugin->SetOpenFromFilePanel(reinterpret_cast<wchar_t*>(Info->Data)))
		{
			// we don't support upwards browsing from NetWare shares -
			// it doesn't work correctly
			return nullptr;
		}
	}
	else
	{
		if (IsFirstRun && Opt.LocalNetwork)
			hPlugin->GotoLocalNetwork();
	}

	IsFirstRun = FALSE;
	wchar_t szCurrDir[MAX_PATH];

	if (GetCurrentDirectory(static_cast<DWORD>(std::size(szCurrDir)), szCurrDir))
	{
		if (*szCurrDir == L'\\' && GetSystemDirectory(szCurrDir, static_cast<DWORD>(std::size(szCurrDir))))
		{
			szCurrDir[2] = L'\0';
			SetCurrentDirectory(szCurrDir);
		}
	}

	return hPlugin.release();
}

void WINAPI ClosePanelW(const ClosePanelInfo* Info)
{
	std::unique_ptr<NetBrowser>(static_cast<NetBrowser*>(Info->hPanel));
}

intptr_t WINAPI GetFindDataW(GetFindDataInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	return (Browser.GetFindData(&Info->PanelItem, &Info->ItemsNumber, Info->OpMode));
}

void WINAPI FreeFindDataW(const FreeFindDataInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	Browser.FreeFindData(Info->PanelItem, Info->ItemsNumber);
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	Browser.GetOpenPanelInfo(Info);
}

intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	return Browser.SetDirectory(Info->Dir, Info->OpMode);
}

intptr_t WINAPI DeleteFilesW(const DeleteFilesInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	return Browser.DeleteFiles(Info->PanelItem, Info->ItemsNumber, Info->OpMode);
}

intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	return Browser.ProcessKey(&Info->Rec);
}

intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo* Info)
{
	auto& Browser = *static_cast<NetBrowser*>(Info->hPanel);
	return Browser.ProcessEvent(Info->Event, Info->Param);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* Info)
{
	return Config();
}

void WINAPI ExitFARW(const ExitInfo* Info)
{
	delete CommonRootResources;
}

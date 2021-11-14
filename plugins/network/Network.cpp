#include "Network.hpp"
#include "version.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

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

//-----------------------------------------------------------------------------
HANDLE WINAPI OpenW(const OpenInfo* Info)
{
	HANDLE hPlugin = new NetBrowser;

	if (!hPlugin)
		return nullptr;

	NetBrowser* Browser = (NetBrowser*)hPlugin;

	if (Info->OpenFrom == OPEN_COMMANDLINE)
	{
		int I = 0;
		auto cmd = const_cast<wchar_t*>(reinterpret_cast<OpenCommandLineInfo*>(Info->Data)->CommandLine); //BUGBUG
		wchar_t* p = wcschr(cmd, L':');

		if (!p || !*p)
		{
			delete Browser;
			return nullptr;
		}

		*p++ = L'\0';
		bool netg;

		if (!lstrcmpi(cmd, L"netg"))
			netg = true;
		else if (!lstrcmpi(cmd, L"net"))
			netg = false;
		else
		{
			delete Browser;
			return nullptr;
		}

		cmd = p;

		if (lstrlen(FSF.Trim(cmd)))
		{
			if (cmd[0] == L'/')
				cmd[0] = L'\\';

			if (cmd[1] == L'/')
				cmd[1] = L'\\';

			if (!netg && !Opt.NavigateToDomains)
			{
				if (cmd[0] == L'\\' && cmd[1] != L'\\')
					I = 1;
				else if (cmd[0] != L'\\' && cmd[1] != L'\\')
					I = 2;
			}

			wchar_t Path[MAX_PATH] = L"\\\\";
			lstrcpy(Path + I, cmd);
			FSF.Unquote(Path);
			// Expanding environment variables.
			{
				wchar_t PathCopy[MAX_PATH];
				lstrcpy(PathCopy, Path);
				ExpandEnvironmentStrings(PathCopy, Path, ARRAYSIZE(Path));
			}
			Browser->SetOpenFromCommandLine(Path);
		}
	}
		/* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
			mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
			in later versions. Please DON'T use it in your plugins. */
	else if (Info->OpenFrom == OPEN_FILEPANEL)
	{
		if (!Browser->SetOpenFromFilePanel((wchar_t*)Info->Data))
		{
			// we don't support upwards browsing from NetWare shares -
			// it doesn't work correctly
			delete Browser;
			return nullptr;
		}
	}
	else
	{
		if (IsFirstRun && Opt.LocalNetwork)
			Browser->GotoLocalNetwork();
	}

	IsFirstRun = FALSE;
	wchar_t szCurrDir[MAX_PATH];

	if (GetCurrentDirectory(ARRAYSIZE(szCurrDir), szCurrDir))
	{
		if (*szCurrDir == L'\\' && GetSystemDirectory(szCurrDir, ARRAYSIZE(szCurrDir)))
		{
			szCurrDir[2] = L'\0';
			SetCurrentDirectory(szCurrDir);
		}
	}

	return (hPlugin);
}

//-----------------------------------------------------------------------------
void WINAPI ClosePanelW(const ClosePanelInfo* Info)
{
	delete(NetBrowser*)Info->hPanel;
}

//-----------------------------------------------------------------------------
intptr_t WINAPI GetFindDataW(GetFindDataInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	return (Browser->GetFindData(&Info->PanelItem, &Info->ItemsNumber, Info->OpMode));
}

//-----------------------------------------------------------------------------
void WINAPI FreeFindDataW(const FreeFindDataInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	Browser->FreeFindData(Info->PanelItem, (int)Info->ItemsNumber);
}

//-----------------------------------------------------------------------------
void WINAPI GetOpenPanelInfoW(OpenPanelInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	Browser->GetOpenPanelInfo(Info);
}

//-----------------------------------------------------------------------------
intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	return (Browser->SetDirectory(Info->Dir, Info->OpMode));
}

//-----------------------------------------------------------------------------
intptr_t WINAPI DeleteFilesW(const DeleteFilesInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	return (Browser->DeleteFiles(Info->PanelItem, (int)Info->ItemsNumber, Info->OpMode));
}

//-----------------------------------------------------------------------------
intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	return (Browser->ProcessKey(&Info->Rec));
}

//-----------------------------------------------------------------------------
intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo* Info)
{
	NetBrowser* Browser = (NetBrowser*)Info->hPanel;
	return Browser->ProcessEvent(Info->Event, Info->Param);
}

//-----------------------------------------------------------------------------

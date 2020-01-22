#include <stdio.h>
#include <time.h>
#include <initguid.h>
#include "guid.hpp"
#include "Proclist.hpp"
#include "Proclng.hpp"
#include "version.hpp"

_Opt Opt;
ui64Table *_ui64Table;

PluginStartupInfo Info;
FarStandardFunctions FSF;
wchar_t *PluginRootKey;

//-----------------------------------------------------------------------------
static LONG WINAPI fNtQueryInformationProcess(
    HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryInformationProcess pNtQueryInformationProcess = fNtQueryInformationProcess;

static LONG WINAPI fNtQueryInformationThread(HANDLE, ULONG, PVOID, DWORD, DWORD*)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryInformationThread pNtQueryInformationThread = fNtQueryInformationThread;

static LONG WINAPI fNtQueryObject(HANDLE, DWORD, VOID*, DWORD, VOID*)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryObject pNtQueryObject = fNtQueryObject;

static LONG WINAPI fNtQuerySystemInformation(DWORD, VOID*, DWORD, ULONG*)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQuerySystemInformation pNtQuerySystemInformation = fNtQuerySystemInformation;

static LONG WINAPI fNtQueryInformationFile(HANDLE, PVOID, PVOID, DWORD, DWORD)
{ return STATUS_NOT_IMPLEMENTED; }
PNtQueryInformationFile pNtQueryInformationFile = fNtQueryInformationFile;


static BOOL WINAPI fIsWow64Process(HANDLE, PBOOL) { return FALSE; }
PIsWow64Process pIsWow64Process = fIsWow64Process;

static DWORD WINAPI fGetGuiResources(HANDLE, DWORD) { return 0; }
PGetGuiResources pGetGuiResources = fGetGuiResources;


static BOOL WINAPI fIsValidSid(PSID) { return FALSE; }
PIsValidSid pIsValidSid = fIsValidSid;

static PSID_IDENTIFIER_AUTHORITY WINAPI fGetSidIdentifierAuthority(PSID)
{ return NULL; }
PGetSidIdentifierAuthority pGetSidIdentifierAuthority = fGetSidIdentifierAuthority;

static PUCHAR WINAPI fGetSidSubAuthorityCount(PSID) { return NULL; }
PGetSidSubAuthorityCount pGetSidSubAuthorityCount = fGetSidSubAuthorityCount;

static PDWORD WINAPI fGetSidSubAuthority(PSID, DWORD) { return NULL; }
PGetSidSubAuthority pGetSidSubAuthority = fGetSidSubAuthority;

static BOOL WINAPI fLookupAccountName(LPCTSTR,LPCTSTR,PSID,LPDWORD,LPTSTR,LPDWORD,PSID_NAME_USE)
{ return FALSE; }
PLookupAccountName pLookupAccountName = fLookupAccountName;


static HRESULT WINAPI fCoSetProxyBlanket(
    IUnknown*, DWORD, DWORD, OLECHAR*, DWORD, DWORD, RPC_AUTH_IDENTITY_HANDLE, DWORD)
{ return E_FAIL; }
PCoSetProxyBlanket pCoSetProxyBlanket = fCoSetProxyBlanket;


static BOOL WINAPI fEnumProcessModulesEx(HANDLE, HMODULE*, DWORD, DWORD*, DWORD)
{ return FALSE; }
PEnumProcessModulesEx pEnumProcessModulesEx = fEnumProcessModulesEx;


static void dynamic_bind(void)
{
	static BOOL Inited;

	if (!Inited)
	{
		HMODULE h;
		FARPROC f;

		if ((h = GetModuleHandle(L"ntdll")) != NULL)
		{
			if ((f = GetProcAddress(h, "NtQueryInformationProcess")) != NULL)
				pNtQueryInformationProcess = (PNtQueryInformationProcess)f;

			if ((f = GetProcAddress(h, "NtQueryInformationThread")) != NULL)
				pNtQueryInformationThread = (PNtQueryInformationThread)f;

			if ((f = GetProcAddress(h, "NtQueryObject")) != NULL)
				pNtQueryObject = (PNtQueryObject)f;

			if ((f = GetProcAddress(h, "NtQuerySystemInformation")) != NULL)
				pNtQuerySystemInformation = (PNtQuerySystemInformation)f;

			if ((f = GetProcAddress(h, "NtQueryInformationFile")) != NULL)
				pNtQueryInformationFile = (PNtQueryInformationFile)f;
		}

		if ((h = GetModuleHandle(L"kernel32")) != NULL)
		{
			if ((f = GetProcAddress(h, "IsWow64Process")) != NULL)
				pIsWow64Process = (PIsWow64Process)f;
		}

		if ((h = GetModuleHandle(L"advapi32")) != NULL)
		{
			if ((f = GetProcAddress(h, "IsValidSid")) != NULL)
				pIsValidSid = (PIsValidSid)f;

			if ((f = GetProcAddress(h, "GetSidIdentifierAuthority")) != NULL)
				pGetSidIdentifierAuthority = (PGetSidIdentifierAuthority)f;

			if ((f = GetProcAddress(h, "GetSidSubAuthorityCount")) != NULL)
				pGetSidSubAuthorityCount = (PGetSidSubAuthorityCount)f;

			if ((f = GetProcAddress(h, "GetSidSubAuthority")) != NULL)
				pGetSidSubAuthority = (PGetSidSubAuthority)f;

			if ((f = GetProcAddress(h, "LookupAccountNameW" )) != NULL)
				pLookupAccountName = (PLookupAccountName)f;
		}

		if ((h = GetModuleHandle(L"user32")) != NULL)
		{
			if ((f = GetProcAddress(h, "GetGuiResources")) != NULL)
				pGetGuiResources = (PGetGuiResources)f;
		}

		if ((h = GetModuleHandle(L"ole32")) != NULL)
		{
			if ((f = GetProcAddress(h, "CoSetProxyBlanket")) != NULL)
				pCoSetProxyBlanket = (PCoSetProxyBlanket)f;
		}

		if ((h = GetModuleHandle(L"psapi")) != NULL)
		{
			if ((f = GetProcAddress(h, "EnumProcessModulesEx")) != NULL)
				pEnumProcessModulesEx = (PEnumProcessModulesEx)f;
		}

		Inited = TRUE;
	}
}

int Message(unsigned Flags, wchar_t *HelpTopic, LPCTSTR*Items, int nItems, int nButtons)
{
	return (int)Info.Message(&MainGuid, nullptr, Flags, HelpTopic, Items, nItems, nButtons);
}

//-----------------------------------------------------------------------------
void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}


void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	dynamic_bind();
	::Info = *Info;
	FSF = *Info->FSF;
	::Info.FSF = &FSF;
	_ui64Table = new ui64Table;
	Opt.Read();
	DebugToken::CreateToken();
}


void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	free(PluginRootKey);
	delete _ui64Table;
}


HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	Opt.Read();
	Plist* hPlugin = new Plist();

	if (OInfo->OpenFrom == OPEN_COMMANDLINE && (NORM_M_PREFIX(reinterpret_cast<OpenCommandLineInfo*>(OInfo->Data)->CommandLine)))
	{
		if (!hPlugin->Connect(reinterpret_cast<OpenCommandLineInfo*>(OInfo->Data)->CommandLine))
		{
			delete hPlugin;
			hPlugin = nullptr;
		}
	}

	return hPlugin;
}


void WINAPI ClosePanelW(const struct ClosePanelInfo *Info)
{
	delete(Plist *)Info->hPanel;
}


intptr_t WINAPI GetFindDataW(struct GetFindDataInfo *Info)
{
	Plist *Panel=(Plist *)Info->hPanel;
	return Panel->GetFindData(Info->PanelItem,Info->ItemsNumber,Info->OpMode);
}


void   WINAPI FreeFindDataW(const struct FreeFindDataInfo *Info)
{
	Plist *Panel=(Plist *)Info->hPanel;
	Panel->FreeFindData(Info->PanelItem,(int)Info->ItemsNumber);
}


void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_NONE;

	if (Opt.AddToPluginsMenu)
	{
		static const wchar_t *PluginMenuStrings[1];
		PluginMenuStrings[0]=GetMsg(MPlistPanel);
		Info->PluginMenu.Guids=&MenuGuid;
		Info->PluginMenu.Strings=PluginMenuStrings;
		Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	}

	if (Opt.AddToDisksMenu)
	{
		static const wchar_t *DiskMenuStrings[1];
		DiskMenuStrings[0]=GetMsg(MPlistPanel);
    	Info->DiskMenu.Guids=&MenuGuid;
    	Info->DiskMenu.Strings=DiskMenuStrings;
    	Info->DiskMenu.Count=ARRAYSIZE(DiskMenuStrings);
	}

	static const wchar_t *PluginCfgStrings[1];
	PluginCfgStrings[0]=GetMsg(MPlistPanel);
	Info->PluginConfig.Guids=&MenuGuid;
	Info->PluginConfig.Strings=PluginCfgStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginCfgStrings);

	Info->CommandPrefix = L"plist";
}


void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
	Plist *Panel=(Plist *)Info->hPanel;
	Panel->GetOpenPanelInfo(Info);
}


intptr_t WINAPI GetFilesW(struct GetFilesInfo *Info)
{
	return ((Plist *)Info->hPanel)->GetFiles(Info->PanelItem,(int)Info->ItemsNumber,Info->Move,&Info->DestPath,Info->OpMode);
}


intptr_t WINAPI DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	return ((Plist *)Info->hPanel)->DeleteFiles(Info->PanelItem,(int)Info->ItemsNumber,Info->OpMode);
}


intptr_t WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	return ((Plist *)Info->hPanel)->ProcessEvent(Info->Event,Info->Param);
}


intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	return ((Plist *)Info->hPanel)->ProcessKey(&Info->Rec);
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *Info)
{
	return Config();
}

intptr_t WINAPI CompareW(const struct CompareInfo *Info)
{
	return ((Plist *)Info->hPanel)->Compare(Info->Item1, Info->Item2, Info->Mode);
}

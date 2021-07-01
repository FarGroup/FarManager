#include <cstdio>
#include <ctime>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "version.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

options Opt;

PluginStartupInfo PsInfo;
FarStandardFunctions FSF;

//-----------------------------------------------------------------------------
static NTSTATUS NTAPI fNtQueryInformationProcess(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG)
{
	return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS NTAPI fNtQueryInformationThread(HANDLE, ULONG, PVOID, DWORD, DWORD*)
{
	return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS NTAPI fNtQueryObject(HANDLE, DWORD, VOID*, DWORD, VOID*)
{
	return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS NTAPI fNtQuerySystemInformation(DWORD, VOID*, DWORD, ULONG*)
{
	return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS NTAPI fNtQueryInformationFile(HANDLE, PVOID, PVOID, DWORD, DWORD)
{
	return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS NTAPI fNtWow64ReadVirtualMemory64(HANDLE, ULONG64, PVOID, ULONG64, PULONG64)
{
	return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS NTAPI fNtWow64QueryInformationProcess64(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG)
{
	return STATUS_NOT_IMPLEMENTED;
}

static BOOL WINAPI fIsWow64Process(HANDLE, PBOOL)
{
	return FALSE;
}

static DWORD WINAPI fGetGuiResources(HANDLE, DWORD)
{
	return 0;
}

static BOOL WINAPI fIsValidSid(PSID)
{
	return FALSE;
}

static PSID_IDENTIFIER_AUTHORITY WINAPI fGetSidIdentifierAuthority(PSID)
{
	return {};
}

static PUCHAR WINAPI fGetSidSubAuthorityCount(PSID)
{
	return {};
}

static PDWORD WINAPI fGetSidSubAuthority(PSID, DWORD)
{
	return {};
}

static HRESULT WINAPI fCoSetProxyBlanket(IUnknown*, DWORD, DWORD, OLECHAR*, DWORD, DWORD, RPC_AUTH_IDENTITY_HANDLE, DWORD)
{
	return E_FAIL;
}

static BOOL WINAPI fEnumProcessModulesEx(HANDLE, HMODULE*, DWORD, DWORD*, DWORD)
{
	return FALSE;
}

#define STATIC_INIT_IMPORT(Name) \
	P ## Name p ## Name = f ## Name

STATIC_INIT_IMPORT(NtQueryInformationProcess);
STATIC_INIT_IMPORT(NtQueryInformationThread);
STATIC_INIT_IMPORT(NtQueryObject);
STATIC_INIT_IMPORT(NtQuerySystemInformation);
STATIC_INIT_IMPORT(NtQueryInformationFile);
STATIC_INIT_IMPORT(NtWow64QueryInformationProcess64);
STATIC_INIT_IMPORT(NtWow64ReadVirtualMemory64);
STATIC_INIT_IMPORT(IsWow64Process);
STATIC_INIT_IMPORT(GetGuiResources);
STATIC_INIT_IMPORT(IsValidSid);
STATIC_INIT_IMPORT(GetSidIdentifierAuthority);
STATIC_INIT_IMPORT(GetSidSubAuthorityCount);
STATIC_INIT_IMPORT(GetSidSubAuthority);
STATIC_INIT_IMPORT(CoSetProxyBlanket);
STATIC_INIT_IMPORT(EnumProcessModulesEx);

#undef STATIC_INIT_IMPORT

static void dynamic_bind()
{
	static bool Inited = false;

	if (Inited)
		return;

#define INIT_IMPORT(Name) \
	if (const auto FunctionAddress = GetProcAddress(Module, # Name)) \
		p ## Name = reinterpret_cast<P ## Name>(reinterpret_cast<void*>(FunctionAddress))


	if (const auto Module = GetModuleHandle(L"ntdll"))
	{
		INIT_IMPORT(NtQueryInformationProcess);
		INIT_IMPORT(NtQueryInformationThread);
		INIT_IMPORT(NtQueryObject);
		INIT_IMPORT(NtQuerySystemInformation);
		INIT_IMPORT(NtQueryInformationFile);
		INIT_IMPORT(NtWow64QueryInformationProcess64);
		INIT_IMPORT(NtWow64ReadVirtualMemory64);
	}

	if (const auto Module = GetModuleHandle(L"kernel32"))
	{
		INIT_IMPORT(IsWow64Process);
	}

	if (const auto Module = GetModuleHandle(L"advapi32"))
	{
		INIT_IMPORT(IsValidSid);
		INIT_IMPORT(GetSidIdentifierAuthority);
		INIT_IMPORT(GetSidSubAuthorityCount);
		INIT_IMPORT(GetSidSubAuthority);
	}

	if (const auto Module = GetModuleHandle(L"user32"))
	{
		INIT_IMPORT(GetGuiResources);
	}

	if (const auto Module = GetModuleHandle(L"ole32"))
	{
		INIT_IMPORT(CoSetProxyBlanket);
	}

	if (const auto Module = GetModuleHandle(L"psapi"))
	{
		INIT_IMPORT(EnumProcessModulesEx);
	}

#undef INIT_IMPORT

	Inited = true;
}

bool is_wow64_process(HANDLE Process)
{
	BOOL IsProcessWow64 = pIsWow64Process(Process, &IsProcessWow64) && IsProcessWow64;
	return IsProcessWow64 != FALSE;
}

int Message(unsigned Flags, const wchar_t* HelpTopic, const wchar_t** Items, size_t nItems, size_t nButtons)
{
	return static_cast<int>(PsInfo.Message(&MainGuid, {}, Flags, HelpTopic, Items, nItems, nButtons));
}

//-----------------------------------------------------------------------------
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
	dynamic_bind();
	PsInfo = *Info;
	FSF = *PsInfo.FSF;
	PsInfo.FSF = &FSF;
	Opt.Read();
	DebugToken::CreateToken();
}


HANDLE WINAPI OpenW(const OpenInfo* Info)
{
	Opt.Read();
	auto hPlugin = std::make_unique<Plist>();

	if (Info->OpenFrom == OPEN_COMMANDLINE && norm_m_prefix(reinterpret_cast<OpenCommandLineInfo*>(Info->Data)->CommandLine))
	{
		if (!hPlugin->Connect(reinterpret_cast<OpenCommandLineInfo*>(Info->Data)->CommandLine))
		{
			return {};
		}
	}

	return hPlugin.release();
}


void WINAPI ClosePanelW(const ClosePanelInfo* Info)
{
	std::unique_ptr<Plist>(static_cast<Plist*>(Info->hPanel));
}


intptr_t WINAPI GetFindDataW(GetFindDataInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	return Panel.GetFindData(Info->PanelItem, Info->ItemsNumber, Info->OpMode);
}


void WINAPI FreeFindDataW(const FreeFindDataInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	Panel.FreeFindData(Info->PanelItem, Info->ItemsNumber);
}

void WINAPI GetPluginInfoW(PluginInfo* Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = PF_NONE;

	if (Opt.AddToPluginsMenu)
	{
		static const wchar_t* PluginMenuStrings[1];
		PluginMenuStrings[0] = GetMsg(MPlistPanel);
		Info->PluginMenu.Guids = &MenuGuid;
		Info->PluginMenu.Strings = PluginMenuStrings;
		Info->PluginMenu.Count = std::size(PluginMenuStrings);
	}

	if (Opt.AddToDisksMenu)
	{
		static const wchar_t* DiskMenuStrings[1];
		DiskMenuStrings[0] = GetMsg(MPlistPanel);
		Info->DiskMenu.Guids = &MenuGuid;
		Info->DiskMenu.Strings = DiskMenuStrings;
		Info->DiskMenu.Count = std::size(DiskMenuStrings);
	}

	static const wchar_t* PluginCfgStrings[1];
	PluginCfgStrings[0] = GetMsg(MPlistPanel);
	Info->PluginConfig.Guids = &MenuGuid;
	Info->PluginConfig.Strings = PluginCfgStrings;
	Info->PluginConfig.Count = std::size(PluginCfgStrings);

	Info->CommandPrefix = L"plist";
}


void WINAPI GetOpenPanelInfoW(OpenPanelInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	Panel.GetOpenPanelInfo(Info);
}


intptr_t WINAPI GetFilesW(GetFilesInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	return Panel.GetFiles(Info->PanelItem, Info->ItemsNumber, Info->Move, &Info->DestPath, Info->OpMode);
}


intptr_t WINAPI DeleteFilesW(const DeleteFilesInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	return Panel.DeleteFiles(Info->PanelItem, Info->ItemsNumber, Info->OpMode);
}


intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	return Panel.ProcessEvent(Info->Event, Info->Param);
}


intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	return Panel.ProcessKey(&Info->Rec);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* Info)
{
	return Config();
}

intptr_t WINAPI CompareW(const CompareInfo* Info)
{
	auto& Panel = *static_cast<Plist*>(Info->hPanel);
	return Panel.Compare(Info->Item1, Info->Item2, Info->Mode);
}

intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo* Info)
{
	if (Info->Event != SE_COMMONSYNCHRO)
		return 0;

	auto& Panel = *static_cast<Plist*>(Info->Param);
	Panel.ProcessSynchroEvent();

	return 0;
}

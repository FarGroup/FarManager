#ifndef PROCLIST_HPP_71FFA62B_457B_416D_B4F5_DAB215BE015F
#define PROCLIST_HPP_71FFA62B_457B_416D_B4F5_DAB215BE015F

#pragma once

#include <memory>
#include <string>
#include <vector>

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <unknwn.h>

#include <plugin.hpp>

#include "format.hpp"


struct free_deleter
{
	void operator()(void* const Ptr) const
	{
		free(Ptr);
	}
};

template<typename T>
using malloc_ptr = std::unique_ptr<T, free_deleter>;

template<typename T>
auto make_malloc(size_t const Size)
{
	return malloc_ptr<T>(static_cast<T*>(malloc(Size)));
}

struct handle_closer
{
	void operator()(HANDLE Handle) const
	{
		CloseHandle(Handle);
	}
};

using handle = std::unique_ptr<void, handle_closer>;

inline HANDLE normalise_handle(HANDLE Handle)
{
	return Handle == INVALID_HANDLE_VALUE? nullptr : Handle;
}

struct local_deleter
{
	void operator()(const void* MemoryBlock) const
	{
		LocalFree(const_cast<HLOCAL>(MemoryBlock));
	}
};

template<typename T>
using local_ptr = std::unique_ptr<T, local_deleter>;


#ifdef _MSC_VER
#pragma hdrstop
#  pragma comment( lib, "version.lib" )
#endif

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#  define BELOW_NORMAL_PRIORITY_CLASS 0x00004000
#  define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
typedef unsigned long ULONG_PTR, * PULONG_PTR;
#endif

inline constexpr auto
	NPANELMODES     = 10,      // Number of panel modes
	MAX_CUSTOM_COLS = 20,      // Max number of custom cols in any panel mode
	MAX_DATETIME    = 50,
	MAXCOLS = MAX_CUSTOM_COLS + 4;

extern PluginStartupInfo PsInfo;
extern FarStandardFunctions FSF;

class PerfThread;
class WMIConnection;

extern struct options
{
	int AddToDisksMenu;
	int AddToPluginsMenu;
	int ExportEnvironment;
	int ExportModuleInfo;
	int ExportModuleVersion;
	int ExportPerformance;
	int ExportHandles;
	int ExportHandlesUnnamed;
	int EnableWMI;

	void Read();
	void Write() const;
}
Opt;


int Message(unsigned Flags, const wchar_t* HelpTopic, const wchar_t** Items, size_t nItems, size_t nButtons = 1);

struct columns
{
	std::wstring
		internal_types,
		widths,
		far_types;
};

struct mode
{
	columns
		panel_columns,
		status_columns;
	PANELMODE_FLAGS Flags;
};

class Plist
{
public:
	Plist();
	~Plist();
	bool Connect(const wchar_t* pMachine, const wchar_t* pUser = {}, const wchar_t* pPasw = {});
	int GetFindData(PluginPanelItem*& pPanelItem, size_t& pItemsNumber, OPERATION_MODES OpMode);
	static void FreeFindData(PluginPanelItem* PanelItem, size_t ItemsNumber);
	void GetOpenPanelInfo(OpenPanelInfo* Info);
	int GetFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, int Move, const wchar_t** DestPath, OPERATION_MODES OpMode, options& LocalOpt = Opt);
	int DeleteFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, OPERATION_MODES OpMode);
	int ProcessEvent(intptr_t Event, void* Param);
	int Compare(const PluginPanelItem* Item1, const PluginPanelItem* Item2, unsigned int Mode) const;
	int ProcessKey(const INPUT_RECORD* Rec);
	void ProcessSynchroEvent();
	PanelMode* PanelModes(size_t& nModes);

	static bool GetVersionInfo(const wchar_t* pFullPath, std::unique_ptr<char[]>& Buffer, const wchar_t*& pVersion, const wchar_t*& pDesc);

	static void SavePanelModes();
	static void InitializePanelModes();
	static bool PanelModesInitialized() { return !m_PanelModesDataLocal.empty(); }

private:
	static void PrintVersionInfo(HANDLE InfoFile, const wchar_t* FullPath);
	void Reread();
	void PutToCmdLine(const wchar_t* tmp);
	static int Menu(unsigned int Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const FarKey* BreakKeys, const FarMenuItem* Items, size_t ItemsNumber);
	void PrintOwnerInfo(HANDLE InfoFile, DWORD dwPid);
	bool ConnectWMI();
	void DisconnectWMI();
	void WmiError() const;

	DWORD LastUpdateTime{};
	std::wstring HostName;
	std::unique_ptr<PerfThread> pPerfThread;
	unsigned StartPanelMode{};
	unsigned SortMode{};
	std::unique_ptr<WMIConnection> pWMI;
	DWORD dwPluginThread;

	static inline std::vector<mode> m_PanelModesDataLocal, m_PanelModesDataRemote;

	mutable class refresh_lock
	{
	public:
		refresh_lock(const refresh_lock&) = delete;
		refresh_lock& operator=(const refresh_lock&) = delete;

		refresh_lock() = default;

		void lock() { ++m_Busy; }
		void unlock() { --m_Busy;}

		bool is_busy() const { return m_Busy != 0; }

	private:
		size_t m_Busy{};
	}
	m_RefreshLock;
};

struct InitDialogItem
{
	unsigned char Type;
	unsigned char X1, Y1, X2, Y2;
	unsigned char Focus;
	DWORD_PTR Selected;
	unsigned int Flags;
	unsigned char DefaultButton;
	const wchar_t* Data;
};

struct ProcessData
{
	DWORD Size{};
	HWND hwnd{};
	//  DWORD Threads;
	DWORD dwPID{};
	DWORD dwParentPID{};
	DWORD dwPrBase{};
	int Bitness{};
	std::wstring FullPath;
	uint64_t dwElapsedTime{};
	std::wstring CommandLine;
};

class PerfThread;

bool GetList(PluginPanelItem*& pPanelItem, size_t& ItemsNumber, PerfThread& PThread);
bool KillProcess(DWORD pid, HWND hwnd);

const wchar_t* GetMsg(int MsgId);
//void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item, int ItemsNumber);
//int LocalStricmp(wchar_t *Str1,wchar_t *Str2);
void ConvertDate(const FILETIME& ft, wchar_t* DateText, wchar_t* TimeText);
int Config();

void SetRegKey(const wchar_t* Key, const wchar_t* ValueName, const wchar_t* ValueData);
void SetRegKey(const wchar_t* Key, const wchar_t* ValueName, DWORD ValueData);
void SetRegKey(const wchar_t* Key, const wchar_t* ValueName, LPBYTE ValueData, DWORD ValueSize);
int GetRegKey(const wchar_t* Key, const wchar_t* ValueName, wchar_t* ValueData, const wchar_t* Default, DWORD DataSize);
int GetRegKey(const wchar_t* Key, const wchar_t* ValueName, LPBYTE ValueData, LPBYTE Default, DWORD DataSize);
int GetRegKey(const wchar_t* Key, const wchar_t* ValueName, int& ValueData, DWORD Default);
int GetRegKey(const wchar_t* Key, const wchar_t* ValueName, DWORD Default);
void DeleteRegKey(const wchar_t* Key);

int WinError(const wchar_t* pSourceModule = {});

class DebugToken
{
public:
	~DebugToken() { Revert(); }

	bool Enable();
	bool Revert();

	static bool CreateToken();
	static void CloseToken();

	// Saved impersonation token
	handle hSavedToken;
	bool saved{};
	bool enabled{};
};

void GetOpenProcessData(
	HANDLE hProcess,
	std::wstring* ProcessName = {},
	std::wstring* FullPath = {},
	std::wstring* CommandLine = {},
	std::wstring* CurDir = {},
	std::wstring* EnvStrings = {}
);

HANDLE OpenProcessForced(DebugToken* token, DWORD dwFlags, DWORD dwProcessId, BOOL bInh = FALSE);

enum
{
	SM_PROCLIST_CUSTOM = 64,
	SM_PROCLIST_PID,
	SM_PROCLIST_PARENTPID,
	SM_PROCLIST_PRIOR,
	SM_PROCLIST_PERFCOUNTER,
	SM_PROCLIST_PERSEC = 128
};

extern wchar_t CustomColumns[10][10];

void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment);
void PrintModules(HANDLE InfoFile, DWORD dwPID, options& LocalOpt);
bool PrintHandleInfo(DWORD dwPID, HANDLE file, bool bIncludeUnnamed, PerfThread* pThread);
struct ProcessPerfData;
bool GetPData(ProcessData& pdata, const ProcessPerfData& pd);

//------
#define DECLARE_IMPORT(Name, ...) \
using  P ## Name = __VA_ARGS__; \
extern P ## Name p ## Name

DECLARE_IMPORT(NtQueryInformationProcess, NTSTATUS (NTAPI*)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG));
DECLARE_IMPORT(NtQueryInformationThread, NTSTATUS (NTAPI*)(HANDLE, ULONG, PVOID, DWORD, DWORD*));
DECLARE_IMPORT(NtQueryObject, NTSTATUS (NTAPI*)(HANDLE, DWORD, VOID*, DWORD, VOID*));
DECLARE_IMPORT(NtQuerySystemInformation, NTSTATUS (NTAPI*)(DWORD, VOID*, DWORD, ULONG*));
DECLARE_IMPORT(NtQueryInformationFile, NTSTATUS (NTAPI*)(HANDLE, PVOID, PVOID, DWORD, DWORD));
DECLARE_IMPORT(NtWow64QueryInformationProcess64, NTSTATUS(NTAPI*)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG));
DECLARE_IMPORT(NtWow64ReadVirtualMemory64, NTSTATUS (NTAPI*)(HANDLE, ULONG64, PVOID, ULONG64, PULONG64));

DECLARE_IMPORT(IsValidSid, BOOL (WINAPI*)(PSID));
DECLARE_IMPORT(GetSidIdentifierAuthority, PSID_IDENTIFIER_AUTHORITY (WINAPI*)(PSID));
DECLARE_IMPORT(GetSidSubAuthorityCount, PUCHAR (WINAPI*)(PSID));
DECLARE_IMPORT(GetSidSubAuthority, PDWORD (WINAPI*)(PSID, DWORD));
DECLARE_IMPORT(IsWow64Process, BOOL (WINAPI*)(HANDLE hProcess, PBOOL Wow64Process));
DECLARE_IMPORT(GetGuiResources, DWORD (WINAPI*)(HANDLE hProcess, DWORD uiFlags));
DECLARE_IMPORT(CoSetProxyBlanket, HRESULT (WINAPI*)(IUnknown*, DWORD, DWORD, OLECHAR*, DWORD, DWORD, RPC_AUTH_IDENTITY_HANDLE, DWORD));
DECLARE_IMPORT(EnumProcessModulesEx, BOOL (WINAPI*)(HANDLE, HMODULE*, DWORD, DWORD*, DWORD));

#undef DECLARE_IMPORT
//------

bool is_wow64_process(HANDLE Process);

std::wstring DurationToText(uint64_t Duration);
std::wstring FileTimeDifferenceToText(const FILETIME& CurFileTime, const FILETIME& SrcTime);

size_t WriteToFile(HANDLE File, const std::wstring_view& Str);
size_t WriteToFile(HANDLE File, wchar_t Char);

//-------
inline bool norm_m_prefix(const wchar_t* Str)
{
	return Str[0] == L'\\' && Str[1] == L'\\';
}

inline namespace literals
{
	using namespace std::literals;
}

#endif // PROCLIST_HPP_71FFA62B_457B_416D_B4F5_DAB215BE015F

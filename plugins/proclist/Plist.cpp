#include <algorithm>
#include <mutex>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"

#include <psapi.h>

using namespace std::literals;

struct UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
};

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
	USHORT                  Flags;
	USHORT                  Length;
	ULONG                   TimeStamp;
	UNICODE_STRING          DosPath;
} RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;


typedef struct _RTL_USER_PROCESS_PARAMETERS
{
	ULONG                   MaximumLength;
	ULONG                   Length;
	ULONG                   Flags;
	ULONG                   DebugFlags;
	PVOID                   ConsoleHandle;
	ULONG                   ConsoleFlags;
	HANDLE                  StdInputHandle;
	HANDLE                  StdOutputHandle;
	HANDLE                  StdErrorHandle;
	UNICODE_STRING          CurrentDirectoryPath;
	HANDLE                  CurrentDirectoryHandle;
	UNICODE_STRING          DllPath;
	UNICODE_STRING          ImagePathName;
	UNICODE_STRING          CommandLine;
	PVOID                   EnvironmentBlock;
	ULONG                   StartingPositionLeft;
	ULONG                   StartingPositionTop;
	ULONG                   Width;
	ULONG                   Height;
	ULONG                   CharWidth;
	ULONG                   CharHeight;
	ULONG                   ConsoleTextAttributes;
	ULONG                   WindowFlags;
	ULONG                   ShowWindowFlags;
	UNICODE_STRING          WindowTitle;
	UNICODE_STRING          DesktopName;
	UNICODE_STRING          ShellInfo;
	UNICODE_STRING          RuntimeData;
	RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];

} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS, PROCESS_PARAMETERS;



typedef struct _LDR_MODULE
{
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	ULONG                   SizeOfImage;
	UNICODE_STRING          FullDllName;
	UNICODE_STRING          BaseDllName;
	ULONG                   Flags;
	SHORT                   LoadCount;
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	ULONG                   TimeDateStamp;
} LDR_MODULE, ModuleData, * PLDR_MODULE;


typedef struct _PEB_LDR_DATA
{
	ULONG                   Length;
	BOOLEAN                 Initialized;
	PVOID                   SsHandle;
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB
{
	BOOLEAN                 InheritedAddressSpace;
	BOOLEAN                 ReadImageFileExecOptions;
	BOOLEAN                 BeingDebugged;
	BOOLEAN                 Spare;
	HANDLE                  Mutant;
	PVOID                   ImageBaseAddress;
	PPEB_LDR_DATA           LoaderData;
	PROCESS_PARAMETERS* ProcessParameters;
	PVOID                   SubSystemData;
	PVOID                   ProcessHeap;
	PVOID                   FastPebLock;
	//  PPEBLOCKROUTINE         FastPebLockRoutine;
	PVOID         FastPebLockRoutine;
	//  PPEBLOCKROUTINE         FastPebUnlockRoutine;
	PVOID         FastPebUnlockRoutine;
	ULONG                   EnvironmentUpdateCount;
	PVOID* KernelCallbackTable;
	PVOID                   EventLogSection;
	PVOID                   EventLog;
	//  PPEB_FREE_BLOCK         FreeList;
	PVOID         FreeList;
	ULONG                   TlsExpansionCounter;
	PVOID                   TlsBitmap;
	ULONG                   TlsBitmapBits[0x2];
	PVOID                   ReadOnlySharedMemoryBase;
	PVOID                   ReadOnlySharedMemoryHeap;
	PVOID* ReadOnlyStaticServerData;
	PVOID                   AnsiCodePageData;
	PVOID                   OemCodePageData;
	PVOID                   UnicodeCaseTableData;
	ULONG                   NumberOfProcessors;
	ULONG                   NtGlobalFlag;
	BYTE                    Spare2[0x4];
	LARGE_INTEGER           CriticalSectionTimeout;
	ULONG                   HeapSegmentReserve;
	ULONG                   HeapSegmentCommit;
	ULONG                   HeapDeCommitTotalFreeThreshold;
	ULONG                   HeapDeCommitFreeBlockThreshold;
	ULONG                   NumberOfHeaps;
	ULONG                   MaximumNumberOfHeaps;
	PVOID** ProcessHeaps;
	PVOID                   GdiSharedHandleTable;
	PVOID                   ProcessStarterHelper;
	PVOID                   GdiDCAttributeList;
	PVOID                   LoaderLock;
	ULONG                   OSMajorVersion;
	ULONG                   OSMinorVersion;
	ULONG                   OSBuildNumber;
	ULONG                   OSPlatformId;
	ULONG                   ImageSubSystem;
	ULONG                   ImageSubSystemMajorVersion;
	ULONG                   ImageSubSystemMinorVersion;
	ULONG                   GdiHandleBuffer[0x22];
	ULONG                   PostProcessInitRoutine;
	ULONG                   TlsExpansionBitmap;
	BYTE                    TlsExpansionBitmapBits[0x80];
	ULONG                   SessionId;
} PEB, * PPEB;

typedef struct _PROCESS_BASIC_INFORMATION
{
	PVOID Reserved1;
	PPEB PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

BOOL GetInternalProcessData(HANDLE hProcess, ModuleData* Data, PROCESS_PARAMETERS*& pProcessParams, char*& pEnd, bool bFirstModule = false)
{
	DWORD ret;
	// From ntddk.h
	PROCESS_BASIC_INFORMATION processInfo;

	if (pNtQueryInformationProcess(hProcess, ProcessBasicInformation, &processInfo, sizeof(processInfo), &ret))
		return FALSE;

	//FindModule, obtained from PSAPI.DLL
	PEB peb;
	PEB_LDR_DATA pld;

	if (ReadProcessMemory(hProcess, processInfo.PebBaseAddress, &peb, sizeof(peb), {}) &&
		ReadProcessMemory(hProcess, peb.LoaderData, &pld, sizeof(pld), {}))
	{
		//pEnd = (void *)((void *)peb.LoaderData+((void *)&pld.InMemoryOrderModuleList-(void *)&pld));
		const auto hModule = peb.ImageBaseAddress;
		pProcessParams = peb.ProcessParameters;
		pEnd = reinterpret_cast<char*>(peb.LoaderData) + sizeof(pld) - sizeof(LIST_ENTRY) * 2;
		auto p4 = reinterpret_cast<char*>(pld.InMemoryOrderModuleList.Flink);

		while (p4)
		{
			if (p4 == pEnd || !ReadProcessMemory(hProcess, p4 - sizeof(PVOID) * 2, Data, sizeof(*Data), {}))
				return FALSE;

			if (bFirstModule)
				return TRUE;

			if (Data->BaseAddress == hModule) break;

			p4 = reinterpret_cast<char*>(Data->InMemoryOrderModuleList.Flink);
		}
	}

	return TRUE;
}

HANDLE OpenProcessForced(DebugToken* const token, DWORD const Flags, DWORD const ProcessId, BOOL const Inh)
{
	if (const auto Process = OpenProcess(Flags, Inh, ProcessId))
		return Process;

	if (GetLastError() == ERROR_ACCESS_DENIED && token->Enable())
		return OpenProcess(Flags, Inh, ProcessId);

	return {};
}

bool GetPData(ProcessData& Data, const ProcessPerfData& pd)
{
	Data.Size = sizeof(Data);
	Data.dwPID = pd.dwProcessId;
	Data.dwPrBase = pd.dwProcessPriority;
	Data.dwParentPID = pd.dwCreatingPID;
	Data.dwElapsedTime = pd.dwElapsedTime;
	Data.FullPath.assign(pd.FullPath, !std::wmemcmp(pd.FullPath.data(), L"\\??\\", 4)? 4 : 0, Data.FullPath.npos); // gcc 7.3-8.1 bug: npos required. TODO: Remove after we move to 8.2 or later
	Data.CommandLine = pd.CommandLine;
	Data.Bitness = pd.Bitness;
	return true;
}

static void WINAPI FreeUserData(void* const UserData, const FarPanelItemFreeInfo* const Info)
{
	delete static_cast<const ProcessData*>(UserData);
}

bool GetList(PluginPanelItem*& pPanelItem, size_t& ItemsNumber, PerfThread& Thread)
{
	//    Lock l(&Thread); // it's already locked in Plist::GetFindData
	FILETIME ftSystemTime;
	//Prepare system time to subtract dwElapsedTime
	GetSystemTimeAsFileTime(&ftSystemTime);
	auto pData = Thread.ProcessData();

	if (pData.empty() || !Thread.IsOK())
		return false;

	pPanelItem = new PluginPanelItem[pData.size()]{};
	ItemsNumber = pData.size();

	for (size_t i = 0; i != pData.size(); ++i)
	{
		auto& CurItem = pPanelItem[i];
		auto& pd = pData[i];
		CurItem.UserData.FreeData = FreeUserData;
		//delete CurItem.FileName;  // ???
		CurItem.FileName = new wchar_t[pd.ProcessName.size() + 1];
		*std::copy(pd.ProcessName.cbegin(), pd.ProcessName.cend(), const_cast<wchar_t*>(CurItem.FileName)) = L'\0';

		if (!pd.Owner.empty())
		{
			CurItem.Owner = new wchar_t[pd.Owner.size() + 1];
			*std::copy(pd.Owner.cbegin(), pd.Owner.cend(), const_cast<wchar_t*>(CurItem.Owner)) = L'\0';
		}

		CurItem.UserData.Data = new ProcessData();

		if (!pd.ftCreation.dwHighDateTime && pd.dwElapsedTime)
		{
			ULARGE_INTEGER St;
			St.LowPart = ftSystemTime.dwLowDateTime;
			St.HighPart = ftSystemTime.dwHighDateTime;
			ULARGE_INTEGER Cr;
			Cr.QuadPart = St.QuadPart - pd.dwElapsedTime * 10000000;
			pd.ftCreation.dwLowDateTime = Cr.LowPart;
			pd.ftCreation.dwHighDateTime = Cr.HighPart;
		}

		CurItem.CreationTime = CurItem.LastWriteTime = CurItem.LastAccessTime = pd.ftCreation;
		const auto ullSize = pd.qwCounters[IDX_WORKINGSET] + pd.qwCounters[IDX_PAGEFILE];
		CurItem.FileSize = ullSize;
		CurItem.AllocationSize = pd.qwResults[IDX_PAGEFILE];
		//yjh:???      CurItem.AllocationSize = pd.dwProcessId;

		CurItem.AlternateFileName = new wchar_t[16];
		FSF.itoa(pd.dwProcessId, (wchar_t*)CurItem.AlternateFileName, 10);

		CurItem.NumberOfLinks = pd.dwThreads;
		GetPData(*static_cast<ProcessData*>(CurItem.UserData.Data), pd);

		if (pd.dwProcessId == 0 && pd.dwThreads > 5) //_Total
			CurItem.FileAttributes |= FILE_ATTRIBUTE_HIDDEN;

		if (pd.Bitness != Thread.GetDefaultBitness())
			CurItem.FileAttributes |= FILE_ATTRIBUTE_READONLY;

	}

	return true;
}

static bool find_terminator(const std::wstring& Str)
{
	for (auto i = Str.cbegin(); i != Str.cend() - 1; ++i)
		if (!*i && !*(i + 1))
			return true;

	return false;
}

static std::wstring read_string(HANDLE Process, const UNICODE_STRING& Str)
{
	std::wstring Result(Str.Length / sizeof(wchar_t), 0);
	if (!ReadProcessMemory(Process, Str.Buffer, Result.data(), Result.size() * sizeof(wchar_t), {}))
		return {};

	return Result;
}

static std::wstring read_string(HANDLE Process, const void* Address)
{
	UNICODE_STRING Str;
	if (!ReadProcessMemory(Process, Address, &Str, sizeof(Str), {}))
		return {};

	return read_string(Process, Str);
}

void GetOpenProcessData(
	HANDLE hProcess,
	std::wstring* ProcessName,
	std::wstring* FullPath,
	std::wstring* CommandLine,
	std::wstring* CurDir,
	std::wstring* EnvStrings
)
{
	ModuleData Data = {};
	char* pEnd;
	PROCESS_PARAMETERS* pProcessParams;

	if (!GetInternalProcessData(hProcess, &Data, pProcessParams, pEnd))
		return;

	if (ProcessName)
	{
		*ProcessName = read_string(hProcess, Data.BaseDllName);
	}

	if (FullPath)
	{
		*FullPath = read_string(hProcess, Data.FullDllName);
	}

	if (CommandLine)
	{
		*CommandLine = read_string(hProcess, &pProcessParams->CommandLine);
	}

	if (CurDir)
	{
		*CurDir = read_string(hProcess, &pProcessParams->CurrentDirectoryPath);
	}

	if (EnvStrings)
	{
		wchar_t* pEnv;

		if (ReadProcessMemory(hProcess, &pProcessParams->EnvironmentBlock, &pEnv, sizeof(pEnv), {}))
		{
			EnvStrings->resize(2048);

			for (;;)
			{
				if (!ReadProcessMemory(hProcess, pEnv, EnvStrings->data(), EnvStrings->size() * 2, {}))
				{
					EnvStrings->clear();
					break;
				}

				if (find_terminator(*EnvStrings))
					break;

				EnvStrings->resize(EnvStrings->size() * 2);
			}
		}
	}
}

// Debug thread token
static handle hDebugToken;

bool DebugToken::Enable()
{
	if (enabled || !hDebugToken)
		return true;

	if (HANDLE RawToken; OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &RawToken))
	{
		hSavedToken.reset(RawToken);
	}
	else
	{
		if (GetLastError() != ERROR_NO_TOKEN)
			return false;
	}

	if (!SetThreadToken({}, hDebugToken.get()))
	{
		hSavedToken = {};
		return false;
	}

	enabled = true;
	return true;
}

bool DebugToken::Revert()
{
	if (!enabled)
		return true;

	if (!SetThreadToken({}, hSavedToken.get()))
		return false;

	hSavedToken = {};

	enabled = false;
	return true;
}

bool DebugToken::CreateToken()
{
	handle ProcessToken;
	if (HANDLE RawProcessToken; !OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &RawProcessToken))
		return false;
	else
		ProcessToken.reset(RawProcessToken);

	handle Token;
	if (HANDLE RawToken; !DuplicateTokenEx(ProcessToken.get(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, {}, SecurityImpersonation, TokenImpersonation, &RawToken))
		return false;
	else
		Token.reset(RawToken);

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValue({}, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		return false;

	if (!AdjustTokenPrivileges(Token.get(), false, &tp, sizeof(tp), {}, {}))
		return false;

	hDebugToken = std::move(Token);
	return true;
}

void DebugToken::CloseToken()
{
	hDebugToken = {};
}

bool KillProcess(DWORD pid, HWND hwnd)
{
	DebugToken token;
	handle Process(OpenProcess(PROCESS_TERMINATE, FALSE, pid));

	// If access denied, try to assign debug privileges
	if (!Process && GetLastError() == ERROR_ACCESS_DENIED)
	{
		const wchar_t* MsgItems[]
		{
			GetMsg(MDeleteTitle),
			GetMsg(MCannotDeleteProc),
			GetMsg(MRetryWithDebug),
			GetMsg(MDangerous),
			GetMsg(MYes),
			GetMsg(MNo)
		};

		if (Message(FMSG_WARNING, {}, MsgItems, std::size(MsgItems), 2) != 0)
			return false;

		if (token.Enable())
			Process.reset(OpenProcess(PROCESS_TERMINATE, FALSE, pid));
	}

	if (!Process)
		return false;

	return TerminateProcess(Process.get(), ERROR_PROCESS_ABORTED);
}

void DumpNTCounters(HANDLE InfoFile, PerfThread& Thread, DWORD dwPid, DWORD dwThreads)
{
	PrintToFile(InfoFile, L'\n');
	const std::scoped_lock l(Thread);
	const auto pdata = Thread.GetProcessData(dwPid, dwThreads);
	if (!pdata)
		return;

	const PerfLib* pf = Thread.GetPerfLib();

	for (size_t i = 0; i != std::size(Counters); i++)
	{
		if (!pf->dwCounterTitles[i]) // counter is absent
			continue;

		PrintToFile(InfoFile, L"%-24s ", (GetMsg(Counters[i].idName) + L":"s).c_str());

		switch (pf->CounterTypes[i])
		{
		case PERF_COUNTER_RAWCOUNT:
		case PERF_COUNTER_LARGE_RAWCOUNT:
			// Display as is.  No Display Suffix.
			PrintToFile(InfoFile, L"%10I64u\n", pdata->qwResults[i]);
			break;

		case PERF_100NSEC_TIMER:
			// 64-bit Timer in 100 nsec units. Display delta divided by delta time. Display suffix: "%"
			PrintToFile(InfoFile, L"%s %7I64u%%\n", DurationToText(pdata->qwCounters[i]).c_str(), pdata->qwResults[i]);
			break;

		case PERF_COUNTER_COUNTER:
			// 32-bit Counter.  Divide delta by delta time.  Display suffix: "/sec"
		case PERF_COUNTER_BULK_COUNT:
			// 64-bit Counter.  Divide delta by delta time. Display Suffix: "/sec"
			PrintToFile(InfoFile, L"%10I64u  %5I64u%s\n", pdata->qwCounters[i], pdata->qwResults[i], GetMsg(MperSec));
			break;

		default:
			PrintToFile(InfoFile, L'\n');
			break;
		}
	}
}

void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment)
{
	std::wstring CurDir, EnvStrings;
	GetOpenProcessData(hProcess, {}, {}, {}, &CurDir, bExportEnvironment? &EnvStrings : nullptr);
	PrintToFile(InfoFile, L'\n');

	if (!CurDir.empty())
	{
		PrintToFile(InfoFile, L"%s %s\n\n", Plist::PrintTitle(MCurDir), CurDir.c_str());
	}

	if (bExportEnvironment && !EnvStrings.empty())
	{
		PrintToFile(InfoFile, L"%s\n\n", GetMsg(MEnvironment));

		for (wchar_t* p = EnvStrings.data(); *p; p += std::wcslen(p) + 1)
		{
			PrintToFile(InfoFile, L"%s\n", p);
		}
	}
}

void PrintModuleVersion(HANDLE InfoFile, const wchar_t* pVersion, const wchar_t* pDesc, size_t len)
{
	do
	{
		PrintToFile(InfoFile, L'\t');
	} while ((len = (len | 7) + 1) < 56);

	len += PrintToFile(InfoFile, L"%s", pVersion? pVersion : L"");

	if (pDesc)
	{
		do
		{
			PrintToFile(InfoFile, L' ');
		} while (len++ < 72);

		PrintToFile(InfoFile, L"%s", pDesc);
	}
}

template<typename callable>
static void print_module(HANDLE const InfoFile, void* const Module, DWORD const SizeOfImage, options& Opt, callable const& GetName)
{
	auto len = PrintToFile(InfoFile, L"  %p  %6X", Module, SizeOfImage);
	WCHAR wszModuleName[MAX_PATH];

	if (GetName(wszModuleName, std::size(wszModuleName)))
	{
		len += PrintToFile(InfoFile, L" %s", wszModuleName);

		const wchar_t* pVersion, * pDesc;
		std::unique_ptr<char[]> Buffer;

		if (Opt.ExportModuleVersion && Plist::GetVersionInfo(static_cast<wchar_t*>(wszModuleName), Buffer, pVersion, pDesc))
		{
			PrintModuleVersion(InfoFile, pVersion, pDesc, len);
		}
	}

	PrintToFile(InfoFile, L'\n');
}

void PrintModules(HANDLE InfoFile, DWORD dwPID, options& Opt)
{
	ModuleData Data;
	DebugToken token;
	const handle Process(OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | READ_CONTROL, dwPID));
	if (!Process)
		return;

	std::vector<HMODULE> Modules(1024);
	DWORD RequiredSize = 0;
	const auto Size = [&] { return static_cast<DWORD>(Modules.size() * sizeof(Modules[0])); };
	const auto Resize = [&] { Modules.resize(RequiredSize / sizeof(Modules[0])); };

	while (pEnumProcessModulesEx(Process.get(), Modules.data(), Size(), &RequiredSize, LIST_MODULES_ALL) && RequiredSize > Size())
	{
		Resize();
	}

	Resize();

	if (RequiredSize)
	{
		for (const auto Module : Modules)
		{
			MODULEINFO Info{};
			GetModuleInformation(Process.get(), Module, &Info, sizeof(Info));

			print_module(InfoFile, Info.lpBaseOfDll, Info.SizeOfImage, Opt, [&](wchar_t* const Buffer, size_t const BufferSize)
			{
				return GetModuleFileNameExW(Process.get(), Module, Buffer, static_cast<DWORD>(BufferSize));
			});
		}
	}
	else
	{
		PROCESS_PARAMETERS* pProcessParams;
		char* pEnd;

		if (GetInternalProcessData(Process.get(), &Data, pProcessParams, pEnd, true))
		{
			char* p4;

			do
			{
				print_module(InfoFile, Data.BaseAddress, Data.SizeOfImage, Opt, [&](wchar_t* const Buffer, size_t const BufferSize)
				{
					return ReadProcessMemory(Process.get(), Data.FullDllName.Buffer, Buffer, BufferSize * sizeof(*Buffer), {});
				});

				p4 = reinterpret_cast<char*>(Data.InMemoryOrderModuleList.Flink);
			} while (p4 && p4 != pEnd && ReadProcessMemory(Process.get(), p4 - sizeof(PVOID) * 2, &Data, sizeof(Data), {}));
		}
	}

	PrintToFile(InfoFile, L'\n');
}

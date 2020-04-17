#include <algorithm>
#include <mutex>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"
#include "ipc.hpp"

#include <psapi.h>

using namespace std::literals;

#ifndef _WIN64
static bool is_wow64_itself()
{
#ifdef _WIN64
	return false;
#else
	static const auto IsWow64 = is_wow64_process(GetCurrentProcess());
	return IsWow64;
#endif
}
#endif

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

		if (pd.dwProcessId == 0 && pd.ProcessName == L"_Total")
			CurItem.FileAttributes |= FILE_ATTRIBUTE_HIDDEN;

		if (pd.Bitness != Thread.GetDefaultBitness())
			CurItem.FileAttributes |= FILE_ATTRIBUTE_READONLY;

	}

	return true;
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
	return (
#ifndef _WIN64
		is_wow64_itself() && !is_wow64_process(hProcess)?
		ipc_functions<x64>::GetOpenProcessData :
#endif
		ipc_functions<same>::GetOpenProcessData
	)
	(
		hProcess,
		ProcessName,
		FullPath,
		CommandLine,
		CurDir,
		EnvStrings
	);
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
			PrintToFile(InfoFile, L"%14I64u\n", pdata->qwResults[i]);
			break;

		case PERF_100NSEC_TIMER:
			// 64-bit Timer in 100 nsec units. Display delta divided by delta time. Display suffix: "%"
			PrintToFile(InfoFile, L"%20s %7I64u%%\n", DurationToText(pdata->qwCounters[i]).c_str(), pdata->qwResults[i]);
			break;

		case PERF_COUNTER_COUNTER:
			// 32-bit Counter.  Divide delta by delta time.  Display suffix: "/sec"
		case PERF_COUNTER_BULK_COUNT:
			// 64-bit Counter.  Divide delta by delta time. Display Suffix: "/sec"
			PrintToFile(InfoFile, L"%14I64u  %9I64u%s\n", pdata->qwCounters[i], pdata->qwResults[i], GetMsg(MperSec));
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

static void print_module_impl(HANDLE const InfoFile, const std::wstring& Module, DWORD const SizeOfImage, options& Opt, const std::function<bool(wchar_t*, size_t)>& GetName)
{
	auto len = PrintToFile(InfoFile, L"  %s %8X", Module.c_str(), SizeOfImage);

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

template<typename module_type>
static void print_module(HANDLE const InfoFile, module_type Module, DWORD const SizeOfImage, options& Opt, const std::function<bool(wchar_t*, size_t)>& GetName)
{
	const auto ModuleStr = str_printf(sizeof(module_type) > sizeof(void*)? L"%016I64X" : L"%p", Module);
	print_module_impl(InfoFile, ModuleStr, SizeOfImage, Opt, GetName);
}

void PrintModules(HANDLE InfoFile, DWORD dwPID, options& Opt)
{
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
		for (const auto Module: Modules)
		{
			MODULEINFO Info{};
			GetModuleInformation(Process.get(), Module, &Info, sizeof(Info));

			print_module(InfoFile, Info.lpBaseOfDll, Info.SizeOfImage, Opt, [&](wchar_t* const Buffer, size_t const BufferSize)
			{
				return GetModuleFileNameExW(Process.get(), Module, Buffer, static_cast<DWORD>(BufferSize)) != 0;
			});
		}
	}
	else
	{
		return (
#ifndef _WIN64
			is_wow64_itself() && !is_wow64_process(Process.get())?
			ipc_functions<x64>::PrintModules :
#endif
			ipc_functions<same>::PrintModules
		)
		(
			Process.get(),
			InfoFile,
			Opt
		);
	}

	PrintToFile(InfoFile, L'\n');
}

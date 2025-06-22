#include <algorithm>
#include <mutex>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"
#include "ipc.hpp"

#include <psapi.h>

using namespace std::literals;

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
	Data.FullPath.assign(pd.FullPath, !std::wmemcmp(pd.FullPath.data(), L"\\??\\", 4)? 4 : 0);
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
	auto pData = Thread.ProcessData();

	if (pData.empty() || !Thread.IsOK())
		return false;

	pPanelItem = new PluginPanelItem[pData.size()]{};
	auto PanelItemIterator = pPanelItem;
	ItemsNumber = pData.size();

	for (auto& i: pData)
	{
		auto& CurItem = *PanelItemIterator;
		++PanelItemIterator;

		auto& pd = i.second;
		//delete CurItem.FileName;  // ???
		CurItem.FileName = new wchar_t[pd.ProcessName.size() + 1];
		*std::copy(pd.ProcessName.cbegin(), pd.ProcessName.cend(), const_cast<wchar_t*>(CurItem.FileName)) = L'\0';

		if (!pd.Owner.empty())
		{
			CurItem.Owner = new wchar_t[pd.Owner.size() + 1];
			*std::copy(pd.Owner.cbegin(), pd.Owner.cend(), const_cast<wchar_t*>(CurItem.Owner)) = L'\0';
		}

		CurItem.UserData.Data = new ProcessData();
		CurItem.UserData.FreeData = FreeUserData;

		ULARGE_INTEGER const CreationTime{ .QuadPart = pd.CreationTime };
		FILETIME const CreationFileTime
		{
			.dwLowDateTime = CreationTime.LowPart,
			.dwHighDateTime = CreationTime.HighPart,
		};

		CurItem.CreationTime = CurItem.LastWriteTime = CurItem.LastAccessTime = CurItem.ChangeTime = CreationFileTime;
		const auto ullSize = pd.qwCounters[IDX_WORKINGSET] + pd.qwCounters[IDX_PAGEFILE];
		CurItem.FileSize = ullSize;
		CurItem.AllocationSize = pd.qwResults[IDX_PAGEFILE];
		//yjh:???      CurItem.AllocationSize = pd.dwProcessId;

		CurItem.AlternateFileName = new wchar_t[16];
		FSF.itoa(pd.dwProcessId, const_cast<wchar_t*>(CurItem.AlternateFileName), 10);

		CurItem.NumberOfLinks = pd.dwThreads;
		GetPData(*static_cast<ProcessData*>(CurItem.UserData.Data), pd);

		if (pd.dwProcessId == 0 && pd.ProcessName == L"_Total"sv)
			CurItem.FileAttributes |= FILE_ATTRIBUTE_HIDDEN;

		if (pd.Bitness != Thread.GetDefaultBitness())
			CurItem.FileAttributes |= FILE_ATTRIBUTE_VIRTUAL;

	}

	return true;
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

void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment)
{
	std::wstring CurDir, EnvStrings;
	get_open_process_data(hProcess, {}, {}, {}, &CurDir, bExportEnvironment? &EnvStrings : nullptr);
	WriteToFile(InfoFile, L'\n');

	if (!CurDir.empty())
	{
		WriteToFile(InfoFile, far::format(L"{}:\n{}\n"sv, GetMsg(MCurDir), CurDir));
	}

	if (bExportEnvironment && !EnvStrings.empty())
	{
		WriteToFile(InfoFile, far::format(L"\n{}:\n"sv, GetMsg(MEnvironment)));

		for (wchar_t* p = EnvStrings.data(); *p; p += std::wcslen(p) + 1)
		{
			WriteToFile(InfoFile, far::format(L"{}\n"sv, p));
		}
	}
}

static void PrintModuleVersion(HANDLE InfoFile, const wchar_t* pVersion, const wchar_t* pDesc, size_t len)
{
	do
	{
		WriteToFile(InfoFile, L'\t');
	} while ((len = (len | 7) + 1) < 56);

	len += WriteToFile(InfoFile, pVersion? pVersion : L"");

	if (pDesc)
	{
		do
		{
			WriteToFile(InfoFile, L' ');
		} while (len++ < 72);

		WriteToFile(InfoFile, pDesc);
	}
}

static void print_module_impl(HANDLE const InfoFile, const std::wstring& Module, DWORD const SizeOfImage, std::wstring const& ModuleName, const options& LocalOpt)
{
	auto len = WriteToFile(InfoFile, far::format(L"{} {:8X}"sv, Module, SizeOfImage));

	if (!ModuleName.empty())
	{
		len += WriteToFile(InfoFile, far::format(L" {}"sv, ModuleName));

		const wchar_t* pVersion, * pDesc;
		std::unique_ptr<char[]> Buffer;

		if (LocalOpt.ExportModuleVersion && Plist::GetVersionInfo(ModuleName.c_str(), Buffer, pVersion, pDesc))
		{
			PrintModuleVersion(InfoFile, pVersion, pDesc, len);
		}
	}

	WriteToFile(InfoFile, L'\n');
}

static void print_module(HANDLE const InfoFile, ULONG64 const Module, DWORD const SizeOfImage, int const ProcessBitness, std::wstring const& ModuleName, options& LocalOpt)
{
	const auto ModuleStr = far::format(L"{:0{}X}"sv, Module, (ProcessBitness == -1? 64 : ProcessBitness) / std::numeric_limits<unsigned char>::digits * 2);
	print_module_impl(InfoFile, ModuleStr, SizeOfImage, ModuleName, LocalOpt);
}

static std::wstring get_module_file_name(HANDLE const Process, HMODULE const Module)
{
	if (wchar_t ModuleName[MAX_PATH]; GetModuleFileNameExW(Process, Module, ModuleName, static_cast<DWORD>(std::size(ModuleName))))
		return ModuleName;

	// TODO: this seems to be broken on Windows 10 for WOW64 processes
	// see https://stackoverflow.com/questions/46403532
	// GetMappedFileName returns a correct path, but converting it from NT to Win32 is a huge PITA
	// Maybe one day

	return {};
}

void PrintModules(HANDLE InfoFile, DWORD dwPID, int const ProcessBitness, options& LocalOpt)
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
			const auto ModuleName = get_module_file_name(Process.get(), Module);
			print_module(InfoFile, reinterpret_cast<uintptr_t>(Info.lpBaseOfDll), Info.SizeOfImage, ProcessBitness, ModuleName, LocalOpt);
		}
	}
	else
	{
		print_modules(Process.get(), InfoFile, LocalOpt, ProcessBitness, print_module);
	}
}

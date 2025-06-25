#ifndef IPC_HPP_B3F2F877_F2DB_4484_8523_255E608E1C0B
#define IPC_HPP_B3F2F877_F2DB_4484_8523_255E608E1C0B

#pragma once

using print_module_t = std::function<void(HANDLE InfoFile, ULONG64 Module, DWORD SizeOfImage, int ProcessBitness, std::wstring const& FullDllName, options& LocalOpt)>;

enum class bitness
{
	same,
	x86,
	x64,
};

template<bitness Bitness>
class ipc_t
{
	friend struct validate;

	using pointer =
		std::conditional_t<Bitness == bitness::same, ULONG_PTR,
		std::conditional_t<Bitness == bitness::x86, ULONG32,
		std::conditional_t<Bitness == bitness::x64, ULONG64,
		void>>>;

	struct UNICODE_STRING
	{
		USHORT  Length;
		USHORT  MaximumLength;
		pointer Buffer;
	};

	struct DRIVE_LETTER_CURDIR
	{
		USHORT         Flags;
		USHORT         Length;
		ULONG          TimeStamp;
		UNICODE_STRING DosPath;
	};

	struct PROCESS_PARAMETERS
	{
		ULONG               MaximumLength;
		ULONG               Length;
		ULONG               Flags;
		ULONG               DebugFlags;
		pointer             ConsoleHandle;
		ULONG               ConsoleFlags;
		pointer             StdInputHandle;
		pointer             StdOutputHandle;
		pointer             StdErrorHandle;
		UNICODE_STRING      CurrentDirectoryPath;
		pointer             CurrentDirectoryHandle;
		UNICODE_STRING      DllPath;
		UNICODE_STRING      ImagePathName;
		UNICODE_STRING      CommandLine;
		pointer             EnvironmentBlock;
		ULONG               StartingPositionLeft;
		ULONG               StartingPositionTop;
		ULONG               Width;
		ULONG               Height;
		ULONG               CharWidth;
		ULONG               CharHeight;
		ULONG               ConsoleTextAttributes;
		ULONG               WindowFlags;
		ULONG               ShowWindowFlags;
		UNICODE_STRING      WindowTitle;
		UNICODE_STRING      DesktopName;
		UNICODE_STRING      ShellInfo;
		UNICODE_STRING      RuntimeData;
		DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
	};

	struct LIST_ENTRY
	{
		pointer Flink;
		pointer Blink;
	};

	struct LDR_MODULE
	{
		LIST_ENTRY     InLoadOrderModuleList;
		LIST_ENTRY     InMemoryOrderModuleList;
		LIST_ENTRY     InInitializationOrderModuleList;
		pointer        BaseAddress;
		pointer        EntryPoint;
		ULONG          SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		ULONG          Flags;
		SHORT          LoadCount;
		SHORT          TlsIndex;
		LIST_ENTRY     HashTableEntry;
		ULONG          TimeDateStamp;
	};

	struct PEB_LDR_DATA
	{
		ULONG      Length;
		BOOLEAN    Initialized;
		pointer    SsHandle;
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
	};

	struct PEB
	{
		BOOLEAN       InheritedAddressSpace;
		BOOLEAN       ReadImageFileExecOptions;
		BOOLEAN       BeingDebugged;
		BOOLEAN       Spare;
		pointer       Mutant;
		pointer       ImageBaseAddress;
		pointer       LoaderData;
		pointer       ProcessParameters;
		pointer       SubSystemData;
		pointer       ProcessHeap;
		pointer       FastPebLock;
		pointer       FastPebLockRoutine;
		pointer       FastPebUnlockRoutine;
		ULONG         EnvironmentUpdateCount;
		pointer       KernelCallbackTable;
		pointer       EventLogSection;
		pointer       EventLog;
		pointer       FreeList;
		ULONG         TlsExpansionCounter;
		pointer       TlsBitmap;
		ULONG         TlsBitmapBits[0x2];
		pointer       ReadOnlySharedMemoryBase;
		pointer       ReadOnlySharedMemoryHeap;
		pointer       ReadOnlyStaticServerData;
		pointer       AnsiCodePageData;
		pointer       OemCodePageData;
		pointer       UnicodeCaseTableData;
		ULONG         NumberOfProcessors;
		ULONG         NtGlobalFlag;
		BYTE          Spare2[0x4];
		LARGE_INTEGER CriticalSectionTimeout;
		ULONG         HeapSegmentReserve;
		ULONG         HeapSegmentCommit;
		ULONG         HeapDeCommitTotalFreeThreshold;
		ULONG         HeapDeCommitFreeBlockThreshold;
		ULONG         NumberOfHeaps;
		ULONG         MaximumNumberOfHeaps;
		pointer       ProcessHeaps;
		pointer       GdiSharedHandleTable;
		pointer       ProcessStarterHelper;
		pointer       GdiDCAttributeList;
		pointer       LoaderLock;
		ULONG         OSMajorVersion;
		ULONG         OSMinorVersion;
		ULONG         OSBuildNumber;
		ULONG         OSPlatformId;
		ULONG         ImageSubSystem;
		ULONG         ImageSubSystemMajorVersion;
		ULONG         ImageSubSystemMinorVersion;
		ULONG         GdiHandleBuffer[0x22];
		ULONG         PostProcessInitRoutine;
		ULONG         TlsExpansionBitmap;
		BYTE          TlsExpansionBitmapBits[0x80];
		ULONG         SessionId;
	};

	struct PROCESS_BASIC_INFORMATION
	{
		pointer Reserved1;
		pointer PebBaseAddress;
		pointer Reserved2[2];
		pointer UniqueProcessId;
		pointer Reserved3;
	};

	static bool read_process_memory(HANDLE Process, pointer const BaseAddress, void* const Buffer, size_t const Size, size_t* const NumberOfBytesRead = {})
	{
#ifndef _WIN64
		if constexpr (Bitness == bitness::x64)
		{
			ULONG64 Read = 0;
			const auto Result = pNtWow64ReadVirtualMemory64(Process, BaseAddress, Buffer, Size, NumberOfBytesRead? &Read : nullptr);

			if (NumberOfBytesRead)
				*NumberOfBytesRead = static_cast<size_t>(Read);

			return NT_SUCCESS(Result);
		}
		else
#endif
		{
			SIZE_T Read = 0;

			const auto Result = ReadProcessMemory(Process, reinterpret_cast<void*>(static_cast<uintptr_t>(BaseAddress)), Buffer, Size, NumberOfBytesRead? &Read : nullptr);

			if (NumberOfBytesRead)
				*NumberOfBytesRead = static_cast<size_t>(Read);

			return Result != FALSE;
		}
	}

	static std::wstring read_string(HANDLE Process, UNICODE_STRING const Str)
	{
		std::wstring Result(Str.Length / sizeof(wchar_t), 0);
		if (!read_process_memory(Process, Str.Buffer, Result.data(), Result.size() * sizeof(wchar_t)))
			return {};

		return Result;
	}

	static std::wstring read_string(HANDLE Process, pointer const Address)
	{
		UNICODE_STRING Str;

		if (!read_process_memory(Process, Address, &Str, sizeof(Str)))
			return {};

		return read_string(Process, Str);
	}

	static NTSTATUS query_information_process(HANDLE const ProcessHandle, PROCESSINFOCLASS const ProcessInformationClass, PVOID const ProcessInformation, ULONG const ProcessInformationLength, PULONG const ReturnLength)
	{
		return (
#ifndef _WIN64
			Bitness == bitness::x64? pNtWow64QueryInformationProcess64 :
#endif
			pNtQueryInformationProcess)
			(
				ProcessHandle,
				ProcessInformationClass,
				ProcessInformation,
				ProcessInformationLength,
				ReturnLength
			);
	}

	static bool get_internal_process_data(
		HANDLE hProcess,
		LDR_MODULE* Data,
		pointer& pProcessParams,
		pointer& pEnd,
		bool bFirstModule = false
	)
	{
		pointer PebBaseAddress{};

		if constexpr (Bitness == bitness::x86)
		{
			ULONG_PTR Wow64Info;
			if (const auto Status = query_information_process(
				hProcess,
				ProcessWow64Information,
				&Wow64Info,
				sizeof(Wow64Info),
				{}
				); !NT_SUCCESS(Status))
				return false;

			PebBaseAddress = Wow64Info;
		}
		else
		{
			PROCESS_BASIC_INFORMATION processInfo;
			if (const auto Status = query_information_process(
				hProcess,
				ProcessBasicInformation,
				&processInfo,
				sizeof(processInfo),
				{}
			); !NT_SUCCESS(Status))
				return false;

			PebBaseAddress = processInfo.PebBaseAddress;
		}

		//FindModule, obtained from PSAPI.DLL
		PEB peb;
		if (!read_process_memory(hProcess, PebBaseAddress, &peb, sizeof(peb)))
			return false;

		if (!peb.LoaderData)
			return false;

		PEB_LDR_DATA pld;
		if (!read_process_memory(hProcess, peb.LoaderData, &pld, sizeof(pld)))
			return false;

		const auto hModule = peb.ImageBaseAddress;
		pProcessParams = peb.ProcessParameters;
		pEnd = peb.LoaderData + sizeof(pld) - sizeof(LIST_ENTRY) * 2;
		auto p4 = pld.InMemoryOrderModuleList.Flink;

		while (p4)
		{
			if (p4 == pEnd || !read_process_memory(hProcess, p4 - sizeof(pointer) * 2, Data, sizeof(*Data)))
				return false;

			if (bFirstModule)
				return true;

			if (Data->BaseAddress == hModule) break;

			p4 = Data->InMemoryOrderModuleList.Flink;
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

public:
	static void get_open_process_data(
		HANDLE hProcess,
		std::wstring* ProcessName,
		std::wstring* FullPath,
		std::wstring* CommandLine,
		std::wstring* CurDir,
		std::wstring* EnvStrings
	)
	{
		LDR_MODULE Data;
		pointer pEnd{};
		pointer pProcessParams{};

		if (!get_internal_process_data(hProcess, &Data, pProcessParams, pEnd))
			return;

		if (ProcessName && ProcessName->empty())
		{
			*ProcessName = read_string(hProcess, Data.BaseDllName);
		}

		if (FullPath)
		{
			*FullPath = read_string(hProcess, Data.FullDllName);
		}

		if (CommandLine)
		{
			*CommandLine = read_string(hProcess, pProcessParams + offsetof(PROCESS_PARAMETERS, CommandLine));
		}

		if (CurDir)
		{
			*CurDir = read_string(hProcess, pProcessParams + offsetof(PROCESS_PARAMETERS, CurrentDirectoryPath));
		}

		if (EnvStrings)
		{
			if (pointer pEnv; read_process_memory(hProcess, pProcessParams + offsetof(PROCESS_PARAMETERS, EnvironmentBlock), &pEnv, sizeof(pEnv)))
			{
				EnvStrings->resize(2048);

				for (;;)
				{
					if (!read_process_memory(hProcess, pEnv, EnvStrings->data(), EnvStrings->size() * 2))
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

	static void print_modules(HANDLE Process, HANDLE InfoFile, options& LocalOpt, int const ProcessBitness, print_module_t const& PrintModule)
	{
		LDR_MODULE Data;
		pointer pProcessParams{};
		pointer pEnd{};

		if (get_internal_process_data(Process, &Data, pProcessParams, pEnd, true))
		{
			pointer p4;

			do
			{
				const auto FullDllName = read_string(Process, Data.FullDllName);
				PrintModule(InfoFile, Data.BaseAddress, Data.SizeOfImage, ProcessBitness, FullDllName, LocalOpt);
				p4 = Data.InMemoryOrderModuleList.Flink;
			}
			while (p4 && p4 != pEnd && read_process_memory(Process, p4 - sizeof(pointer) * 2, &Data, sizeof(Data)));
		}
	}
};

struct validate
{
	static constexpr auto Code64 =
#ifdef _WIN64
		true
#else
		false
#endif
	;

	using ipc64 = ipc_t<bitness::x64>;
	using ipc_same = ipc_t<bitness::same>;

	static_assert(sizeof(ipc64::UNICODE_STRING) == 16);
	static_assert(sizeof(ipc_same::UNICODE_STRING) == (Code64? sizeof(ipc64::UNICODE_STRING) : 8));

	static_assert(sizeof(ipc64::DRIVE_LETTER_CURDIR) == 24);
	static_assert(sizeof(ipc_same::DRIVE_LETTER_CURDIR) == (Code64? sizeof(ipc64::DRIVE_LETTER_CURDIR) : 16));

	static_assert(sizeof(ipc64::PROCESS_PARAMETERS) == 1008);
	static_assert(sizeof(ipc_same::PROCESS_PARAMETERS) == (Code64? sizeof(ipc64::PROCESS_PARAMETERS) : 656));

	static_assert(sizeof(ipc64::LIST_ENTRY) == 16);
	static_assert(sizeof(ipc_same::LIST_ENTRY) == (Code64? sizeof(ipc64::LIST_ENTRY) : 8));

	static_assert(sizeof(ipc64::LDR_MODULE) == 136);
	static_assert(sizeof(ipc_same::LDR_MODULE) == (Code64? sizeof(ipc64::LDR_MODULE) : 72));

	static_assert(sizeof(ipc64::PEB_LDR_DATA) == 64);
	static_assert(sizeof(ipc_same::PEB_LDR_DATA) == (Code64? sizeof(ipc64::PEB_LDR_DATA) : 36));

	static_assert(sizeof(ipc64::PEB) == 584);
	static_assert(sizeof(ipc_same::PEB) == (Code64? sizeof(ipc64::PEB) : 472));

	static_assert(sizeof(ipc64::PROCESS_BASIC_INFORMATION) == 48);
	static_assert(sizeof(ipc_same::PROCESS_BASIC_INFORMATION) == (Code64? sizeof(ipc64::PROCESS_BASIC_INFORMATION) : 24));
};

inline bitness get_bitness(HANDLE const Process)
{
	if (is_wow64_process(Process))
		return bitness::x86;

	if (is_wow64_itself())
		return bitness::x64;

	return bitness::same;
}

inline void get_open_process_data(
	HANDLE const Process,
	std::wstring* const ProcessName,
	std::wstring* const FullPath,
	std::wstring* const CommandLine,
	std::wstring* const CurDir,
	std::wstring* const EnvStrings
)
{
	switch (get_bitness(Process))
	{
	case bitness::same: return ipc_t<bitness::same>::get_open_process_data(Process, ProcessName, FullPath, CommandLine, CurDir, EnvStrings);
	case bitness::x86:  return ipc_t<bitness::x86>::get_open_process_data(Process, ProcessName, FullPath, CommandLine, CurDir, EnvStrings);
	case bitness::x64:  return ipc_t<bitness::x64>::get_open_process_data(Process, ProcessName, FullPath, CommandLine, CurDir, EnvStrings);
	}
}

inline void print_modules(HANDLE const Process, HANDLE const InfoFile, options& LocalOpt, int const ProcessBitness, print_module_t const& PrintModule)
{
	switch (get_bitness(Process))
	{
	case bitness::same: return ipc_t<bitness::same>::print_modules(Process, InfoFile, LocalOpt, ProcessBitness, PrintModule);
	case bitness::x86:  return ipc_t<bitness::x86>::print_modules(Process, InfoFile, LocalOpt, ProcessBitness, PrintModule);
	case bitness::x64:  return ipc_t<bitness::x64>::print_modules(Process, InfoFile, LocalOpt, ProcessBitness, PrintModule);
	}
}

#endif // IPC_HPP_B3F2F877_F2DB_4484_8523_255E608E1C0B

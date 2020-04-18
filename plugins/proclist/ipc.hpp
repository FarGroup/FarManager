#ifndef IPC_HPP_B3F2F877_F2DB_4484_8523_255E608E1C0B
#define IPC_HPP_B3F2F877_F2DB_4484_8523_255E608E1C0B

#pragma once

constexpr auto Code64 =
#ifdef _WIN64
	true
#else
	false
#endif
;

enum ipc
{
	same,
#ifndef _WIN64
	x64
#endif
};

namespace detail
{
	struct UNICODE_STRING_64
	{
		USHORT  Length;
		USHORT  MaximumLength;
		ULONG64 Buffer;
	};

	static_assert(sizeof(UNICODE_STRING_64) == 16);

	struct UNICODE_STRING_SAME
	{
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	};

	static_assert(sizeof(UNICODE_STRING_SAME) == (Code64? sizeof(UNICODE_STRING_64) : 8));


	struct DRIVE_LETTER_CURDIR_64
	{
		USHORT            Flags;
		USHORT            Length;
		ULONG             TimeStamp;
		UNICODE_STRING_64 DosPath;
	};

	static_assert(sizeof(DRIVE_LETTER_CURDIR_64) == 24);

	struct DRIVE_LETTER_CURDIR_SAME
	{
		USHORT              Flags;
		USHORT              Length;
		ULONG               TimeStamp;
		UNICODE_STRING_SAME DosPath;
	};

	static_assert(sizeof(DRIVE_LETTER_CURDIR_SAME) == (Code64? sizeof(DRIVE_LETTER_CURDIR_64) : 16));


	struct PROCESS_PARAMETERS_64
	{
		ULONG                  MaximumLength;
		ULONG                  Length;
		ULONG                  Flags;
		ULONG                  DebugFlags;
		ULONG64                ConsoleHandle;
		ULONG                  ConsoleFlags;
		ULONG64                StdInputHandle;
		ULONG64                StdOutputHandle;
		ULONG64                StdErrorHandle;
		UNICODE_STRING_64      CurrentDirectoryPath;
		ULONG64                CurrentDirectoryHandle;
		UNICODE_STRING_64      DllPath;
		UNICODE_STRING_64      ImagePathName;
		UNICODE_STRING_64      CommandLine;
		ULONG64                EnvironmentBlock;
		ULONG                  StartingPositionLeft;
		ULONG                  StartingPositionTop;
		ULONG                  Width;
		ULONG                  Height;
		ULONG                  CharWidth;
		ULONG                  CharHeight;
		ULONG                  ConsoleTextAttributes;
		ULONG                  WindowFlags;
		ULONG                  ShowWindowFlags;
		UNICODE_STRING_64      WindowTitle;
		UNICODE_STRING_64      DesktopName;
		UNICODE_STRING_64      ShellInfo;
		UNICODE_STRING_64      RuntimeData;
		DRIVE_LETTER_CURDIR_64 DLCurrentDirectory[0x20];
	};

	static_assert(sizeof(PROCESS_PARAMETERS_64) == 1008);

	struct PROCESS_PARAMETERS_SAME
	{
		ULONG                    MaximumLength;
		ULONG                    Length;
		ULONG                    Flags;
		ULONG                    DebugFlags;
		PVOID                    ConsoleHandle;
		ULONG                    ConsoleFlags;
		HANDLE                   StdInputHandle;
		HANDLE                   StdOutputHandle;
		HANDLE                   StdErrorHandle;
		UNICODE_STRING_SAME      CurrentDirectoryPath;
		HANDLE                   CurrentDirectoryHandle;
		UNICODE_STRING_SAME      DllPath;
		UNICODE_STRING_SAME      ImagePathName;
		UNICODE_STRING_SAME      CommandLine;
		PVOID                    EnvironmentBlock;
		ULONG                    StartingPositionLeft;
		ULONG                    StartingPositionTop;
		ULONG                    Width;
		ULONG                    Height;
		ULONG                    CharWidth;
		ULONG                    CharHeight;
		ULONG                    ConsoleTextAttributes;
		ULONG                    WindowFlags;
		ULONG                    ShowWindowFlags;
		UNICODE_STRING_SAME      WindowTitle;
		UNICODE_STRING_SAME      DesktopName;
		UNICODE_STRING_SAME      ShellInfo;
		UNICODE_STRING_SAME      RuntimeData;
		DRIVE_LETTER_CURDIR_SAME DLCurrentDirectory[0x20];
	};

	static_assert(sizeof(PROCESS_PARAMETERS_SAME) == (Code64? sizeof(PROCESS_PARAMETERS_64) : 656));


	struct LIST_ENTRY_64
	{
		ULONG64 Flink;
		ULONG64 Blink;
	};

	static_assert(sizeof(LIST_ENTRY_64) == 16);

	struct LIST_ENTRY_SAME
	{
		LIST_ENTRY_SAME* Flink;
		LIST_ENTRY_SAME* Blink;
	};

	static_assert(sizeof(LIST_ENTRY_SAME) == (Code64? sizeof(LIST_ENTRY_64) : 8));


	struct LDR_MODULE_64
	{
		LIST_ENTRY_64     InLoadOrderModuleList;
		LIST_ENTRY_64     InMemoryOrderModuleList;
		LIST_ENTRY_64     InInitializationOrderModuleList;
		ULONG64           BaseAddress;
		ULONG64           EntryPoint;
		ULONG             SizeOfImage;
		UNICODE_STRING_64 FullDllName;
		UNICODE_STRING_64 BaseDllName;
		ULONG             Flags;
		SHORT             LoadCount;
		SHORT             TlsIndex;
		LIST_ENTRY_64     HashTableEntry;
		ULONG             TimeDateStamp;
	};

	static_assert(sizeof(LDR_MODULE_64) == 136);

	struct LDR_MODULE_SAME
	{
		LIST_ENTRY_SAME     InLoadOrderModuleList;
		LIST_ENTRY_SAME     InMemoryOrderModuleList;
		LIST_ENTRY_SAME     InInitializationOrderModuleList;
		PVOID               BaseAddress;
		PVOID               EntryPoint;
		ULONG               SizeOfImage;
		UNICODE_STRING_SAME FullDllName;
		UNICODE_STRING_SAME BaseDllName;
		ULONG               Flags;
		SHORT               LoadCount;
		SHORT               TlsIndex;
		LIST_ENTRY_SAME     HashTableEntry;
		ULONG               TimeDateStamp;
	};

	static_assert(sizeof(LDR_MODULE_SAME) == (Code64? sizeof(LDR_MODULE_64) : 72));


	struct PEB_LDR_DATA_64
	{
		ULONG         Length;
		BOOLEAN       Initialized;
		ULONG64       SsHandle;
		LIST_ENTRY_64 InLoadOrderModuleList;
		LIST_ENTRY_64 InMemoryOrderModuleList;
		LIST_ENTRY_64 InInitializationOrderModuleList;
	};

	static_assert(sizeof(PEB_LDR_DATA_64) == 64);

	struct PEB_LDR_DATA_SAME
	{
		ULONG           Length;
		BOOLEAN         Initialized;
		PVOID           SsHandle;
		LIST_ENTRY_SAME InLoadOrderModuleList;
		LIST_ENTRY_SAME InMemoryOrderModuleList;
		LIST_ENTRY_SAME InInitializationOrderModuleList;
	};

	static_assert(sizeof(PEB_LDR_DATA_SAME) == (Code64? sizeof(PEB_LDR_DATA_64) : 36));


	struct PEB_64
	{
		BOOLEAN                InheritedAddressSpace;
		BOOLEAN                ReadImageFileExecOptions;
		BOOLEAN                BeingDebugged;
		BOOLEAN                Spare;
		ULONG64                Mutant;
		ULONG64                ImageBaseAddress;
		ULONG64                LoaderData;
		ULONG64                ProcessParameters;
		ULONG64                SubSystemData;
		ULONG64                ProcessHeap;
		ULONG64                FastPebLock;
		ULONG64                FastPebLockRoutine;
		ULONG64                FastPebUnlockRoutine;
		ULONG                  EnvironmentUpdateCount;
		ULONG64                KernelCallbackTable;
		ULONG64                EventLogSection;
		ULONG64                EventLog;
		ULONG64                FreeList;
		ULONG                  TlsExpansionCounter;
		ULONG64                TlsBitmap;
		ULONG                  TlsBitmapBits[0x2];
		ULONG64                ReadOnlySharedMemoryBase;
		ULONG64                ReadOnlySharedMemoryHeap;
		ULONG64                ReadOnlyStaticServerData;
		ULONG64                AnsiCodePageData;
		ULONG64                OemCodePageData;
		ULONG64                UnicodeCaseTableData;
		ULONG                  NumberOfProcessors;
		ULONG                  NtGlobalFlag;
		BYTE                   Spare2[0x4];
		LARGE_INTEGER          CriticalSectionTimeout;
		ULONG                  HeapSegmentReserve;
		ULONG                  HeapSegmentCommit;
		ULONG                  HeapDeCommitTotalFreeThreshold;
		ULONG                  HeapDeCommitFreeBlockThreshold;
		ULONG                  NumberOfHeaps;
		ULONG                  MaximumNumberOfHeaps;
		ULONG64                ProcessHeaps;
		ULONG64                GdiSharedHandleTable;
		ULONG64                ProcessStarterHelper;
		ULONG64                GdiDCAttributeList;
		ULONG64                LoaderLock;
		ULONG                  OSMajorVersion;
		ULONG                  OSMinorVersion;
		ULONG                  OSBuildNumber;
		ULONG                  OSPlatformId;
		ULONG                  ImageSubSystem;
		ULONG                  ImageSubSystemMajorVersion;
		ULONG                  ImageSubSystemMinorVersion;
		ULONG                  GdiHandleBuffer[0x22];
		ULONG                  PostProcessInitRoutine;
		ULONG                  TlsExpansionBitmap;
		BYTE                   TlsExpansionBitmapBits[0x80];
		ULONG                  SessionId;
	};

	static_assert(sizeof(PEB_64) == 584);

	struct PEB_SAME
	{
		BOOLEAN                    InheritedAddressSpace;
		BOOLEAN                    ReadImageFileExecOptions;
		BOOLEAN                    BeingDebugged;
		BOOLEAN                    Spare;
		HANDLE                     Mutant;
		PVOID                      ImageBaseAddress;
		PEB_LDR_DATA_SAME*       LoaderData;
		PROCESS_PARAMETERS_SAME* ProcessParameters;
		PVOID                      SubSystemData;
		PVOID                      ProcessHeap;
		PVOID                      FastPebLock;
		PVOID                      FastPebLockRoutine;
		PVOID                      FastPebUnlockRoutine;
		ULONG                      EnvironmentUpdateCount;
		PVOID*                     KernelCallbackTable;
		PVOID                      EventLogSection;
		PVOID                      EventLog;
		PVOID                      FreeList;
		ULONG                      TlsExpansionCounter;
		PVOID                      TlsBitmap;
		ULONG                      TlsBitmapBits[0x2];
		PVOID                      ReadOnlySharedMemoryBase;
		PVOID                      ReadOnlySharedMemoryHeap;
		PVOID*                     ReadOnlyStaticServerData;
		PVOID                      AnsiCodePageData;
		PVOID                      OemCodePageData;
		PVOID                      UnicodeCaseTableData;
		ULONG                      NumberOfProcessors;
		ULONG                      NtGlobalFlag;
		BYTE                       Spare2[0x4];
		LARGE_INTEGER              CriticalSectionTimeout;
		ULONG                      HeapSegmentReserve;
		ULONG                      HeapSegmentCommit;
		ULONG                      HeapDeCommitTotalFreeThreshold;
		ULONG                      HeapDeCommitFreeBlockThreshold;
		ULONG                      NumberOfHeaps;
		ULONG                      MaximumNumberOfHeaps;
		PVOID**                    ProcessHeaps;
		PVOID                      GdiSharedHandleTable;
		PVOID                      ProcessStarterHelper;
		PVOID                      GdiDCAttributeList;
		PVOID                      LoaderLock;
		ULONG                      OSMajorVersion;
		ULONG                      OSMinorVersion;
		ULONG                      OSBuildNumber;
		ULONG                      OSPlatformId;
		ULONG                      ImageSubSystem;
		ULONG                      ImageSubSystemMajorVersion;
		ULONG                      ImageSubSystemMinorVersion;
		ULONG                      GdiHandleBuffer[0x22];
		ULONG                      PostProcessInitRoutine;
		ULONG                      TlsExpansionBitmap;
		BYTE                       TlsExpansionBitmapBits[0x80];
		ULONG                      SessionId;
	};

	static_assert(sizeof(PEB_SAME) == (Code64? sizeof(PEB_64) : 472));


	struct PROCESS_BASIC_INFORMATION_64
	{
		ULONG64 Reserved1;
		ULONG64 PebBaseAddress;
		ULONG64 Reserved2[2];
		ULONG64 UniqueProcessId;
		ULONG64 Reserved3;
	};

	static_assert(sizeof(PROCESS_BASIC_INFORMATION_64) == 48);

	struct PROCESS_BASIC_INFORMATION_SAME
	{
		PVOID Reserved1;
		PEB_SAME* PebBaseAddress;
		PVOID Reserved2[2];
		ULONG_PTR UniqueProcessId;
		PVOID Reserved3;
	};

	static_assert(sizeof(PROCESS_BASIC_INFORMATION_SAME) == (Code64? sizeof(PROCESS_BASIC_INFORMATION_64) : 24));

	template<ipc Ipc>
	struct ipc_types;

	template<>
	struct ipc_types<same>
	{
		using UNICODE_STRING            = UNICODE_STRING_SAME;
		using PROCESS_PARAMETERS        = PROCESS_PARAMETERS_SAME;
		using LIST_ENTRY                = LIST_ENTRY_SAME;
		using LDR_MODULE                = LDR_MODULE_SAME;
		using PEB_LDR_DATA              = PEB_LDR_DATA_SAME;
		using PEB                       = PEB_SAME;
		using PROCESS_BASIC_INFORMATION = PROCESS_BASIC_INFORMATION_SAME;
		using PTR                       = const char*;
		using ANY_PTR                   = const void*;
	};

#ifndef _WIN64
	template<>
	struct ipc_types<x64>
	{
		using UNICODE_STRING            = UNICODE_STRING_64;
		using PROCESS_PARAMETERS        = PROCESS_PARAMETERS_64;
		using LIST_ENTRY                = LIST_ENTRY_64;
		using LDR_MODULE                = LDR_MODULE_64;
		using PEB_LDR_DATA              = PEB_LDR_DATA_64;
		using PEB                       = PEB_64;
		using PROCESS_BASIC_INFORMATION = PROCESS_BASIC_INFORMATION_64;
		using PTR                       = ULONG64;
		using ANY_PTR                   = ULONG64;
	};
#endif
}

template<ipc Ipc>
class ipc_functions
{
public:
	using types = detail::ipc_types<Ipc>;

	static bool read_process_memory(HANDLE Process, typename types::ANY_PTR const BaseAddress, void* const Buffer, size_t const Size, size_t* const NumberOfBytesRead = {})
	{
#ifndef _WIN64
		if constexpr (Ipc == x64)
		{
			ULONG64 Read = 0;
			const auto Result = pNtWow64ReadVirtualMemory64(Process, BaseAddress, Buffer, Size, NumberOfBytesRead? &Read : nullptr);

			if (NumberOfBytesRead)
				*NumberOfBytesRead = static_cast<size_t>(Read);

			return Result == STATUS_SUCCESS;
		}
		else
#endif
		{
			SIZE_T Read = 0;

			const auto Result = ReadProcessMemory(Process, BaseAddress, Buffer, Size, NumberOfBytesRead? &Read : nullptr);

			if (NumberOfBytesRead)
				*NumberOfBytesRead = static_cast<size_t>(Read);

			return Result != FALSE;
		}
	}

	static std::wstring read_string(HANDLE Process, const typename types::UNICODE_STRING& Str)
	{
		std::wstring Result(Str.Length / sizeof(wchar_t), 0);
		if (!read_process_memory(Process, Str.Buffer, Result.data(), Result.size() * sizeof(wchar_t)))
			return {};

		return Result;
	}

	static std::wstring read_string(HANDLE Process, typename types::ANY_PTR const Address)
	{
		typename types::UNICODE_STRING Str;

		if (!read_process_memory(Process, Address, &Str, sizeof(Str)))
			return {};

		return read_string(Process, Str);
	}

	static bool GetInternalProcessData(
		HANDLE hProcess,
		typename types::LDR_MODULE* Data,
		typename types::PTR& pProcessParams,
		typename types::PTR& pEnd,
		bool bFirstModule = false
	)
	{
		typename types::PROCESS_BASIC_INFORMATION processInfo;
		if (
			(
#ifndef _WIN64
				Ipc == x64? pNtWow64QueryInformationProcess64 :
#endif
				pNtQueryInformationProcess)
			(
				hProcess,
				ProcessBasicInformation,
				&processInfo,
				sizeof(processInfo), {}
			) != STATUS_SUCCESS)
			return false;

		//FindModule, obtained from PSAPI.DLL
		typename types::PEB peb;
		typename types::PEB_LDR_DATA pld;

		if (read_process_memory(hProcess, processInfo.PebBaseAddress, &peb, sizeof(peb)) &&
			read_process_memory(hProcess, peb.LoaderData, &pld, sizeof(pld)))
		{
			//pEnd = (void *)((void *)peb.LoaderData+((void *)&pld.InMemoryOrderModuleList-(void *)&pld));
			const auto hModule = peb.ImageBaseAddress;
			pProcessParams = static_cast<typename types::PTR>(static_cast<typename types::ANY_PTR>(peb.ProcessParameters));
			pEnd = static_cast<typename types::PTR>(static_cast<typename types::ANY_PTR>(peb.LoaderData)) + sizeof(pld) - sizeof(typename types::LIST_ENTRY) * 2;
			auto p4 = static_cast<typename types::PTR>(static_cast<typename types::ANY_PTR>(pld.InMemoryOrderModuleList.Flink));

			while (p4)
			{
				if (p4 == pEnd || !read_process_memory(hProcess, p4 - sizeof(typename types::PTR) * 2, Data, sizeof(*Data)))
					return false;

				if (bFirstModule)
					return true;

				if (Data->BaseAddress == hModule) break;

				p4 = static_cast<typename types::PTR>(static_cast<typename types::ANY_PTR>(Data->InMemoryOrderModuleList.Flink));
			}
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

	static void GetOpenProcessData(
		HANDLE hProcess,
		std::wstring* ProcessName,
		std::wstring* FullPath,
		std::wstring* CommandLine,
		std::wstring* CurDir,
		std::wstring* EnvStrings
	)
	{
		typename types::LDR_MODULE Data;
		typename types::PTR pEnd{};
		typename types::PTR pProcessParams{};

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
			*CommandLine = read_string(hProcess, pProcessParams + offsetof(typename types::PROCESS_PARAMETERS, CommandLine));
		}

		if (CurDir)
		{
			*CurDir = read_string(hProcess, pProcessParams + offsetof(typename types::PROCESS_PARAMETERS, CurrentDirectoryPath));
		}

		if (EnvStrings)
		{
			typename types::PTR pEnv;

			if (read_process_memory(hProcess, pProcessParams + offsetof(typename types::PROCESS_PARAMETERS, EnvironmentBlock), &pEnv, sizeof(pEnv)))
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

	static void PrintModules(HANDLE Process, HANDLE InfoFile, options& Opt)
	{
		typename types::LDR_MODULE Data;
		typename types::PTR pProcessParams{};
		typename types::PTR pEnd{};

		if (GetInternalProcessData(Process, &Data, pProcessParams, pEnd, true))
		{
			typename types::PTR p4;

			do
			{
				print_module(InfoFile, Data.BaseAddress, Data.SizeOfImage, Opt, [&](wchar_t* const Buffer, size_t const BufferSize)
				{
					return read_process_memory(Process, Data.FullDllName.Buffer, Buffer, BufferSize * sizeof(*Buffer));
				});

				p4 = static_cast<typename types::PTR>(static_cast<typename types::ANY_PTR>(Data.InMemoryOrderModuleList.Flink));
			}
			while (p4 && p4 != pEnd && read_process_memory(Process, p4 - sizeof(typename types::PTR) * 2, &Data, sizeof(Data)));
		}
	}
};

#endif // IPC_HPP_B3F2F877_F2DB_4484_8523_255E608E1C0B

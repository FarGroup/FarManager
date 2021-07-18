﻿/*
platform.process.cpp

*/
/*
Copyright © 2020 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.process.hpp"

// Internal:
#include "exception.hpp"
#include "imports.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/bytes_view.hpp"
#include "common/io.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::process
{
	template<typename pointer>
	struct peb_t
	{
		BOOLEAN  Reserved1[4];
		pointer  Reserved2;
		pointer  ImageBaseAddress;
		pointer  Reserved3[7];
		ULONG    Reserved4;
		pointer  Reserved5;
		ULONG    Reserved6[2];
		pointer  Reserved7;
		ULONG    Reserved8;
		pointer  Reserved9;
		ULONG    Reserved10[2];
		pointer  Reserved11[6];
		ULONG    Reserved12[2];
		LONGLONG Reserved13;
		pointer  Reserved14[4];
		ULONG    Reserved15[2];
		pointer  Reserved16[3];
		ULONG    Reserved17;
		pointer  Reserved18;
		ULONG    Reserved19[2];
		USHORT   Reserved20[2];
		ULONG    Reserved21;
		ULONG    ImageSubsystem;
		ULONG    ImageSubsystemMajorVersion;
		ULONG    ImageSubsystemMinorVersion;
	};

	template<typename pointer>
	struct process_basic_information
	{
		pointer Reserved1;
		pointer PebBaseAddress;
		pointer Reserved2[4];
	};

	template<bool wow>
	using ipc_ptr = std::conditional_t<wow, ULONG64, ULONG_PTR>;

	static_assert(sizeof(peb_t<ipc_ptr<true>>) == 312);
	static_assert(sizeof(peb_t<ipc_ptr<false>>) ==
#ifdef _WIN64
		312
#else
		192
#endif
	);

	template<bool wow>
	static auto query_information_process(HANDLE Process, PROCESSINFOCLASS ProcessInformationClass, void* ProcessInformation, ULONG ProcessInformationLength, ULONG* ReturnLength)
	{
#ifndef _WIN64
		if constexpr (wow)
			return imports.NtWow64QueryInformationProcess64 && imports.NtWow64QueryInformationProcess64(Process, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength) == STATUS_SUCCESS;
		else
#endif
			return imports.NtQueryInformationProcess && imports.NtQueryInformationProcess(Process, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength) == STATUS_SUCCESS;
	}

	template<bool wow>
	static auto read_process_memory(HANDLE Process, ipc_ptr<wow> BaseAddress, void* Buffer, ipc_ptr<wow> Size, ipc_ptr<wow>* NumberOfBytesRead)
	{
#ifndef _WIN64
		if constexpr (wow)
			return imports.NtWow64ReadVirtualMemory64 && imports.NtWow64ReadVirtualMemory64(Process, BaseAddress, Buffer, Size, NumberOfBytesRead) == STATUS_SUCCESS;
		else
#endif
			return ReadProcessMemory(Process, reinterpret_cast<void*>(BaseAddress), Buffer, Size, NumberOfBytesRead) != FALSE;
	}

	static auto subsystem_to_type(int const Subsystem)
	{
		switch (Subsystem)
		{
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
		case IMAGE_SUBSYSTEM_POSIX_CUI:
			return image_type::console;

		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
			return image_type::graphical;

		default:
			return image_type::unknown;
		}
	}

	template<bool wow>
	static auto get_process_subsystem_from_memory(HANDLE const Process)
	{
		process_basic_information<ipc_ptr<wow>> ProcessInfo;
		if (!query_information_process<wow>(Process, ProcessBasicInformation, &ProcessInfo, sizeof(ProcessInfo), {}))
			return image_type::unknown;

		using peb = peb_t<ipc_ptr<wow>>;
		peb Peb;
		ipc_ptr<wow> ReadSize;
		if (!read_process_memory<wow>(Process, ProcessInfo.PebBaseAddress, &Peb, sizeof(Peb), &ReadSize))
			return image_type::unknown;

		if (ReadSize >= offsetof(peb, ImageSubsystem) + sizeof(Peb.ImageSubsystem))
			return subsystem_to_type(Peb.ImageSubsystem);

		LONG e_lfanew;
		if (!read_process_memory<wow>(Process, Peb.ImageBaseAddress + offsetof(IMAGE_DOS_HEADER, e_lfanew), &e_lfanew, sizeof(e_lfanew), {}))
			return image_type::unknown;

		WORD Subsystem;
		if (!read_process_memory<wow>(Process, Peb.ImageBaseAddress + e_lfanew + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem), &Subsystem, sizeof(Subsystem), {}))
			return image_type::unknown;

		return subsystem_to_type(Subsystem);
	}

	static auto get_process_subsystem_from_memory(HANDLE const Process)
	{
		return
#ifndef _WIN64
			IsWow64Process()?
			get_process_subsystem_from_memory<true>(Process) :
#endif
			get_process_subsystem_from_memory<false>(Process);
	}

	static auto get_process_subsystem_from_module_impl(std::istream& Stream)
	{
		IMAGE_DOS_HEADER DOSHeader;
		if (io::read(Stream, edit_bytes(DOSHeader)) != sizeof(DOSHeader))
			return image_type::unknown;

		if (DOSHeader.e_magic != IMAGE_DOS_SIGNATURE)
			return image_type::unknown;

		Stream.seekg(DOSHeader.e_lfanew);

		static_assert(offsetof(IMAGE_NT_HEADERS64, OptionalHeader) == offsetof(IMAGE_NT_HEADERS32, OptionalHeader));
		static_assert(offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem) == offsetof(IMAGE_OPTIONAL_HEADER32, Subsystem));
		static_assert(sizeof(IMAGE_OPTIONAL_HEADER64::Subsystem) == sizeof(IMAGE_OPTIONAL_HEADER32::Subsystem));

		IMAGE_NT_HEADERS64 ImageHeader;
		if (io::read(Stream, edit_bytes(ImageHeader)) != sizeof(ImageHeader))
			return image_type::unknown;

		if (ImageHeader.Signature != IMAGE_NT_SIGNATURE)
			return image_type::unknown;

		if (ImageHeader.FileHeader.Characteristics & IMAGE_FILE_DLL)
			return image_type::unknown;

		if (none_of(ImageHeader.OptionalHeader.Magic, IMAGE_NT_OPTIONAL_HDR32_MAGIC, IMAGE_NT_OPTIONAL_HDR64_MAGIC))
			return image_type::unknown;

		return subsystem_to_type(ImageHeader.OptionalHeader.Subsystem);
	}

	static auto get_process_subsystem_from_module(HANDLE const Process)
	{
		string ProcessFileName;
		if (!fs::get_module_file_name(Process, {}, ProcessFileName))
			return image_type::unknown;

		const fs::file ModuleFile(ProcessFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
		if (!ModuleFile)
			return image_type::unknown;

		try
		{
			fs::filebuf StreamBuffer(ModuleFile, std::ios::in);
			std::istream Stream(&StreamBuffer);
			Stream.exceptions(Stream.badbit | Stream.failbit);
			return get_process_subsystem_from_module_impl(Stream);
		}
		catch (const std::exception&)
		{
			return image_type::unknown;
		}
	}

	image_type get_process_subsystem(HANDLE const Process)
	{
		if (const auto Type = get_process_subsystem_from_memory(Process); Type != image_type::unknown)
			return Type;

		if (const auto Type = get_process_subsystem_from_module(Process); Type != image_type::unknown)
			return Type;

		return image_type::unknown;
	}

	static string get_process_name(DWORD const Pid, chrono::nt_clock::time_point const StartTime)
	{
		const auto Process = handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid));
		if (!Process)
		{
			LOGWARNING(L"OpenProcess({}): {}"sv, Pid, last_error());
			return {};
		}

		chrono::time_point CreationTime;
		if (!get_process_creation_time(Process.native_handle(), CreationTime))
		{
			LOGWARNING(L"get_process_creation_time({}): {}"sv, Pid, last_error());
			return {};
		}

		if (StartTime != CreationTime)
		{
			LOGWARNING(L"Process creation time mismatch"sv);
			return {};
		}

		string Name;
		if (!fs::get_module_file_name(Process.native_handle(), {}, Name))
		{
			LOGWARNING(L"get_module_file_name({}): {}"sv, Pid, last_error());
			return {};
		}

		return Name;
	}

	size_t enumerate_rm_processes(const string& Filename, DWORD& Reasons, function_ref<bool(string&&)> const Handler)
	{
		if (!imports.RmStartSession)
			return 0;

		DWORD Session;
		wchar_t SessionKey[CCH_RM_SESSION_KEY + 1] = {};
		if (const auto Result = imports.RmStartSession(&Session, 0, SessionKey); Result != ERROR_SUCCESS)
		{
			LOGWARNING(L"RmStartSession(): {}"sv, format_error(Result));
			return 0;
		}

		SCOPE_EXIT
		{
			if (!imports.RmEndSession)
				return;

			if (const auto Result = imports.RmEndSession(Session); Result != ERROR_SUCCESS)
			{
				LOGWARNING(L"RmEndSession(): {}"sv, format_error(Result));
			}
		};

		if (!imports.RmRegisterResources)
			return 0;

		auto FilenamePtr = Filename.c_str();
		if (const auto Result = imports.RmRegisterResources(Session, 1, &FilenamePtr, 0, {}, 0, {}); Result != ERROR_SUCCESS)
		{
			LOGWARNING(L"RmRegisterResources({}): {}"sv, Filename, format_error(Result));
			return 0;
		}

		if (!imports.RmGetList)
			return 0;

		DWORD RmGetListResult;
		unsigned ProceccInfoSizeNeeded = 0, ProcessInfoSize = 128;
		std::vector<RM_PROCESS_INFO> ProcessInfos(ProcessInfoSize);
		while ((RmGetListResult = imports.RmGetList(Session, &ProceccInfoSizeNeeded, &ProcessInfoSize, ProcessInfos.data(), &Reasons)) == ERROR_MORE_DATA)
		{
			ProcessInfoSize = ProceccInfoSizeNeeded;
			ProcessInfos.resize(ProcessInfoSize);
		}

		if (RmGetListResult != ERROR_SUCCESS)
		{
			LOGWARNING(L"RmGetList(): {}"sv, format_error(RmGetListResult));
			return 0;
		}

		ProcessInfos.resize(ProcessInfoSize);

		for (const auto& Info: ProcessInfos)
		{
			auto Str = *Info.strAppName? Info.strAppName : L"Unknown"s;

			if (*Info.strServiceShortName)
				append(Str, L" ["sv, Info.strServiceShortName, L']');

			append(Str, L" (PID: "sv, str(Info.Process.dwProcessId));

			if (const auto Name = get_process_name(Info.Process.dwProcessId, chrono::nt_clock::from_filetime(Info.Process.ProcessStartTime)); !Name.empty())
				append(Str, L", "sv, Name);

			Str += L')';

			if (!Handler(std::move(Str)))
				break;
		}

		return ProcessInfos.size();

	}
}

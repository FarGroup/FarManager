/*
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
#include "imports.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/bytes_view.hpp"
#include "common/io.hpp"

// External:

//----------------------------------------------------------------------------

namespace
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
			return os::process::image_type::console;

		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
			return os::process::image_type::graphical;

		default:
			return os::process::image_type::unknown;
		}
	}

	template<bool wow>
	static auto get_process_subsystem_from_memory(HANDLE const Process)
	{
		process_basic_information<ipc_ptr<wow>> ProcessInfo;
		if (!query_information_process<wow>(Process, ProcessBasicInformation, &ProcessInfo, sizeof(ProcessInfo), {}))
			return os::process::image_type::unknown;

		using peb = peb_t<ipc_ptr<wow>>;
		peb Peb;
		ipc_ptr<wow> ReadSize;
		if (!read_process_memory<wow>(Process, ProcessInfo.PebBaseAddress, &Peb, sizeof(Peb), &ReadSize))
			return os::process::image_type::unknown;

		if (ReadSize >= offsetof(peb, ImageSubsystem) + sizeof(Peb.ImageSubsystem))
			return subsystem_to_type(Peb.ImageSubsystem);

		LONG e_lfanew;
		if (!read_process_memory<wow>(Process, Peb.ImageBaseAddress + offsetof(IMAGE_DOS_HEADER, e_lfanew), &e_lfanew, sizeof(e_lfanew), {}))
			return os::process::image_type::unknown;

		WORD Subsystem;
		if (!read_process_memory<wow>(Process, Peb.ImageBaseAddress + e_lfanew + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem), &Subsystem, sizeof(Subsystem), {}))
			return os::process::image_type::unknown;

		return subsystem_to_type(Subsystem);
	}

	static auto get_process_subsystem_from_memory(HANDLE const Process)
	{
		return
#ifndef _WIN64
			os::IsWow64Process()?
			get_process_subsystem_from_memory<true>(Process) :
#endif
			get_process_subsystem_from_memory<false>(Process);
	}

	static auto get_process_subsystem_from_module_impl(std::istream& Stream)
	{
		IMAGE_DOS_HEADER DOSHeader;
		if (io::read(Stream, edit_bytes(DOSHeader)) != sizeof(DOSHeader))
			return os::process::image_type::unknown;

		if (DOSHeader.e_magic != IMAGE_DOS_SIGNATURE)
			return os::process::image_type::unknown;

		Stream.seekg(DOSHeader.e_lfanew);

		static_assert(offsetof(IMAGE_NT_HEADERS64, OptionalHeader) == offsetof(IMAGE_NT_HEADERS32, OptionalHeader));
		static_assert(offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem) == offsetof(IMAGE_OPTIONAL_HEADER32, Subsystem));
		static_assert(sizeof(IMAGE_OPTIONAL_HEADER64::Subsystem) == sizeof(IMAGE_OPTIONAL_HEADER32::Subsystem));

		IMAGE_NT_HEADERS64 ImageHeader;
		if (io::read(Stream, edit_bytes(ImageHeader)) != sizeof(ImageHeader))
			return os::process::image_type::unknown;

		if (ImageHeader.Signature != IMAGE_NT_SIGNATURE)
			return os::process::image_type::unknown;

		if (ImageHeader.FileHeader.Characteristics & IMAGE_FILE_DLL)
			return os::process::image_type::unknown;

		if (none_of(ImageHeader.OptionalHeader.Magic, IMAGE_NT_OPTIONAL_HDR32_MAGIC, IMAGE_NT_OPTIONAL_HDR64_MAGIC))
			return os::process::image_type::unknown;

		return subsystem_to_type(ImageHeader.OptionalHeader.Subsystem);
	}

	static auto get_process_subsystem_from_module(HANDLE const Process)
	{
		string ProcessFileName;
		if (!os::fs::GetModuleFileName(Process, {}, ProcessFileName))
			return os::process::image_type::unknown;

		const os::fs::file ModuleFile(ProcessFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
		if (!ModuleFile)
			return os::process::image_type::unknown;

		try
		{
			os::fs::filebuf StreamBuffer(ModuleFile, std::ios::in);
			std::istream Stream(&StreamBuffer);
			Stream.exceptions(Stream.badbit | Stream.failbit);
			return get_process_subsystem_from_module_impl(Stream);
		}
		catch (const std::exception&)
		{
			return os::process::image_type::unknown;
		}
	}
}

namespace os::process
{
	image_type get_process_subsystem(HANDLE const Process)
	{
		if (const auto Type = get_process_subsystem_from_memory(Process); Type != image_type::unknown)
			return Type;

		if (const auto Type = get_process_subsystem_from_module(Process); Type != image_type::unknown)
			return Type;

		return image_type::unknown;
	}
}

#ifndef IMPORTS_HPP_0589C56B_4071_48EE_B07F_312C2E392280
#define IMPORTS_HPP_0589C56B_4071_48EE_B07F_312C2E392280
#pragma once

/*
imports.hpp

импортируемые функции
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

// Internal:
#include "platform.hpp"

// Platform:

// Common:
#include "common/nifty_counter.hpp"

// External:

//----------------------------------------------------------------------------

namespace imports_detail
{

class imports
{
private:
#define MODULE(MODULE) m_##MODULE{WIDE_SV(#MODULE)}

	const os::rtdl::module
		MODULE(ntdll),
		MODULE(kernel32),
		MODULE(shell32),
		MODULE(user32),
		MODULE(virtdisk),
		MODULE(rstrtmgr),
		MODULE(netapi32),
		MODULE(dbgeng),
		MODULE(dbghelp),
		MODULE(dwmapi);

#undef MODULE


	template<const os::rtdl::module imports::* ModuleAccessor, auto Name, auto StubFunction>
	class unique_function_pointer
	{
		using function_type = decltype(StubFunction);

	public:
		unique_function_pointer() = default;
		NONCOPYABLE(unique_function_pointer);

		explicit(false) operator function_type() const { return get_pointer(); }
		explicit operator bool() const noexcept { return get_pointer() != StubFunction; }

	private:
		auto get_pointer() const;
		mutable function_type m_Pointer{};
	};

#define DEFINE_IMPORT_FUNCTION(MODULE, FALLBACK_DO, FALLBACK_RET, CALLTYPE, RETTYPE, NAME, ...) \
private: \
	static constexpr char name_##NAME[] = #NAME; \
	static RETTYPE CALLTYPE stub_##NAME(__VA_ARGS__) \
	{ \
		log_usage(name_##NAME); \
		do_##FALLBACK_DO(); \
		return ret_##FALLBACK_RET(); \
	} \
public: \
	const unique_function_pointer<&imports::m_##MODULE, name_##NAME, stub_##NAME> NAME

	// NT4 < 2k < XP < 2k3 < Vista < 7 < 8 < 10
	// Rock bottom is 2k

	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQueryDirectoryFile, HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQueryInformationFile, HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtSetInformationFile, HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQueryObject, HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtOpenDirectoryObject, PHANDLE DirectoryHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQueryDirectoryObject, HANDLE DirectoryHandle, PVOID Buffer, ULONG Length, BOOLEAN ReturnSingleEntry, BOOLEAN RestartScan, PULONG  Context, PULONG  ReturnLength);
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtOpenSymbolicLinkObject, PHANDLE LinkHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQuerySymbolicLinkObject, HANDLE LinkHandle, PUNICODE_STRING LinkTarget, PULONG ReturnedLength); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtClose, HANDLE Handle); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, le,    NTAPI, ULONG,    RtlNtStatusToDosError, NTSTATUS Status); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, false, NTAPI, BOOLEAN,  RtlAcquireResourceExclusive, PRTL_RESOURCE Res, BOOLEAN WaitForAccess); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, false, NTAPI, BOOLEAN,  RtlAcquireResourceShared, PRTL_RESOURCE Res, BOOLEAN WaitForAccess); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, void,  NTAPI, void,     RtlInitializeResource, PRTL_RESOURCE Res); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, void,  NTAPI, void,     RtlReleaseResource, PRTL_RESOURCE Res); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, void,  NTAPI, void,     RtlDeleteResource, PRTL_RESOURCE Res); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQueryInformationProcess, HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQueryInformationThread, HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtQuerySystemInformation, SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, zero,  NTAPI, WORD,     RtlCaptureStackBackTrace, DWORD FramesToSkip, DWORD FramesToCapture, PVOID* BackTrace, PDWORD BackTraceHash); // NT4
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, RtlGetVersion, PRTL_OSVERSIONINFOW VersionInformation); // 2k
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, RtlGetLastNtStatus); // XP
#ifndef _WIN64
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtWow64QueryInformationProcess64, HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength); // 2k3
	DEFINE_IMPORT_FUNCTION(ntdll, nop, nt,    NTAPI, NTSTATUS, NtWow64ReadVirtualMemory64, HANDLE Process, ULONG64 BaseAddress, LPVOID Buffer, ULONG64 Size, PULONG64 NumberOfBytesRead); // 2k3
#endif

	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetConsoleKeyboardLayoutNameW, LPWSTR Buffer); // NT4
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    SetConsoleKeyShortcuts, BOOL Set, BYTE ReserveKeys, LPVOID AppKeys, DWORD NumAppKeys); // NT4
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    HeapSetInformation, HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength); // 2k + KB816542
	DEFINE_IMPORT_FUNCTION(kernel32, le, nullptr, WINAPI,  PVOID,   AddVectoredExceptionHandler, ULONG First, PVECTORED_EXCEPTION_HANDLER Handler); // XP
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  ULONG,   RemoveVectoredExceptionHandler, PVOID Handle); // XP
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    TzSpecificLocalTimeToSystemTime, const TIME_ZONE_INFORMATION* TimeZoneInformation, const SYSTEMTIME* LocalTime, LPSYSTEMTIME UniversalTime); // XP
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetModuleHandleExW, DWORD Flags, LPCWSTR ModuleName, HMODULE* Module);
	DEFINE_IMPORT_FUNCTION(kernel32, le, handle,  WINAPI,  HANDLE,  FindFirstStreamW, LPCWSTR FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID FindStreamData, DWORD Flags); // 2k3
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    FindNextStreamW, HANDLE FindStream, LPVOID FindStreamData); // 2k3
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetVolumePathNamesForVolumeNameW, LPCWSTR VolumeName, LPWSTR VolumePathNames, DWORD BufferLength, PDWORD ReturnLength); // 2k3
#ifndef _WIN64
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    IsWow64Process, HANDLE Process, PBOOL Wow64Process); // 2k3
#endif
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOLEAN, CreateSymbolicLinkW, LPCWSTR SymlinkFileName, LPCWSTR TargetFileName, DWORD Flags); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, handle,  WINAPI,  HANDLE,  FindFirstFileNameW, LPCWSTR FileName, DWORD Flags, LPDWORD StringLength, LPWSTR LinkName); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    FindNextFileNameW, HANDLE FindStream, LPDWORD StringLength, PWCHAR LinkName); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, zero,    WINAPI,  DWORD,   GetFinalPathNameByHandleW, HANDLE File, LPWSTR FilePath, DWORD FilePathSize, DWORD Flags); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetNamedPipeServerProcessId, HANDLE Pipe, PULONG ServerProcessId); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    CancelSynchronousIo, HANDLE Thread); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetConsoleScreenBufferInfoEx, HANDLE ConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX ConsoleScreenBufferInfoEx); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    SetConsoleScreenBufferInfoEx, HANDLE ConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX ConsoleScreenBufferInfoEx); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    QueryFullProcessImageNameW, HANDLE Process, DWORD Flags, LPWSTR ExeName, PDWORD Size); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, void,    WINAPI,  void,    InitializeSRWLock, PSRWLOCK SRWLock); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, void,    WINAPI,  void,    AcquireSRWLockExclusive, PSRWLOCK SRWLock); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, void,    WINAPI,  void,    AcquireSRWLockShared, PSRWLOCK SRWLock); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, void,    WINAPI,  void,    ReleaseSRWLockExclusive, PSRWLOCK SRWLock); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, void,    WINAPI,  void,    ReleaseSRWLockShared, PSRWLOCK SRWLock); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, zero,    WINAPI,  int,     CompareStringOrdinal, LPCWCH String1, int Count1, LPCWCH String2, int Count2, BOOL IgnoreCase); // Vista
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetProductInfo, DWORD OSMajorVersion, DWORD OSMinorVersion, DWORD SpMajorVersion, DWORD SpMinorVersion, PDWORD ReturnedProductType);
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOL,    GetPhysicallyInstalledSystemMemory, PULONGLONG TotalMemoryInKilobytes); // Vista SP1
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOLEAN, TryAcquireSRWLockExclusive, PSRWLOCK SRWLock); // 7
	DEFINE_IMPORT_FUNCTION(kernel32, le, false,   WINAPI,  BOOLEAN, TryAcquireSRWLockShared, PSRWLOCK SRWLock); // 7
	DEFINE_IMPORT_FUNCTION(kernel32, le, void,    WINAPI,  void,    GetSystemTimePreciseAsFileTime, LPFILETIME SystemTimeAsFileTime); // 8
	DEFINE_IMPORT_FUNCTION(kernel32, le, hr,      WINAPI,  HRESULT, SetThreadDescription, HANDLE Thread, PCWSTR ThreadDescription); // 10
	DEFINE_IMPORT_FUNCTION(kernel32, le, hr,      WINAPI,  HRESULT, GetThreadDescription, HANDLE Thread, PWSTR* ThreadDescription); // 10

	DEFINE_IMPORT_FUNCTION(shell32, nop, hr, STDAPICALLTYPE, HRESULT, SHCreateAssociationRegistration, REFIID riid, void** ppv); // Vista

	DEFINE_IMPORT_FUNCTION(user32, le, nullptr, WINAPI, HPOWERNOTIFY, RegisterPowerSettingNotification, HANDLE hRecipient, LPCGUID PowerSettingGuid, DWORD Flags); // Vista
	DEFINE_IMPORT_FUNCTION(user32, le, false,   WINAPI, BOOL,         UnregisterPowerSettingNotification, HPOWERNOTIFY Handle); // Vista

	DEFINE_IMPORT_FUNCTION(virtdisk, nop, le, WINAPI, DWORD, GetStorageDependencyInformation, HANDLE ObjectHandle, GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed); // 7
	DEFINE_IMPORT_FUNCTION(virtdisk, nop, le, WINAPI, DWORD, OpenVirtualDisk, PVIRTUAL_STORAGE_TYPE VirtualStorageType, PCWSTR Path, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, POPEN_VIRTUAL_DISK_PARAMETERS Parameters, PHANDLE Handle); // 7
	DEFINE_IMPORT_FUNCTION(virtdisk, nop, le, WINAPI, DWORD, DetachVirtualDisk, HANDLE VirtualDiskHandle, DETACH_VIRTUAL_DISK_FLAG Flags, ULONG ProviderSpecificFlags); // 7

	DEFINE_IMPORT_FUNCTION(rstrtmgr, nop, le, WINAPI, DWORD, RmStartSession, DWORD *SessionHandle, DWORD SessionFlags, WCHAR strSessionKey[]); // Vista
	DEFINE_IMPORT_FUNCTION(rstrtmgr, nop, le, WINAPI, DWORD, RmEndSession, DWORD dwSessionHandle); // Vista
	DEFINE_IMPORT_FUNCTION(rstrtmgr, nop, le, WINAPI, DWORD, RmRegisterResources, DWORD dwSessionHandle, UINT nFiles, LPCWSTR rgsFilenames[], UINT nApplications, RM_UNIQUE_PROCESS rgApplications[], UINT nServices, LPCWSTR rgsServiceNames[]); // Vista
	DEFINE_IMPORT_FUNCTION(rstrtmgr, nop, le, WINAPI, DWORD, RmGetList, DWORD dwSessionHandle, UINT *pnProcInfoNeeded, UINT *pnProcInfo, RM_PROCESS_INFO rgAffectedApps[], LPDWORD lpdwRebootReasons); // Vista

	DEFINE_IMPORT_FUNCTION(netapi32, nop, net, NET_API_FUNCTION, NET_API_STATUS, NetDfsGetInfo, LPWSTR DfsEntryPath, LPWSTR ServerName, LPWSTR ShareName, DWORD Level, LPBYTE* Buffer); // 2k
	DEFINE_IMPORT_FUNCTION(netapi32, nop, net, NET_API_FUNCTION, NET_API_STATUS, NetDfsGetClientInfo, LPWSTR DfsEntryPath, LPWSTR ServerName, LPWSTR ShareName, DWORD Level, LPBYTE* Buffer); // 2k

	DEFINE_IMPORT_FUNCTION(dbgeng, nop, hr, STDAPICALLTYPE, HRESULT, DebugCreate, REFIID InterfaceId, PVOID* Interface);

	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymInitialize, HANDLE Process, PCSTR UserSearchPath, BOOL InvadeProcess); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetSearchPath, HANDLE Process, PSTR SearchPath, DWORD SearchPathLength); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymSetSearchPath, HANDLE Process, PCSTR SearchPath); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymCleanup, HANDLE Process); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, zero,    WINAPI, DWORD,   SymSetOptions, DWORD SymOptions); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    StackWalk64, DWORD MachineType, HANDLE Process, HANDLE Thread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetModuleInfoW64, HANDLE Process, DWORD64 Addr, PIMAGEHLP_MODULEW64 ModuleInfo); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, nullptr, WINAPI, PVOID,   SymFunctionTableAccess64, HANDLE Process, DWORD64 AddrBase); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, zero,    WINAPI, DWORD64, SymGetModuleBase64, HANDLE Process, DWORD64 Address); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymFromAddr, HANDLE Process, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetSymFromAddr64, HANDLE Process, DWORD64 Addr, PDWORD64 Displacement, PIMAGEHLP_SYMBOL64 Symbol); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetLineFromAddr64, HANDLE Process, DWORD64 Addr, PDWORD Displacement, PIMAGEHLP_LINE64 Line); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, zero,    WINAPI, DWORD,   UnDecorateSymbolName, PCSTR Name, PSTR OutputString, DWORD MaxStringLength, DWORD Flags); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymRegisterCallback64, HANDLE Process, PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction, ULONG64 UserContext); // 2k
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    MiniDumpWriteDump, HANDLE Process, DWORD ProcessId, HANDLE File, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam); // XP
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymInitializeW, HANDLE Process, PCWSTR UserSearchPath, BOOL InvadeProcess); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetSearchPathW, HANDLE Process, PWSTR SearchPath, DWORD SearchPathLength); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymSetSearchPathW, HANDLE Process, PCWSTR SearchPath); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymFromAddrW, HANDLE Process, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetLineFromAddrW64, HANDLE Process, DWORD64 Addr, PDWORD Displacement, PIMAGEHLP_LINEW64 Line); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, zero,    WINAPI, DWORD,   UnDecorateSymbolNameW, PCWSTR Name, PWSTR OutputString, DWORD MaxStringLength, DWORD Flags); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymRegisterCallbackW64, HANDLE Process, PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction, ULONG64 UserContext); // Vista
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    StackWalkEx, DWORD MachineType, HANDLE Process, HANDLE Thread, LPSTACKFRAME_EX StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress, DWORD Flags); // 8
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymFromInlineContextW, HANDLE Process, DWORD64 Address, ULONG InlineContext, PDWORD64 Displacement, PSYMBOL_INFOW Symbol); // 8
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymGetLineFromInlineContextW, HANDLE Process, DWORD64 Address, ULONG InlineContext, DWORD64 ModuleBaseAddress, PDWORD Displacement, PIMAGEHLP_LINEW64 Line); // 8
	DEFINE_IMPORT_FUNCTION(dbghelp, le, zero,    WINAPI, DWORD,   SymAddrIncludeInlineTrace, HANDLE Process, DWORD64 Address); // 8
	DEFINE_IMPORT_FUNCTION(dbghelp, le, false,   WINAPI, BOOL,    SymQueryInlineTrace, HANDLE Process, DWORD64 StartAddress, DWORD StartContext, DWORD64 StartRetAddress, DWORD64 CurAddress, LPDWORD CurContext, LPDWORD CurFrameIndex); // 8

	DEFINE_IMPORT_FUNCTION(dwmapi, nop, hr, WINAPI, HRESULT, DwmGetWindowAttribute, HWND Hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute); // Vista

#undef DEFINE_IMPORT_FUNCTION

	static void* get_pointer_impl(const os::rtdl::module& Module, const char* Name);
	static void log_missing_import(const os::rtdl::module& Module, std::string_view Name);
	static void log_usage(std::string_view Name);

	static void do_le();
	static void do_nop();

	static NTSTATUS       ret_nt();
	static DWORD          ret_le();
	static HANDLE         ret_handle();
	static HRESULT        ret_hr();
	static NET_API_STATUS ret_net();
	static std::nullptr_t ret_nullptr();
	static int            ret_zero();
	static bool           ret_false();
	static void           ret_void();
};

}

NIFTY_DECLARE(imports_detail::imports, imports);

namespace imports_detail
{
	template<const os::rtdl::module imports::* ModuleAccessor, auto Name, auto StubFunction>
	auto imports::unique_function_pointer<ModuleAccessor, Name, StubFunction>::get_pointer() const
	{
		if (m_Pointer)
			return m_Pointer;

		if (const auto DynamicPointer = std::bit_cast<function_type>(get_pointer_impl(std::invoke(ModuleAccessor, ::imports), Name)))
		{
			m_Pointer = DynamicPointer;
			return m_Pointer;
		}

		log_missing_import(std::invoke(ModuleAccessor, ::imports), Name);

		m_Pointer = StubFunction;
		return m_Pointer;
	}
}

#endif // IMPORTS_HPP_0589C56B_4071_48EE_B07F_312C2E392280

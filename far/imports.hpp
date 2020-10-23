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
public:
	imports() = default;

private:
#define DECLARE_MODULE(MODULE) const os::rtdl::module m_##MODULE{L###MODULE}

	DECLARE_MODULE(ntdll);
	DECLARE_MODULE(kernel32);
	DECLARE_MODULE(shell32);
	DECLARE_MODULE(user32);
	DECLARE_MODULE(virtdisk);
	DECLARE_MODULE(rstrtmgr);
	DECLARE_MODULE(netapi32);
	DECLARE_MODULE(dbghelp);
	DECLARE_MODULE(dwmapi);

#undef DECLARE_MODULE

	template<typename text_type, auto StubFunction>
	class unique_function_pointer
	{
		// The indirection is a workaround for MSVC
		using function_type = std::enable_if_t<true, decltype(StubFunction)>;

	public:
		NONCOPYABLE(unique_function_pointer);

		explicit unique_function_pointer(const os::rtdl::module& Module): m_module(&Module) {}
		operator function_type() const { return get_pointer(); }
		explicit operator bool() const noexcept { return get_pointer() != StubFunction; }

	private:
		auto get_pointer() const
		{
			static const auto DynamicPointer = m_module->GetProcAddress<function_type>(text_type::name);
			// TODO: log if nullptr
			static const auto Pointer = DynamicPointer? DynamicPointer : StubFunction;
			return Pointer;
		}

		const os::rtdl::module* m_module;
	};

#define DECLARE_IMPORT_FUNCTION(MODULE, CALLTYPE, RETTYPE, NAME, ...)\
private: static RETTYPE CALLTYPE stub_##NAME(__VA_ARGS__);\
private: struct name_##NAME { static constexpr auto name = #NAME; };\
public: const unique_function_pointer<name_##NAME, stub_##NAME> NAME{m_##MODULE}

	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtQueryDirectoryFile, HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtQueryInformationFile, HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtSetInformationFile, HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtQueryObject, HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtOpenSymbolicLinkObject, PHANDLE LinkHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtQuerySymbolicLinkObject, HANDLE LinkHandle, PUNICODE_STRING LinkTarget, PULONG ReturnedLength);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, NtClose, HANDLE Handle);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, RtlGetLastNtStatus);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, RtlNtStatusToDosError, NTSTATUS Status);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, NTSTATUS, RtlGetVersion, PRTL_OSVERSIONINFOW VersionInformation);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, BOOLEAN, RtlAcquireResourceExclusive, PRTL_RESOURCE Res, BOOLEAN WaitForAccess);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, BOOLEAN, RtlAcquireResourceShared, PRTL_RESOURCE Res, BOOLEAN WaitForAccess);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, void, RtlInitializeResource, PRTL_RESOURCE Res);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, void, RtlReleaseResource, PRTL_RESOURCE Res);
	DECLARE_IMPORT_FUNCTION(ntdll, NTAPI, void, RtlDeleteResource, PRTL_RESOURCE Res);

	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, GetConsoleKeyboardLayoutNameW, LPWSTR Buffer);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOLEAN, CreateSymbolicLinkW, LPCWSTR SymlinkFileName, LPCWSTR TargetFileName, DWORD Flags);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, HANDLE, FindFirstFileNameW, LPCWSTR FileName, DWORD Flags, LPDWORD StringLength, LPWSTR LinkName);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, FindNextFileNameW, HANDLE FindStream, LPDWORD StringLength, PWCHAR LinkName);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, HANDLE, FindFirstStreamW, LPCWSTR FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID FindStreamData, DWORD Flags);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, FindNextStreamW, HANDLE FindStream, LPVOID FindStreamData);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, DWORD, GetFinalPathNameByHandleW, HANDLE File, LPWSTR FilePath, DWORD FilePathSize, DWORD Flags);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, GetVolumePathNamesForVolumeNameW, LPCWSTR VolumeName, LPWSTR VolumePathNames, DWORD BufferLength, PDWORD ReturnLength);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, GetPhysicallyInstalledSystemMemory, PULONGLONG TotalMemoryInKilobytes);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, HeapSetInformation, HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, IsWow64Process, HANDLE Process, PBOOL Wow64Process);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, GetNamedPipeServerProcessId, HANDLE Pipe, PULONG ServerProcessId);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, CancelSynchronousIo, HANDLE Thread);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, SetConsoleKeyShortcuts, BOOL Set, BYTE ReserveKeys, LPVOID AppKeys, DWORD NumAppKeys);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, GetConsoleScreenBufferInfoEx, HANDLE ConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX ConsoleScreenBufferInfoEx);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, QueryFullProcessImageNameW, HANDLE Process, DWORD Flags, LPWSTR ExeName, PDWORD Size);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOL, TzSpecificLocalTimeToSystemTime, const TIME_ZONE_INFORMATION* TimeZoneInformation, const SYSTEMTIME* LocalTime, LPSYSTEMTIME UniversalTime);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, void, InitializeSRWLock, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, void, AcquireSRWLockExclusive, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, void, AcquireSRWLockShared, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, void, ReleaseSRWLockExclusive, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, void, ReleaseSRWLockShared, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOLEAN, TryAcquireSRWLockExclusive, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, BOOLEAN, TryAcquireSRWLockShared, PSRWLOCK SRWLock);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, void, GetSystemTimePreciseAsFileTime, LPFILETIME SystemTimeAsFileTime);
	DECLARE_IMPORT_FUNCTION(kernel32, WINAPI, int, CompareStringOrdinal, LPCWCH String1, int Count1, LPCWCH String2, int Count2, BOOL IgnoreCase);
	DECLARE_IMPORT_FUNCTION(kernel32, NTAPI,  WORD, RtlCaptureStackBackTrace, DWORD FramesToSkip, DWORD FramesToCapture, PVOID* BackTrace, PDWORD BackTraceHash);

	DECLARE_IMPORT_FUNCTION(shell32, STDAPICALLTYPE, HRESULT, SHCreateAssociationRegistration, REFIID riid, void** ppv);

	DECLARE_IMPORT_FUNCTION(user32, WINAPI, HPOWERNOTIFY, RegisterPowerSettingNotification, HANDLE hRecipient, LPCGUID PowerSettingGuid, DWORD Flags);
	DECLARE_IMPORT_FUNCTION(user32, WINAPI, BOOL, UnregisterPowerSettingNotification, HPOWERNOTIFY Handle);

	DECLARE_IMPORT_FUNCTION(virtdisk, WINAPI, DWORD, GetStorageDependencyInformation, HANDLE ObjectHandle, GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed);
	DECLARE_IMPORT_FUNCTION(virtdisk, WINAPI, DWORD, OpenVirtualDisk, PVIRTUAL_STORAGE_TYPE VirtualStorageType, PCWSTR Path, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, POPEN_VIRTUAL_DISK_PARAMETERS Parameters, PHANDLE Handle);
	DECLARE_IMPORT_FUNCTION(virtdisk, WINAPI, DWORD, DetachVirtualDisk, HANDLE VirtualDiskHandle, DETACH_VIRTUAL_DISK_FLAG Flags, ULONG ProviderSpecificFlags);

	DECLARE_IMPORT_FUNCTION(rstrtmgr, WINAPI, DWORD, RmStartSession, DWORD *SessionHandle, DWORD SessionFlags, WCHAR strSessionKey[]);
	DECLARE_IMPORT_FUNCTION(rstrtmgr, WINAPI, DWORD, RmEndSession, DWORD dwSessionHandle);
	DECLARE_IMPORT_FUNCTION(rstrtmgr, WINAPI, DWORD, RmRegisterResources, DWORD dwSessionHandle, UINT nFiles, LPCWSTR rgsFilenames[], UINT nApplications, RM_UNIQUE_PROCESS rgApplications[], UINT nServices, LPCWSTR rgsServiceNames[]);
	DECLARE_IMPORT_FUNCTION(rstrtmgr, WINAPI, DWORD, RmGetList, DWORD dwSessionHandle, UINT *pnProcInfoNeeded, UINT *pnProcInfo, RM_PROCESS_INFO rgAffectedApps[], LPDWORD lpdwRebootReasons);

	DECLARE_IMPORT_FUNCTION(netapi32, NET_API_FUNCTION, NET_API_STATUS, NetDfsGetInfo, LPWSTR DfsEntryPath, LPWSTR ServerName, LPWSTR ShareName, DWORD Level, LPBYTE* Buffer);
	DECLARE_IMPORT_FUNCTION(netapi32, NET_API_FUNCTION, NET_API_STATUS, NetDfsGetClientInfo, LPWSTR DfsEntryPath, LPWSTR ServerName, LPWSTR ShareName, DWORD Level, LPBYTE* Buffer);

	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, MiniDumpWriteDump, HANDLE Process, DWORD ProcessId, HANDLE File, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, StackWalk64, DWORD MachineType, HANDLE Process, HANDLE Thread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymInitialize, HANDLE Process, PCSTR UserSearchPath, BOOL InvadeProcess);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymInitializeW, HANDLE Process, PCWSTR UserSearchPath, BOOL InvadeProcess);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymCleanup, HANDLE Process);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymFromAddr, HANDLE Process, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymFromAddrW, HANDLE Process, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, DWORD, SymSetOptions, DWORD SymOptions);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymGetLineFromAddr64, HANDLE Process, DWORD64 Addr, PDWORD Displacement, PIMAGEHLP_LINE64 Line);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymGetLineFromAddrW64, HANDLE Process, DWORD64 Addr, PDWORD Displacement, PIMAGEHLP_LINEW64 Line);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, BOOL, SymGetModuleInfoW64, HANDLE Process, DWORD64 Addr, PIMAGEHLP_MODULEW64 ModuleInfo);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, PVOID, SymFunctionTableAccess64, HANDLE Process, DWORD64 AddrBase);
	DECLARE_IMPORT_FUNCTION(dbghelp, WINAPI, DWORD64, SymGetModuleBase64, HANDLE Process, DWORD64 Address);

	DECLARE_IMPORT_FUNCTION(dwmapi, WINAPI, HRESULT, DwmGetWindowAttribute, HWND Hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);

#undef DECLARE_IMPORT_FUNCTION
};

}

NIFTY_DECLARE(imports_detail::imports, imports);

#endif // IMPORTS_HPP_0589C56B_4071_48EE_B07F_312C2E392280

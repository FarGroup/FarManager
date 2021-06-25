/*
imports.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "imports.hpp"

// Internal:
#include "log.hpp"
#include "encoding.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

namespace imports_detail
{
	void imports::log_missing_import(const os::rtdl::module& Module, std::string_view const Name)
	{
		if (Module)
			LOGWARNING(L"[{}:{}]: function not found"sv, Module.name(), encoding::utf8::get_chars(Name));
		else
			LOGWARNING(L"[{}:{}]: module not loaded"sv, Module.name(), encoding::utf8::get_chars(Name));
	}

// ntdll
NTSTATUS NTAPI imports::stub_NtQueryDirectoryFile(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtQueryInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtSetInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtQueryObject(HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtOpenSymbolicLinkObject(PHANDLE LinkHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtQuerySymbolicLinkObject(HANDLE LinkHandle, PUNICODE_STRING LinkTarget, PULONG ReturnedLength)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtClose(HANDLE Handle)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_RtlGetLastNtStatus()
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_RtlNtStatusToDosError(NTSTATUS Status)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_RtlGetVersion(PRTL_OSVERSIONINFOW VersionInformation)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

BOOLEAN NTAPI imports::stub_RtlAcquireResourceExclusive(PRTL_RESOURCE Res, BOOLEAN WaitForAccess)
{
	LOGWARNING(L"Stub call"sv);
	return FALSE;
}

BOOLEAN NTAPI imports::stub_RtlAcquireResourceShared(PRTL_RESOURCE Res, BOOLEAN WaitForAccess)
{
	LOGWARNING(L"Stub call"sv);
	return FALSE;
}

void NTAPI imports::stub_RtlInitializeResource(PRTL_RESOURCE Res)
{
	LOGWARNING(L"Stub call"sv);
}

void NTAPI imports::stub_RtlReleaseResource(PRTL_RESOURCE Res)
{
	LOGWARNING(L"Stub call"sv);
}

void NTAPI imports::stub_RtlDeleteResource(PRTL_RESOURCE Res)
{
	LOGWARNING(L"Stub call"sv);
}

NTSTATUS NTAPI imports::stub_NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

WORD NTAPI imports::stub_RtlCaptureStackBackTrace(DWORD FramesToSkip, DWORD FramesToCapture, PVOID* BackTrace, PDWORD BackTraceHash)
{
	LOGWARNING(L"Stub call"sv);
	return 0;
}

#ifndef _WIN64
NTSTATUS NTAPI imports::stub_NtWow64QueryInformationProcess64(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS NTAPI imports::stub_NtWow64ReadVirtualMemory64(HANDLE Process, ULONG64 BaseAddress, LPVOID Buffer, ULONG64 Size, PULONG64 NumberOfBytesRead)
{
	LOGWARNING(L"Stub call"sv);
	return STATUS_NOT_IMPLEMENTED;
}
#endif

// kernel32
BOOL WINAPI imports::stub_GetConsoleKeyboardLayoutNameW(LPWSTR Buffer)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOLEAN WINAPI imports::stub_CreateSymbolicLinkW(LPCWSTR SymlinkFileName, LPCWSTR TargetFileName, DWORD Flags)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

HANDLE WINAPI imports::stub_FindFirstFileNameW(LPCWSTR FileName, DWORD Flags, LPDWORD StringLength, LPWSTR LinkName)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return INVALID_HANDLE_VALUE;
}

BOOL WINAPI imports::stub_FindNextFileNameW(HANDLE FindStream, LPDWORD StringLength, PWCHAR LinkName)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

HANDLE WINAPI imports::stub_FindFirstStreamW(LPCWSTR FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID FindStreamData, DWORD Flags)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return INVALID_HANDLE_VALUE;
}

BOOL WINAPI imports::stub_FindNextStreamW(HANDLE FindStream, LPVOID FindStreamData)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

DWORD WINAPI imports::stub_GetFinalPathNameByHandleW(HANDLE File, LPWSTR FilePath, DWORD FilePathSize, DWORD Flags)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return 0;
}

BOOL WINAPI imports::stub_GetVolumePathNamesForVolumeNameW(LPCWSTR VolumeName, LPWSTR VolumePathNames, DWORD BufferLength, PDWORD ReturnLength)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_GetPhysicallyInstalledSystemMemory(PULONGLONG TotalMemoryInKilobytes)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_HeapSetInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

#ifndef _WIN64
BOOL WINAPI imports::stub_IsWow64Process(HANDLE Process, PBOOL Wow64Process)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}
#endif

BOOL WINAPI imports::stub_GetNamedPipeServerProcessId(HANDLE Pipe, PULONG ServerProcessId)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_CancelSynchronousIo(HANDLE Thread)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SetConsoleKeyShortcuts(BOOL Set, BYTE ReserveKeys, LPVOID AppKeys, DWORD NumAppKeys)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_GetConsoleScreenBufferInfoEx(HANDLE ConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX ConsoleScreenBufferInfoEx)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_QueryFullProcessImageNameW(HANDLE Process, DWORD Flags, LPWSTR ExeName, PDWORD Size)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_TzSpecificLocalTimeToSystemTime(const TIME_ZONE_INFORMATION* TimeZoneInformation, const SYSTEMTIME* LocalTime, LPSYSTEMTIME UniversalTime)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

void WINAPI imports::stub_InitializeSRWLock(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

void WINAPI imports::stub_AcquireSRWLockExclusive(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

void WINAPI imports::stub_AcquireSRWLockShared(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

void WINAPI imports::stub_ReleaseSRWLockExclusive(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

void WINAPI imports::stub_ReleaseSRWLockShared(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

BOOLEAN WINAPI imports::stub_TryAcquireSRWLockExclusive(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOLEAN WINAPI imports::stub_TryAcquireSRWLockShared(PSRWLOCK SRWLock)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

void WINAPI imports::stub_GetSystemTimePreciseAsFileTime(LPFILETIME SystemTimeAsFileTime)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

int WINAPI imports::stub_CompareStringOrdinal(LPCWCH String1, int Count1, LPCWCH String2, int Count2, BOOL IgnoreCase)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return 0;
}

HRESULT WINAPI imports::stub_SetThreadDescription(HANDLE Thread, PCWSTR ThreadDescription)
{
	// TODO: log
	return E_NOTIMPL;
}

// shell32
HRESULT STDAPICALLTYPE imports::stub_SHCreateAssociationRegistration(REFIID riid, void ** ppv)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

// user32
BOOL WINAPI imports::stub_UnregisterPowerSettingNotification(HPOWERNOTIFY Handle)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

HPOWERNOTIFY WINAPI imports::stub_RegisterPowerSettingNotification(HANDLE hRecipient, LPCGUID PowerSettingGuid, DWORD Flags)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return nullptr;
}

// virtdisk
DWORD WINAPI imports::stub_GetStorageDependencyInformation(HANDLE ObjectHandle, GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI imports::stub_OpenVirtualDisk(PVIRTUAL_STORAGE_TYPE VirtualStorageType, PCWSTR Path, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, POPEN_VIRTUAL_DISK_PARAMETERS Parameters, PHANDLE Handle)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI imports::stub_DetachVirtualDisk(HANDLE VirtualDiskHandle, DETACH_VIRTUAL_DISK_FLAG Flags, ULONG ProviderSpecificFlags)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

// rstrtmgr
DWORD WINAPI imports::stub_RmStartSession(DWORD *SessionHandle, DWORD SessionFlags, WCHAR strSessionKey[])
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI imports::stub_RmEndSession(DWORD dwSessionHandle)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI imports::stub_RmRegisterResources(DWORD dwSessionHandle, UINT nFiles, LPCWSTR rgsFilenames[], UINT nApplications, RM_UNIQUE_PROCESS rgApplications[], UINT nServices, LPCWSTR rgsServiceNames[])
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI imports::stub_RmGetList(DWORD dwSessionHandle, UINT *pnProcInfoNeeded, UINT *pnProcInfo, RM_PROCESS_INFO rgAffectedApps[], LPDWORD lpdwRebootReasons)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

// netapi32
NET_API_STATUS NET_API_FUNCTION imports::stub_NetDfsGetInfo(LPWSTR DfsEntryPath, LPWSTR ServerName, LPWSTR ShareName, DWORD Level, LPBYTE* Buffer)
{
	LOGWARNING(L"Stub call"sv);
	return NERR_InvalidAPI;
}

// netapi32
NET_API_STATUS NET_API_FUNCTION imports::stub_NetDfsGetClientInfo(LPWSTR DfsEntryPath, LPWSTR ServerName, LPWSTR ShareName, DWORD Level, LPBYTE* Buffer)
{
	LOGWARNING(L"Stub call"sv);
	return NERR_InvalidAPI;
}

// dbghelp
BOOL WINAPI imports::stub_MiniDumpWriteDump(HANDLE Process, DWORD ProcessId, HANDLE File, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_StackWalk64(DWORD MachineType, HANDLE Process, HANDLE Thread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymInitialize(HANDLE Process, PCSTR UserSearchPath, BOOL InvadeProcess)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymInitializeW(HANDLE Process, LPCWSTR UserSearchPath, BOOL InvadeProcess)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymCleanup(HANDLE Process)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymFromAddr(HANDLE Process, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymFromAddrW(HANDLE Process, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

DWORD WINAPI imports::stub_SymSetOptions(DWORD SymOptions)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return 0;
}

BOOL WINAPI imports::stub_SymGetSymFromAddr64(HANDLE Process, DWORD64 Addr, PDWORD64 Displacement, PIMAGEHLP_SYMBOL64 Symbol)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymGetLineFromAddr64(HANDLE Process, DWORD64 Addr, PDWORD Displacement, PIMAGEHLP_LINE64 Line)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymGetLineFromAddrW64(HANDLE Process, DWORD64 Addr, PDWORD Displacement, PIMAGEHLP_LINEW64 Line)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL WINAPI imports::stub_SymGetModuleInfoW64(HANDLE Process, DWORD64 Addr, PIMAGEHLP_MODULEW64 ModuleInfo)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

PVOID WINAPI imports::stub_SymFunctionTableAccess64(HANDLE Process, DWORD64 AddrBase)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return nullptr;
}

DWORD64 WINAPI imports::stub_SymGetModuleBase64(HANDLE Process, DWORD64 Address)
{
	LOGWARNING(L"Stub call"sv);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return 0;
}

// dwmapi
HRESULT WINAPI imports::stub_DwmGetWindowAttribute(HWND Hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute)
{
	LOGWARNING(L"Stub call"sv);
	return ERROR_CALL_NOT_IMPLEMENTED;
}

}

NIFTY_DEFINE(imports_detail::imports, imports);

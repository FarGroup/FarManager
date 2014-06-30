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

#include "headers.hpp"
#pragma hdrstop

#include "imports.hpp"
#include "farexcpt.hpp"

ImportedFunctions& Imports()
{
	static ImportedFunctions ifn;
	return ifn;
}

ImportedFunctions::module::module(const wchar_t* name):
	m_module(GetModuleHandle(name)),
	m_loaded(false)
{
	if (!m_module)
	{
		m_loaded = (m_module = LoadLibrary(name)) != nullptr;
	}
}

ImportedFunctions::module::~module()
{
	if (m_loaded)
	{
		FreeLibrary(m_module);
	}
}

FARPROC ImportedFunctions::module::GetProcAddress(const char* name) const
{
	return ::GetProcAddress(m_module, name);
}

ImportedFunctions::ImportedFunctions():
	m_Ntdll(L"ntdll"),
	m_Kernel(L"kernel32"),
	m_Shell(L"shell32"),
	m_User32(L"user32"),
	m_NetApi(L"netapi32"),
	m_VirtDisk(L"virtdisk"),
	m_RstrtMgr(L"rstrtmgr")
{
	#define InitImport(Module, Name)\
	Name = Module.GetProcAddress(#Name);\
	if (!Name)\
		Name = &ImportedFunctions::stub_##Name;

	if (m_Kernel)
	{
		InitImport(m_Kernel, GetConsoleKeyboardLayoutNameW);
		InitImport(m_Kernel, CreateSymbolicLinkW);
		InitImport(m_Kernel, FindFirstFileNameW);
		InitImport(m_Kernel, FindNextFileNameW);
		InitImport(m_Kernel, FindFirstStreamW);
		InitImport(m_Kernel, FindNextStreamW);
		InitImport(m_Kernel, GetFinalPathNameByHandleW);
		InitImport(m_Kernel, GetVolumePathNamesForVolumeNameW);
		InitImport(m_Kernel, GetPhysicallyInstalledSystemMemory);
		InitImport(m_Kernel, HeapSetInformation);
		InitImport(m_Kernel, IsWow64Process);
		InitImport(m_Kernel, GetNamedPipeServerProcessId);
		InitImport(m_Kernel, CancelSynchronousIo);
		InitImport(m_Kernel, SetConsoleKeyShortcuts);
		InitImport(m_Kernel, GetConsoleScreenBufferInfoEx);
		InitImport(m_Kernel, QueryFullProcessImageNameW);
		InitImport(m_Kernel, TzSpecificLocalTimeToSystemTime);
	}

	if (m_Ntdll)
	{
		InitImport(m_Ntdll, NtQueryDirectoryFile);
		InitImport(m_Ntdll, NtQueryInformationFile);
		InitImport(m_Ntdll, NtSetInformationFile);
		InitImport(m_Ntdll, NtQueryObject);
		InitImport(m_Ntdll, NtOpenSymbolicLinkObject);
		InitImport(m_Ntdll, NtQuerySymbolicLinkObject);
		InitImport(m_Ntdll, NtClose);
		InitImport(m_Ntdll, RtlGetLastNtStatus);
		InitImport(m_Ntdll, RtlNtStatusToDosError);
	}

	if (m_Shell)
	{
		InitImport(m_Shell, SHCreateAssociationRegistration);
	}

	if (m_NetApi)
	{
		InitImport(m_NetApi, NetDfsGetInfo);
	}

	if (m_VirtDisk)
	{
		InitImport(m_VirtDisk, GetStorageDependencyInformation);
		InitImport(m_VirtDisk, OpenVirtualDisk);
		InitImport(m_VirtDisk, DetachVirtualDisk);
	}

	if (m_User32)
	{
		InitImport(m_User32, RegisterPowerSettingNotification);
		InitImport(m_User32, UnregisterPowerSettingNotification);
	}

	if (m_RstrtMgr)
	{
		InitImport(m_RstrtMgr, RmStartSession);
		InitImport(m_RstrtMgr, RmEndSession);
		InitImport(m_RstrtMgr, RmRegisterResources);
		InitImport(m_RstrtMgr, RmGetList);
	}

	#undef InitImport
}


BOOL ImportedFunctions::stub_GetConsoleKeyboardLayoutNameW(LPWSTR Buffer)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOLEAN ImportedFunctions::stub_CreateSymbolicLinkW(LPCWSTR SymlinkFileName, LPCWSTR TargetFileName, DWORD Flags)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

HANDLE ImportedFunctions::stub_FindFirstFileNameW(LPCWSTR FileName, DWORD Flags, LPDWORD StringLength, LPWSTR LinkName)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return INVALID_HANDLE_VALUE;
}

BOOL ImportedFunctions::stub_FindNextFileNameW(HANDLE FindStream, LPDWORD StringLength, PWCHAR LinkName)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

HANDLE ImportedFunctions::stub_FindFirstStreamW(LPCWSTR FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID FindStreamData, DWORD Flags)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return INVALID_HANDLE_VALUE;
}

BOOL ImportedFunctions::stub_FindNextStreamW(HANDLE FindStream, LPVOID FindStreamData)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

DWORD ImportedFunctions::stub_GetFinalPathNameByHandleW(HANDLE File, LPWSTR FilePath, DWORD FilePathSize, DWORD Flags)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return 0;
}

BOOL ImportedFunctions::stub_GetVolumePathNamesForVolumeNameW(LPCWSTR VolumeName, LPWSTR VolumePathNames, DWORD BufferLength, PDWORD ReturnLength)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_GetPhysicallyInstalledSystemMemory(PULONGLONG TotalMemoryInKilobytes)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_HeapSetInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_IsWow64Process(HANDLE Process, PBOOL Wow64Process)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_GetNamedPipeServerProcessId(HANDLE Pipe, PULONG ServerProcessId)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_CancelSynchronousIo(HANDLE Thread)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_SetConsoleKeyShortcuts(BOOL Set, BYTE ReserveKeys, LPVOID AppKeys, DWORD NumAppKeys)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_GetConsoleScreenBufferInfoEx(HANDLE ConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX ConsoleScreenBufferInfoEx)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL ImportedFunctions::stub_TzSpecificLocalTimeToSystemTime(const TIME_ZONE_INFORMATION* TimeZoneInformation, const SYSTEMTIME* LocalTime, LPSYSTEMTIME UniversalTime)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}


NTSTATUS ImportedFunctions::stub_NtQueryDirectoryFile(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_NtQueryInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_NtSetInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_NtQueryObject(HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_NtOpenSymbolicLinkObject(PHANDLE LinkHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_NtQuerySymbolicLinkObject(HANDLE LinkHandle, PUNICODE_STRING LinkTarget, PULONG ReturnedLength)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_NtClose(HANDLE Handle)
{
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_RtlGetLastNtStatus()
{
		return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ImportedFunctions::stub_RtlNtStatusToDosError(NTSTATUS Status)
{
	return STATUS_NOT_IMPLEMENTED;
}


HRESULT ImportedFunctions::stub_SHCreateAssociationRegistration(REFIID riid, void ** ppv)
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD ImportedFunctions::stub_GetStorageDependencyInformation(HANDLE ObjectHandle, GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed)
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD ImportedFunctions::stub_OpenVirtualDisk(PVIRTUAL_STORAGE_TYPE VirtualStorageType, PCWSTR Path, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, POPEN_VIRTUAL_DISK_PARAMETERS Parameters, PHANDLE Handle)
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD ImportedFunctions::stub_DetachVirtualDisk(HANDLE VirtualDiskHandle, DETACH_VIRTUAL_DISK_FLAG Flags, ULONG ProviderSpecificFlags)
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

BOOL ImportedFunctions::stub_UnregisterPowerSettingNotification(HPOWERNOTIFY Handle)
{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
}

HPOWERNOTIFY ImportedFunctions::stub_RegisterPowerSettingNotification(HANDLE hRecipient, LPCGUID PowerSettingGuid, DWORD Flags)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return nullptr;
}

BOOL ImportedFunctions::stub_QueryFullProcessImageNameW(HANDLE Process, DWORD Flags, LPWSTR ExeName, PDWORD Size)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

DWORD ImportedFunctions::stub_RmStartSession(DWORD *SessionHandle, DWORD SessionFlags, WCHAR strSessionKey [])
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD ImportedFunctions::stub_RmEndSession(DWORD dwSessionHandle)
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD ImportedFunctions::stub_RmRegisterResources(DWORD dwSessionHandle, UINT nFiles, LPCWSTR rgsFilenames [], UINT nApplications, RM_UNIQUE_PROCESS rgApplications [], UINT nServices, LPCWSTR rgsServiceNames [])
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD ImportedFunctions::stub_RmGetList(DWORD dwSessionHandle, UINT *pnProcInfoNeeded, UINT *pnProcInfo, RM_PROCESS_INFO rgAffectedApps [], LPDWORD lpdwRebootReasons)
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}

NET_API_STATUS ImportedFunctions::stub_NetDfsGetInfo(LPWSTR path, LPWSTR reserved1, LPWSTR reserved2, DWORD level, LPBYTE *buff)
{
	return NERR_InvalidAPI;
}

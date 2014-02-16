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
	#define InitImport(Module, Name) pfn##Name = Module.GetProcAddress(#Name)

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


BOOL ImportedFunctions::GetConsoleKeyboardLayoutNameW(LPWSTR Buffer) const
{
	if(pfnGetConsoleKeyboardLayoutNameW)
	{
		return pfnGetConsoleKeyboardLayoutNameW(Buffer);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOLEAN ImportedFunctions::CreateSymbolicLinkW(LPCWSTR SymlinkFileName, LPCWSTR TargetFileName, DWORD Flags) const
{
	if(pfnCreateSymbolicLinkW)
	{
		return pfnCreateSymbolicLinkW(SymlinkFileName, TargetFileName, Flags);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

HANDLE ImportedFunctions::FindFirstFileNameW(LPCWSTR FileName, DWORD Flags, LPDWORD StringLength, LPWSTR LinkName) const
{
	if(pfnFindFirstFileNameW)
	{
		return pfnFindFirstFileNameW(FileName, Flags, StringLength, LinkName);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return INVALID_HANDLE_VALUE;
	}
}

BOOL ImportedFunctions::FindNextFileNameW(HANDLE FindStream, LPDWORD StringLength, PWCHAR LinkName) const
{
	if(pfnFindNextFileNameW)
	{
		return pfnFindNextFileNameW(FindStream, StringLength, LinkName);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

HANDLE ImportedFunctions::FindFirstStreamW(LPCWSTR FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID FindStreamData, DWORD Flags) const
{
	if(pfnFindFirstStreamW)
	{
		return pfnFindFirstStreamW(FileName, InfoLevel, FindStreamData, Flags);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return INVALID_HANDLE_VALUE;
	}
}

BOOL ImportedFunctions::FindNextStreamW(HANDLE FindStream, LPVOID FindStreamData) const
{
	if(pfnFindNextStreamW)
	{
		return pfnFindNextStreamW(FindStream, FindStreamData);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

DWORD ImportedFunctions::GetFinalPathNameByHandleW(HANDLE File, LPWSTR FilePath, DWORD FilePathSize, DWORD Flags) const
{
	if(pfnGetFinalPathNameByHandleW)
	{
		// It is known that GetFinalPathNameByHandle crashes on Windows 7 with Ext2FSD
		try
		{
			return pfnGetFinalPathNameByHandleW(File, FilePath, FilePathSize, Flags);
		}
		catch (SException& e)
		{
			if (e.GetCode() == EXCEPTION_ACCESS_VIOLATION)
				return 0;
			throw;
		}
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return 0;
	}
}

BOOL ImportedFunctions::GetVolumePathNamesForVolumeNameW(LPCWSTR VolumeName, LPWSTR VolumePathNames, DWORD BufferLength, PDWORD ReturnLength) const
{
	if(pfnGetVolumePathNamesForVolumeNameW)
	{
		return pfnGetVolumePathNamesForVolumeNameW(VolumeName, VolumePathNames, BufferLength, ReturnLength);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::GetPhysicallyInstalledSystemMemory(PULONGLONG TotalMemoryInKilobytes) const
{
	if(pfnGetPhysicallyInstalledSystemMemory)
	{
		return pfnGetPhysicallyInstalledSystemMemory(TotalMemoryInKilobytes);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::HeapSetInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength) const
{
	if(pfnHeapSetInformation)
	{
		return pfnHeapSetInformation(HeapHandle, HeapInformationClass, HeapInformation, HeapInformationLength);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::IsWow64Process(HANDLE Process, PBOOL Wow64Process) const
{
	if(pfnIsWow64Process)
	{
		return pfnIsWow64Process(Process, Wow64Process);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::GetNamedPipeServerProcessId(HANDLE Pipe, PULONG ServerProcessId) const
{
	if(pfnGetNamedPipeServerProcessId)
	{
		return pfnGetNamedPipeServerProcessId(Pipe, ServerProcessId);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::CancelSynchronousIo(HANDLE Thread) const
{
	if(pfnCancelSynchronousIo)
	{
		return pfnCancelSynchronousIo(Thread);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::SetConsoleKeyShortcuts(BOOL Set, BYTE ReserveKeys, LPVOID AppKeys, DWORD NumAppKeys) const
{
	if(pfnSetConsoleKeyShortcuts)
	{
		return pfnSetConsoleKeyShortcuts(Set, ReserveKeys, AppKeys, NumAppKeys);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::GetConsoleScreenBufferInfoEx(HANDLE ConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX ConsoleScreenBufferInfoEx) const
{
	if(pfnGetConsoleScreenBufferInfoEx)
	{
		return pfnGetConsoleScreenBufferInfoEx(ConsoleOutput, ConsoleScreenBufferInfoEx);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

BOOL ImportedFunctions::TzSpecificLocalTimeToSystemTime(const TIME_ZONE_INFORMATION* TimeZoneInformation, const SYSTEMTIME* LocalTime, LPSYSTEMTIME UniversalTime) const
{
	if(pfnTzSpecificLocalTimeToSystemTime)
	{
		return pfnTzSpecificLocalTimeToSystemTime(TimeZoneInformation, LocalTime, UniversalTime);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}


NTSTATUS ImportedFunctions::NtQueryDirectoryFile(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan) const
{
	if(pfnNtQueryDirectoryFile)
	{
		return pfnNtQueryDirectoryFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::NtQueryInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass) const
{
	if(pfnNtQueryInformationFile)
	{
		return pfnNtQueryInformationFile(FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::NtSetInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass) const
{
	if(pfnNtSetInformationFile)
	{
		return pfnNtSetInformationFile(FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::NtQueryObject(HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength) const
{
	if(pfnNtQueryObject)
	{
		return pfnNtQueryObject(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::NtOpenSymbolicLinkObject(PHANDLE LinkHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes) const
{
	if(pfnNtOpenSymbolicLinkObject)
	{
		return pfnNtOpenSymbolicLinkObject(LinkHandle, DesiredAccess, ObjectAttributes);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::NtQuerySymbolicLinkObject(HANDLE LinkHandle, PUNICODE_STRING LinkTarget, PULONG ReturnedLength) const
{
	if(pfnNtQuerySymbolicLinkObject)
	{
		return pfnNtQuerySymbolicLinkObject(LinkHandle, LinkTarget, ReturnedLength);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::NtClose(HANDLE Handle) const
{
	if(pfnNtClose)
	{
		return pfnNtClose(Handle);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::RtlGetLastNtStatus() const
{
	if(pfnRtlGetLastNtStatus)
	{
		return pfnRtlGetLastNtStatus();
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}

NTSTATUS ImportedFunctions::RtlNtStatusToDosError(NTSTATUS Status) const
{
	if(pfnRtlNtStatusToDosError)
	{
		return pfnRtlNtStatusToDosError(Status);
	}
	else
	{
		return STATUS_NOT_IMPLEMENTED;
	}
}


HRESULT ImportedFunctions::SHCreateAssociationRegistration(REFIID riid, void ** ppv) const
{
	if(pfnSHCreateAssociationRegistration)
	{
		return pfnSHCreateAssociationRegistration(riid, ppv);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}


DWORD ImportedFunctions::GetStorageDependencyInformation(HANDLE ObjectHandle, GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed) const
{
	if(pfnGetStorageDependencyInformation)
	{
		return pfnGetStorageDependencyInformation(ObjectHandle, Flags, StorageDependencyInfoSize, StorageDependencyInfo, SizeUsed);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

DWORD ImportedFunctions::OpenVirtualDisk(PVIRTUAL_STORAGE_TYPE VirtualStorageType, PCWSTR Path, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, POPEN_VIRTUAL_DISK_PARAMETERS Parameters, PHANDLE Handle) const
{
	if(pfnOpenVirtualDisk)
	{
		return pfnOpenVirtualDisk(VirtualStorageType, Path, VirtualDiskAccessMask, Flags, Parameters, Handle);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

DWORD ImportedFunctions::DetachVirtualDisk(HANDLE VirtualDiskHandle, DETACH_VIRTUAL_DISK_FLAG Flags, ULONG ProviderSpecificFlags) const
{
	if(pfnDetachVirtualDisk)
	{
		return pfnDetachVirtualDisk(VirtualDiskHandle, Flags, ProviderSpecificFlags);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

BOOL ImportedFunctions::UnregisterPowerSettingNotification(HPOWERNOTIFY Handle) const
{
	if(pfnUnregisterPowerSettingNotification)
	{
		return pfnUnregisterPowerSettingNotification(Handle);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

HPOWERNOTIFY ImportedFunctions::RegisterPowerSettingNotification(HANDLE hRecipient,LPCGUID PowerSettingGuid,DWORD Flags) const
{
	if(pfnRegisterPowerSettingNotification)
	{
		return pfnRegisterPowerSettingNotification(hRecipient,PowerSettingGuid,Flags);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return nullptr;
	}
}

BOOL ImportedFunctions::QueryFullProcessImageNameW(HANDLE Process, DWORD Flags, LPWSTR ExeName, PDWORD Size) const
{
	if(pfnQueryFullProcessImageNameW)
	{
		return pfnQueryFullProcessImageNameW(Process, Flags, ExeName, Size);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
}

DWORD ImportedFunctions::RmStartSession(DWORD *SessionHandle, DWORD SessionFlags, WCHAR strSessionKey[]) const
{
	if(pfnRmStartSession)
	{
		return pfnRmStartSession(SessionHandle, SessionFlags, strSessionKey);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

DWORD ImportedFunctions::RmEndSession(DWORD dwSessionHandle) const
{
	if(pfnRmEndSession)
	{
		return pfnRmEndSession(dwSessionHandle);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

DWORD ImportedFunctions::RmRegisterResources(DWORD dwSessionHandle, UINT nFiles, LPCWSTR rgsFilenames[], UINT nApplications, RM_UNIQUE_PROCESS rgApplications[], UINT nServices, LPCWSTR rgsServiceNames[]) const
{
	if(pfnRmRegisterResources)
	{
		return pfnRmRegisterResources(dwSessionHandle, nFiles, rgsFilenames, nApplications, rgApplications, nServices, rgsServiceNames);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

DWORD ImportedFunctions::RmGetList(DWORD dwSessionHandle, UINT *pnProcInfoNeeded, UINT *pnProcInfo, RM_PROCESS_INFO rgAffectedApps[], LPDWORD lpdwRebootReasons) const
{
	if(pfnRmGetList)
	{
		return pfnRmGetList(dwSessionHandle, pnProcInfoNeeded, pnProcInfo, rgAffectedApps, lpdwRebootReasons);
	}
	else
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

NET_API_STATUS ImportedFunctions::NetDfsGetInfo(LPWSTR path, LPWSTR reserved1, LPWSTR reserved2, DWORD level, LPBYTE *buff) const
{
	if (pfnNetDfsGetInfo)
	{
		return pfnNetDfsGetInfo(path, reserved1, reserved2, level, buff);
	}
	else
	{
		return NERR_InvalidAPI;
	}
}

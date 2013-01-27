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

template<typename T>
inline void InitImport(HMODULE Module, T& Address, const char * ProcName)
{
	Address = reinterpret_cast<T>(GetProcAddress(Module, ProcName));
}

ImportedFunctions::ImportedFunctions()
{
	ClearStructUnsafe(*this);
	HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
	HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
	HMODULE hShell = GetModuleHandle(L"shell32.dll");
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	hVirtDisk = LoadLibrary(L"virtdisk.dll");
	hRstrtMgr = LoadLibrary(L"rstrtmgr.dll");

	#define InitImport(Module, Name) InitImport(Module, pfn##Name, #Name)

	if (hKernel)
	{
		InitImport(hKernel, GetConsoleKeyboardLayoutNameW);
		InitImport(hKernel, CreateSymbolicLinkW);
		InitImport(hKernel, FindFirstFileNameW);
		InitImport(hKernel, FindNextFileNameW);
		InitImport(hKernel, FindFirstStreamW);
		InitImport(hKernel, FindNextStreamW);
		InitImport(hKernel, GetFinalPathNameByHandleW);
		InitImport(hKernel, GetVolumePathNamesForVolumeNameW);
		InitImport(hKernel, GetPhysicallyInstalledSystemMemory);
		InitImport(hKernel, HeapSetInformation);
		InitImport(hKernel, IsWow64Process);
		InitImport(hKernel, GetNamedPipeServerProcessId);
		InitImport(hKernel, CancelSynchronousIo);
		InitImport(hKernel, SetConsoleKeyShortcuts);
		InitImport(hKernel, GetConsoleScreenBufferInfoEx);
		InitImport(hKernel, QueryFullProcessImageNameW);
		InitImport(hKernel, TzSpecificLocalTimeToSystemTime);
	}

	if (hNtdll)
	{
		InitImport(hNtdll, NtQueryDirectoryFile);
		InitImport(hNtdll, NtQueryInformationFile);
		InitImport(hNtdll, NtSetInformationFile);
		InitImport(hNtdll, NtQueryObject);
		InitImport(hNtdll, NtOpenSymbolicLinkObject);
		InitImport(hNtdll, NtQuerySymbolicLinkObject);
		InitImport(hNtdll, NtClose);
		InitImport(hNtdll, RtlGetLastNtStatus);
		InitImport(hNtdll, RtlNtStatusToDosError);
	}

	if (hShell)
	{
		InitImport(hShell, SHCreateAssociationRegistration);
	}

	if (hVirtDisk)
	{
		InitImport(hVirtDisk, GetStorageDependencyInformation);
		InitImport(hVirtDisk, OpenVirtualDisk);
		InitImport(hVirtDisk, DetachVirtualDisk);
	}

	if (hUser32)
	{
		InitImport(hUser32, RegisterPowerSettingNotification);
		InitImport(hUser32, UnregisterPowerSettingNotification);
	}

	if (hRstrtMgr)
	{
		InitImport(hRstrtMgr, RmStartSession);
		InitImport(hRstrtMgr, RmEndSession);
		InitImport(hRstrtMgr, RmRegisterResources);
		InitImport(hRstrtMgr, RmGetList);
	}

	#undef InitImport

}


ImportedFunctions::~ImportedFunctions()
{
	if(hRstrtMgr)
	{
		FreeLibrary(hRstrtMgr);
	}

	if(hVirtDisk)
	{
		FreeLibrary(hVirtDisk);
	}
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
		SEH_TRY
		{
			return pfnGetFinalPathNameByHandleW(File, FilePath, FilePathSize, Flags);
		}
		SEH_EXCEPT(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return 0;
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
		return NULL;
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

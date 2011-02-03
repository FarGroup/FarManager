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

#include "headers.hpp"
#pragma hdrstop

// kernel32

typedef BOOL (WINAPI *PGETCONSOLEKEYBOARDLAYOUTNAME)(
	wchar_t*
);

typedef BOOLEAN (WINAPI *PCREATESYMBOLICLINK)(
	const wchar_t *lpSymlinkFileName,
	const wchar_t *lpTargetFileName,
	DWORD dwFlags);

typedef HANDLE(WINAPI *FINDFIRSTFILENAMEW)(
	LPCWSTR lpFileName,
	DWORD dwFlags,
	LPDWORD StringLength,
	LPWSTR LinkName
);

typedef BOOL(WINAPI *FINDNEXTFILENAMEW)(
	HANDLE hFindStream,
	LPDWORD StringLength,
	PWCHAR LinkName
);

typedef HANDLE(WINAPI *FINDFIRSTSTREAMW)(
	LPCWSTR lpFileName,
	STREAM_INFO_LEVELS InfoLevel,
	LPVOID lpFindStreamData,
	DWORD dwFlags
);

typedef BOOL(WINAPI * FINDNEXTSTREAMW)(
	HANDLE hFindStream,
	LPVOID lpFindStreamData
);

typedef DWORD (WINAPI *GETFINALPATHNAMEBYHANDLE)(
	HANDLE hFile,
	LPTSTR lpszFilePath,
	DWORD cchFilePath,
	DWORD dwFlags
);

typedef BOOL (WINAPI *GETVOLUMEPATHNAMESFORVOLUMENAME)(
	LPCTSTR lpszVolumeName,
	LPTSTR lpszVolumePathNames,
	DWORD cchBufferLength,
	PDWORD lpcchReturnLength
);

typedef BOOL (WINAPI* GETPHYSICALLYINSTALLEDSYSTEMMEMORY)(
	PULONGLONG TotalMemoryInKilobytes
);

typedef BOOL (WINAPI *HEAPSETINFORMATION)(
	HANDLE HeapHandle,
	HEAP_INFORMATION_CLASS HeapInformationClass,
	PVOID HeapInformation,
	SIZE_T HeapInformationLength
);

typedef BOOL (WINAPI *ISWOW64PROCESS)(
	HANDLE hProcess,
	PBOOL Wow64Process
);

typedef BOOL (WINAPI* GETNAMEDPIPESERVERPROCESSID)(
	HANDLE Pipe,
	PULONG ServerProcessId
);

// ntdll

typedef NTSTATUS (WINAPI *NTQUERYDIRECTORYFILE)(
	HANDLE FileHandle,
	HANDLE Event,
	PVOID ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN ReturnSingleEntry,
	PUNICODE_STRING FileName,
	BOOLEAN RestartScan
);

typedef NTSTATUS(WINAPI *NTQUERYINFORMATIONFILE)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass
);

typedef NTSTATUS(WINAPI *NTSETINFORMATIONFILE)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass
);

typedef NTSTATUS(NTAPI *NTQUERYOBJECT)(
	HANDLE Handle,
	OBJECT_INFORMATION_CLASS ObjectInformationClass,
	PVOID ObjectInformation,
	ULONG ObjectInformationLength,
	PULONG ReturnLength
);

typedef NTSTATUS(NTAPI *NTOPENSYMBOLICLINKOBJECT)(
	PHANDLE LinkHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);

typedef NTSTATUS(NTAPI *NTQUERYSYMBOLICLINKOBJECT)(
	HANDLE LinkHandle,
	PUNICODE_STRING LinkTarget,
	PULONG ReturnedLength
);

typedef NTSTATUS(NTAPI *NTCLOSE)(
	HANDLE Handle
);

typedef NTSTATUS(NTAPI *RTLGETLASTNTSTATUS)(
);

typedef ULONG (NTAPI *RTLNTSTATUSTODOSERROR)(
NTSTATUS Status
);

// shell32

typedef HRESULT(WINAPI *PSHCREATEASSOCIATIONREGISTRATION)(
	REFIID riid,
	void ** ppv
);

// virtdisk

typedef DWORD (WINAPI *GETSTORAGEDEPENDENCYINFORMATION)(
	HANDLE ObjectHandle,
	GET_STORAGE_DEPENDENCY_FLAG Flags,
	ULONG StorageDependencyInfoSize,
	PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo,
	PULONG SizeUsed
);

typedef DWORD (WINAPI *OPENVIRTUALDISK)(
	PVIRTUAL_STORAGE_TYPE VirtualStorageType,
	PCWSTR Path,
	VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask,
	OPEN_VIRTUAL_DISK_FLAG Flags,
	POPEN_VIRTUAL_DISK_PARAMETERS Parameters,
	PHANDLE Handle
);

typedef DWORD (WINAPI *DETACHVIRTUALDISK)(
	HANDLE VirtualDiskHandle,
	DETACH_VIRTUAL_DISK_FLAG Flags,
	ULONG ProviderSpecificFlags
);


class ImportedFunctions
{
public:
	PGETCONSOLEKEYBOARDLAYOUTNAME pfnGetConsoleKeyboardLayoutName;
	PCREATESYMBOLICLINK pfnCreateSymbolicLink;
	FINDFIRSTFILENAMEW pfnFindFirstFileNameW;
	FINDNEXTFILENAMEW pfnFindNextFileNameW;
	FINDFIRSTSTREAMW pfnFindFirstStreamW;
	FINDNEXTSTREAMW pfnFindNextStreamW;
	GETFINALPATHNAMEBYHANDLE pfnGetFinalPathNameByHandle;
	GETVOLUMEPATHNAMESFORVOLUMENAME pfnGetVolumePathNamesForVolumeName;
	GETPHYSICALLYINSTALLEDSYSTEMMEMORY pfnGetPhysicallyInstalledSystemMemory;
	HEAPSETINFORMATION pfnHeapSetInformation;
	ISWOW64PROCESS pfnIsWow64Process;
	GETNAMEDPIPESERVERPROCESSID pGetNamedPipeServerProcessId;

	NTQUERYDIRECTORYFILE pfnNtQueryDirectoryFile;
	NTQUERYINFORMATIONFILE pfnNtQueryInformationFile;
	NTSETINFORMATIONFILE pfnNtSetInformationFile;
	NTQUERYOBJECT pfnNtQueryObject;
	NTOPENSYMBOLICLINKOBJECT pfnNtOpenSymbolicLinkObject;
	NTQUERYSYMBOLICLINKOBJECT pfnNtQuerySymbolicLinkObject;
	NTCLOSE pfnNtClose;
	RTLGETLASTNTSTATUS pfnRtlGetLastNtStatus;
	RTLNTSTATUSTODOSERROR pfnRtlNtStatusToDosError;

	PSHCREATEASSOCIATIONREGISTRATION pfnSHCreateAssociationRegistration;

	GETSTORAGEDEPENDENCYINFORMATION pfnGetStorageDependencyInformation;
	OPENVIRTUALDISK pfnOpenVirtualDisk;
	DETACHVIRTUALDISK pfnDetachVirtualDisk;

	ImportedFunctions();
	~ImportedFunctions();

private:
	HMODULE hVirtDisk;
};

extern ImportedFunctions ifn;

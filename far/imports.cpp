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

ImportedFunctions ifn;

ImportedFunctions::ImportedFunctions()
{
	memset(this,0,sizeof(*this));
	HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
	HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
	HMODULE hShell = GetModuleHandle(L"shell32.dll");
	hVirtDisk = LoadLibrary(L"virtdisk.dll");

	if (hKernel)
	{
		pfnGetConsoleKeyboardLayoutName = reinterpret_cast<PGETCONSOLEKEYBOARDLAYOUTNAME>(GetProcAddress(hKernel, "GetConsoleKeyboardLayoutNameW"));
		pfnCreateSymbolicLink = reinterpret_cast<PCREATESYMBOLICLINK>(GetProcAddress(hKernel, "CreateSymbolicLinkW"));
		pfnFindFirstFileNameW = reinterpret_cast<FINDFIRSTFILENAMEW>(GetProcAddress(hKernel, "FindFirstFileNameW"));
		pfnFindNextFileNameW = reinterpret_cast<FINDNEXTFILENAMEW>(GetProcAddress(hKernel, "FindNextFileNameW"));
		pfnFindFirstStreamW = reinterpret_cast<FINDFIRSTSTREAMW>(GetProcAddress(hKernel, "FindFirstStreamW"));
		pfnFindNextStreamW = reinterpret_cast<FINDNEXTSTREAMW>(GetProcAddress(hKernel, "FindNextStreamW"));
		pfnGetFinalPathNameByHandle = reinterpret_cast<GETFINALPATHNAMEBYHANDLE>(GetProcAddress(hKernel, "GetFinalPathNameByHandleW"));
		pfnGetVolumePathNamesForVolumeName = reinterpret_cast<GETVOLUMEPATHNAMESFORVOLUMENAME>(GetProcAddress(hKernel, "GetVolumePathNamesForVolumeNameW"));
		pfnGetPhysicallyInstalledSystemMemory = reinterpret_cast<GETPHYSICALLYINSTALLEDSYSTEMMEMORY>(GetProcAddress(hKernel, "GetPhysicallyInstalledSystemMemory"));
		pfnHeapSetInformation = reinterpret_cast<HEAPSETINFORMATION>(GetProcAddress(hKernel, "HeapSetInformation"));
		pfnIsWow64Process = reinterpret_cast<ISWOW64PROCESS>(GetProcAddress(hKernel, "IsWow64Process"));
		pGetNamedPipeServerProcessId = reinterpret_cast<GETNAMEDPIPESERVERPROCESSID>(GetProcAddress(hKernel, "GetNamedPipeServerProcessId"));
	}

	if (hNtdll)
	{
		pfnNtQueryDirectoryFile = reinterpret_cast<NTQUERYDIRECTORYFILE>(GetProcAddress(hNtdll, "NtQueryDirectoryFile"));
		pfnNtQueryInformationFile = reinterpret_cast<NTQUERYINFORMATIONFILE>(GetProcAddress(hNtdll, "NtQueryInformationFile"));
		pfnNtSetInformationFile = reinterpret_cast<NTSETINFORMATIONFILE>(GetProcAddress(hNtdll, "NtSetInformationFile"));
		pfnNtQueryObject = reinterpret_cast<NTQUERYOBJECT>(GetProcAddress(hNtdll, "NtQueryObject"));
		pfnNtOpenSymbolicLinkObject = reinterpret_cast<NTOPENSYMBOLICLINKOBJECT>(GetProcAddress(hNtdll, "NtOpenSymbolicLinkObject"));
		pfnNtQuerySymbolicLinkObject = reinterpret_cast<NTQUERYSYMBOLICLINKOBJECT>(GetProcAddress(hNtdll, "NtQuerySymbolicLinkObject"));
		pfnNtClose = reinterpret_cast<NTCLOSE>(GetProcAddress(hNtdll, "NtClose"));
		pfnRtlGetLastNtStatus = reinterpret_cast<RTLGETLASTNTSTATUS>(GetProcAddress(hNtdll, "RtlGetLastNtStatus"));
		pfnRtlNtStatusToDosError = reinterpret_cast<RTLNTSTATUSTODOSERROR>(GetProcAddress(hNtdll, "RtlNtStatusToDosError"));
	}

	if (hShell)
	{
		pfnSHCreateAssociationRegistration = reinterpret_cast<PSHCREATEASSOCIATIONREGISTRATION>(GetProcAddress(hShell, "SHCreateAssociationRegistration"));
	}

	if(hVirtDisk)
	{
		pfnGetStorageDependencyInformation = reinterpret_cast<GETSTORAGEDEPENDENCYINFORMATION>(GetProcAddress(hVirtDisk, "GetStorageDependencyInformation"));
		pfnOpenVirtualDisk = reinterpret_cast<OPENVIRTUALDISK>(GetProcAddress(hVirtDisk, "OpenVirtualDisk"));
		pfnDetachVirtualDisk = reinterpret_cast<DETACHVIRTUALDISK>(GetProcAddress(hVirtDisk, "DetachVirtualDisk"));
	}
}

ImportedFunctions::~ImportedFunctions()
{
	if(hVirtDisk)
	{
		FreeLibrary(hVirtDisk);
	}
}

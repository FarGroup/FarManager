/*
imports.cpp

импортируемые функции
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

void ImportedFunctions::Load()
{
	memset(this,0,sizeof(*this));
	HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
	HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
	HMODULE hShell = GetModuleHandle(L"shell32.dll");
	HMODULE hSetupAPI = LoadLibrary(L"setupapi.dll");

	if (hSetupAPI)
	{
		pfnGetDevNodeRegistryProperty = (PCMGETDEVNODEREGISTRYPROPERTY)GetProcAddress(
		                                    hSetupAPI,
		                                    "CM_Get_DevNode_Registry_PropertyW"
		                                );
		pfnGetDevNodeStatus = (PCMGETDEVNODESTATUS)GetProcAddress(
		                          hSetupAPI,
		                          "CM_Get_DevNode_Status"
		                      );
		pfnGetDeviceID = (PCMGETDEVICEID)GetProcAddress(
		                     hSetupAPI,
		                     "CM_Get_Device_IDW"
		                 );
		pfnGetDeviceIDListSize = (PCMGETDEVICEIDLISTSIZE)GetProcAddress(
		                             hSetupAPI,
		                             "CM_Get_Device_ID_List_SizeW"
		                         );
		pfnGetDeviceIDList = (PCMGETDEVICEIDLIST)GetProcAddress(
		                         hSetupAPI,
		                         "CM_Get_Device_ID_ListW"
		                     );
		pfnGetDeviceInterfaceListSize = (PCMGETDEVICEINTERFACELISTSIZE)GetProcAddress(
		                                    hSetupAPI,
		                                    "CM_Get_Device_Interface_List_SizeW"
		                                );
		pfnGetDeviceInterfaceList = (PCMGETDEVICEINTERFACELIST)GetProcAddress(
		                                hSetupAPI,
		                                "CM_Get_Device_Interface_ListW"
		                            );
		pfnLocateDevNode = (PCMLOCATEDEVNODE)GetProcAddress(
		                       hSetupAPI,
		                       "CM_Locate_DevNodeW"
		                   );
		pfnGetChild = (PCMGETCHILD)GetProcAddress(
		                  hSetupAPI,
		                  "CM_Get_Child"
		              );
		pfnGetSibling  = (PCMGETCHILD)GetProcAddress(
		                     hSetupAPI,
		                     "CM_Get_Sibling"
		                 );
		pfnRequestDeviceEject = (PCMREQUESTDEVICEEJECT)GetProcAddress(
		                            hSetupAPI,
		                            "CM_Request_Device_EjectW"
		                        );
	}

	bSetupAPIFunctions = (
	                         pfnGetDevNodeRegistryProperty &&
	                         pfnGetDevNodeStatus &&
	                         pfnGetDeviceID &&
	                         pfnGetDeviceIDListSize &&
	                         pfnGetDeviceIDList &&
	                         pfnGetDeviceInterfaceListSize &&
	                         pfnGetDeviceInterfaceList &&
	                         pfnLocateDevNode &&
	                         pfnGetChild &&
	                         pfnGetSibling &&
	                         pfnRequestDeviceEject
	                     );

	if (hKernel)
	{
		pfnGetConsoleKeyboardLayoutName = (PGETCONSOLEKEYBOARDLAYOUTNAME)GetProcAddress(hKernel, "GetConsoleKeyboardLayoutNameW");
		pfnCreateSymbolicLink = (PCREATESYMBOLICLINK)GetProcAddress(hKernel, "CreateSymbolicLinkW");
		pfnFindFirstFileNameW = (FINDFIRSTFILENAMEW)GetProcAddress(hKernel, "FindFirstFileNameW");
		pfnFindNextFileNameW = (FINDNEXTFILENAMEW)GetProcAddress(hKernel, "FindNextFileNameW");
		pfnFindFirstStreamW = (FINDFIRSTSTREAMW)GetProcAddress(hKernel, "FindFirstStreamW");
		pfnFindNextStreamW = (FINDNEXTSTREAMW)GetProcAddress(hKernel, "FindNextStreamW");
		pfnGetFinalPathNameByHandle = (GETFINALPATHNAMEBYHANDLE)GetProcAddress(hKernel, "GetFinalPathNameByHandleW");
		pfnGetVolumePathNamesForVolumeName = (GETVOLUMEPATHNAMESFORVOLUMENAME)GetProcAddress(hKernel, "GetVolumePathNamesForVolumeNameW");
		pfnGetPhysicallyInstalledSystemMemory = (GETPHYSICALLYINSTALLEDSYSTEMMEMORY)GetProcAddress(hKernel, "GetPhysicallyInstalledSystemMemory");
		pfnHeapSetInformation = (HEAPSETINFORMATION)GetProcAddress(hKernel, "HeapSetInformation");
	}

	if (hNtdll)
	{
		pfnNtQueryInformationFile = (NTQUERYINFORMATIONFILE)GetProcAddress(hNtdll, "NtQueryInformationFile");
		pfnNtQueryObject = (NTQUERYOBJECT)GetProcAddress(hNtdll, "NtQueryObject");
		pfnNtOpenSymbolicLinkObject = (NTOPENSYMBOLICLINKOBJECT)GetProcAddress(hNtdll, "NtOpenSymbolicLinkObject");
		pfnNtQuerySymbolicLinkObject = (NTQUERYSYMBOLICLINKOBJECT)GetProcAddress(hNtdll, "NtQuerySymbolicLinkObject");
		pfnNtClose = (NTCLOSE)GetProcAddress(hNtdll, "NtClose");
		pfnRtlGetLastNtStatus = (RTLGETLASTNTSTATUS)GetProcAddress(hNtdll, "RtlGetLastNtStatus");
	}

	if (hShell)
	{
		pfnSHCreateAssociationRegistration = (PSHCREATEASSOCIATIONREGISTRATION)GetProcAddress(hShell, "SHCreateAssociationRegistration");
	}
}

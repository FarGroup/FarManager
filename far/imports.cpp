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

	HMODULE hKernel = LoadLibraryW (L"kernel32.dll");
	HMODULE hAdvapi = LoadLibraryW (L"advapi32.dll");
	HMODULE hSetupAPI = LoadLibraryW (L"setupapi.dll");
	HMODULE hWinMM = LoadLibraryW (L"winmm.dll"); //useless in 2.0

	if (hKernel)
	{
		pfnIsDebuggerPresent = (PISDEBUGGERPRESENT)GetProcAddress (hKernel, "IsDebuggerPresent");

		pfnCopyFileEx = (PCOPYFILEEX)GetProcAddress (hKernel, "CopyFileExW");
		pfnGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX)GetProcAddress (hKernel, "GetDiskFreeSpaceExW");
		pfnSetFilePointerEx = (PSETFILEPOINTEREX)GetProcAddress(hKernel, "SetFilePointerEx");

		pfnEncryptFile = (PENCRYPTFILE)GetProcAddress (hKernel, "EncryptFileW");

		pfnDecryptFile = (PDECRYPTFILE)GetProcAddress(hKernel, "DecryptFileW");
	}

	if (hAdvapi)
	{
		if ( !pfnEncryptFile )
			pfnEncryptFile = (PENCRYPTFILE)GetProcAddress(hAdvapi, "EncryptFileW");

		if ( !pfnDecryptFile )
			pfnDecryptFile = (PDECRYPTFILE)GetProcAddress(hAdvapi, "DecryptFileW");
	}

	bEncryptFunctions = (pfnEncryptFile && pfnDecryptFile);

//
	if (hKernel)
	{
		pfnGetVolumeNameForVolumeMountPoint = (PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress (
				hKernel,
				"GetVolumeNameForVolumeMountPointW"
				);

		pfnSetVolumeMountPoint = (PSETVOLUMEMOUNTPOINT)GetProcAddress (
				hKernel,
				"SetVolumeMountPointW"
				);
	}

	bVolumeMountPointFunctions = (pfnGetVolumeNameForVolumeMountPoint && pfnSetVolumeMountPoint);

//
	if (hSetupAPI)
	{
		pfnGetDevNodeRegistryProperty = (PCMGETDEVNODEREGISTRYPROPERTY)GetProcAddress (
				hSetupAPI,
				"CM_Get_DevNode_Registry_PropertyW"
				);

		pfnGetDevNodeStatus = (PCMGETDEVNODESTATUS)GetProcAddress (
				hSetupAPI,
				"CM_Get_DevNode_Status"
				);

		pfnGetDeviceID = (PCMGETDEVICEID)GetProcAddress (
				hSetupAPI,
				"CM_Get_Device_IDW"
				);

		pfnGetDeviceIDListSize = (PCMGETDEVICEIDLISTSIZE)GetProcAddress (
				hSetupAPI,
				"CM_Get_Device_ID_List_SizeW"
				);

		pfnGetDeviceIDList = (PCMGETDEVICEIDLIST)GetProcAddress (
				hSetupAPI,
				"CM_Get_Device_ID_ListW"
				);

		pfnGetDeviceInterfaceListSize = (PCMGETDEVICEINTERFACELISTSIZE)GetProcAddress (
				hSetupAPI,
				"CM_Get_Device_Interface_List_SizeW"
				);

		pfnGetDeviceInterfaceList = (PCMGETDEVICEINTERFACELIST)GetProcAddress (
				hSetupAPI,
				"CM_Get_Device_Interface_ListW"
				);

		pfnLocateDevNode = (PCMLOCATEDEVNODE)GetProcAddress (
				hSetupAPI,
				"CM_Locate_DevNodeW"
				);

		pfnGetChild = (PCMGETCHILD)GetProcAddress (
				hSetupAPI,
				"CM_Get_Child"
				);

		pfnGetSibling  = (PCMGETCHILD)GetProcAddress (
				hSetupAPI,
				"CM_Get_Sibling"
				);

		pfnRequestDeviceEject = (PCMREQUESTDEVICEEJECT)GetProcAddress (
				hSetupAPI,
				"CM_Request_Device_EjectW"
				);
	}

	bSetupAPIFunctions = (
			pfnGetVolumeNameForVolumeMountPoint &&
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

//
	if (hKernel)
	{
		pfnGetConsoleWindow = (PGETCONSOLEWINDOW)GetProcAddress (hKernel, "GetConsoleWindow");
		pfnGetConsoleKeyboardLayoutName = (PGETCONSOLEKEYBOARDLAYOUTNAME)GetProcAddress (hKernel, "GetConsoleKeyboardLayoutNameW");
		pfnSetConsoleDisplayMode = (PSETCONSOLEDISPLAYMODE)GetProcAddress (hKernel, "SetConsoleDisplayMode");
		pfnGetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress (hKernel, "GetConsoleAliasW");

		pfnGlobalMemoryStatusEx = (PGLOBALMEMORYSTATUSEX)GetProcAddress (hKernel, "GlobalMemoryStatusEx");
		pfnCreateHardLink = (PCREATEHARDLINK)GetProcAddress(hKernel, "CreateHardLinkW");
		pfnCreateSymbolicLink = (PCREATESYMBOLICLINK)GetProcAddress(hKernel, "CreateSymbolicLinkW");
	}

	if (hWinMM)
	{
		pfnmciSendString = (PMCISENDSTRING)GetProcAddress(hWinMM, "mciSendStringA");
	}
}

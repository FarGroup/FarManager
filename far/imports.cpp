#include "headers.hpp"
#pragma hdrstop

#include "imports.hpp"

ImportedFunctions ifn;

void ImportedFunctions::Load()
{
    HMODULE hKernel = LoadLibraryW (L"kernel32.dll");
    HMODULE hAdvapi = LoadLibraryW (L"advapi32.dll");
    HMODULE hSetupAPI = LoadLibraryW (L"setupapi.dll");
    HMODULE hWinMM = LoadLibraryW (L"winmm.dll"); //useless in 1.8
	
	pfnIsDebuggerPresent = (PISDEBUGGERPRESENT)GetProcAddress (hKernel, "IsDebuggerPresent");

	pfnCopyFileEx = (PCOPYFILEEX)GetProcAddress (hKernel, "CopyFileExW");
	pfnGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX)GetProcAddress (hKernel, "GetDiskFreeSpaceExW");
    pfnSetFilePointerEx = (PSETFILEPOINTEREX)GetProcAddress(hKernel, "SetFilePointerEx");

    pfnEncryptFile = (PENCRYPTFILE)GetProcAddress (hKernel, "EncryptFileW");

    if ( !pfnEncryptFile )
		pfnEncryptFile = (PENCRYPTFILE)GetProcAddress(hAdvapi, "EncryptFileW");

	pfnDecryptFile = (PDECRYPTFILE)GetProcAddress(hKernel, "DecryptFileW");

	if ( !pfnDecryptFile )
		pfnDecryptFile = (PDECRYPTFILE)GetProcAddress(hAdvapi, "DecryptFileW");


	bEncryptFunctions = (pfnEncryptFile && pfnDecryptFile);

//
	pfnGetVolumeNameForVolumeMountPoint = (PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress (
			hKernel,
			"GetVolumeNameForVolumeMountPointW"
			);

	pfnSetVolumeMountPoint = (PSETVOLUMEMOUNTPOINT)GetProcAddress (
			hKernel,
			"SetVolumeMountPointW"
			);

	bVolumeMountPointFunctions = (pfnGetVolumeNameForVolumeMountPoint && pfnSetVolumeMountPoint);

//
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

	pfnGetConsoleWindow = (PGETCONSOLEWINDOW)GetProcAddress (hKernel, "GetConsoleWindow");
	pfnGetConsoleKeyboardLayoutName = (PGETCONSOLEKEYBOARDLAYOUTNAME)GetProcAddress (hKernel, "GetConsoleKeyboardLayoutNameW");
	pfnSetConsoleDisplayMode = (PSETCONSOLEDISPLAYMODE)GetProcAddress (hKernel, "SetConsoleDisplayMode");
    pfnGetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress (hKernel, "GetConsoleAliasW");

	pfnGlobalMemoryStatusEx = (PGLOBALMEMORYSTATUSEX)GetProcAddress (hKernel, "GlobalMemoryStatusEx");
	pfnCreateHardLink = (PCREATEHARDLINK)GetProcAddress(hKernel, "CreateHardLinkW");

	pfnmciSendString = (PMCISENDSTRING)GetProcAddress(hWinMM, "mciSendStringA");
}

void UnloadImports()
{
}

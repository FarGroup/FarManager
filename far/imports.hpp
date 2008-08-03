#ifndef __IMPORTS_HPP__
#define __IMPORTS_HPP__


#include "headers.hpp"
#pragma hdrstop

#if defined(__GNUC__)  || (defined(_MSC_VER) && _MSC_VER <= 1200)

#define __NTDDK_H
#if defined(__GNUC__)
#include <ddk/cfgmgr32.h>
#else
#include <cfgmgr32.h>
#endif
#ifdef __cplusplus
  #define MY_EXTERN_C extern "C"
#else
  #define MY_EXTERN_C extern
#endif
#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  MY_EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define VolumeClassGuid             GUID_DEVINTERFACE_VOLUME
MY_DEFINE_GUID(GUID_DEVINTERFACE_VOLUME, 0x53f5630dL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#define CM_DRP_FRIENDLYNAME         (0x0000000D)
#define CM_DRP_DEVICEDESC           (0x00000001)
#define CM_DRP_CAPABILITIES         (0x00000010)
#define CM_DEVCAP_REMOVABLE         (0x00000004)
#define CM_DEVCAP_SURPRISEREMOVALOK (0x00000080)
#define CM_DEVCAP_DOCKDEVICE        (0x00000008)

#else
#include <cfgmgr32.h>
#endif

#include <setupapi.h>


typedef BOOL (__stdcall *PISDEBUGGERPRESENT)();

typedef BOOL (__stdcall *PCOPYFILEEX) (
		const wchar_t *lpwszExistingFileName,
		const wchar_t *lpwszNewFileName,
		void *lpProgressRoutine,
		LPVOID lpData,
		LPBOOL pbCancel,
		DWORD dwCopyFlags
		);

typedef BOOL (WINAPI *PENCRYPTFILE) (const wchar_t *lpwszFileName);
typedef BOOL (WINAPI *PDECRYPTFILE) (const wchar_t *lpwszFileName, DWORD dwReserved);


typedef BOOL (WINAPI *PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)(
          const wchar_t *lpwszVolumeMountPoint, // volume mount point or directory
          wchar_t *lpwszVolumeName,        // volume name buffer
          DWORD cchBufferLength
          );       // size of volume name buffer

typedef BOOL (WINAPI *PSETVOLUMEMOUNTPOINT)(
          const wchar_t *lpwszVolumeMountPoint, // mount point
          const wchar_t *lpwszVolumeName
          );        // volume to be mounted


typedef DWORD (__stdcall *PCMGETDEVNODEREGISTRYPROPERTY) (
		DEVINST dnDevInst,
		ULONG ulProperty,
		PULONG pulRegDataType,
		PVOID Buffer,
		PULONG pulLength,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETDEVNODESTATUS) (
		PULONG pulStatus,
		PULONG pulProblemNumber,
		DEVINST dnDevInst,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETDEVICEID) (
		DEVINST dnDevInst,
		wchar_t *Buffer,
		ULONG BufferLen,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETDEVICEIDLISTSIZE) (
		PULONG pulLen,
		const wchar_t *pszFilter,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETDEVICEIDLIST) (
		const wchar_t *pszFilter,
		wchar_t *Buffer,
		ULONG BufferLen,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETDEVICEINTERFACELISTSIZE) (
		PULONG pulLen,
		LPGUID InterfaceClassGuid,
		DEVINSTID_W pDeviceID,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETDEVICEINTERFACELIST) (
		LPGUID InterfaceClassGuid,
		DEVINSTID_W pDeviceID,
		wchar_t *Buffer,
		ULONG BufferLen,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMLOCATEDEVNODE) (
		PDEVINST pdnDevInst,
		DEVINSTID_W pDeviceID,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMGETCHILD) (
		PDEVINST pdnDevInst,
		DEVINST DevInst,
		ULONG ulFlags
		);


typedef CONFIGRET (__stdcall *PCMGETSIBLING) (
		PDEVINST pdnDevInst,
		DEVINST DevInst,
		ULONG ulFlags
		);

typedef CONFIGRET (__stdcall *PCMREQUESTDEVICEEJECT) (
		DEVINST dnDevInst,
		PPNP_VETO_TYPE pVetoType,
		wchar_t *pszVetoName,
		ULONG ulNameLength,
		ULONG ulFlags
		);

typedef HWND (__stdcall *PGETCONSOLEWINDOW)();
typedef BOOL (__stdcall *PGETCONSOLEKEYBOARDLAYOUTNAME)(wchar_t*);


typedef BOOL (__stdcall *PGETDISKFREESPACEEX) (
		const wchar_t *lpwszDirectoryName,
		PULARGE_INTEGER lpFreeBytesAvailableToCaller,
		PULARGE_INTEGER lpTotalNumberOfBytes,
		PULARGE_INTEGER lpTotalNumberOfFreeBytes
		);

typedef BOOL (__stdcall *PGLOBALMEMORYSTATUSEX)(LPMEMORYSTATUSEX lpBuffer);

typedef BOOL (__stdcall *PCREATEHARDLINK)(
		const wchar_t *lpFileName,                         // new file name
		const wchar_t *lpExistingFileName,                 // extant file name
		LPSECURITY_ATTRIBUTES lpSecurityAttributes  // SD
		);

typedef BOOL (__stdcall *PSETCONSOLEDISPLAYMODE) (
		HANDLE hConsoleOutput,
		DWORD dwFlags,
		PCOORD lpNewScreenBufferDimensions
		);

typedef BOOL (__stdcall *PSETFILEPOINTEREX) (
		HANDLE hFile,
		LARGE_INTEGER liDistanceToMove,
		PLARGE_INTEGER lpNewFilePointer,
		DWORD dwMoveMethod
		);

typedef DWORD (__stdcall *PGETCONSOLEALIAS) (
		LPWSTR lpSource,
		LPWSTR lpTargetBuffer,
		DWORD TargetBufferLength,
		LPWSTR lpExeName
		);


typedef MCIERROR (__stdcall *PMCISENDSTRING) (
		LPCSTR lpstrCommand, 
		LPSTR lpstrReturnString, 
		UINT uReturnLength, 
		HWND hwndCallback
		);

struct ImportedFunctions {

	PISDEBUGGERPRESENT pfnIsDebuggerPresent; //kernel

	PCOPYFILEEX pfnCopyFileEx; //kernel W
	PGETDISKFREESPACEEX pfnGetDiskFreeSpaceEx;
	PSETFILEPOINTEREX pfnSetFilePointerEx;

	//
	PENCRYPTFILE pfnEncryptFile; //kernel, advapi
	PDECRYPTFILE pfnDecryptFile; //kernel, advapi

	bool bEncryptFunctions;
	//
	PGETVOLUMENAMEFORVOLUMEMOUNTPOINT pfnGetVolumeNameForVolumeMountPoint;
	PSETVOLUMEMOUNTPOINT pfnSetVolumeMountPoint;

	bool bVolumeMountPointFunctions;
	//

	PCMGETDEVNODEREGISTRYPROPERTY pfnGetDevNodeRegistryProperty;
	PCMGETDEVNODESTATUS pfnGetDevNodeStatus;
	PCMGETDEVICEID pfnGetDeviceID;
	PCMGETDEVICEIDLISTSIZE pfnGetDeviceIDListSize;
	PCMGETDEVICEIDLIST pfnGetDeviceIDList;
	PCMGETDEVICEINTERFACELISTSIZE pfnGetDeviceInterfaceListSize;
	PCMGETDEVICEINTERFACELIST pfnGetDeviceInterfaceList;
	PCMLOCATEDEVNODE pfnLocateDevNode;
	PCMGETCHILD pfnGetChild;
	PCMGETSIBLING pfnGetSibling;
	PCMREQUESTDEVICEEJECT pfnRequestDeviceEject;

	bool bSetupAPIFunctions;
	//
	PGETCONSOLEWINDOW pfnGetConsoleWindow;
	PGETCONSOLEKEYBOARDLAYOUTNAME pfnGetConsoleKeyboardLayoutName;
	PSETCONSOLEDISPLAYMODE pfnSetConsoleDisplayMode;
	PGETCONSOLEALIAS pfnGetConsoleAlias;

	PGLOBALMEMORYSTATUSEX pfnGlobalMemoryStatusEx;

	PCREATEHARDLINK pfnCreateHardLink;

	PMCISENDSTRING pfnmciSendString;

	void Load();
};

extern ImportedFunctions ifn;

#endif  // __IMPORTS_HPP__
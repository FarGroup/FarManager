#ifndef __IMPORTS_HPP__
#define __IMPORTS_HPP__
/*
imports.hpp

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

typedef BOOL (WINAPI *PCREATESYMBOLICLINK)(
		const wchar_t *lpSymlinkFileName,
		const wchar_t *lpTargetFileName,
		DWORD dwFlags);

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

	PCREATESYMBOLICLINK pfnCreateSymbolicLink;

	PMCISENDSTRING pfnmciSendString;

	void Load();
};

extern ImportedFunctions ifn;

#endif  // __IMPORTS_HPP__

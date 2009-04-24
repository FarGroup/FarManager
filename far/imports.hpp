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

typedef BOOL (__stdcall *PGETCONSOLEKEYBOARDLAYOUTNAME)(wchar_t*);


typedef BOOL (WINAPI *PCREATESYMBOLICLINK)(
		const wchar_t *lpSymlinkFileName,
		const wchar_t *lpTargetFileName,
		DWORD dwFlags);

typedef BOOL (__stdcall *PSETCONSOLEDISPLAYMODE) (
		HANDLE hConsoleOutput,
		DWORD dwFlags,
		PCOORD lpNewScreenBufferDimensions
		);

typedef HRESULT (WINAPI *PSHCREATEASSOCIATIONREGISTRATION)(REFIID, void **);

typedef HANDLE(WINAPI *FINDFIRSTFILENAMEW) (
		LPCWSTR lpFileName,
		DWORD dwFlags,
		LPDWORD StringLength,
		LPWSTR LinkName
		);

typedef BOOL(WINAPI *FINDNEXTFILENAMEW) (
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

typedef NTSTATUS (WINAPI *NTQUERYINFORMATIONFILE)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	int FileInformationClass
	);

struct ImportedFunctions {

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
	PGETCONSOLEKEYBOARDLAYOUTNAME pfnGetConsoleKeyboardLayoutName;
	PSETCONSOLEDISPLAYMODE pfnSetConsoleDisplayMode;

	PCREATESYMBOLICLINK pfnCreateSymbolicLink;

	PSHCREATEASSOCIATIONREGISTRATION pfnSHCreateAssociationRegistration;
	
	FINDFIRSTFILENAMEW pfnFindFirstFileNameW;
	FINDNEXTFILENAMEW pfnFindNextFileNameW;

	FINDFIRSTSTREAMW pfnFindFirstStreamW;
	FINDNEXTSTREAMW pfnFindNextStreamW;

	NTQUERYINFORMATIONFILE pfnNtQueryInformationFile;

	void Load();
};

extern ImportedFunctions ifn;

#endif  // __IMPORTS_HPP__

#ifndef SDK_COMMON_H_8710A968_FB61_435A_B9F6_166D668B92A9
#define SDK_COMMON_H_8710A968_FB61_435A_B9F6_166D668B92A9
#pragma once

/*
sdk_common.h

Типы и определения, отсутствующие в SDK.
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

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8     0x0602
#endif

#ifndef SYMLINK_FLAG_RELATIVE
#define SYMLINK_FLAG_RELATIVE 1
#endif

#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 2
#endif


#ifndef IO_REPARSE_TAG_DRIVE_EXTENDER
#define IO_REPARSE_TAG_DRIVE_EXTENDER           (0x80000005L)
#endif

#ifndef IO_REPARSE_TAG_FILTER_MANAGER
#define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)
#endif

#ifndef IO_REPARSE_TAG_IIS_CACHE
#define IO_REPARSE_TAG_IIS_CACHE                (0xA0000010L)
#endif

#ifndef IO_REPARSE_TAG_APPXSTRM
#define IO_REPARSE_TAG_APPXSTRM                 (0xC0000014L)
#endif

#ifndef IO_REPARSE_TAG_DFM
#define IO_REPARSE_TAG_DFM                      (0x80000016L)
#endif

#ifndef IO_REPARSE_TAG_DEDUP
#define IO_REPARSE_TAG_DEDUP                    (0x80000013L)
#endif

#ifndef IO_REPARSE_TAG_NFS
#define IO_REPARSE_TAG_NFS                      (0x80000014L)
#endif

#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#define IO_REPARSE_TAG_FILE_PLACEHOLDER         (0x80000015L)
#endif

#ifndef IO_REPARSE_TAG_WOF
#define IO_REPARSE_TAG_WOF                      (0x80000017L)
#endif

#ifndef IO_REPARSE_TAG_WCI
#define IO_REPARSE_TAG_WCI                      (0x80000018L)
#endif

#ifndef IO_REPARSE_TAG_WCI_1
#define IO_REPARSE_TAG_WCI_1                    (0x90001018L)
#endif

#ifndef IO_REPARSE_TAG_GLOBAL_REPARSE
#define IO_REPARSE_TAG_GLOBAL_REPARSE           (0xA0000019L)
#endif

#ifndef IO_REPARSE_TAG_CLOUD
#define IO_REPARSE_TAG_CLOUD                    (0x9000001AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_1
#define IO_REPARSE_TAG_CLOUD_1                  (0x9000101AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_2
#define IO_REPARSE_TAG_CLOUD_2                  (0x9000201AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_3
#define IO_REPARSE_TAG_CLOUD_3                  (0x9000301AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_4
#define IO_REPARSE_TAG_CLOUD_4                  (0x9000401AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_5
#define IO_REPARSE_TAG_CLOUD_5                  (0x9000501AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_6
#define IO_REPARSE_TAG_CLOUD_6                  (0x9000601AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_7
#define IO_REPARSE_TAG_CLOUD_7                  (0x9000701AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_8
#define IO_REPARSE_TAG_CLOUD_8                  (0x9000801AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_9
#define IO_REPARSE_TAG_CLOUD_9                  (0x9000901AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_A
#define IO_REPARSE_TAG_CLOUD_A                  (0x9000A01AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_B
#define IO_REPARSE_TAG_CLOUD_B                  (0x9000B01AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_C
#define IO_REPARSE_TAG_CLOUD_C                  (0x9000C01AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_D
#define IO_REPARSE_TAG_CLOUD_D                  (0x9000D01AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_E
#define IO_REPARSE_TAG_CLOUD_E                  (0x9000E01AL)
#endif

#ifndef IO_REPARSE_TAG_CLOUD_F
#define IO_REPARSE_TAG_CLOUD_F                  (0x9000F01AL)
#endif

#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK              (0x8000001BL)
#endif

#ifndef IO_REPARSE_TAG_PROJFS
#define IO_REPARSE_TAG_PROJFS                   (0x9000001CL)
#endif

#ifndef IO_REPARSE_TAG_STORAGE_SYNC
#define IO_REPARSE_TAG_STORAGE_SYNC             (0x8000001EL)
#endif

#ifndef IO_REPARSE_TAG_WCI_TOMBSTONE
#define IO_REPARSE_TAG_WCI_TOMBSTONE            (0xA000001FL)
#endif

#ifndef IO_REPARSE_TAG_UNHANDLED
#define IO_REPARSE_TAG_UNHANDLED                (0x80000020L)
#endif

#ifndef IO_REPARSE_TAG_ONEDRIVE
#define IO_REPARSE_TAG_ONEDRIVE                 (0x80000021L)
#endif

#ifndef IO_REPARSE_TAG_PROJFS_TOMBSTONE
#define IO_REPARSE_TAG_PROJFS_TOMBSTONE         (0xA0000022L)
#endif

#ifndef IO_REPARSE_TAG_AF_UNIX
#define IO_REPARSE_TAG_AF_UNIX                  (0x80000023L)
#endif

#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK               (0xA000001DL)
#endif

#ifndef IO_REPARSE_TAG_LX_FIFO
#define IO_REPARSE_TAG_LX_FIFO                  (0x80000024L)
#endif

#ifndef IO_REPARSE_TAG_LX_CHR
#define IO_REPARSE_TAG_LX_CHR                   (0x80000025L)
#endif

#ifndef IO_REPARSE_TAG_LX_BLK
#define IO_REPARSE_TAG_LX_BLK                   (0x80000026L)
#endif


#ifndef FILE_ATTRIBUTE_INTEGRITY_STREAM
#define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x8000
#endif

#ifndef FILE_ATTRIBUTE_NO_SCRUB_DATA
#define FILE_ATTRIBUTE_NO_SCRUB_DATA 0x20000
#endif

#ifndef FILE_ATTRIBUTE_PINNED
#define FILE_ATTRIBUTE_PINNED 0x80000
#endif

#ifndef FILE_ATTRIBUTE_UNPINNED
#define FILE_ATTRIBUTE_UNPINNED 0x100000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_OPEN
#define FILE_ATTRIBUTE_RECALL_ON_OPEN 0x00040000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#endif

#ifndef FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL
#define FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL 0x20000000
#endif

// wincon.h
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif

// winnls.h
#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif

typedef struct _FILE_BASIC_INFORMATION
{
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	ULONG FileAttributes;
}
FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	CCHAR ShortNameLength;
	WCHAR ShortName[12];
	WCHAR FileName[1];
}
FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	CCHAR ShortNameLength;
	WCHAR ShortName[12];
	LARGE_INTEGER FileId;
	WCHAR FileName[1];
}
FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _FILE_STREAM_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG StreamNameLength;
	LARGE_INTEGER StreamSize;
	LARGE_INTEGER StreamAllocationSize;
	WCHAR StreamName[1];
}
FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION
{
	UNICODE_STRING Name;
}
OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

#define RTL_RESOURCE_FLAG_LONG_TERM (0x00000001ul)

typedef struct _RTL_RESOURCE
{
	RTL_CRITICAL_SECTION Lock;
	HANDLE SharedSemaphore;
	ULONG SharedWaiters;
	HANDLE ExclusiveSemaphore;
	ULONG ExclusiveWaiters;
	LONG NumberActive;
	HANDLE OwningThread;
	ULONG Flags;
	PVOID DebugInfo;
}
RTL_RESOURCE, *PRTL_RESOURCE;

#if _WIN32_WINNT >= 0x0603
#include <versionhelpers.h>
#else
#define _WIN32_WINNT_WIN8 0x0602
#define _WIN32_WINNT_WINBLUE 0x0603

#define VERSIONHELPERAPI inline bool

VERSIONHELPERAPI
IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
		VerSetConditionMask(
		0, VER_MAJORVERSION, VER_GREATER_EQUAL),
		VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

VERSIONHELPERAPI
IsWindowsXPOrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0);
}

VERSIONHELPERAPI
IsWindowsXPSP1OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 1);
}

VERSIONHELPERAPI
IsWindowsXPSP2OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 2);
}

VERSIONHELPERAPI
IsWindowsXPSP3OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 3);
}

VERSIONHELPERAPI
IsWindowsVistaOrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
}

VERSIONHELPERAPI
IsWindowsVistaSP1OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 1);
}

VERSIONHELPERAPI
IsWindowsVistaSP2OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 2);
}

VERSIONHELPERAPI
IsWindows7OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0);
}

VERSIONHELPERAPI
IsWindows7SP1OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 1);
}

VERSIONHELPERAPI
IsWindows8OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
}

VERSIONHELPERAPI
IsWindows8Point1OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0);
}

VERSIONHELPERAPI
IsWindowsServer()
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0, 0, VER_NT_WORKSTATION };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);

	return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
}
#endif

#if !defined _WIN32_WINNT_WIN10

#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0xA00
#endif

VERSIONHELPERAPI IsWindows10OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0);
}
#endif

#endif // SDK_COMMON_H_8710A968_FB61_435A_B9F6_166D668B92A9

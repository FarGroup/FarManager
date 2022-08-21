﻿#ifndef SDK_COMMON_H_8710A968_FB61_435A_B9F6_166D668B92A9
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

#ifdef __GNUC__
#pragma GCC system_header
#endif

// winnt.h
#ifndef IO_REPARSE_TAG_DRIVE_EXTENDER
#define IO_REPARSE_TAG_DRIVE_EXTENDER 0x80000005L
#endif

#ifndef IO_REPARSE_TAG_FILTER_MANAGER
#define IO_REPARSE_TAG_FILTER_MANAGER 0x8000000BL
#endif

#ifndef IO_REPARSE_TAG_IIS_CACHE
#define IO_REPARSE_TAG_IIS_CACHE 0xA0000010L
#endif

#ifndef IO_REPARSE_TAG_APPXSTRM
#define IO_REPARSE_TAG_APPXSTRM 0xC0000014L
#endif

#ifndef IO_REPARSE_TAG_DFM
#define IO_REPARSE_TAG_DFM 0x80000016L
#endif


#ifndef IO_REPARSE_TAG_PROJFS
#define IO_REPARSE_TAG_PROJFS 0x9000001CL
#endif

#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK 0xA000001DL
#endif

#ifndef IO_REPARSE_TAG_STORAGE_SYNC
#define IO_REPARSE_TAG_STORAGE_SYNC 0x8000001EL
#endif

#ifndef IO_REPARSE_TAG_WCI_TOMBSTONE
#define IO_REPARSE_TAG_WCI_TOMBSTONE 0xA000001FL
#endif

#ifndef IO_REPARSE_TAG_UNHANDLED
#define IO_REPARSE_TAG_UNHANDLED 0x80000020L
#endif

#ifndef IO_REPARSE_TAG_ONEDRIVE
#define IO_REPARSE_TAG_ONEDRIVE 0x80000021L
#endif

#ifndef IO_REPARSE_TAG_PROJFS_TOMBSTONE
#define IO_REPARSE_TAG_PROJFS_TOMBSTONE 0xA0000022L
#endif

#ifndef IO_REPARSE_TAG_AF_UNIX
#define IO_REPARSE_TAG_AF_UNIX 0x80000023L
#endif
#ifndef IO_REPARSE_TAG_LX_FIFO
#define IO_REPARSE_TAG_LX_FIFO 0x80000024L
#endif

#ifndef IO_REPARSE_TAG_LX_CHR
#define IO_REPARSE_TAG_LX_CHR 0x80000025L
#endif

#ifndef IO_REPARSE_TAG_LX_BLK
#define IO_REPARSE_TAG_LX_BLK 0x80000026L
#endif

// ntrtl.h
#define RTL_RESOURCE_FLAG_LONG_TERM 0x00000001ul

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

// wdm.h
typedef struct _FILE_PROCESS_IDS_USING_FILE_INFORMATION
{
	ULONG NumberOfProcessIdsInList;
	ULONG_PTR ProcessIdList[1];
}
FILE_PROCESS_IDS_USING_FILE_INFORMATION, *PFILE_PROCESS_IDS_USING_FILE_INFORMATION;

inline constexpr auto FileProcessIdsUsingFileInformation  = static_cast<FILE_INFORMATION_CLASS>(47);

#endif // SDK_COMMON_H_8710A968_FB61_435A_B9F6_166D668B92A9

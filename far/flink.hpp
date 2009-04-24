#ifndef __FLINK_HPP__
#define __FLINK_HPP__
/*
flink.hpp

«аголовочный файл дл€ работы с Hard & SymLink
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

int   WINAPI MkHardLink(const wchar_t *Src,const wchar_t *Dest);
int   WINAPI FarMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);

BOOL  WINAPI CanCreateHardLinks(const wchar_t *TargetFile,const wchar_t *HardLinkName);
int   WINAPI GetNumberOfLinks(const wchar_t *Name);
int   WINAPI CreateVolumeMountPoint(const wchar_t *SrcVolume, const wchar_t *LinkFolder);

BOOL  WINAPI CreateReparsePoint(const wchar_t *szMountDir, const wchar_t *szDestDir,DWORD Type=RP_JUNCTION);
BOOL  WINAPI DeleteReparsePoint(const wchar_t *szMountDir);

DWORD WINAPI GetReparsePointInfo(const wchar_t *szMountDir, string &szDestBuff,LPDWORD lpReparseTag=NULL);

bool GetSubstName(int DriveType,const wchar_t *LocalName,string &strSubstName);

int DelSubstDrive(const wchar_t *DosDeviceName);
void GetPathRoot(const wchar_t *Path, string &strRoot, int Reenter=0);
void GetPathRootOne(const wchar_t *Path, string &strRoot);

// перечисл€тель дл€ EnumNTFSStreams
// в параметре sid поле cStreamName не актуально, т.к. готовое им€ потока
//    передаетс€ в параметре StreamName
typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams);

bool EnumStreams(const wchar_t *FileName,UINT64 &StreamsSize,DWORD &StreamsCount);

bool SetPrivilege(LPCWSTR Privilege,BOOL bEnable);

bool DuplicateReparsePoint(const wchar_t *Src,const wchar_t *Dst);

#endif // __FLINK_HPP__

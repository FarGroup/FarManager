#ifndef FLINK_HPP_76B08BB3_29AE_4BCA_B01B_C600603A2996
#define FLINK_HPP_76B08BB3_29AE_4BCA_B01B_C600603A2996
#pragma once

/*
flink.hpp

Заголовочный файл для работы с Hard & SymLink
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

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

struct error_state_ex;

enum ReparsePointTypes: int
{
	RP_EXACTCOPY,   // для копирования/переноса ссылок, копия существующего
	RP_HARDLINK,    // жёсткая ссылка
	RP_JUNCTION,    // связь
	RP_VOLMOUNT,    // монтированный том
	RP_SYMLINK, // ссылка, NT>=6
	RP_SYMLINKFILE, // файл-ссылка, NT>=6
	RP_SYMLINKDIR,  // каталог-ссылка, NT>=6
};

bool MkHardLink(string_view ExistingName, string_view NewName, std::optional<error_state_ex>& ErrorState, bool Silent = false);

std::optional<size_t> GetNumberOfLinks(string_view Name);
bool CreateVolumeMountPoint(string_view TargetVolume, const string& Object);

bool CreateReparsePoint(string_view Target, string_view Object, ReparsePointTypes Type = RP_JUNCTION);
bool DeleteReparsePoint(string_view Object);
bool ModifyReparsePoint(string_view Object, string_view Target);

bool GetReparsePointInfo(string_view Object, string &DestBuffer,LPDWORD ReparseTag=nullptr);

bool GetSubstName(int DriveType, string_view Path, string& strTargetPath);
bool GetVHDInfo(string_view RootDirectory, string &strVolumePath, VIRTUAL_STORAGE_TYPE* StorageType = nullptr);
bool detach_vhd(string_view RootDirectory, bool& IsVhd);

bool DelSubstDrive(string_view DeviceName);
string GetPathRoot(string_view Path);

// перечислятель для EnumNTFSStreams
// в параметре sid поле cStreamName не актуально, т.к. готовое имя потока
//    передается в параметре StreamName
//typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
//int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum, long long* SizeStreams);

bool EnumStreams(string_view FileName, unsigned long long& StreamsSize, size_t& StreamsCount);

void NormalizeSymlinkName(string &strLinkName);

bool MkSymLink(string_view Target, string_view LinkName, ReparsePointTypes LinkType, std::optional<error_state_ex>& ErrorState, bool Silent = false, bool HoldTarget = false);

bool reparse_tag_to_string(DWORD ReparseTag, string& Str, bool ShowUnknown);

#endif // FLINK_HPP_76B08BB3_29AE_4BCA_B01B_C600603A2996

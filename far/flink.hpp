﻿#ifndef FLINK_HPP_76B08BB3_29AE_4BCA_B01B_C600603A2996
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

int MkHardLink(const string& ExistingName,const string& NewName, bool Silent = false);

int GetNumberOfLinks(const string& Name, bool negative_if_error=false);
bool CreateVolumeMountPoint(const string& TargetVolume, const string& Object);

bool CreateReparsePoint(const string& Target, const string& Object,ReparsePointTypes Type=RP_JUNCTION);
bool DeleteReparsePoint(const string& Object);
bool ModifyReparsePoint(const string& Object,const string& NewData);

bool GetReparsePointInfo(const string& Object, string &szDestBuff,LPDWORD lpReparseTag=nullptr);

bool GetSubstName(int DriveType,const string& DeviceName,string &strTargetPath);
bool GetVHDInfo(const string& DeviceName, string &strVolumePath, VIRTUAL_STORAGE_TYPE* StorageType = nullptr);

bool DelSubstDrive(const string& DeviceName);
string GetPathRoot(string_view Path);

// перечислятель для EnumNTFSStreams
// в параметре sid поле cStreamName не актуально, т.к. готовое имя потока
//    передается в параметре StreamName
//typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
//int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum, long long* SizeStreams);

bool EnumStreams(const string& FileName, unsigned long long& StreamsSize,DWORD &StreamsCount);

bool DuplicateReparsePoint(const string& Src,const string& Dst);

void NormalizeSymlinkName(string &strLinkName);

int MkSymLink(const string& Target,const string& LinkName, ReparsePointTypes LinkType, bool Silent=false, bool HoldTarget=false);

#endif // FLINK_HPP_76B08BB3_29AE_4BCA_B01B_C600603A2996

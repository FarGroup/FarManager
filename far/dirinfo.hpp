﻿#ifndef DIRINFO_HPP_DA86BD11_D517_4EC9_8324_44EDF0CC7C9A
#define DIRINFO_HPP_DA86BD11_D517_4EC9_8324_44EDF0CC7C9A
#pragma once

/*
dirinfo.hpp

GetDirInfo & GetPluginDirInfo
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

class FileFilter;
struct PluginPanelItem;
struct UserDataItem;

enum GETDIRINFOFLAGS
{
	GETDIRINFO_ENHBREAK           =0x00000001,
	GETDIRINFO_SCANSYMLINK        =0x00000004,
	GETDIRINFO_SCANSYMLINKDEF     =0x00000008,
	GETDIRINFO_USEFILTER          =0x00000010,
};

struct DirInfoData
{
	unsigned long long FileSize;
	unsigned long long AllocationSize;
	unsigned long long FilesSlack;
	unsigned long long MFTOverhead;
	unsigned long long ClusterSize;
	DWORD DirCount;
	DWORD FileCount;
};

enum getdirinfo_message_delay
{
	getdirinfo_infinite_delay = -1,
	getdirinfo_no_delay = 0,
	getdirinfo_default_delay = 500, // ms
};

using dirinfo_callback = std::function<void(string_view Name, unsigned long long Items, unsigned long long Size)>;
int GetDirInfo(const string& DirName, DirInfoData& Data, FileFilter *Filter, const dirinfo_callback& Callback, DWORD Flags = GETDIRINFO_SCANSYMLINKDEF);
void DirInfoMsg(string_view Title, string_view Name, unsigned long long Items, unsigned long long Size);

class plugin_panel;

bool GetPluginDirInfo(const plugin_panel* hPlugin,const string& DirName, const UserDataItem* UserData, unsigned long& DirCount, unsigned long& FileCount,unsigned long long& FileSize, unsigned long long& CompressedFileSize);

bool GetPluginDirList(class Plugin* PluginNumber, HANDLE hPlugin, const string& Dir, const UserDataItem* UserData, std::vector<PluginPanelItem>& Items);
void FreePluginDirList(HANDLE hPlugin, std::vector<PluginPanelItem>& Items);

#endif // DIRINFO_HPP_DA86BD11_D517_4EC9_8324_44EDF0CC7C9A

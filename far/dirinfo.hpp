#ifndef DIRINFO_HPP_DA86BD11_D517_4EC9_8324_44EDF0CC7C9A
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

// Internal:
#include "common/function_ref.hpp"

// Platform:

// Common:
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

class multifilter;
struct PluginPanelItem;
struct UserDataItem;

enum GETDIRINFOFLAGS
{
	GETDIRINFO_ENHBREAK           = 0_bit,
	GETDIRINFO_SCANSYMLINK        = 2_bit,
	GETDIRINFO_SCANSYMLINKDEF     = 3_bit,
	GETDIRINFO_USEFILTER          = 4_bit,
};

struct BasicDirInfoData
{
	DWORD DirCount{};
	DWORD FileCount{};
	unsigned long long FileSize{};
	unsigned long long AllocationSize{};
};

struct DirInfoData: public BasicDirInfoData
{
	unsigned long long FilesSlack{};
	unsigned long long MFTOverhead{};
	unsigned long long ClusterSize{};
};

using dirinfo_callback = function_ref<void(string_view Name, unsigned long long Items, unsigned long long Size)>;
int GetDirInfo(string_view DirName, DirInfoData& Data, multifilter* Filter, dirinfo_callback Callback, DWORD Flags = GETDIRINFO_SCANSYMLINKDEF);

class plugin_panel;

bool GetPluginDirInfo(const plugin_panel* hPlugin, string_view DirName, const UserDataItem* UserData, BasicDirInfoData& Data, dirinfo_callback Callback);

bool GetPluginDirList(class Plugin* PluginNumber, HANDLE hPlugin, string_view Dir, const UserDataItem* UserData, std::vector<PluginPanelItem>& Items, dirinfo_callback Callback);
void FreePluginDirList(HANDLE hPlugin, std::vector<PluginPanelItem>& Items);

#endif // DIRINFO_HPP_DA86BD11_D517_4EC9_8324_44EDF0CC7C9A

#ifndef MIX_HPP_67869A41_F20D_4C95_86E1_4D598A356EE1
#define MIX_HPP_67869A41_F20D_4C95_86E1_4D598A356EE1
#pragma once

/*
mix.hpp

Mix
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

#include "platform.fwd.hpp"
#include "plugin.hpp"

template<class T>
auto ToPercent(T Value, T Base)
{
	if (!Base)
		return 0;

	return static_cast<int>(std::numeric_limits<T>::max() / 100 > Value?
		Value * 100 / Base :
		Value / Base * 100);
}

template<typename T>
T FromPercent(int Percent, T Base)
{
	if (!Percent)
		return 0;

	return std::numeric_limits<T>::max() / Percent > Base?
		Base * Percent / 100 :
		Base / 100 * Percent;
};

bool FarMkTempEx(string &strDest, string_view Prefix = {}, bool WithTempPath = true, string_view UserTempPath = {});

void PluginPanelItemToFindDataEx(const PluginPanelItem& Src, os::fs::find_data& Dest);

class PluginPanelItemHolder
{
public:
	NONCOPYABLE(PluginPanelItemHolder);

	PluginPanelItemHolder() = default;
	~PluginPanelItemHolder();

	PluginPanelItem Item{};
};

class PluginPanelItemHolderNonOwning: public PluginPanelItemHolder
{
public:
	~PluginPanelItemHolderNonOwning()
	{
		Item = {};
	}
};

void FindDataExToPluginPanelItemHolder(const os::fs::find_data& Src, PluginPanelItemHolder& Holder);

void FreePluginPanelItem(const PluginPanelItem& Data);
void FreePluginPanelItems(const std::vector<PluginPanelItem>& Items);

void FreePluginPanelItemsUserData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);

template<class T>
void DeleteRawArray(range<T> Data)
{
	for (const auto& i : Data)
	{
		delete[] i;
	}
	
	delete[] Data.data();
}

WINDOWINFO_TYPE WindowTypeToPluginWindowType(int fType);

class SetAutocomplete: noncopyable
{
public:
	explicit SetAutocomplete(class EditControl* edit, bool NewState = false);
	explicit SetAutocomplete(class DlgEdit* dedit, bool NewState = false);
	explicit SetAutocomplete(class CommandLine* cedit, bool NewState = false);
	~SetAutocomplete();

private:
	class EditControl* edit;
	bool OldState;
};

struct uuid_hash
{
	size_t operator ()(const GUID& Key) const
	{
		RPC_STATUS Status;
		return UuidHash(const_cast<UUID*>(&Key), &Status);
	}
};

void ReloadEnvironment();

unsigned int CRC32(unsigned int crc, const void* buffer, size_t size);

#endif // MIX_HPP_67869A41_F20D_4C95_86E1_4D598A356EE1

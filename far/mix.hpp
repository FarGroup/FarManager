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

// Internal:
#include "plugin.hpp"

// Platform:
#include "platform.fwd.hpp"

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

unsigned int ToPercent(unsigned long long Value, unsigned long long Base, unsigned Max = 100);
unsigned long long FromPercent(unsigned int Percent, unsigned long long Base);

string MakeTemp(string_view Prefix = {}, bool WithTempPath = true, string_view UserTempPath = {});
string MakeTempInSameDir(string_view FileName);

void PluginPanelItemToFindDataEx(const PluginPanelItem& Src, os::fs::find_data& Dest);

class PluginPanelItemHolder
{
public:
	virtual void set_name(string_view Value) = 0;
	virtual void set_alt_name(string_view Value) = 0;
	virtual void set_description(string_view Value) = 0;
	virtual void set_owner(string_view Value) = 0;
	virtual void set_columns(std::span<const wchar_t* const> Value) = 0;

	PluginPanelItem Item{};

protected:
	virtual ~PluginPanelItemHolder() = default;
};

class PluginPanelItemHolderRef final: public PluginPanelItemHolder
{
public:
	~PluginPanelItemHolderRef() override = default;

	void set_name(string_view Value) override;
	void set_alt_name(string_view Value) override;
	void set_description(string_view Value) override;
	void set_owner(string_view Value) override;
	void set_columns(std::span<const wchar_t* const> Value) override;
};

class PluginPanelItemHolderHeap: public PluginPanelItemHolder
{
public:
	NONCOPYABLE(PluginPanelItemHolderHeap);

	PluginPanelItemHolderHeap() = default;
	~PluginPanelItemHolderHeap() override;

	void set_name(string_view Value) override;
	void set_alt_name(string_view Value) override;
	void set_description(string_view Value) override;
	void set_owner(string_view Value) override;
	void set_columns(std::span<const wchar_t* const> Values) override;

private:
	static const wchar_t* make_copy(string_view Value);
};

class PluginPanelItemHolderHeapNonOwning final: public PluginPanelItemHolderHeap
{
public:
	~PluginPanelItemHolderHeapNonOwning() override
	{
		Item = {};
	}
};

void FindDataExToPluginPanelItemHolder(const os::fs::find_data& Src, PluginPanelItemHolder& Holder);

void FreePluginPanelItemData(const PluginPanelItem& Data);
void FreePluginPanelItemUserData(HANDLE hPlugin, const UserDataItem& Data);
void FreePluginPanelItemsData(std::span<PluginPanelItem> Items);

class plugin_item_list
{
public:
	NONCOPYABLE(plugin_item_list);
	MOVE_CONSTRUCTIBLE(plugin_item_list);

	plugin_item_list() = default;
	~plugin_item_list();

	void emplace_back(const PluginPanelItem& Item);
	void reserve(size_t Size);

	auto begin() { return m_Data.begin(); }
	auto begin() const { return m_Data.begin(); }

	auto end() { return m_Data.end(); }
	auto end() const { return m_Data.end(); }

	const PluginPanelItem* data() const;
	PluginPanelItem* data();
	size_t size() const;
	bool empty() const;

private:
	std::vector<PluginPanelItem> m_Data;
};

template<class T>
void DeleteRawArray(std::span<T> Data)
{
	for (const auto& i: Data)
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

template<>
struct std::hash<UUID>
{
	size_t operator()(const UUID& Value) const noexcept
	{
		RPC_STATUS Status;
		return UuidHash(const_cast<UUID*>(&Value), &Status);
	}
};

void ReloadEnvironment();

string version_to_string(const VersionInfo& Version);

#endif // MIX_HPP_67869A41_F20D_4C95_86E1_4D598A356EE1

/*
plugsettings.cpp

API для хранения плагинами настроек.
*/
/*
Copyright © 2011 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "plugsettings.hpp"

// Internal:
#include "ctrlobj.hpp"
#include "history.hpp"
#include "uuids.far.hpp"
#include "shortcuts.hpp"
#include "dizlist.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "plugins.hpp"
#include "configdb.hpp"
#include "global.hpp"
#include "exception.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/function_ref.hpp"
#include "common/string_utils.hpp"
#include "common/uuid.hpp"

// External:

//----------------------------------------------------------------------------

const wchar_t* AbstractSettings::Add(string_view const String)
{
	return static_cast<const wchar_t*>(Add(String.data(), (String.size() + 1) * sizeof(wchar_t)));
}

const void* AbstractSettings::Add(const void* Data, size_t Size)
{
	const auto Dest = Allocate(Size);
	copy_memory(Data, Dest, Size);
	return Dest;
}

void* AbstractSettings::Allocate(size_t Size)
{
	m_Data.emplace_back(Size);
	return m_Data.back().data();
}

class FarSettingsNameItems
{
public:
	NONCOPYABLE(FarSettingsNameItems);
	MOVE_CONSTRUCTIBLE(FarSettingsNameItems);

	FarSettingsNameItems() = default;

	void add(FarSettingsName& Item, string&& String)
	{
		m_Strings.emplace_front(std::move(String));
		Item.Name = m_Strings.front().c_str();
		m_Items.emplace_back(Item);
	}

	void get(FarSettingsEnum& e) const
	{
		e.Count = m_Items.size();
		e.Items = e.Count? m_Items.data() : nullptr;
	}

private:
	std::vector<FarSettingsName> m_Items;
	// String address must always remain valid, hence the list
	std::forward_list<string> m_Strings;
};

class PluginSettings final: public AbstractSettings
{
public:
	PluginSettings(const Plugin* pPlugin, bool Local);
	~PluginSettings() override;

	bool Set(const FarSettingsItem& Item) override;
	bool Get(FarSettingsItem& Item) override;
	bool Enum(FarSettingsEnum& Enum) override;
	bool Delete(const FarSettingsValue& Value) override;
	int SubKey(const FarSettingsValue& Value, bool bCreate) override;

private:
	std::vector<FarSettingsNameItems> m_Enum;
	std::vector<HierarchicalConfig::key> m_Keys;
	HierarchicalConfigUniquePtr PluginsCfg;
};


std::unique_ptr<AbstractSettings> AbstractSettings::CreatePluginSettings(const UUID& Uuid, bool const Local)
{
	const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(Uuid);
	if (!pPlugin)
		return nullptr;

	try
	{
		return std::make_unique<PluginSettings>(pPlugin, Local);
	}
	catch (far_exception const&)
	{
		return nullptr;
	}
}


PluginSettings::PluginSettings(const Plugin* const pPlugin, bool const Local)
{
	const auto strUuid = uuid::str(pPlugin->Id());
	PluginsCfg = ConfigProvider().CreatePluginsConfig(strUuid, Local, false);
	PluginsCfg->BeginTransaction();

	const auto Key = PluginsCfg->CreateKey(HierarchicalConfig::root_key, strUuid);
	PluginsCfg->SetKeyDescription(Key, pPlugin->Title());
	m_Keys.emplace_back(Key);

	if (!Global->Opt->ReadOnlyConfig)
	{
		DizList Diz;
		const auto DbPath = path::join(Local? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath, L"PluginsData"sv);
		Diz.Read(DbPath);
		const string DbName(PointToName(PluginsCfg->GetName()));
		const auto Description = concat(pPlugin->Title(), L" ("sv, pPlugin->Description(), L')');
		if (Description != Diz.Get(DbName, {}, 0))
		{
			Diz.Set(DbName, {}, Description);
			Diz.Flush(DbPath);
		}
	}
}

PluginSettings::~PluginSettings()
{
	if (PluginsCfg)
		PluginsCfg->EndTransaction();
}

bool PluginSettings::Set(const FarSettingsItem& Item)
{
	if (Item.Root >= m_Keys.size())
		return false;

	const auto name = NullToEmpty(Item.Name);
	switch(Item.Type)
	{
	case FST_SUBKEY:
		return false;

	case FST_QWORD:
		PluginsCfg->SetValue(m_Keys[Item.Root], name, Item.Number);
		return true;

	case FST_STRING:
		PluginsCfg->SetValue(m_Keys[Item.Root], name, NullToEmpty(Item.String));
		return true;

	case FST_DATA:
		PluginsCfg->SetValue(m_Keys[Item.Root], name, view_bytes(Item.Data.Data, Item.Data.Size));
		return true;

	default:
		return false;
	}
}

bool PluginSettings::Get(FarSettingsItem& Item)
{
	if (Item.Root >= m_Keys.size())
		return false;

	const auto name = NullToEmpty(Item.Name);
	switch(Item.Type)
	{
	case FST_SUBKEY:
		return false;

	case FST_QWORD:
		{
			unsigned long long value;
			if (PluginsCfg->GetValue(m_Keys[Item.Root], name, value))
			{
				Item.Number = value;
				return true;
			}
		}
		break;

	case FST_STRING:
		{
			string data;
			if (PluginsCfg->GetValue(m_Keys[Item.Root], name, data))
			{
				Item.String = Add(data);
				return true;
			}
		}
		break;

	case FST_DATA:
		{
			bytes data;
			if (PluginsCfg->GetValue(m_Keys[Item.Root], name, data))
			{
				Item.Data.Data = Add(data.data(), data.size());
				Item.Data.Size = data.size();
				return true;
			}
		}
		break;

	default:
		return false;
	}

	return false;
}

class FarSettingsHistoryItems
{
public:
	NONCOPYABLE(FarSettingsHistoryItems);
	MOVE_CONSTRUCTIBLE(FarSettingsHistoryItems);

	FarSettingsHistoryItems() = default;

	void add(FarSettingsHistory& Item, string&& Name, string&& Param, string&& File, const UUID& Uuid)
	{
		m_Names.emplace_front(std::move(Name));
		m_Params.emplace_front(std::move(Param));
		m_Files.emplace_front(std::move(File));
		Item.Name = m_Names.front().c_str();
		Item.Param = m_Params.front().c_str();
		Item.File = m_Files.front().c_str();
		Item.PluginId = Uuid;
		m_Items.emplace_back(Item);
	}

	void get(FarSettingsEnum& e) const
	{
		e.Count = m_Items.size();
		e.Histories = e.Count? m_Items.data() : nullptr;
	}

private:
	std::vector<FarSettingsHistory> m_Items;
	// String address must always remain valid, hence the list
	std::forward_list<string> m_Names, m_Params, m_Files;
};

class FarSettings final: public AbstractSettings
{
public:
	bool Set(const FarSettingsItem& Item) override;
	bool Get(FarSettingsItem& Item) override;
	bool Enum(FarSettingsEnum& Enum) override;
	bool Delete(const FarSettingsValue& Value) override;
	int SubKey(const FarSettingsValue& Value, bool bCreate) override;

private:
	bool FillHistory(int Type, string_view HistoryName, FarSettingsEnum& Enum, function_ref<bool(history_record_type)> Filter);
	std::vector<FarSettingsHistoryItems> m_Enum;
	std::vector<string> m_Keys;
};


std::unique_ptr<AbstractSettings> AbstractSettings::CreateFarSettings()
{
	return std::make_unique<FarSettings>();
}

bool PluginSettings::Enum(FarSettingsEnum& Enum)
{
	if (Enum.Root >= m_Keys.size())
		return false;

	FarSettingsName item{ nullptr, FST_SUBKEY };

	FarSettingsNameItems NewEnumItem;

	const auto& root = m_Keys[Enum.Root];

	{
		string KeyName;
		for (const auto& Key : PluginsCfg->KeysEnumerator(root))
		{
			if (PluginsCfg->GetKeyName(root, Key, KeyName))
				NewEnumItem.add(item, std::move(KeyName));
		}
	}

	for(auto& [Name, Value]: PluginsCfg->ValuesEnumerator(root))
	{
		item.Type = static_cast<FARSETTINGSTYPES>(PluginsCfg->ToSettingsType(Value));

		if(item.Type!=FST_UNKNOWN)
		{
			NewEnumItem.add(item, std::move(Name));
		}
	}
	NewEnumItem.get(Enum);
	m_Enum.emplace_back(std::move(NewEnumItem));

	return true;
}

bool PluginSettings::Delete(const FarSettingsValue& Value)
{
	if (Value.Root >= m_Keys.size())
		return false;

	Value.Value?
		PluginsCfg->DeleteValue(m_Keys[Value.Root], Value.Value) :
		PluginsCfg->DeleteKeyTree(m_Keys[Value.Root]);

	return true;
}

int PluginSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	//Don't allow illegal key names - empty names or with backslashes
	if (Value.Root >= m_Keys.size() || !Value.Value || !*Value.Value || contains(Value.Value, '\\'))
		return 0;

	const auto root = bCreate? PluginsCfg->CreateKey(m_Keys[Value.Root], Value.Value) : PluginsCfg->FindByName(m_Keys[Value.Root], Value.Value);
	if (!bCreate && !root)
		return 0;

	m_Keys.emplace_back(root);
	return static_cast<int>(m_Keys.size() - 1);
}

bool FarSettings::Set(const FarSettingsItem& Item)
{
	return false;
}

bool FarSettings::Get(FarSettingsItem& Item)
{
	const auto Data = Global->Opt->GetConfigValue(Item.Root, Item.Name);
	if (!Data)
		return false;

	Data->Export(Item);

	if (Item.Type == FST_STRING)
		Item.String = Add(Item.String);

	return true;
}

bool FarSettings::Enum(FarSettingsEnum& Enum)
{
	const auto FilterNone = [](history_record_type) { return true; };

	switch(Enum.Root)
	{
	case FSSF_HISTORY_CMD:
		return FillHistory(HISTORYTYPE_CMD, {}, Enum, FilterNone);

	case FSSF_HISTORY_FOLDER:
		return FillHistory(HISTORYTYPE_FOLDER, {}, Enum, FilterNone);

	case FSSF_HISTORY_VIEW:
		return FillHistory(HISTORYTYPE_VIEW, {}, Enum, [](history_record_type Type) { return Type == HR_VIEWER; });

	case FSSF_HISTORY_EDIT:
		return FillHistory(HISTORYTYPE_VIEW, {}, Enum, [](history_record_type Type) { return Type == HR_EDITOR || Type == HR_EDITOR_RO; });

	case FSSF_HISTORY_EXTERNAL:
		return FillHistory(HISTORYTYPE_VIEW, {}, Enum, [](history_record_type Type) { return Type == HR_EXTERNAL || Type == HR_EXTERNAL_WAIT; });

	case FSSF_FOLDERSHORTCUT_0:
	case FSSF_FOLDERSHORTCUT_1:
	case FSSF_FOLDERSHORTCUT_2:
	case FSSF_FOLDERSHORTCUT_3:
	case FSSF_FOLDERSHORTCUT_4:
	case FSSF_FOLDERSHORTCUT_5:
	case FSSF_FOLDERSHORTCUT_6:
	case FSSF_FOLDERSHORTCUT_7:
	case FSSF_FOLDERSHORTCUT_8:
	case FSSF_FOLDERSHORTCUT_9:
		{
			FarSettingsHistory item{};
			FarSettingsHistoryItems NewEnumItem;
			for(auto& i: Shortcuts(Enum.Root - FSSF_FOLDERSHORTCUT_0).Enumerator())
			{
				NewEnumItem.add(item, std::move(i.Folder), std::move(i.PluginData), std::move(i.PluginFile), i.PluginUuid);
			}
			NewEnumItem.get(Enum);
			m_Enum.emplace_back(std::move(NewEnumItem));
			return true;
		}

	default:
		if(Enum.Root >= FSSF_COUNT)
		{
			const auto root = Enum.Root - FSSF_COUNT;
			if(root < m_Keys.size())
			{
				return FillHistory(HISTORYTYPE_DIALOG, m_Keys[root], Enum, FilterNone);
			}
		}
		return false;
	}
}

bool FarSettings::Delete(const FarSettingsValue& Value)
{
	return false;
}

int FarSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	if (bCreate || Value.Root != FSSF_ROOT)
		return 0;

	const size_t Position = std::ranges::find(m_Keys, Value.Value) - m_Keys.cbegin();
	if (Position == m_Keys.size())
		m_Keys.emplace_back(Value.Value);

	return static_cast<int>(Position + FSSF_COUNT);
}

static const auto& HistoryRef(int Type)
{
	const auto IsPersistent = [Type]() -> bool
	{
		switch (Type)
		{
		case HISTORYTYPE_CMD: return Global->Opt->SaveHistory != BSTATE_UNCHECKED;
		case HISTORYTYPE_FOLDER: return Global->Opt->SaveFoldersHistory != BSTATE_UNCHECKED;
		case HISTORYTYPE_VIEW: return Global->Opt->SaveViewHistory != BSTATE_UNCHECKED;
		case HISTORYTYPE_DIALOG: return Global->Opt->Dialogs.EditHistory != BSTATE_UNCHECKED;
		default: return true;
		}
	};

	return IsPersistent()? ConfigProvider().HistoryCfg() : ConfigProvider().HistoryCfgMem();
}

bool FarSettings::FillHistory(int Type, string_view const HistoryName, FarSettingsEnum& Enum, function_ref<bool(history_record_type)> const Filter)
{
	FarSettingsHistory item{};
	FarSettingsHistoryItems NewEnumItem;

	for(auto& i: HistoryRef(Type)->Enumerator(Type, HistoryName))
	{
		if (!Filter(i.Type))
			continue;

		item.Time = os::chrono::nt_clock::to_filetime(i.Time);
		item.Lock = i.Lock;
		const auto Uuid = uuid::try_parse(i.Uuid);
		NewEnumItem.add(item, std::move(i.Name), std::move(i.Data), std::move(i.File), Uuid? *Uuid : FarUuid);
	}

	NewEnumItem.get(Enum);
	m_Enum.emplace_back(std::move(NewEnumItem));
	return true;
}

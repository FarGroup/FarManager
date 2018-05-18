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

#include "plugsettings.hpp"

#include "ctrlobj.hpp"
#include "history.hpp"
#include "FarGuid.hpp"
#include "shortcuts.hpp"
#include "dizlist.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "plugins.hpp"
#include "sqlitedb.hpp"
#include "configdb.hpp"
#include "global.hpp"

#include "common/bytes_view.hpp"

const wchar_t* AbstractSettings::Add(const string& String)
{
	return static_cast<const wchar_t*>(Add(String.data(), (String.size() + 1) * sizeof(wchar_t)));
}

const void* AbstractSettings::Add(const void* Data, size_t Size)
{
	return memcpy(Allocate(Size), Data, Size);
}

void* AbstractSettings::Allocate(size_t Size)
{
	m_Data.emplace_back(Size);
	return m_Data.back().get();
}

class PluginSettings: public AbstractSettings
{
public:
	PluginSettings(const GUID& Guid, bool Local);

	bool IsValid() const override;
	bool Set(const FarSettingsItem& Item) override;
	bool Get(FarSettingsItem& Item) override;
	bool Enum(FarSettingsEnum& Enum) override;
	bool Delete(const FarSettingsValue& Value) override;
	int SubKey(const FarSettingsValue& Value, bool bCreate) override;

	class FarSettingsNameItems;

private:
	std::vector<FarSettingsNameItems> m_Enum;
	std::vector<HierarchicalConfig::key> m_Keys;
	HierarchicalConfigUniquePtr PluginsCfg;
};


AbstractSettings* AbstractSettings::CreatePluginSettings(const GUID& Guid, bool Local)
{
	return new PluginSettings(Guid, Local);
}


PluginSettings::PluginSettings(const GUID& Guid, bool Local)
{
	const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(Guid);
	if (!pPlugin)
		return;

	const auto strGuid = GuidToStr(Guid);
	PluginsCfg = ConfigProvider().CreatePluginsConfig(strGuid, Local);
	m_Keys.emplace_back(PluginsCfg->CreateKey(HierarchicalConfig::root_key(), strGuid, &pPlugin->GetTitle()));

	if (!Global->Opt->ReadOnlyConfig)
	{
		DizList Diz;
		const auto DbPath = path::join(Local? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath, L"PluginsData");
		Diz.Read(DbPath);
		const auto DbName = strGuid + L".db";
		const auto Description = concat(pPlugin->GetTitle(), L" ("sv, pPlugin->GetDescription(), L')');
		if (Description != NullToEmpty(Diz.Get(DbName, L"", 0)))
		{
			Diz.Set(DbName, L"", Description);
			Diz.Flush(DbPath);
		}
	}
}

bool PluginSettings::IsValid() const
{
	return !m_Keys.empty();
}

bool PluginSettings::Set(const FarSettingsItem& Item)
{
	if (Item.Root >= m_Keys.size())
		return false;

	switch(Item.Type)
	{
	case FST_SUBKEY:
		return false;

	case FST_QWORD:
		return PluginsCfg->SetValue(m_Keys[Item.Root], Item.Name, Item.Number);

	case FST_STRING:
		return PluginsCfg->SetValue(m_Keys[Item.Root], Item.Name, Item.String);

	case FST_DATA:
		return PluginsCfg->SetValue(m_Keys[Item.Root], Item.Name, bytes_view(Item.Data.Data, Item.Data.Size));

	default:
		return false;
	}
}

bool PluginSettings::Get(FarSettingsItem& Item)
{
	if (Item.Root >= m_Keys.size())
		return false;

	switch(Item.Type)
	{
	case FST_SUBKEY:
		return false;

	case FST_QWORD:
		{
			unsigned long long value;
			if (PluginsCfg->GetValue(m_Keys[Item.Root], Item.Name, value))
			{
				Item.Number = value;
				return true;
			}
		}
		break;

	case FST_STRING:
		{
			string data;
			if (PluginsCfg->GetValue(m_Keys[Item.Root], Item.Name, data))
			{
				Item.String = Add(data);
				return true;
			}
		}
		break;

	case FST_DATA:
		{
			bytes data;
			if (PluginsCfg->GetValue(m_Keys[Item.Root], Item.Name, data))
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

class PluginSettings::FarSettingsNameItems
{
public:
	NONCOPYABLE(FarSettingsNameItems);
	MOVABLE(FarSettingsNameItems);

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

class FarSettings: public AbstractSettings
{
public:
	bool IsValid() const override { return true; }
	bool Set(const FarSettingsItem& Item) override;
	bool Get(FarSettingsItem& Item) override;
	bool Enum(FarSettingsEnum& Enum) override;
	bool Delete(const FarSettingsValue& Value) override;
	int SubKey(const FarSettingsValue& Value, bool bCreate) override;

	class FarSettingsHistoryItems;

private:
	bool FillHistory(int Type, const string& HistoryName, FarSettingsEnum& Enum, const std::function<bool(history_record_type)>& Filter);
	std::vector<FarSettingsHistoryItems> m_Enum;
	std::vector<string> m_Keys;
};


AbstractSettings* AbstractSettings::CreateFarSettings()
{
	return new FarSettings();
}


class FarSettings::FarSettingsHistoryItems
{
public:
	NONCOPYABLE(FarSettingsHistoryItems);
	MOVABLE(FarSettingsHistoryItems);

	FarSettingsHistoryItems() = default;

	void add(FarSettingsHistory& Item, string&& Name, string&& Param, string&& File, const GUID& Guid)
	{
		m_Names.emplace_front(std::move(Name));
		m_Params.emplace_front(std::move(Param));
		m_Files.emplace_front(std::move(File));
		Item.Name = m_Names.front().c_str();
		Item.Param = m_Params.front().c_str();
		Item.File = m_Files.front().c_str();
		Item.PluginId = Guid;
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

bool PluginSettings::Enum(FarSettingsEnum& Enum)
{
	if (Enum.Root >= m_Keys.size())
		return false;

	FarSettingsName item;
	item.Type=FST_SUBKEY;

	FarSettingsNameItems NewEnumItem;

	const auto& root = m_Keys[Enum.Root];

	for(auto& i: PluginsCfg->KeysEnumerator(root))
	{
		NewEnumItem.add(item, std::move(i));
	}

	for(auto& i: PluginsCfg->ValuesEnumerator(root))
	{
		switch (static_cast<SQLiteDb::column_type>(i.second))
		{
		case SQLiteDb::column_type::integer:
			item.Type = FST_QWORD;
			break;

		case SQLiteDb::column_type::string:
			item.Type = FST_STRING;
			break;

		case SQLiteDb::column_type::blob:
			item.Type = FST_DATA;
			break;

		case SQLiteDb::column_type::unknown:
		default:
			item.Type = FST_UNKNOWN;
			break;
		}
		if(item.Type!=FST_UNKNOWN)
		{
			NewEnumItem.add(item, std::move(i.first));
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

	return Value.Value?
		PluginsCfg->DeleteValue(m_Keys[Value.Root], Value.Value) :
		PluginsCfg->DeleteKeyTree(m_Keys[Value.Root]);
}

int PluginSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	//Don't allow illegal key names - empty names or with backslashes
	if (Value.Root >= m_Keys.size() || !Value.Value || !*Value.Value || contains(Value.Value, '\\'))
		return 0;

	const auto root = bCreate? PluginsCfg->CreateKey(m_Keys[Value.Root], Value.Value) : PluginsCfg->FindByName(m_Keys[Value.Root], Value.Value);
	if (!root)
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
	const auto& FilterNone = [](history_record_type) { return true; };

	switch(Enum.Root)
	{
	case FSSF_HISTORY_CMD:
		return FillHistory(HISTORYTYPE_CMD,L"", Enum, FilterNone);
	
	case FSSF_HISTORY_FOLDER:
		return FillHistory(HISTORYTYPE_FOLDER, L"", Enum, FilterNone);
	
	case FSSF_HISTORY_VIEW:
		return FillHistory(HISTORYTYPE_VIEW, L"", Enum, [](history_record_type Type) { return Type == HR_VIEWER; });
	
	case FSSF_HISTORY_EDIT:
		return FillHistory(HISTORYTYPE_VIEW, L"", Enum, [](history_record_type Type) { return Type == HR_EDITOR || Type == HR_EDITOR_RO; });
	
	case FSSF_HISTORY_EXTERNAL:
		return FillHistory(HISTORYTYPE_VIEW, L"", Enum, [](history_record_type Type) { return Type == HR_EXTERNAL || Type == HR_EXTERNAL_WAIT; });
	
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
				NewEnumItem.add(item, std::move(i.Folder), std::move(i.PluginData), std::move(i.PluginFile), i.PluginGuid);
			}
			NewEnumItem.get(Enum);
			m_Enum.emplace_back(std::move(NewEnumItem));
			return true;
		}

	default:
		if(Enum.Root >= FSSF_COUNT)
		{
			size_t root = Enum.Root - FSSF_COUNT;
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

	m_Keys.emplace_back(Value.Value);
	return static_cast<int>(m_Keys.size() - 1 + FSSF_COUNT);
}

static const auto& HistoryRef(int Type)
{
	const auto& IsSave = [](int Type) -> bool
	{
		switch (Type)
		{
		case HISTORYTYPE_CMD: return Global->Opt->SaveHistory;
		case HISTORYTYPE_FOLDER: return Global->Opt->SaveFoldersHistory;
		case HISTORYTYPE_VIEW: return Global->Opt->SaveViewHistory;
		case HISTORYTYPE_DIALOG: return Global->Opt->Dialogs.EditHistory;
		default: return true;
		}
	};

	return IsSave(Type)? ConfigProvider().HistoryCfg() : ConfigProvider().HistoryCfgMem();
}

bool FarSettings::FillHistory(int Type,const string& HistoryName,FarSettingsEnum& Enum, const std::function<bool(history_record_type)>& Filter)
{
	FarSettingsHistory item = {};
	FarSettingsHistoryItems NewEnumItem;

	for(auto& i: HistoryRef(Type)->Enumerator(Type, HistoryName))
	{
		if(Filter(i.Type))
		{
			item.Time = os::chrono::nt_clock::to_filetime(i.Time);
			item.Lock = i.Lock;
			GUID Guid;
			if (i.Guid.empty() || !StrToGuid(i.Guid, Guid))
				Guid = FarGuid;
			NewEnumItem.add(item, std::move(i.Name), std::move(i.Data), std::move(i.File), Guid);
		}
	}
	NewEnumItem.get(Enum);
	m_Enum.emplace_back(std::move(NewEnumItem));
	return true;
}

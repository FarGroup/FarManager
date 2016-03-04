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

#include "headers.hpp"
#pragma hdrstop

#include "plugsettings.hpp"
#include "ctrlobj.hpp"
#include "strmix.hpp"
#include "history.hpp"
#include "datetime.hpp"
#include "FarGuid.hpp"
#include "shortcuts.hpp"
#include "dizlist.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "plugins.hpp"
#include "sqlitedb.hpp"

wchar_t* AbstractSettings::Add(const string& String)
{
	const auto size = (String.size() + 1) * sizeof(wchar_t);
	return static_cast<wchar_t*>(memcpy(Allocate(size), String.data(), size));
}

void* AbstractSettings::Add(const void* Data, size_t Size)
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
	virtual ~PluginSettings();
	virtual bool IsValid() const override;
	virtual int Set(const FarSettingsItem& Item) override;
	virtual int Get(FarSettingsItem& Item) override;
	virtual int Enum(FarSettingsEnum& Enum) override;
	virtual int Delete(const FarSettingsValue& Value) override;
	virtual int SubKey(const FarSettingsValue& Value, bool bCreate) override;

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


PluginSettings::PluginSettings(const GUID& Guid, bool Local):
	PluginsCfg(nullptr)
{
	const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(Guid);
	if (pPlugin)
	{
		string strGuid = GuidToStr(Guid);
		PluginsCfg = ConfigProvider().CreatePluginsConfig(strGuid, Local);
		m_Keys.emplace_back(PluginsCfg->CreateKey(HierarchicalConfig::root_key(), strGuid, &pPlugin->GetTitle()));

		if (!Global->Opt->ReadOnlyConfig)
		{
			DizList Diz;
			string strDbPath = Local ? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath;
			AddEndSlash(strDbPath);
			strDbPath += L"PluginsData\\";
			Diz.Read(strDbPath);
			string strDbName = strGuid + L".db";
			string Description = string(pPlugin->GetTitle()) + L" (" + pPlugin->GetDescription() + L")";
			if(Description != NullToEmpty(Diz.Get(strDbName, L"", 0)))
			{
				Diz.Set(strDbName, L"", Description);
				Diz.Flush(strDbPath);
			}
		}
	}
}

PluginSettings::~PluginSettings()
{
}

bool PluginSettings::IsValid() const
{
	return !m_Keys.empty();
}

int PluginSettings::Set(const FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.size())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				if (PluginsCfg->SetValue(m_Keys[Item.Root],Item.Name,Item.Number))
					result=TRUE;
				break;
			case FST_STRING:
				if (PluginsCfg->SetValue(m_Keys[Item.Root],Item.Name,Item.String))
					result=TRUE;
				break;
			case FST_DATA:
				if (PluginsCfg->SetValue(m_Keys[Item.Root], Item.Name, blob(Item.Data.Data, Item.Data.Size)))
					result = TRUE;
				break;
			default:
				break;
		}
	}
	return result;
}

int PluginSettings::Get(FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.size())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				{
					unsigned __int64 value;
					if (PluginsCfg->GetValue(m_Keys[Item.Root], Item.Name, value))
					{
						result=TRUE;
						Item.Number=value;
					}
				}
				break;
			case FST_STRING:
				{
					string data;
					if (PluginsCfg->GetValue(m_Keys[Item.Root],Item.Name,data))
					{
						result=TRUE;
						Item.String = Add(data);
					}
				}
				break;
			case FST_DATA:
				{
					writable_blob data;
					if (PluginsCfg->GetValue(m_Keys[Item.Root], Item.Name, data))
					{
						result = TRUE;
						Item.Data.Data = Add(data.data(), data.size());
						Item.Data.Size = data.size();
					}
				}
				break;
			default:
				break;
		}
	}
	return result;
}

static wchar_t* AddString(const string& String)
{
	const auto result = new wchar_t[String.size() + 1];
	std::copy(ALL_CONST_RANGE(String), result);
	result[String.size()] = 0;
	return result;
}

class PluginSettings::FarSettingsNameItems: ::noncopyable, swapable<FarSettingsNameItems>
{
public:
	FarSettingsNameItems() {}
	FarSettingsNameItems(FarSettingsNameItems&& rhs) noexcept { *this = std::move(rhs); }
	~FarSettingsNameItems()
	{
		std::for_each(CONST_RANGE(Items, i)
		{
			delete[] i.Name;
		});
	}

	MOVE_OPERATOR_BY_SWAP(FarSettingsNameItems);

	void swap(FarSettingsNameItems& rhs) noexcept
	{
		Items.swap(rhs.Items);
	}

	void add(FarSettingsName& Item, const string& String);

	void get(FarSettingsEnum& e) const
	{
		e.Count = Items.size();
		e.Items = e.Count ? Items.data() : nullptr;
	}

private:
	std::vector<FarSettingsName> Items;
};

void PluginSettings::FarSettingsNameItems::add(FarSettingsName& Item, const string& String)
{
	Item.Name=AddString(String);
	Items.emplace_back(Item);
}

class FarSettings: public AbstractSettings
{
public:
	FarSettings();
	virtual ~FarSettings();
	virtual bool IsValid() const override { return true; }
	virtual int Set(const FarSettingsItem& Item) override;
	virtual int Get(FarSettingsItem& Item) override;
	virtual int Enum(FarSettingsEnum& Enum) override;
	virtual int Delete(const FarSettingsValue& Value) override;
	virtual int SubKey(const FarSettingsValue& Value, bool bCreate) override;

	class FarSettingsHistoryItems;

private:
	int FillHistory(int Type, const string& HistoryName, FarSettingsEnum& Enum, const std::function<bool(history_record_type)>& Filter);
	std::vector<FarSettingsHistoryItems> m_Enum;
	std::vector<string> m_Keys;
};


AbstractSettings* AbstractSettings::CreateFarSettings()
{
	return new FarSettings();
}


class FarSettings::FarSettingsHistoryItems: ::noncopyable, swapable<FarSettingsHistoryItems>
{
public:
	FarSettingsHistoryItems() {}
	FarSettingsHistoryItems(FarSettingsHistoryItems&& rhs) noexcept { *this = std::move(rhs); }
	~FarSettingsHistoryItems()
	{
		std::for_each(CONST_RANGE(Items, i)
		{
			delete[] i.Name;
			delete[] i.Param;
			delete[] i.File;
		});
	}

	MOVE_OPERATOR_BY_SWAP(FarSettingsHistoryItems);

	void swap(FarSettingsHistoryItems& rhs) noexcept
	{
		Items.swap(rhs.Items);
	}

	void add(FarSettingsHistory& Item, const string& Name, const string& Param, const GUID& Guid, const string& File);

	void get(FarSettingsEnum& e) const
	{
		e.Count = Items.size();
		e.Histories = e.Count ? Items.data() : nullptr;
	}

private:
	std::vector<FarSettingsHistory> Items;
};

void FarSettings::FarSettingsHistoryItems::add(FarSettingsHistory& Item, const string& Name, const string& Param, const GUID& Guid, const string& File)
{
	Item.Name=AddString(Name);
	Item.Param=AddString(Param);
	Item.PluginId=Guid;
	Item.File=AddString(File);
	Items.emplace_back(Item);
}

int PluginSettings::Enum(FarSettingsEnum& Enum)
{
	int result=FALSE;
	if(Enum.Root<m_Keys.size())
	{
		FarSettingsName item;
		DWORD Index=0,Type;
		string strName;

		const auto& root = m_Keys[Enum.Root];
		item.Type=FST_SUBKEY;
		FarSettingsNameItems NewEnumItem;
		while (PluginsCfg->EnumKeys(root, Index++, strName))
		{
			NewEnumItem.add(item, strName);
		}
		Index=0;
		while (PluginsCfg->EnumValues(root, Index++, strName, Type))
		{
			item.Type=FST_UNKNOWN;
			switch (Type)
			{
			case SQLiteDb::TYPE_INTEGER:
				item.Type = FST_QWORD;
				break;
			case SQLiteDb::TYPE_STRING:
				item.Type = FST_STRING;
				break;
			case SQLiteDb::TYPE_BLOB:
				item.Type = FST_DATA;
				break;
			}
			if(item.Type!=FST_UNKNOWN)
			{
				NewEnumItem.add(item, strName);
			}
		}
		NewEnumItem.get(Enum);
		m_Enum.emplace_back(std::move(NewEnumItem));
		result=TRUE;
	}
	return result;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	int result=FALSE;
	if(Value.Root<m_Keys.size())
	{
		if (!Value.Value)
		{
			if (PluginsCfg->DeleteKeyTree(m_Keys[Value.Root]))
				result=TRUE;
		}
		else
		{
			if (PluginsCfg->DeleteValue(m_Keys[Value.Root],Value.Value))
				result=TRUE;
		}
	}
	return result;
}

int PluginSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	int result=0;
	//Don't allow illegal key names - empty names or with backslashes
	if(Value.Root<m_Keys.size() && Value.Value && *Value.Value && !wcschr(Value.Value,'\\'))
	{
		if (const auto root = bCreate? PluginsCfg->CreateKey(m_Keys[Value.Root], Value.Value) : PluginsCfg->FindByName(m_Keys[Value.Root], Value.Value))
		{
			result=static_cast<int>(m_Keys.size());
			m_Keys.emplace_back(root);
		}
	}
	return result;
}

FarSettings::FarSettings()
{
}

FarSettings::~FarSettings()
{
}

int FarSettings::Set(const FarSettingsItem& Item)
{
	return FALSE;
}

int FarSettings::Get(FarSettingsItem& Item)
{
	if (const auto Data = Global->Opt->GetConfigValue(Item.Root, Item.Name))
	{
		Data->Export(Item);

		if (Item.Type == FST_STRING)
			Item.String = Add(Item.String);

		return TRUE;
	}
	return FALSE;
}

int FarSettings::Enum(FarSettingsEnum& Enum)
{
	const auto FilterNone = [](history_record_type) { return true; };

	switch(Enum.Root)
	{
		case FSSF_HISTORY_CMD:
			return FillHistory(HISTORYTYPE_CMD,L"",Enum,FilterNone);
		case FSSF_HISTORY_FOLDER:
			return FillHistory(HISTORYTYPE_FOLDER,L"",Enum,FilterNone);
		case FSSF_HISTORY_VIEW:
			return FillHistory(HISTORYTYPE_VIEW,L"",Enum, [](history_record_type Type) { return Type == HR_VIEWER; });
		case FSSF_HISTORY_EDIT:
			return FillHistory(HISTORYTYPE_VIEW,L"",Enum, [](history_record_type Type) { return Type == HR_EDITOR || Type == HR_EDITOR_RO; });
		case FSSF_HISTORY_EXTERNAL:
			return FillHistory(HISTORYTYPE_VIEW,L"",Enum, [](history_record_type Type) { return Type == HR_EXTERNAL || Type == HR_EXTERNAL_WAIT; });
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
				FarSettingsHistory item = {};
				string strName,strFile,strData;
				GUID plugin; size_t index=0;
				FarSettingsHistoryItems NewEnumItem;
				while (Shortcuts().Get(Enum.Root - FSSF_FOLDERSHORTCUT_0, index++, &strName, &plugin, &strFile, &strData))
				{
					NewEnumItem.add(item, strName, strData, plugin, strFile);
				}
				NewEnumItem.get(Enum);
				m_Enum.emplace_back(std::move(NewEnumItem));
				return TRUE;
			}
			break;
		default:
			if(Enum.Root>=FSSF_COUNT)
			{
				size_t root=Enum.Root-FSSF_COUNT;
				if(root < m_Keys.size())
				{
					return FillHistory(HISTORYTYPE_DIALOG, m_Keys[root], Enum, FilterNone);
				}
			}
	}
	return FALSE;
}

int FarSettings::Delete(const FarSettingsValue& Value)
{
	return FALSE;
}

int FarSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	if(bCreate||Value.Root!=FSSF_ROOT) return 0;
	int result=static_cast<int>(m_Keys.size());
	m_Keys.emplace_back(Value.Value);
	return result+FSSF_COUNT;
}

static const std::unique_ptr<HistoryConfig>& HistoryRef(int Type)
{
	bool Save=true;
	switch(Type)
	{
		case HISTORYTYPE_CMD:
			Save=Global->Opt->SaveHistory;
			break;
		case HISTORYTYPE_FOLDER:
			Save=Global->Opt->SaveFoldersHistory;
			break;
		case HISTORYTYPE_VIEW:
			Save=Global->Opt->SaveViewHistory;
			break;
		case HISTORYTYPE_DIALOG:
			Save=Global->Opt->Dialogs.EditHistory;
			break;
	}
	return Save? ConfigProvider().HistoryCfg() : ConfigProvider().HistoryCfgMem();
}

int FarSettings::FillHistory(int Type,const string& HistoryName,FarSettingsEnum& Enum, const std::function<bool(history_record_type)>& Filter)
{
	FarSettingsHistory item = {};
	DWORD Index=0;
	string strName,strGuid,strFile,strData;

	unsigned __int64 id;
	history_record_type HType;
	bool HLock;
	unsigned __int64 Time;
	FarSettingsHistoryItems NewEnumItem;
	while (HistoryRef(Type)->Enum(Index++, Type, HistoryName, &id, strName, &HType, &HLock, &Time, strGuid, strFile, strData))
	{
		if(Filter(HType))
		{
			item.Time = UI64ToFileTime(Time);
			item.Lock=HLock;
			GUID Guid;
			if(strGuid.empty()||!StrToGuid(strGuid,Guid)) Guid=FarGuid;
			NewEnumItem.add(item, strName, strData, Guid, strFile);
		}
	}
	NewEnumItem.get(Enum);
	m_Enum.emplace_back(std::move(NewEnumItem));
	return TRUE;
}

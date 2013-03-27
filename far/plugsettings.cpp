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

char* AbstractSettings::Add(const string& String)
{
	return Add(String.CPtr(),(String.GetLength()+1)*sizeof(wchar_t));
}

char* AbstractSettings::Add(const wchar_t* Data,size_t Size)
{
	char* result=Add(Size);
	memcpy(result,Data,Size);
	return result;
}

char* AbstractSettings::Add(size_t Size)
{
	m_Data.emplace_back(Size);
	return m_Data.back().get();
}

bool AbstractSettings::IsValid(void)
{
	return true;
}

PluginSettings::PluginSettings(const GUID& Guid, bool Local):
	PluginsCfg(nullptr)
{
	Plugin* pPlugin = Global->CtrlObject->Plugins->FindPlugin(Guid);
	if (pPlugin)
	{
		string strGuid = GuidToStr(Guid);
		PluginsCfg = Global->Db->CreatePluginsConfig(strGuid, Local);
		m_Keys.resize(1);
		unsigned __int64& root = m_Keys.front();
		root=PluginsCfg->CreateKey(0, strGuid, pPlugin->GetTitle());

		if (!Global->Opt->ReadOnlyConfig)
		{
			DizList Diz;
			string strDbPath = Local ? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath;
			AddEndSlash(strDbPath);
			strDbPath += L"PluginsData\\";
			Diz.Read(strDbPath);
			string strDbName = strGuid + L".db";
			string Description = string(pPlugin->GetTitle()) + L" (" + pPlugin->GetDescription() + L")";
			if(StrCmp(Diz.GetDizTextAddr(strDbName, L"", 0), Description))
			{
				Diz.AddDizText(strDbName, L"", Description);
				Diz.Flush(strDbPath);
			}
		}
	}
}

bool PluginSettings::IsValid(void)
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
				if (PluginsCfg->SetValue(m_Keys[Item.Root],Item.Name,Item.Number)) result=TRUE;
				break;
			case FST_STRING:
				//не даём изменить "description" корня, он выставляется в plugin title фаром
				if (Item.Root==0 && !Item.Name) break;
				if (PluginsCfg->SetValue(m_Keys[Item.Root],Item.Name,Item.String)) result=TRUE;
				break;
			case FST_DATA:
				if (PluginsCfg->SetValue(m_Keys[Item.Root],Item.Name,(const char *)Item.Data.Data,(int)Item.Data.Size)) result=TRUE;
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
					if (PluginsCfg->GetValue(m_Keys[Item.Root],Item.Name,&value))
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
						Item.String=(wchar_t*)Add(data);
					}
				}
				break;
			case FST_DATA:
				{
					int size=PluginsCfg->GetValue(m_Keys[Item.Root],Item.Name,nullptr,0);
					if (size)
					{
						char* data=Add(size);
						int checkedSize=PluginsCfg->GetValue(m_Keys[Item.Root],Item.Name,data,size);
						if (size==checkedSize)
						{
							result=TRUE;
							Item.Data.Data=data;
							Item.Data.Size=size;
						}
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
	size_t size=String.GetLength()+1;
	wchar_t* result=new wchar_t[size];
	wmemcpy(result,String.CPtr(),size);
	return result;
}

static void AddItem(FarSettingsNameItems* Array, FarSettingsName& Item, const string& String)
{
	Item.Name=AddString(String);
	Array->Items.emplace_back(Item);
}

static void AddItem(FarSettingsHistoryItems* Array, FarSettingsHistory& Item, const string& Name, const string& Param, const GUID& Guid, const string& File)
{
	Item.Name=AddString(Name);
	Item.Param=AddString(Param);
	Item.PluginId=Guid;
	Item.File=AddString(File);
	Array->Items.emplace_back(Item);
}

int PluginSettings::Enum(FarSettingsEnum& Enum)
{
	int result=FALSE;
	if(Enum.Root<m_Keys.size())
	{
		m_Enum.emplace_back(new FarSettingsNameItems);
		FarSettingsName item;
		DWORD Index=0,Type;
		string strName;

		unsigned __int64 root = m_Keys[Enum.Root];
		item.Type=FST_SUBKEY;
		while (PluginsCfg->EnumKeys(root,Index++,strName))
		{
			AddItem(m_Enum.back().get(),item,strName);
		}
		Index=0;
		while (PluginsCfg->EnumValues(root,Index++,strName,&Type))
		{
			item.Type=FST_UNKNOWN;
			switch (Type)
			{
				case HierarchicalConfig::TYPE_INTEGER:
					item.Type=FST_QWORD;
					break;
				case HierarchicalConfig::TYPE_STRING:
					item.Type=FST_STRING;
					break;
				case HierarchicalConfig::TYPE_BLOB:
					item.Type=FST_DATA;
					break;
			}
			if(item.Type!=FST_UNKNOWN)
			{
				AddItem(m_Enum.back().get(),item,strName);
			}
		}
		if (!m_Enum.back()->Items.empty())
		{
			Enum.Count = m_Enum.back()->Items.size();
			Enum.Items = m_Enum.back()->Items.data();
		}
		else
		{
			Enum.Count = 0;
			Enum.Items = nullptr;
		}
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
	if(Value.Root<m_Keys.size() && !wcschr(Value.Value,'\\'))
	{
		unsigned __int64 root = 0;
		if (bCreate)
			root = PluginsCfg->CreateKey(m_Keys[Value.Root],Value.Value);
		else
			root = PluginsCfg->GetKeyID(m_Keys[Value.Root],Value.Value);
		if (root)
		{
			result=static_cast<int>(m_Keys.size());
			m_Keys.emplace_back(root);
		}
	}
	return result;
}

int FarSettings::Set(const FarSettingsItem& Item)
{
	return FALSE;
}

int FarSettings::Get(FarSettingsItem& Item)
{
	Option::OptionType Type;
	Option* Data;
	if(GetConfigValue(Item.Root,Item.Name,Type,Data))
	{
		Item.Type=FST_UNKNOWN;
		switch(Type)
		{
			case Option::TYPE_BOOLEAN:
				Item.Type=FST_QWORD;
				Item.Number=static_cast<BoolOption*>(Data)->Get();
				break;
			case Option::TYPE_BOOLEAN3:
				Item.Type=FST_QWORD;
				Item.Number=static_cast<Bool3Option*>(Data)->Get();
				break;
			case Option::TYPE_INTEGER:
				Item.Type=FST_QWORD;
				Item.Number=static_cast<IntOption*>(Data)->Get();
				break;
			case Option::TYPE_STRING:
				Item.Type=FST_STRING;
				Item.String=(wchar_t*)Add(static_cast<StringOption*>(Data)->Get());
				break;
		}
		if(FST_UNKNOWN!=Item.Type) return TRUE;
	}
	return FALSE;
}

static bool FilterNone(int)
{
	return true;
}

static bool FilterView(int Type)
{
	return (Type==0)?true:false;
}

static bool FilterEdit(int Type)
{
	return (Type==1||Type==4)?true:false;
}

static bool FilterExt(int Type)
{
	return (Type==2||Type==3)?true:false;
}

int FarSettings::Enum(FarSettingsEnum& Enum)
{
	switch(Enum.Root)
	{
		case FSSF_HISTORY_CMD:
			return FillHistory(HISTORYTYPE_CMD,L"",Enum,FilterNone);
		case FSSF_HISTORY_FOLDER:
			return FillHistory(HISTORYTYPE_FOLDER,L"",Enum,FilterNone);
		case FSSF_HISTORY_VIEW:
			return FillHistory(HISTORYTYPE_VIEW,L"",Enum,FilterView);
		case FSSF_HISTORY_EDIT:
			return FillHistory(HISTORYTYPE_VIEW,L"",Enum,FilterEdit);
		case FSSF_HISTORY_EXTERNAL:
			return FillHistory(HISTORYTYPE_VIEW,L"",Enum,FilterExt);
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
				m_Enum.emplace_back(new FarSettingsHistoryItems);
				FarSettingsHistory item={0};
				string strName,strFile,strData;
				GUID plugin; size_t index=0;
				while(Shortcuts().Get(Enum.Root-FSSF_FOLDERSHORTCUT_0,index++,&strName,&plugin,&strFile,&strData))
				{
					AddItem(m_Enum.back().get(),item,strName,strData,plugin,strFile);
				}
				if (!m_Enum.back()->Items.empty())
				{
					Enum.Count = m_Enum.back()->Items.size();
					Enum.Histories = m_Enum.back()->Items.data();
				}
				else
				{
					Enum.Count = 0;
					Enum.Items = nullptr;
				}
				return TRUE;
			}
			break;
		default:
			if(Enum.Root>=FSSF_COUNT)
			{
				size_t root=Enum.Root-FSSF_COUNT;
				if((size_t)root<m_Keys.size())
				{
					return FillHistory(HISTORYTYPE_DIALOG,*m_Keys[root],Enum,FilterNone);
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
	m_Keys.emplace_back(new string(Value.Value));
	return result+FSSF_COUNT;
}

static HistoryConfig* HistoryRef(int Type)
{
	int Save=true;
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
	return Save? Global->Db->HistoryCfg().get() : Global->Db->HistoryCfgMem().get();
}

int FarSettings::FillHistory(int Type,const string& HistoryName,FarSettingsEnum& Enum,HistoryFilter Filter)
{
	m_Enum.emplace_back(new FarSettingsHistoryItems);
	FarSettingsHistory item={0};
	DWORD Index=0;
	string strName,strGuid,strFile,strData;

	unsigned __int64 id;
	int HType;
	bool HLock;
	unsigned __int64 Time;
	while(HistoryRef(Type)->Enum(Index++,Type,HistoryName,&id,strName,&HType,&HLock,&Time,strGuid,strFile,strData,false))
	{
		if(Filter(HType))
		{
			UI64ToFileTime(Time,&item.Time);
			item.Lock=HLock;
			GUID Guid;
			if(strGuid.IsEmpty()||!StrToGuid(strGuid,Guid)) Guid=FarGuid;
			AddItem(m_Enum.back().get(),item,strName,strData,Guid,strFile);
		}
	}
	if (!m_Enum.back()->Items.empty())
	{
		Enum.Count = m_Enum.back()->Items.size();
		Enum.Histories = m_Enum.back()->Items.data();
	}
	else
	{
		Enum.Count = 0;
		Enum.Items = nullptr;
	}
	return TRUE;
}

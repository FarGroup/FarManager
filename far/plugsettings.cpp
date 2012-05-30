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

template<> void DeleteItems<FarSettingsHistory>(FarSettingsHistory* Items,size_t Size)
{
	for(size_t ii=0;ii<Size;++ii)
	{
		delete [] Items[ii].Name;
		delete [] Items[ii].Param;
		delete [] Items[ii].File;
	}
}

AbstractSettings::~AbstractSettings()
{
	for(size_t ii=0;ii<m_Data.getCount();++ii)
	{
		delete [] *m_Data.getItem(ii);
	}
}

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
	char** item=m_Data.addItem();
	*item=new char[Size];
	return *item;
}

bool AbstractSettings::IsValid(void)
{
	return true;
}

PluginSettings::PluginSettings(const GUID& Guid, bool Local) : PluginsCfg(nullptr)
{
	//хак чтоб SCTL_* могли работать при ExitFarW.
	extern PluginManager *PluginManagerForExitFar;
	Plugin* pPlugin=CtrlObject?CtrlObject->Plugins->FindPlugin(Guid):(PluginManagerForExitFar?PluginManagerForExitFar->FindPlugin(Guid):nullptr);
	if (pPlugin)
	{
		string strGuid = GuidToStr(Guid);
		PluginsCfg = CreatePluginsConfig(strGuid, Local);
		unsigned __int64& root(*m_Keys.insertItem(0));
		root=PluginsCfg->CreateKey(0, strGuid, pPlugin->GetTitle());

		if (!Opt.ReadOnlyConfig)
		{
			DizList Diz;
			string strDbPath = Local ? Opt.LocalProfilePath : Opt.ProfilePath;
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

PluginSettings::~PluginSettings()
{
	if (PluginsCfg)
		delete PluginsCfg;
}

bool PluginSettings::IsValid(void)
{
	return m_Keys.getCount()!=0;
}

int PluginSettings::Set(const FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				if (PluginsCfg->SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.Number)) result=TRUE;
				break;
			case FST_STRING:
				//не даём изменить "description" корня, он выставляется в plugin title фаром
				if (Item.Root==0 && !Item.Name) break;
				if (PluginsCfg->SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.String)) result=TRUE;
				break;
			case FST_DATA:
				if (PluginsCfg->SetValue(*m_Keys.getItem(Item.Root),Item.Name,(const char *)Item.Data.Data,(int)Item.Data.Size)) result=TRUE;
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
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				{
					unsigned __int64 value;
					if (PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,&value))
					{
						result=TRUE;
						Item.Number=value;
					}
				}
				break;
			case FST_STRING:
				{
					string data;
					if (PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,data))
					{
						result=TRUE;
						Item.String=(wchar_t*)Add(data);
					}
				}
				break;
			case FST_DATA:
				{
					int size=PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,nullptr,0);
					if (size)
					{
						char* data=Add(size);
						int checkedSize=PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,data,size);
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

static void AddItem(Vector<FarSettingsName>& Array, FarSettingsName& Item, const string& String)
{
	Item.Name=AddString(String);
	Array.AddItem(Item);
}

static void AddItem(Vector<FarSettingsHistory>& Array, FarSettingsHistory& Item, const string& Name, const string& Param, const GUID& Guid, const string& File)
{
	Item.Name=AddString(Name);
	Item.Param=AddString(Param);
	Item.PluginId=Guid;
	Item.File=AddString(File);
	Array.AddItem(Item);
}

int PluginSettings::Enum(FarSettingsEnum& Enum)
{
	int result=FALSE;
	if(Enum.Root<m_Keys.getCount())
	{
		Vector<FarSettingsName>& array=*m_Enum.addItem();
		FarSettingsName item;
		DWORD Index=0,Type;
		string strName;

		unsigned __int64 root = *m_Keys.getItem(Enum.Root);
		item.Type=FST_SUBKEY;
		while (PluginsCfg->EnumKeys(root,Index++,strName))
		{
			AddItem(array,item,strName);
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
				AddItem(array,item,strName);
			}
		}
		Enum.Count=array.GetSize();
		Enum.Items=array.GetItems();
		result=TRUE;
	}
	return result;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	int result=FALSE;
	if(Value.Root<m_Keys.getCount())
	{
		if (!Value.Value)
		{
			if (PluginsCfg->DeleteKeyTree(*m_Keys.getItem(Value.Root)))
				result=TRUE;
		}
		else
		{
			if (PluginsCfg->DeleteValue(*m_Keys.getItem(Value.Root),Value.Value))
				result=TRUE;
		}
	}
	return result;
}

int PluginSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	int result=0;
	if(Value.Root<m_Keys.getCount()&&!wcschr(Value.Value,'\\'))
	{
		unsigned __int64 root = 0;
		if (bCreate)
			root = PluginsCfg->CreateKey(*m_Keys.getItem(Value.Root),Value.Value);
		else
			root = PluginsCfg->GetKeyID(*m_Keys.getItem(Value.Root),Value.Value);
		if (root)
		{
			result=static_cast<int>(m_Keys.getCount());
			*m_Keys.insertItem(result) = root;
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
	GeneralConfig::OptionType Type;
	Option* Data;
	if(GetConfigValue(Item.Root,Item.Name,Type,Data))
	{
		Item.Type=FST_UNKNOWN;
		switch(Type)
		{
			case GeneralConfig::TYPE_BOOLEAN:
				Item.Type=FST_QWORD;
				Item.Number=static_cast<BoolOption*>(Data)->Get();
				break;
			case GeneralConfig::TYPE_INTEGER:
				Item.Type=FST_QWORD;
				Item.Number=static_cast<IntOption*>(Data)->Get();
				break;
			case GeneralConfig::TYPE_STRING:
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
				Vector<FarSettingsHistory>& array=*m_Enum.addItem();
				FarSettingsHistory item={0};
				string strName,strFile,strData;
				GUID plugin; size_t index=0;
				while(CtrlObject->FolderShortcuts->Get(Enum.Root-FSSF_FOLDERSHORTCUT_0,index++,&strName,&plugin,&strFile,&strData))
				{
					AddItem(array,item,strName,strData,plugin,strFile);
				}
				Enum.Count=array.GetSize();
				Enum.Histories=array.GetItems();
				return TRUE;
			}
			break;
		default:
			if(Enum.Root>=FSSF_COUNT)
			{
				size_t root=Enum.Root-FSSF_COUNT;
				if((size_t)root<m_Keys.getCount())
				{
					return FillHistory(HISTORYTYPE_DIALOG,*m_Keys.getItem(root),Enum,FilterNone);
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
	int result=static_cast<int>(m_Keys.getCount());
	*m_Keys.insertItem(result)=Value.Value;
	return result+FSSF_COUNT;
}

static HistoryConfig* HistoryRef(int Type)
{
	int Save=true;
	switch(Type)
	{
		case HISTORYTYPE_CMD:
			Save=Opt.SaveHistory;
			break;
		case HISTORYTYPE_FOLDER:
			Save=Opt.SaveFoldersHistory;
			break;
		case HISTORYTYPE_VIEW:
			Save=Opt.SaveViewHistory;
			break;
		case HISTORYTYPE_DIALOG:
			Save=Opt.Dialogs.EditHistory;
			break;
	}
	return Save?HistoryCfg:HistoryCfgMem;
}

int FarSettings::FillHistory(int Type,const string& HistoryName,FarSettingsEnum& Enum,HistoryFilter Filter)
{
	Vector<FarSettingsHistory>& array=*m_Enum.addItem();
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
			AddItem(array,item,strName,strData,Guid,strFile);
		}
	}
	Enum.Count=array.GetSize();
	Enum.Histories=array.GetItems();
	return TRUE;
}

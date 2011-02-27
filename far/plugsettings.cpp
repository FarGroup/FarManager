/*
plugsettings.hpp

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
#include "registry.hpp"
#include "strmix.hpp"

PluginSettings::PluginSettings(const GUID& Guid)
{
	Plugin* pPlugin=CtrlObject?CtrlObject->Plugins.FindPlugin(Guid):nullptr;
	if (pPlugin)
	{
		string& root(*m_Keys.insertItem(0));
		root=string(L"Plugins\\")+GuidToStr(Guid);
	}
}

PluginSettings::~PluginSettings()
{
	for(size_t ii=0;ii<m_Data.getCount();++ii)
	{
		delete [] *m_Data.getItem(ii);
	}
	for(size_t ii=0;ii<m_Enum.getCount();++ii)
	{
		FarSettingsName* array=m_Enum.getItem(ii)->GetItems();
		for(size_t jj=0;jj<m_Enum.getItem(ii)->GetSize();++jj)
		{
			delete [] array[jj].Name;
		}
	}
}

int PluginSettings::Set(const FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				{
					FarSettingsValue value={Item.Root,Item.Name};
					int key=SubKey(value);
					HKEY hKey=CreateRegKey(m_Keys.getItem(key)->CPtr());
					if (hKey)
					{
						result=TRUE;
						RegCloseKey(hKey);
					}
				}
				break;
			case FST_QWORD:
				if (SetRegKey64(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,Item.Number)==ERROR_SUCCESS) result=TRUE;
				break;
			case FST_STRING:
				if (SetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,Item.String)==ERROR_SUCCESS) result=TRUE;
				break;
			case FST_DATA:
				if (SetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,(const BYTE*)Item.Data.Data,static_cast<DWORD>(Item.Data.Size))==ERROR_SUCCESS) result=TRUE;
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
					__int64 value;
					if (GetRegKey64(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,value,0LL))
					{
						result=TRUE;
						Item.Number=value;
					}
				}
				break;
			case FST_STRING:
				{
					string data;
					if (GetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,data,L"",nullptr))
					{
						result=TRUE;
						char** item=m_Data.addItem();
						size_t size=(data.GetLength()+1)*sizeof(wchar_t);
						*item=new char[size];
						memcpy(*item,data.CPtr(),size);
						Item.String=(wchar_t*)*item;
					}
				}
				break;
			case FST_DATA:
				{
					int size=GetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,nullptr,nullptr,0,nullptr);
					if (size)
					{
						char** item=m_Data.addItem();
						*item=new char[size];
						int checkedSize=GetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,(BYTE*)*item,nullptr,size,nullptr);
						if (size==checkedSize)
						{
							result=TRUE;
							Item.Data.Data=*item;
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

static void AddString(Vector<FarSettingsName>& Array, FarSettingsName& Item, string& String)
{
	size_t size=String.GetLength()+1;
	Item.Name=new wchar_t[size];
	wmemcpy((wchar_t*)Item.Name,String.CPtr(),size);
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
		HKEY hKey=OpenRegKey(m_Keys.getItem(Enum.Root)->CPtr());
		string strName,strValue;

		if (hKey)
		{
			item.Type=FST_SUBKEY;
			while (apiRegEnumKeyEx(hKey,Index++,strName)==ERROR_SUCCESS)
			{
				AddString(array,item,strName);
			}
			RegCloseKey(hKey);
			Index=0;
			while(EnumRegValueEx(m_Keys.getItem(Enum.Root)->CPtr(),Index++,strName,strValue,nullptr,nullptr,&Type)!=REG_NONE)
			{
				item.Type=FST_UNKNOWN;
				switch(Type)
				{
					case REG_QWORD:
						item.Type=FST_QWORD;
						break;
					case REG_SZ:
						item.Type=FST_STRING;
						break;
					case REG_BINARY:
						item.Type=FST_DATA;
						break;
				}
				if(item.Type!=FST_UNKNOWN)
				{
					AddString(array,item,strName);
				}
			}
			Enum.Count=array.GetSize();
			Enum.Items=array.GetItems();
			result=TRUE;
		}
	}
	return result;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	int result=FALSE;
	if(Value.Root<m_Keys.getCount())
	{
		bool isValue=false;
		HKEY hKey=OpenRegKey(m_Keys.getItem(Value.Root)->CPtr());
		if (hKey)
		{
			DWORD QueryDataSize=0;
			if(ERROR_SUCCESS==RegQueryValueEx(hKey,Value.Value,0,nullptr,nullptr,&QueryDataSize)) isValue=true;
			RegCloseKey(hKey);
		}
		if (isValue)
		{
			DeleteRegValue(m_Keys.getItem(Value.Root)->CPtr(),Value.Value);
		}
		else
		{
			int key=SubKey(Value);
			DeleteKeyTree(m_Keys.getItem(key)->CPtr());
		}
		result=TRUE;
	}
	return result;
}

int PluginSettings::SubKey(const FarSettingsValue& Value)
{
	int result=0;
	if(Value.Root<m_Keys.getCount()&&!wcschr(Value.Value,'\\'))
	{
		result=static_cast<int>(m_Keys.getCount());
		string& root(*m_Keys.insertItem(result));
		root=*m_Keys.getItem(Value.Root)+L"\\"+Value.Value;
	}
	return result;
}

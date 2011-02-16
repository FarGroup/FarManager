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

#include "plugsettings.hpp"
#include "ctrlobj.hpp"
#include "registry.hpp"

PluginSettings::PluginSettings(const GUID& Guid)
{
	Plugin* pPlugin=CtrlObject?CtrlObject->Plugins.FindPlugin(Guid):nullptr;
	if (pPlugin)
	{
		string& root(*m_Keys.insertItem(0));
		root=string(L"Plugins\\")+pPlugin->GetTitle();
	}
}

PluginSettings::~PluginSettings()
{
	for(size_t ii=0;ii<m_Data.getCount();++ii)
	{
		delete [] *m_Data.getItem(ii);
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
			    //BUGBUG: NOT IMPLEMENTED
				break;
			case FST_QWORD:
				if (SetRegKey64(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,Item.Value.Number)==ERROR_SUCCESS) result=TRUE;
				break;
			case FST_STRING:
				if (SetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,Item.Value.String)==ERROR_SUCCESS) result=TRUE;
				break;
			case FST_DATA:
				if (SetRegKey(m_Keys.getItem(Item.Root)->CPtr(),Item.Name,(const BYTE*)Item.Value.Data.Data,Item.Value.Data.Size)==ERROR_SUCCESS) result=TRUE;
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
						Item.Value.Number=value;
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
						Item.Value.String=(wchar_t*)*item;
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
							Item.Value.Data.Data=*item;
							Item.Value.Data.Size=size;
						}
					}
				}
				break;
		}
	}
	return result;
}

int PluginSettings::Enum(FarSettingsEnum& Enum)
{
	int result=FALSE;
	if(Enum.Root<m_Keys.getCount())
	{
	    //BUGBUG: NOT IMPLEMENTED
	}
	return result;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	int result=FALSE;
	if(Value.Root<m_Keys.getCount())
	{
	    //BUGBUG: NOT IMPLEMENTED
	}
	return result;
}

int PluginSettings::SubKey(const FarSettingsValue& Value)
{
	int result=0;
	if(Value.Root<m_Keys.getCount()&&!wcschr(Value.Value,'\\'))
	{
		result=m_Keys.getCount();
		string& root(*m_Keys.insertItem(result));
		root=*m_Keys.getItem(Value.Root)+L"\\"+Value.Value;
	}
	return result;
}

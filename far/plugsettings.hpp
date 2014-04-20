#pragma once

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

#include "configdb.hpp"

class AbstractSettings: NonCopyable
{
public:
	virtual ~AbstractSettings(){};
	virtual bool IsValid() const = 0;
	virtual int Set(const FarSettingsItem& Item) = 0;
	virtual int Get(FarSettingsItem& Item) = 0;
	virtual int Enum(FarSettingsEnum& Enum) = 0;
	virtual int Delete(const FarSettingsValue& Value) = 0;
	virtual int SubKey(const FarSettingsValue& Value, bool bCreate) = 0;

protected:
	wchar_t* Add(const string& String);
	void* Add(size_t Size);

private:
	std::list<char_ptr> m_Data;
};

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
	std::vector<uint64_t> m_Keys;
	HierarchicalConfigUniquePtr PluginsCfg;
};


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

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
	virtual bool IsValid() const override;
	virtual int Set(const FarSettingsItem& Item) override;
	virtual int Get(FarSettingsItem& Item) override;
	virtual int Enum(FarSettingsEnum& Enum) override;
	virtual int Delete(const FarSettingsValue& Value) override;
	virtual int SubKey(const FarSettingsValue& Value, bool bCreate) override;

private:
	class FarSettingsNameItems: ::NonCopyable
	{
	public:
		FarSettingsNameItems() {}
		FarSettingsNameItems(FarSettingsNameItems&& rhs) { *this = std::move(rhs); }
		~FarSettingsNameItems()
		{
			std::for_each(CONST_RANGE(Items, i)
			{
				delete [] i.Name;
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
			e.Items = e.Count? Items.data() : nullptr;
		}

	private:
		std::vector<FarSettingsName> Items;
	};
	ALLOW_SWAP_ACCESS(FarSettingsNameItems);

	std::vector<FarSettingsNameItems> m_Enum;
	std::vector<unsigned __int64> m_Keys;
	HierarchicalConfigUniquePtr PluginsCfg;
};

STD_SWAP_SPEC(PluginSettings::FarSettingsNameItems);

class FarSettings: public AbstractSettings
{
public:
	virtual bool IsValid() const override { return true; }
	virtual int Set(const FarSettingsItem& Item) override;
	virtual int Get(FarSettingsItem& Item) override;
	virtual int Enum(FarSettingsEnum& Enum) override;
	virtual int Delete(const FarSettingsValue& Value) override;
	virtual int SubKey(const FarSettingsValue& Value, bool bCreate) override;

private:
	class FarSettingsHistoryItems: ::NonCopyable
	{
	public:
		FarSettingsHistoryItems() {}
		FarSettingsHistoryItems(FarSettingsHistoryItems&& rhs) { *this = std::move(rhs); }
		~FarSettingsHistoryItems()
		{
			std::for_each(CONST_RANGE(Items, i)
			{
				delete [] i.Name;
				delete [] i.Param;
				delete [] i.File;
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
			e.Histories = e.Count? Items.data() : nullptr;
		}
			
	private:
		std::vector<FarSettingsHistory> Items;
	};
	ALLOW_SWAP_ACCESS(FarSettingsHistoryItems);

	int FillHistory(int Type, const string& HistoryName, FarSettingsEnum& Enum, const std::function<bool(history_record_type)>& Filter);
	std::vector<FarSettingsHistoryItems> m_Enum;
	std::vector<string> m_Keys;
};

STD_SWAP_SPEC(FarSettings::FarSettingsHistoryItems);

/*
mix.cpp

Куча разных вспомогательных функций
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

#include "mix.hpp"

#include "pathmix.hpp"
#include "window.hpp"
#include "cmdline.hpp"
#include "dlgedit.hpp"
#include "strmix.hpp"

#include "platform.env.hpp"
#include "platform.fs.hpp"

#include "common/enum_substrings.hpp"

string MakeTemp(string_view Prefix, bool const WithTempPath, string_view const UserTempPath)
{
	static UINT s_shift = 0;

	if (Prefix.empty())
		Prefix = L"FAR"sv;

	auto strPath = L"."s;

	if (WithTempPath)
	{
		os::fs::GetTempPath(strPath);
	}
	else if(!UserTempPath.empty())
	{
		assign(strPath, UserTempPath);
	}

	AddEndSlash(strPath);

	wchar_t_ptr_n<MAX_PATH> Buffer(Prefix.size() + strPath.size() + 13);

	auto Unique = 23 * GetCurrentProcessId() + s_shift;
	const auto UniqueCopy = Unique? Unique : 1;
	s_shift = (s_shift + 1) % 23;

	null_terminated PrefixStr(Prefix);

	bool UseSystemFunction = true;

	const auto& Generator = [&]()
	{
		if (!UseSystemFunction || !GetTempFileName(strPath.c_str(), PrefixStr.c_str(), Unique, Buffer.get()))
		{
			// GetTempFileName uses only the last 16 bits of Unique.
			// We either did a full round trip through them or GetTempFileName failed for whatever reason.
			// Let's try the full 32-bit range manually.
			return path::join(strPath, concat(Prefix, to_hex_wstring(Unique), L".tmp"sv));
		}

		return string(Buffer.get());
	};

	for (;;)
	{
		if (!Unique) ++Unique;

		const auto Str = Generator();

		const auto Find = os::fs::enum_files(Str, false);
		if (Find.begin() == Find.end())
			return Str;

		if ((++Unique & 0xffff) == (UniqueCopy & 0xffff))
		{
			UseSystemFunction = false;
		}
	}
}

void PluginPanelItemToFindDataEx(const PluginPanelItem& Src, os::fs::find_data& Dest)
{
	Dest = {};
	Dest.CreationTime = os::chrono::nt_clock::from_filetime(Src.CreationTime);
	Dest.LastAccessTime = os::chrono::nt_clock::from_filetime(Src.LastAccessTime);
	Dest.LastWriteTime = os::chrono::nt_clock::from_filetime(Src.LastWriteTime);
	Dest.ChangeTime = os::chrono::nt_clock::from_filetime(Src.ChangeTime);
	Dest.FileSize = Src.FileSize;
	Dest.AllocationSize = Src.AllocationSize;
	Dest.FileName = NullToEmpty(Src.FileName);
	Dest.SetAlternateFileName(NullToEmpty(Src.AlternateFileName));
	Dest.Attributes = Src.FileAttributes;
}

void FindDataExToPluginPanelItemHolder(const os::fs::find_data& Src, PluginPanelItemHolder& Holder)
{
	auto& Dest = Holder.Item;
	Dest = {};

	Dest.CreationTime = os::chrono::nt_clock::to_filetime(Src.CreationTime);
	Dest.LastAccessTime = os::chrono::nt_clock::to_filetime(Src.LastAccessTime);
	Dest.LastWriteTime = os::chrono::nt_clock::to_filetime(Src.LastWriteTime);
	Dest.ChangeTime = os::chrono::nt_clock::to_filetime(Src.ChangeTime);
	Dest.FileSize = Src.FileSize;
	Dest.AllocationSize = Src.AllocationSize;
	Dest.FileAttributes = Src.Attributes;
	Dest.NumberOfLinks = 1;

	const auto& MakeCopy = [](string_view const Str)
	{
		auto Buffer = std::make_unique<wchar_t[]>(Str.size() + 1);
		*std::copy(ALL_CONST_RANGE(Str), Buffer.get()) = L'\0';
		return Buffer.release();
	};

	Dest.FileName = MakeCopy(Src.FileName);
	Dest.AlternateFileName = MakeCopy(Src.AlternateFileName());
}

PluginPanelItemHolder::~PluginPanelItemHolder()
{
	FreePluginPanelItemNames(Item);
}

void FreePluginPanelItemNames(const PluginPanelItem& Data)
{
	delete[] Data.FileName;
	delete[] Data.AlternateFileName;
}

void FreePluginPanelItemUserData(HANDLE hPlugin, const UserDataItem& Data)
{
	if (!Data.FreeData)
		return;

	FarPanelItemFreeInfo info{ sizeof(FarPanelItemFreeInfo), hPlugin };
	Data.FreeData(Data.Data, &info);
}

void FreePluginPanelItemDescriptionOwnerAndColumns(const PluginPanelItem & Data)
{
	delete[] Data.Description;
	delete[] Data.Owner;
	DeleteRawArray(make_range(Data.CustomColumnData, Data.CustomColumnNumber));
}

void FreePluginPanelItemsNames(const std::vector<PluginPanelItem>& Items)
{
	std::for_each(ALL_CONST_RANGE(Items), FreePluginPanelItemNames);
}

plugin_item_list::~plugin_item_list()
{
	FreePluginPanelItemsNames(m_Data);
}

const PluginPanelItem* plugin_item_list::data() const
{
	return m_Data.data();
}

PluginPanelItem* plugin_item_list::data()
{
	return m_Data.data();
}

size_t plugin_item_list::size() const
{
	return m_Data.size();
}

bool plugin_item_list::empty() const
{
	return m_Data.empty();
}

const std::vector<PluginPanelItem>& plugin_item_list::items() const
{
	return m_Data;
}

void plugin_item_list::emplace_back(const PluginPanelItem& Item)
{
	m_Data.emplace_back(Item);
}

void plugin_item_list::reserve(size_t const Size)
{
	m_Data.reserve(Size);
}

WINDOWINFO_TYPE WindowTypeToPluginWindowType(const int fType)
{
	static const std::pair<window_type, WINDOWINFO_TYPE> TypesMap[] =
	{
		{windowtype_desktop,    WTYPE_DESKTOP},
		{windowtype_panels,     WTYPE_PANELS},
		{windowtype_viewer,     WTYPE_VIEWER},
		{windowtype_editor,     WTYPE_EDITOR},
		{windowtype_dialog,     WTYPE_DIALOG},
		{windowtype_menu,       WTYPE_VMENU},
		{windowtype_help,       WTYPE_HELP},
		{windowtype_combobox,   WTYPE_COMBOBOX},
		{windowtype_findfolder, WTYPE_FINDFOLDER},
		{windowtype_grabber,    WTYPE_GRABBER},
		{windowtype_hmenu,      WTYPE_HMENU},
	};

	const auto ItemIterator = std::find_if(CONST_RANGE(TypesMap, i)
	{
		return i.first == fType;
	});
	return ItemIterator == std::cend(TypesMap)? static_cast<WINDOWINFO_TYPE>(-1) : ItemIterator->second;
}

SetAutocomplete::SetAutocomplete(EditControl* edit, bool NewState):
	edit(edit),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::SetAutocomplete(DlgEdit* dedit, bool NewState):
	edit(dedit->lineEdit.get()),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::SetAutocomplete(CommandLine* cedit, bool NewState):
	edit(&cedit->CmdStr),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::~SetAutocomplete()
{
	edit->SetAutocomplete(OldState);
};

void ReloadEnvironment()
{
	// these are handled incorrectly by CreateEnvironmentBlock
	std::vector<string_view> PreservedNames =
	{
		L"USERDOMAIN"sv, // absent
		L"USERNAME"sv, //absent
	};

#ifndef _WIN64
	if (os::IsWow64Process())
	{
		PreservedNames.emplace_back(L"PROCESSOR_ARCHITECTURE"sv); // Incorrect under WOW64
	}
#endif

	std::vector<std::pair<string_view, string>> PreservedVariables;
	PreservedVariables.reserve(std::size(PreservedNames));

	std::transform(ALL_CONST_RANGE(PreservedNames), std::back_inserter(PreservedVariables), [](string_view const i)
	{
		return std::make_pair(i, os::env::get(i));
	});

	{
		const os::env::provider::block EnvBlock;
		const auto EnvBlockPtr = EnvBlock.data();
		for (const auto& i: enum_substrings(EnvBlockPtr))
		{
			const auto Data = split_name_value(i);
			os::env::set(Data.first, Data.second);
		}
	}

	for (const auto& i: PreservedVariables)
	{
		os::env::set(i.first, i.second);
	}
}

unsigned int CRC32(unsigned int crc, const void* buffer, size_t size)
{
	static unsigned int crc_table[256];

	if (!crc_table[1])
	{
		for (unsigned int n = 0; n < 256; ++n)
		{
			unsigned int c = n;

			for (unsigned int k = 0; k < 8; k++)
				c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);

			crc_table[n] = c;
		}
	}

	crc = crc ^ ~0u;

	auto buf = reinterpret_cast<const unsigned char*>(buffer);

	while (size--)
	{
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
	}

	return crc ^ ~0u;
}

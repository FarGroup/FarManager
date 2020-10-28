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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "mix.hpp"

// Internal:
#include "pathmix.hpp"
#include "window.hpp"
#include "cmdline.hpp"
#include "dlgedit.hpp"
#include "strmix.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/enum_substrings.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

constexpr auto max_integer_in_double = bit(std::numeric_limits<double>::digits);

unsigned int ToPercent(unsigned long long const Value, unsigned long long const Base)
{
	if (!Value || !Base)
		return 0;

	if (Value == Base)
		return 100;

	if (Value <= max_integer_in_double && Base <= max_integer_in_double)
		return static_cast<int>(static_cast<double>(Value) / static_cast<double>(Base) * 100);

	const auto Step = Base / 100;
	const auto Result = Value / Step;

	return Result == 100? 99 : Result;
}

unsigned long long FromPercent(unsigned int const Percent, unsigned long long const Base)
{
	if (!Percent || !Base)
		return 0;

	if (Percent == 100)
		return Base;

	if (Base <= max_integer_in_double)
		return static_cast<double>(Base) / 100 * Percent;

	return Base / 100 * Percent;
}

string MakeTemp(string_view Prefix, bool const WithTempPath, string_view const UserTempPath)
{
	static unsigned s_shift = 0;

	Prefix = Prefix.empty()? L"FAR"sv : Prefix.substr(0, 3);

	auto strPath = L"."s;

	if (WithTempPath)
	{
		// BUGBUG check result
		(void)os::fs::GetTempPath(strPath);
	}
	else if(!UserTempPath.empty())
	{
		strPath = UserTempPath;
	}

	AddEndSlash(strPath);

	wchar_t_ptr_n<os::default_buffer_size> Buffer(Prefix.size() + strPath.size() + 13);

	auto Unique = 23 * GetCurrentProcessId() + s_shift;
	const auto UniqueCopy = Unique? Unique : 1;
	s_shift = (s_shift + 1) % 23;

	const null_terminated PrefixStr(Prefix);

	bool UseSystemFunction = true;

	const auto Generator = [&]
	{
		if (!UseSystemFunction || !GetTempFileName(strPath.c_str(), PrefixStr.c_str(), Unique, Buffer.data()))
		{
			// GetTempFileName uses only the last 16 bits of Unique.
			// We either did a full round trip through them or GetTempFileName failed for whatever reason.
			// Let's try the full 32-bit range manually.
			return path::join(strPath, concat(Prefix, to_hex_wstring(Unique), L".tmp"sv));
		}

		return string(Buffer.data());
	};

	for (;;)
	{
		if (!Unique) ++Unique;

		const auto Str = Generator();

		const auto Find = os::fs::enum_files(Str, false);
		if (Find.empty())
			return Str;

		if ((++Unique & 0xffff) == (UniqueCopy & 0xffff))
		{
			UseSystemFunction = false;
		}
	}
}

string MakeTempInSameDir(string_view FileName)
{
	return MakeTemp({}, false, CutToParent(FileName)? FileName : L"."sv);
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

	const auto MakeCopy = [](string_view const Str)
	{
		auto Buffer = std::make_unique<wchar_t[]>(Str.size() + 1);
		*copy_string(Str, Buffer.get()) = {};
		return Buffer.release();
	};

	Dest.FileName = MakeCopy(Src.FileName);
	Dest.AlternateFileName = MakeCopy(Src.AlternateFileName());
}

PluginPanelItemHolder::~PluginPanelItemHolder()
{
	FreePluginPanelItemData(Item);
}

void FreePluginPanelItemData(const PluginPanelItem& Data)
{
	delete[] Data.FileName;
	delete[] Data.AlternateFileName;
	delete[] Data.Description;
	delete[] Data.Owner;
	DeleteRawArray(span(Data.CustomColumnData, Data.CustomColumnNumber));
}

void FreePluginPanelItemUserData(HANDLE hPlugin, const UserDataItem& Data)
{
	if (!Data.FreeData)
		return;

	FarPanelItemFreeInfo info{ sizeof(FarPanelItemFreeInfo), hPlugin };
	Data.FreeData(Data.Data, &info);
}

void FreePluginPanelItemsData(span<PluginPanelItem> const Items)
{
	for (const auto& i: Items)
	{
		FreePluginPanelItemData(i);
	}
}

plugin_item_list::~plugin_item_list()
{
	FreePluginPanelItemsData(m_Data);
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
	return ItemIterator == std::cend(TypesMap)? WTYPE_UNKNOWN : ItemIterator->second;
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
}

void ReloadEnvironment()
{
	std::unordered_map<string_view, string> PreservedVariables;

	if (os::IsWow64Process())
	{
		PreservedVariables.emplace(L"PROCESSOR_ARCHITECTURE"sv, L""sv); // Incorrect under WOW64
	}

	for (auto& [Name, Value]: PreservedVariables)
	{
		Value = os::env::get(Name);
	}

	{
		const os::env::provider::block EnvBlock;
		for (const auto& i: enum_substrings(EnvBlock.data()))
		{
			const auto [Name, Value] = split(i);
			os::env::set(Name, Value);
		}
	}

	for (const auto& [Name, Value]: PreservedVariables)
	{
		os::env::set(Name, Value);
	}
}

string version_to_string(const VersionInfo& Version)
{
	static const string_view Stage[]
	{
		L"Release"sv,
		L"Alpha"sv,
		L"Beta"sv,
		L"RC"sv,
		L"Special"sv,
		L"Private"sv
	};

	static_assert(std::size(Stage) == VS_PRIVATE + 1);

	auto VersionStr = format(FSTR(L"{0}.{1}.{2}.{3}"), Version.Major, Version.Minor, Version.Build, Version.Revision);
	if (Version.Stage != VS_RELEASE && static_cast<size_t>(Version.Stage) < std::size(Stage))
	{
		append(VersionStr, L" ("sv, Stage[Version.Stage], L')');
	}
	return VersionStr;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"


TEST_CASE("to_percent")
{
	const auto Max = std::numeric_limits<unsigned long long>::max();

	static const struct
	{
		unsigned long long Value, Base, Result;
	}
	Tests[]
	{
		{ 0,                     0,                      0   },
		{ 1,                     0,                      0   },
		{ 1,                     1,                      100 },
		{ 0,                     1,                      0   },
		{ 1,                     2,                      50  },
		{ 2,                     1,                      200 },
		{ 3,                     4,                      75  },
		{ 0,                     Max,                    0   },
		{ 1,                     Max,                    0   },
		{ Max,                   0,                      0   },
		{ Max,                   Max,                    100 },
		{ 50_bit - 2,            50_bit - 1,             99  },
		{ 51_bit - 2,            51_bit - 1,             99  },
		{ 52_bit - 2,            52_bit - 1,             99  },
		{ 53_bit - 2,            53_bit - 1,             99  },
		{ 54_bit - 2,            54_bit - 1,             99  },
		{ Max - 1,               Max,                    99  },
		{ (50_bit - 2) / 2,      50_bit - 1,             49  },
		{ (51_bit - 2) / 2,      51_bit - 1,             49  },
		{ (52_bit - 2) / 2,      52_bit - 1,             49  },
		{ (53_bit - 2) / 2,      53_bit - 1,             49  },
		{ (54_bit - 2) / 2,      54_bit - 1,             50  },
		{ (Max - 2) / 2,         Max,                    50  },
		{ 850536266682995018u,   3335436339933313800u,   25  },
		{ 3552239702028979196u,  10006309019799941400u,  35  },
		{ 1680850982666015624u,  2384185791015625000u,   70  },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(ToPercent(i.Value, i.Base) == i.Result);
	}
}

TEST_CASE("from_percent")
{
	const auto Max = std::numeric_limits<unsigned long long>::max();

	static const struct
	{
		unsigned long long Value, Base, Result;
	}
	Tests[]
	{
		{ 0,           0,       0   },
		{ 1,           0,       0   },
		{ 100,         1,       1   },
		{ 0,           1,       0   },
		{ 50,          1,       0   },
		{ 50,          2,       1   },
		{ 200,         1,       2   },
		{ 75,          4,       3   },
		{ 0,           Max,     0   },
		{ 100,         Max,     Max },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(FromPercent(i.Value, i.Base) == i.Result);
	}
}

TEST_CASE("version_to_string")
{
	static const struct
	{
		VersionInfo Src;
		string_view Result;
	}
	Tests[]
	{
		{ { 0,  1,  3,  2,  VS_RELEASE },    L"0.1.2.3"sv,               },
		{ { 4,  5,  7,  6,  VS_ALPHA   },    L"4.5.6.7 (Alpha)"sv,       },
		{ { 8,  9,  11, 10, VS_BETA    },    L"8.9.10.11 (Beta)"sv,      },
		{ { 12, 13, 15, 14, VS_RC      },    L"12.13.14.15 (RC)"sv,      },
		{ { 16, 17, 19, 18, VS_SPECIAL },    L"16.17.18.19 (Special)"sv, },
		{ { 20, 21, 23, 22, VS_PRIVATE },    L"20.21.22.23 (Private)"sv, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(version_to_string(i.Src) == i.Result);
	}
}
#endif

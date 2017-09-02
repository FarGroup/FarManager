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

#include "headers.hpp"
#pragma hdrstop

#include "mix.hpp"
#include "pathmix.hpp"
#include "window.hpp"

#include "cmdline.hpp"
#include "dlgedit.hpp"
#include "strmix.hpp"

/*
             v - точка
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
bool FarMkTempEx(string &strDest, const wchar_t* Prefix, bool WithTempPath, const wchar_t* UserTempPath)
{
	static UINT s_shift = 0;
	if (!(Prefix && *Prefix))
		Prefix=L"F3T";

	string strPath = L".";

	if (WithTempPath)
	{
		os::GetTempPath(strPath);
	}
	else if(UserTempPath)
	{
		strPath=UserTempPath;
	}

	AddEndSlash(strPath);

	wchar_t_ptr_n<MAX_PATH> Buffer(wcslen(Prefix) + strPath.size() + 13);

	UINT uniq = 23*GetCurrentProcessId() + s_shift, uniq0 = uniq ? uniq : 1;
	s_shift = (s_shift + 1) % 23;

	for (;;)
	{
		if (!uniq) ++uniq;

		if (GetTempFileName(strPath.data(), Prefix, uniq, Buffer.get()))
		{
			const string Path = Buffer.get();
			const auto Find = os::fs::enum_files(Path, false);
			if (Find.begin() == Find.end())
				break;
		}

		if ((++uniq & 0xffff) == (uniq0 & 0xffff))
		{
			Buffer[0] = L'\0';
			break;
		}
	}

	strDest = Buffer.get();
	return !strDest.empty();
}

void PluginPanelItemToFindDataEx(const PluginPanelItem& Src, os::FAR_FIND_DATA& Dest)
{
	Dest.ftCreationTime = Src.CreationTime;
	Dest.ftLastAccessTime = Src.LastAccessTime;
	Dest.ftLastWriteTime = Src.LastWriteTime;
	Dest.ftChangeTime = Src.ChangeTime;
	Dest.nFileSize = Src.FileSize;
	Dest.nAllocationSize = Src.AllocationSize;
	Dest.FileId = 0;
	Dest.strFileName = NullToEmpty(Src.FileName);
	Dest.strAlternateFileName = NullToEmpty(Src.AlternateFileName);
	Dest.dwFileAttributes = Src.FileAttributes;
	Dest.dwReserved0 = 0;
}

void FindDataExToPluginPanelItemHolder(const os::FAR_FIND_DATA& Src, PluginPanelItemHolder& Holder)
{
	auto& Dest = Holder.Item;

	Dest.CreationTime = Src.ftCreationTime;
	Dest.LastAccessTime = Src.ftLastAccessTime;
	Dest.LastWriteTime = Src.ftLastWriteTime;
	Dest.ChangeTime = Src.ftChangeTime;
	Dest.FileSize = Src.nFileSize;
	Dest.AllocationSize = Src.nAllocationSize;

	auto Buffer = std::make_unique<wchar_t[]>(Src.strFileName.size() + 1);
	*std::copy(ALL_CONST_RANGE(Src.strFileName), Buffer.get()) = L'\0';
	Dest.FileName = Buffer.release();

	Buffer = std::make_unique<wchar_t[]>(Src.strAlternateFileName.size() + 1);
	*std::copy(ALL_CONST_RANGE(Src.strAlternateFileName), Buffer.get()) = L'\0';
	Dest.AlternateFileName = Buffer.release();

	Dest.Description = nullptr;
	Dest.Owner = nullptr;
	Dest.CustomColumnData = nullptr;
	Dest.CustomColumnNumber = 0;
	Dest.Flags = 0;
	Dest.UserData = {};
	Dest.FileAttributes = Src.dwFileAttributes;
	Dest.NumberOfLinks = 1;
	Dest.CRC32 = 0;
	std::fill(ALL_RANGE(Dest.Reserved), 0);
}

PluginPanelItemHolder::~PluginPanelItemHolder()
{
	FreePluginPanelItem(Item);
}

void FreePluginPanelItem(const PluginPanelItem& Data)
{
	delete[] Data.FileName;
	delete[] Data.AlternateFileName;
}

void FreePluginPanelItems(std::vector<PluginPanelItem>& Items)
{
	std::for_each(ALL_RANGE(Items), FreePluginPanelItem);
}

void FreePluginPanelItemsUserData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber)
{
	std::for_each(PanelItem, PanelItem + ItemsNumber, [&hPlugin](const PluginPanelItem& i)
	{
		if (i.UserData.FreeData)
		{
			FarPanelItemFreeInfo info = { sizeof(FarPanelItemFreeInfo), hPlugin };
			i.UserData.FreeData(i.UserData.Data, &info);
		}}
	);
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
	std::vector<const wchar_t*> PreservedNames =
	{
		L"USERDOMAIN", // absent
		L"USERNAME", //absent
	};

#ifndef _WIN64
	if (os::IsWow64Process())
	{
		PreservedNames.emplace_back(L"PROCESSOR_ARCHITECTURE"); // Incorrect under WOW64
	}
#endif

	std::vector<std::pair<const wchar_t*, string>> PreservedVariables;
	PreservedVariables.reserve(std::size(PreservedNames));

	std::transform(ALL_CONST_RANGE(PreservedNames), std::back_inserter(PreservedVariables), [](const wchar_t* i)
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

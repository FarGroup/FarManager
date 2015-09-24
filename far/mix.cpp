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


/*
             v - точка
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
bool FarMkTempEx(string &strDest, const wchar_t *Prefix, BOOL WithTempPath, const wchar_t *UserTempPath)
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

	wchar_t_ptr Buffer(StrLength(Prefix) + strPath.size() + 13);

	UINT uniq = 23*GetCurrentProcessId() + s_shift, uniq0 = uniq ? uniq : 1;
	s_shift = (s_shift + 1) % 23;

	for (;;)
	{
		if (!uniq) ++uniq;

		if (GetTempFileName(strPath.data(), Prefix, uniq, Buffer.get()))
		{
			os::fs::enum_file f(Buffer.get(), false);
			if (f.begin() == f.end())
				break;
		}

		if ((++uniq & 0xffff) == (uniq0 & 0xffff))
		{
			*Buffer = L'\0';
			break;
		}
	}

	strDest = Buffer.get();
	return !strDest.empty();
}

void PluginPanelItemToFindDataEx(const PluginPanelItem *pSrc, os::FAR_FIND_DATA *pDest)
{
	pDest->dwFileAttributes = pSrc->FileAttributes;
	pDest->ftCreationTime = pSrc->CreationTime;
	pDest->ftLastAccessTime = pSrc->LastAccessTime;
	pDest->ftLastWriteTime = pSrc->LastWriteTime;
	pDest->ftChangeTime = pSrc->ChangeTime;
	pDest->nFileSize = pSrc->FileSize;
	pDest->nAllocationSize = pSrc->AllocationSize;
	pDest->strFileName = NullToEmpty(pSrc->FileName);
	pDest->strAlternateFileName = NullToEmpty(pSrc->AlternateFileName);
}

void FindDataExToPluginPanelItem(const os::FAR_FIND_DATA *pSrc, PluginPanelItem *pDest)
{
	pDest->FileAttributes = pSrc->dwFileAttributes;
	pDest->CreationTime = pSrc->ftCreationTime;
	pDest->LastAccessTime = pSrc->ftLastAccessTime;
	pDest->LastWriteTime = pSrc->ftLastWriteTime;
	pDest->ChangeTime = pSrc->ftChangeTime;
	pDest->FileSize = pSrc->nFileSize;
	pDest->AllocationSize = pSrc->nAllocationSize;
	pDest->FileName = DuplicateString(pSrc->strFileName.data());
	pDest->AlternateFileName = DuplicateString(pSrc->strAlternateFileName.data());
}

void FreePluginPanelItem(const PluginPanelItem& Data)
{
	delete[] Data.FileName;
	delete[] Data.AlternateFileName;
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
	static const simple_pair<window_type, WINDOWINFO_TYPE> TypesMap[] =
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

	auto ItemIterator = std::find_if(CONST_RANGE(TypesMap, i)
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
	const wchar_t* PreservedNames[] =
	{
#ifndef _WIN64 // TODO: Use IsWow64Process() too?
		L"PROCESSOR_ARCHITECTURE", // incorrect under WOW64
#endif
		L"USERDOMAIN", // absent
		L"USERNAME", //absent
	};

	std::vector<std::pair<const wchar_t*, string>> PreservedVariables(ARRAYSIZE(PreservedNames));

	std::transform(ALL_CONST_RANGE(PreservedNames), PreservedVariables.begin(), [](const wchar_t* i)
	{
		return std::make_pair(i, os::env::get_variable(i));
	});

	FOR(const auto& i, (os::enum_strings_t<os::env::provider::block>()))
	{
		const auto Data = os::env::split(i.data());
		os::env::set_variable(Data.first, Data.second);
	}

	FOR(const auto& i, PreservedVariables)
	{
		os::env::set_variable(i.first, i.second);
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

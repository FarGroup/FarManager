#pragma once

/*
shortcuts.hpp

Folder shortcuts
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

#include "FarGuid.hpp"

struct ShortcutItem: NonCopyable
{
	ShortcutItem(): PluginGuid(FarGuid) {}
	ShortcutItem(const string& Name, const string Folder, const string& PluginFile, const string& PluginData, const GUID& PluginGuid):
		strName(Name),
		strFolder(Folder),
		strPluginFile(PluginFile),
		strPluginData(PluginData),
		PluginGuid(PluginGuid)
	{
	}

	ShortcutItem(ShortcutItem&& rhs): PluginGuid(FarGuid) { *this = std::move(rhs); }

	MOVE_OPERATOR_BY_SWAP(ShortcutItem);
	bool operator==(const ShortcutItem& Item) const;
	bool operator!=(const ShortcutItem& Item) const { return !(*this == Item); }
	void swap(ShortcutItem& rhs)
	{
		strName.swap(rhs.strName);
		strFolder.swap(rhs.strFolder);
		strPluginFile.swap(rhs.strPluginFile);
		strPluginData.swap(rhs.strPluginData);
		std::swap(PluginGuid, rhs.PluginGuid);
	}

	ShortcutItem clone()
	{
		return ShortcutItem(strName, strFolder, strPluginFile, strPluginData, PluginGuid);
	}

	string strName;
	string strFolder;
	string strPluginFile;
	string strPluginData;
	GUID PluginGuid;
};

STD_SWAP_SPEC(ShortcutItem);

class VMenu2;
struct MenuItemEx;

class Shortcuts
{
public:
	Shortcuts();
	~Shortcuts();
	bool Get(size_t Pos, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData, bool raw=false);
	bool Get(size_t Pos, size_t Index, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData);
	void Set(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData);
	void Add(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData);
	void Configure();

private:
	std::array<std::list<ShortcutItem>, 10> Items;
	bool Changed;
	void MakeItemName(size_t Pos, MenuItemEx* str);
	void EditItem(VMenu2* Menu, ShortcutItem& Item, bool Root, bool raw=false);
	bool Accept();
};

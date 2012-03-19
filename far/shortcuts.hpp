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

#include "DList.hpp"

struct ShortcutItem
{
	string strName;
	string strFolder;
	GUID PluginGuid;
	string strPluginFile;
	string strPluginData;
	ShortcutItem();
};

class VMenu;
struct MenuItemEx;

class Shortcuts
{
public:
	Shortcuts();
	~Shortcuts();
	bool Get(size_t Pos, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData);
	bool Get(size_t Pos, size_t Index, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData);
	void Set(size_t Pos, const wchar_t* Folder, const GUID& PluginGuid, const wchar_t* PluginFile, const wchar_t* PluginData);
	void Add(size_t Pos, const wchar_t* Folder, const GUID& PluginGuid, const wchar_t* PluginFile, const wchar_t* PluginData);
	void Configure();

private:
	static const size_t KeyCount = 10;
	DList<ShortcutItem> Items[KeyCount];
	bool Changed;
	void MakeItemName(size_t Pos, MenuItemEx* str);
	void EditItem(VMenu* Menu, ShortcutItem* Item, bool Root);
	bool Accept(void);
};

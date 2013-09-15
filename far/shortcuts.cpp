/*
shortcuts.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "shortcuts.hpp"
#include "keys.hpp"
#include "vmenu2.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "interf.hpp"
#include "dialog.hpp"
#include "FarDlgBuilder.hpp"
#include "plugins.hpp"
#include "configdb.hpp"
#include "FarGuid.hpp"
#include "DlgGuid.hpp"

enum PSCR_RECTYPE
{
	PSCR_RT_SHORTCUT,
	PSCR_RT_NAME,
	PSCR_RT_PLUGINGUID,
	PSCR_RT_PLUGINFILE,
	PSCR_RT_PLUGINDATA,
};

static const wchar_t *RecTypeName[]=
{
	L"Shortcut",
	L"Name",
	L"PluginGuid",
	L"PluginFile",
	L"PluginData",
};

static const wchar_t* FolderShortcutsKey = L"Shortcuts";
static const wchar_t* HelpFolderShortcuts = L"FolderShortcuts";


ShortcutItem::ShortcutItem():
	PluginGuid(FarGuid)
{
}

bool ShortcutItem::operator==(const ShortcutItem& Item) const
{
	return
	  strName == Item.strName &&
	  strFolder == Item.strFolder &&
	  PluginGuid == Item.PluginGuid &&
	  strPluginFile == Item.strPluginFile &&
	  strPluginData == Item.strPluginData;
}

Shortcuts::Shortcuts()
{
	Changed = false;

	auto cfg = Global->Db->CreateShortcutsConfig();
	unsigned __int64 root = cfg->GetKeyID(0,FolderShortcutsKey);

	if (root)
	{
		for_each_cnt(RANGE(Items, i, size_t index)
		{
			i.clear();
			unsigned __int64 key = cfg->GetKeyID(root, std::to_wstring(index));
			if (key)
			{
				for(size_t j=0; ; j++)
				{
					FormatString ValueName;
					ValueName << RecTypeName[PSCR_RT_SHORTCUT] << j;
					string strValue;
					if (!cfg->GetValue(key, ValueName, strValue))
						break;
					ShortcutItem Item;
					Item.strFolder = strValue;

					ValueName.clear();
					ValueName << RecTypeName[PSCR_RT_NAME] << j;
					cfg->GetValue(key, ValueName, Item.strName);

					ValueName.clear();
					ValueName << RecTypeName[PSCR_RT_PLUGINGUID] << j;
					string strPluginGuid;
					cfg->GetValue(key, ValueName, strPluginGuid);
					if(!StrToGuid(strPluginGuid,Item.PluginGuid)) Item.PluginGuid=FarGuid;

					ValueName.clear();
					ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << j;
					cfg->GetValue(key, ValueName, Item.strPluginFile);

					ValueName.clear();
					ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << j;
					cfg->GetValue(key, ValueName, Item.strPluginData);
					i.emplace_back(Item);
				}
			}
		});
	}
}

Shortcuts::~Shortcuts()
{
	if (!Changed)
		return;

	auto cfg = Global->Db->CreateShortcutsConfig();
	unsigned __int64 root = cfg->GetKeyID(0,FolderShortcutsKey);
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(0,FolderShortcutsKey);

	if (root)
	{
		for_each_cnt(CONST_RANGE(Items, i, size_t index)
		{
			unsigned __int64 key = cfg->CreateKey(root, std::to_wstring(index));
			if (key)
			{
				for_each_cnt(CONST_RANGE(i, j, size_t index)
				{
					FormatString ValueName;
					ValueName << RecTypeName[PSCR_RT_SHORTCUT] << index;
					cfg->SetValue(key, ValueName, j.strFolder);

					ValueName.clear();
					ValueName << RecTypeName[PSCR_RT_NAME] << index;
					cfg->SetValue(key, ValueName, j.strName);

					if(j.PluginGuid != FarGuid)
					{
						ValueName.clear();
						ValueName << RecTypeName[PSCR_RT_PLUGINGUID] << index;
						string strPluginGuid=GuidToStr(j.PluginGuid);
						cfg->SetValue(key, ValueName, strPluginGuid);
					}

					if(!j.strPluginFile.empty())
					{
						ValueName.clear();
						ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << index;
						cfg->SetValue(key, ValueName, j.strPluginFile);
					}

					if(!j.strPluginData.empty())
					{
						ValueName.clear();
						ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << index;
						cfg->SetValue(key, ValueName, j.strPluginData);
					}
				});
			}
		});
	}
}

static void Fill(const ShortcutItem& RetItem, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(Folder)
	{
		*Folder = api::ExpandEnvironmentStrings(RetItem.strFolder);
	}
	if(PluginGuid)
	{
		*PluginGuid = RetItem.PluginGuid;
	}
	if(PluginFile)
	{
		*PluginFile = RetItem.strPluginFile;
	}
	if(PluginData)
	{
		*PluginData = RetItem.strPluginData;
	}
}

static string MakeName(const ShortcutItem& Item)
{
	Plugin* plugin = nullptr;
	string result(MSG(MShortcutNone));

	if (Item.PluginGuid == FarGuid)
	{
		if(!Item.strName.empty())
		{
			result = Item.strName;
		}
		else if(!Item.strFolder.empty())
		{
			result = Item.strFolder;
		}
		result = api::ExpandEnvironmentStrings(result);
	}
	else
	{
		if ((plugin = Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid)))
		{
			if(!Item.strName.empty())
			{
				result = Item.strName;
			}
			else
			{
				result = plugin->GetTitle();
				string TechInfo;

				if (!Item.strPluginFile.empty())
					TechInfo.append(MSG(MFSShortcutPluginFile)).append(L" ").append(Item.strPluginFile + L", ");
				if (!Item.strFolder.empty())
					TechInfo.append(MSG(MFSShortcutPath)).append(L" ").append(Item.strFolder + L", ");
				if (!Item.strPluginData.empty()) {
					string t = Item.strPluginData;
					for (size_t i = 0; i < t.size(); ++i) // cut not printable plugindata
						if (t[i] < L' ')
							t.resize(i);
					if (!t.empty())
						TechInfo.append(MSG(MFSShortcutPluginData)).append(L" ").append(t + L", ");
				}

				if (!TechInfo.empty())
				{
					TechInfo.resize(TechInfo.size() - 2);
					result += L" (" + TechInfo + L")";
				}
			}
		}
		else
			result.clear();
	}
	return result;
}

static void FillMenu(VMenu2& Menu, const std::list<ShortcutItem>& List, bool raw_mode=false)
{
	Menu.DeleteItems();
	FOR_CONST_RANGE(List, i)
	{
		MenuItemEx ListItem={};
		ListItem.strName = MakeName(*i);
		if (ListItem.strName.empty())
			continue;

		ListItem.UserData = &i;
		ListItem.UserDataSize = sizeof(i);
		if (!raw_mode && i->PluginGuid == FarGuid && i->strFolder.empty())
		{
			if (ListItem.strName != L"--")
			{
				if (Menu.GetItemCount() == 0)
					Menu.SetTitle(ListItem.strName);

				continue;
			}

			ListItem.strName.clear();
			ListItem.Flags = LIF_SEPARATOR;
		}
		Menu.AddItem(&ListItem);
	}
}

bool Shortcuts::Get(size_t Pos, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData, bool raw)
{
	bool Result = false;
	if(!Items[Pos].empty())
	{
		auto RetItem = Items[Pos].end();
		if(Items[Pos].size()>1)
		{
			VMenu2 FolderList(MSG(MFolderShortcutsTitle),nullptr,0,ScrY-4);
			FolderList.SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
			FolderList.SetHelp(HelpFolderShortcuts);
			FolderList.SetBottomTitle(MSG(MFolderShortcutBottomSub));
			FillMenu(FolderList, Items[Pos], raw);

			int ExitCode=FolderList.Run([&](int Key)->int
			{
				int ItemPos = FolderList.GetSelectPos();
				ITERATOR(Items[Pos])* Item = static_cast<decltype(Item)>(FolderList.GetUserData(nullptr, 0, ItemPos));
				int KeyProcessed = 1;
				switch (Key)
				{
				case KEY_NUMPAD0:
				case KEY_INS:
					if (!Accept()) break;
				case KEY_NUMDEL:
				case KEY_DEL:
					{
						Changed = true;
						if (Key == KEY_INS || Key == KEY_NUMPAD0)
						{
							ShortcutItem NewItem;
							Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
							Global->CtrlObject->CmdLine->GetCurDir(NewItem.strFolder);
							if (ActivePanel->GetMode() == PLUGIN_PANEL)
							{
								OpenPanelInfo Info;
								ActivePanel->GetOpenPanelInfo(&Info);
								string strTemp;
								PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();
								NewItem.PluginGuid = ph->pPlugin->GetGUID();
								NewItem.strPluginFile = NullToEmpty(Info.HostFile);
								NewItem.strPluginData = NullToEmpty(Info.ShortcutData);
							}
							else
							{
								NewItem.PluginGuid = FarGuid;
								NewItem.strPluginFile.clear();
								NewItem.strPluginData.clear();
							}
							auto newIter = Items[Pos].emplace(Item ? *Item : Items[Pos].end(), NewItem);

							MenuItemEx NewMenuItem = {};
							NewMenuItem.strName = NewItem.strFolder;

							NewMenuItem.UserData = &newIter;
							NewMenuItem.UserDataSize = sizeof(newIter);
							FolderList.AddItem(&NewMenuItem, ItemPos);
							FolderList.SetSelectPos(ItemPos, 1);
						}
						else
						{
							if(!Items[Pos].empty())
							{
								Items[Pos].erase(*Item);
								FolderList.DeleteItem(FolderList.GetSelectPos());
							}
						}
					}
					break;

				case KEY_F4:
					{
						EditItem(&FolderList, **Item, false, raw);
					}
					break;

				case KEY_CTRLUP:
				case KEY_RCTRLUP:
					{
						if (*Item != Items[Pos].begin())
						{
							auto i = *Item;
							--i;
							Items[Pos].splice(i, Items[Pos], *Item);
							FillMenu(FolderList, Items[Pos], raw);
							FolderList.SetSelectPos(--ItemPos);
							Changed = true;
						}
					}
					break;
				case KEY_CTRLDOWN:
				case KEY_RCTRLDOWN:
					{
						auto i = *Item;
						++i;
						if (i != Items[Pos].end())
						{
							Items[Pos].splice(*Item, Items[Pos], i);
							FillMenu(FolderList, Items[Pos], raw);
							FolderList.SetSelectPos(++ItemPos);
							Changed = true;
						}
					}
					break;
				default:
					KeyProcessed = 0;
				}
				return KeyProcessed;
			});
			if (ExitCode>=0)
			{
				RetItem = *static_cast<decltype(&RetItem)>(FolderList.GetUserData(nullptr, 0, ExitCode));
			}
		}
		else
		{
			RetItem = Items[Pos].begin();
		}

		if(RetItem != Items[Pos].end())
		{
			Fill(*RetItem,Folder,PluginGuid,PluginFile,PluginData);
			Result = true;
		}
	}
	return Result;
}

bool Shortcuts::Get(size_t Pos, size_t Index, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(Items[Pos].size()<=Index)
		return false;
	auto RetItem = Items[Pos].begin();
	std::advance(RetItem, Index);
	Fill(*RetItem,Folder,PluginGuid,PluginFile,PluginData);
	return true;
}

void Shortcuts::Set(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	Changed = true;
	if(Items[Pos].empty())
	{
		Items[Pos].resize(1);
	}
	auto ItemIterator = Items[Pos].begin();
	ItemIterator->strFolder = Folder;
	ItemIterator->PluginGuid = PluginGuid;
	ItemIterator->strPluginFile = PluginFile;
	ItemIterator->strPluginData = PluginData;
}

void Shortcuts::Add(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	Changed = true;
	ShortcutItem Item;
	Item.strFolder = Folder;
	Item.PluginGuid = PluginGuid;
	Item.strPluginFile = PluginFile;
	Item.strPluginData = PluginData;
	Items[Pos].emplace_back(Item);
}

void Shortcuts::MakeItemName(size_t Pos, MenuItemEx* MenuItem)
{
	string ItemName(MSG(MShortcutNone));

	if(!Items[Pos].empty())
	{
		ItemName = MakeName(Items[Pos].front());
	}

	MenuItem->strName = string(MSG(MRightCtrl)) + L"+&" + std::to_wstring(Pos) + L" \x2502 " + ItemName;
	if(Items[Pos].size() > 1)
	{
		MenuItem->Flags|=MIF_SUBMENU;
	}
	else
	{
		MenuItem->Flags&=~MIF_SUBMENU;
	}
}

void Shortcuts::EditItem(VMenu2* Menu, ShortcutItem& Item, bool Root, bool raw)
{
	ShortcutItem NewItem = Item;

	DialogBuilder Builder(MFolderShortcutsTitle, HelpFolderShortcuts);
	Builder.AddText(MFSShortcutName);
	Builder.AddEditField(&NewItem.strName, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(MFSShortcutPath);
	Builder.AddEditField(&NewItem.strFolder, 50, L"FS_Path", DIF_EDITPATH);
	if (Item.PluginGuid != FarGuid)
	{
		Builder.AddSeparator(Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid)->GetTitle().data());
		Builder.AddText(MFSShortcutPluginFile);
		Builder.AddEditField(&NewItem.strPluginFile, 50, L"FS_Path", DIF_EDITPATH);
		Builder.AddText(MFSShortcutPluginData);
		Builder.AddEditField(&NewItem.strPluginData, 50, L"FS_Path", DIF_EDITPATH);
	}
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		bool Save=true;
		if (Item.PluginGuid == FarGuid)
		{
			Unquote(NewItem.strFolder);

			bool PathRoot = false;
			PATH_TYPE Type = ParsePath(NewItem.strFolder, nullptr, &PathRoot);
			if(!(PathRoot && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID)))
			{
				DeleteEndSlash(NewItem.strFolder);
			}

			string strTemp = api::ExpandEnvironmentStrings(NewItem.strFolder);

			if ((!raw || !strTemp.empty()) && api::GetFileAttributes(strTemp) == INVALID_FILE_ATTRIBUTES)
			{
				Global->CatchError();
				Save=!Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), NewItem.strFolder.data(), MSG(MSaveThisShortcut), MSG(MYes), MSG(MNo));
			}
		}

		if (Save && NewItem != Item)
		{
			Changed = true;
			Item = NewItem;

			MenuItemEx* MenuItem = Menu->GetItemPtr();
			if(Root)
			{
				MakeItemName(Menu->GetSelectPos(), MenuItem);
			}
			else
				MenuItem->strName = MakeName(Item);
		}
	}
}

void Shortcuts::Configure()
{
	VMenu2 FolderList(MSG(MFolderShortcutsTitle),nullptr,0,ScrY-4);
	FolderList.SetFlags(VMENU_WRAPMODE);
	FolderList.SetHelp(HelpFolderShortcuts);
	FolderList.SetBottomTitle(MSG(MFolderShortcutBottom));
	FolderList.SetId(FolderShortcutsId);

	for (int I=0; I < 10; I++)
	{
		MenuItemEx ListItem={};
		MakeItemName(I, &ListItem);
		FolderList.AddItem(&ListItem);
	}

	bool raw_mode = false;

	int ExitCode=FolderList.Run([&](int Key)->int
	{
		int Pos = FolderList.GetSelectPos();
		auto ItemIterator = Items[Pos].begin();
		int KeyProcessed = 1;

		switch (Key)
		{
		case KEY_NUMPAD0:
		case KEY_INS:
		case KEY_SHIFTINS:
		case KEY_SHIFTNUMPAD0:
			if (!Accept()) break;
		case KEY_NUMDEL:
		case KEY_DEL:
			{
				MenuItemEx* MenuItem = FolderList.GetItemPtr();
				if (Key == KEY_INS || Key == KEY_NUMPAD0 || Key&KEY_SHIFT)
				{
					if(ItemIterator == Items[Pos].end() || !(Key&KEY_SHIFT))
					{
						Items[Pos].resize(Items[Pos].size()+1);
						ItemIterator = Items[Pos].end();
						--ItemIterator;
					}
					Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
					Global->CtrlObject->CmdLine->GetCurDir(ItemIterator->strFolder);
					if (ActivePanel->GetMode() == PLUGIN_PANEL)
					{
						OpenPanelInfo Info;
						ActivePanel->GetOpenPanelInfo(&Info);
						string strTemp;
						PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();
						ItemIterator->PluginGuid = ph->pPlugin->GetGUID();
						ItemIterator->strPluginFile = NullToEmpty(Info.HostFile);
						ItemIterator->strPluginData = NullToEmpty(Info.ShortcutData);
					}
					else
					{
						ItemIterator->PluginGuid = FarGuid;
						ItemIterator->strPluginFile.clear();
						ItemIterator->strPluginData.clear();
					}
					MakeItemName(Pos, MenuItem);
				}
				else
				{
					if(ItemIterator != Items[Pos].end())
					{
						Items[Pos].erase(ItemIterator);
						MakeItemName(Pos, MenuItem);
					}
				}
				INT64 Flags = MenuItem->Flags;
				MenuItem->Flags = 0;
				FolderList.UpdateItemFlags(FolderList.GetSelectPos(), Flags);
				Changed = true;
			}
			break;

		case KEY_F4:
			{
				raw_mode = true;
				if(ItemIterator == Items[Pos].end())
				{
					Items[Pos].resize(Items[Pos].size()+1);
					ItemIterator = Items[Pos].end();
					--ItemIterator;
				}
				if(Items[Pos].size()>1)
				{
					FolderList.Close();
				}
				else
				{
					EditItem(&FolderList, *ItemIterator, true);
				}
				Changed = true;
			}
			break;

		default:
			KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	if(ExitCode>=0)
	{
		Global->CtrlObject->Cp()->ActivePanel->ExecShortcutFolder(ExitCode, raw_mode);
	}
}

bool Shortcuts::Accept(void)
{
	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
	if (ActivePanel->GetMode() == PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		ActivePanel->GetOpenPanelInfo(&Info);
		if(!(Info.Flags&OPIF_SHORTCUT)) return false;
	}
	return true;
}

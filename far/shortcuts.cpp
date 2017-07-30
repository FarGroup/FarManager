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
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "message.hpp"
#include "pathmix.hpp"
#include "interf.hpp"
#include "dialog.hpp"
#include "FarDlgBuilder.hpp"
#include "plugins.hpp"
#include "configdb.hpp"
#include "FarGuid.hpp"
#include "DlgGuid.hpp"
#include "lang.hpp"
#include "strmix.hpp"

enum PSCR_RECTYPE
{
	PSCR_RT_SHORTCUT,
	PSCR_RT_NAME,
	PSCR_RT_PLUGINGUID,
	PSCR_RT_PLUGINFILE,
	PSCR_RT_PLUGINDATA,
};

static const wchar_t* const RecTypeName[]=
{
	L"Shortcut",
	L"Name",
	L"PluginGuid",
	L"PluginFile",
	L"PluginData",
};

static const wchar_t FolderShortcutsKey[] = L"Shortcuts";
static const wchar_t HelpFolderShortcuts[] = L"FolderShortcuts";


struct Shortcuts::shortcut: public rel_ops<Shortcuts::shortcut>
{
private:
	auto tie() const
	{
		return std::tie(strName, strFolder, PluginGuid, strPluginFile, strPluginData);
	}

public:
	NONCOPYABLE(shortcut);
	MOVABLE(shortcut);

	shortcut(): PluginGuid(FarGuid) {}

	shortcut(const string& Name, const string& Folder, const string& PluginFile, const string& PluginData, const GUID& PluginGuid):
		strName(Name),
		strFolder(Folder),
		strPluginFile(PluginFile),
		strPluginData(PluginData),
		PluginGuid(PluginGuid)
	{
	}

	bool operator==(const shortcut& rhs) const { return tie() == rhs.tie(); }

	shortcut clone() const
	{
		return shortcut(strName, strFolder, strPluginFile, strPluginData, PluginGuid);
	}

	string strName;
	string strFolder;
	string strPluginFile;
	string strPluginData;
	GUID PluginGuid;
};

Shortcuts::Shortcuts()
{
	Changed = false;

	const auto cfg = ConfigProvider().CreateShortcutsConfig();

	if (const auto root = cfg->FindByName(cfg->root_key(), FolderShortcutsKey))
	{
		for_each_cnt(RANGE(Items, i, size_t index)
		{
			i.clear();
			if (const auto key = cfg->FindByName(root, str(index)))
			{
				for(size_t j=0; ; j++)
				{
					const auto sIndex = str(j);

					shortcut Item;
					if (!cfg->GetValue(key, RecTypeName[PSCR_RT_SHORTCUT] + sIndex, Item.strFolder))
						break;

					cfg->GetValue(key, RecTypeName[PSCR_RT_NAME] + sIndex, Item.strName);

					string strPluginGuid;
					cfg->GetValue(key, RecTypeName[PSCR_RT_PLUGINGUID] + sIndex, strPluginGuid);
					if(!StrToGuid(strPluginGuid, Item.PluginGuid))
						Item.PluginGuid=FarGuid;

					cfg->GetValue(key, RecTypeName[PSCR_RT_PLUGINFILE] + sIndex, Item.strPluginFile);
					cfg->GetValue(key, RecTypeName[PSCR_RT_PLUGINDATA] + sIndex, Item.strPluginData);

					i.emplace_back(std::move(Item));
				}
			}
		});
	}
}

Shortcuts::~Shortcuts()
{
	Save();
}

void Shortcuts::Save()
{
	if (!Changed)
		return;

	const auto cfg = ConfigProvider().CreateShortcutsConfig();
	auto root = cfg->FindByName(cfg->root_key(), FolderShortcutsKey);
	if (root)
		cfg->DeleteKeyTree(root);

	if ((root = cfg->CreateKey(cfg->root_key(), FolderShortcutsKey)))
	{
		for_each_cnt(CONST_RANGE(Items, i, size_t OuterIndex)
		{
			if (const auto Key = cfg->CreateKey(root, str(OuterIndex)))
			{
				for_each_cnt(CONST_RANGE(i, j, size_t InnerIndex)
				{
					const auto sIndex = str(InnerIndex);

					cfg->SetValue(Key, RecTypeName[PSCR_RT_SHORTCUT] + sIndex, j.strFolder);
					cfg->SetValue(Key, RecTypeName[PSCR_RT_NAME] + sIndex, j.strName);

					if(j.PluginGuid != FarGuid)
					{
						cfg->SetValue(Key, RecTypeName[PSCR_RT_PLUGINGUID] + sIndex, GuidToStr(j.PluginGuid));
					}

					if(!j.strPluginFile.empty())
					{
						cfg->SetValue(Key, RecTypeName[PSCR_RT_PLUGINFILE] + sIndex, j.strPluginFile);
					}

					if(!j.strPluginData.empty())
					{
						cfg->SetValue(Key, RecTypeName[PSCR_RT_PLUGINDATA] + sIndex, j.strPluginData);
					}
				});
			}
		});
	}
}

static void Fill(const Shortcuts::shortcut& RetItem, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(Folder)
	{
		*Folder = os::env::expand(RetItem.strFolder);
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

static string MakeName(const Shortcuts::shortcut& Item)
{
	string result(msg(lng::MShortcutNone));

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
		result = os::env::expand(result);
	}
	else
	{
		if (const auto plugin = Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid))
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
					append(TechInfo, msg(lng::MFSShortcutPluginFile), L' ', Item.strPluginFile, L", "_sv);

				if (!Item.strFolder.empty())
					append(TechInfo, msg(lng::MFSShortcutPath), L' ', Item.strFolder, L", "_sv);

				if (!Item.strPluginData.empty())
				{
					const string PrintablePluginData(Item.strPluginData.cbegin(), std::find_if(ALL_CONST_RANGE(Item.strPluginData), [](const auto i) { return i < L' '; }));
					if (!PrintablePluginData.empty())
						append(TechInfo, msg(lng::MFSShortcutPluginData), L' ', PrintablePluginData, L", "_sv);
				}

				if (!TechInfo.empty())
				{
					TechInfo.resize(TechInfo.size() - 2);
					append(result, L" ("_sv, TechInfo, L')');
				}
			}
		}
		else
			result.clear();
	}
	return result;
}

static void FillMenu(VMenu2& Menu, std::list<Shortcuts::shortcut>& List, bool raw_mode = false)
{
	Menu.clear();
	FOR_RANGE(List, i)
	{
		MenuItemEx ListItem(MakeName(*i));
		if (ListItem.strName.empty())
			continue;

		ListItem.UserData = i;
		if (!raw_mode && i->PluginGuid == FarGuid && i->strFolder.empty())
		{
			if (ListItem.strName != L"--")
			{
				if (Menu.empty())
					Menu.SetTitle(ListItem.strName);

				continue;
			}

			ListItem.strName.clear();
			ListItem.Flags = LIF_SEPARATOR;
		}
		Menu.AddItem(ListItem);
	}
}

static bool Accept()
{
	const auto& ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	if (ActivePanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		ActivePanel->GetOpenPanelInfo(&Info);
		if (!(Info.Flags&OPIF_SHORTCUT))
			return false;
	}
	return true;
}

bool Shortcuts::Get(size_t Pos, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData, bool raw)
{
	bool Result = false;
	if(!Items[Pos].empty())
	{
		auto RetItem = Items[Pos].end();
		if(Items[Pos].size()>1)
		{
			const auto FolderList = VMenu2::create(msg(lng::MFolderShortcutsTitle), nullptr, 0, ScrY - 4);
			FolderList->SetMenuFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
			FolderList->SetHelp(HelpFolderShortcuts);
			FolderList->SetBottomTitle(msg(lng::MFolderShortcutBottomSub));
			FolderList->SetId(FolderShortcutsMoreId);
			FillMenu(*FolderList, Items[Pos], raw);

			int ExitCode=FolderList->Run([&](const Manager::Key& RawKey)
			{
				const auto Key=RawKey();
				int ItemPos = FolderList->GetSelectPos();
				const auto Item = FolderList->GetUserDataPtr<ITERATOR(Items[Pos])>(ItemPos);
				int KeyProcessed = 1;
				switch (Key)
				{
				case KEY_NUMPAD0:
				case KEY_INS:
					if (!Accept())
						break;
					// fallthrough
				case KEY_NUMDEL:
				case KEY_DEL:
					{
						Changed = true;
						if (Key == KEY_INS || Key == KEY_NUMPAD0)
						{
							shortcut NewItem;
							const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
							NewItem.strFolder = Global->CtrlObject->CmdLine()->GetCurDir();
							if (ActivePanel->GetMode() == panel_mode::PLUGIN_PANEL)
							{
								OpenPanelInfo Info;
								ActivePanel->GetOpenPanelInfo(&Info);
								const auto ph = ActivePanel->GetPluginHandle();
								NewItem.PluginGuid = ph->plugin()->GetGUID();
								NewItem.strPluginFile = NullToEmpty(Info.HostFile);
								NewItem.strPluginData = NullToEmpty(Info.ShortcutData);
							}
							else
							{
								NewItem.PluginGuid = FarGuid;
								NewItem.strPluginFile.clear();
								NewItem.strPluginData.clear();
							}
							const auto newIter = Items[Pos].emplace(Item ? *Item : Items[Pos].end(), std::move(NewItem));

							MenuItemEx NewMenuItem(newIter->strFolder);
							NewMenuItem.UserData = newIter;
							FolderList->AddItem(NewMenuItem, ItemPos);
							FolderList->SetSelectPos(ItemPos, 1);
						}
						else
						{
							if(!Items[Pos].empty())
							{
								Items[Pos].erase(*Item);
								FolderList->DeleteItem(FolderList->GetSelectPos());
							}
						}
					}
					break;

				case KEY_F4:
					{
						EditItem(*FolderList, **Item, false, raw);
					}
					break;

				case KEY_CTRLUP:
				case KEY_RCTRLUP:
					{
						if (*Item != Items[Pos].begin())
						{
							const auto i = std::prev(*Item);
							Items[Pos].splice(i, Items[Pos], *Item);
							FillMenu(*FolderList, Items[Pos], raw);
							FolderList->SetSelectPos(--ItemPos);
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
							FillMenu(*FolderList, Items[Pos], raw);
							FolderList->SetSelectPos(++ItemPos);
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
				RetItem = *FolderList->GetUserDataPtr<ITERATOR(Items[Pos])>(ExitCode);
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
	const auto RetItem = std::next(Items[Pos].begin(), Index);
	Fill(*RetItem,Folder,PluginGuid,PluginFile,PluginData);
	return true;
}

void Shortcuts::Set(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	if(Items[Pos].empty())
	{
		return Add(Pos, Folder, PluginGuid, PluginFile, PluginData);
	}
	auto& Item = Items[Pos].front();
	Item.strFolder = Folder;
	Item.PluginGuid = PluginGuid;
	Item.strPluginFile = PluginFile;
	Item.strPluginData = PluginData;
	Changed = true;
}

void Shortcuts::Add(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	Items[Pos].emplace_back(string{}, Folder, PluginFile, PluginData, PluginGuid);
	Changed = true;
}

void Shortcuts::MakeItemName(size_t Pos, MenuItemEx& MenuItem)
{
	string ItemName(msg(lng::MShortcutNone));

	if(!Items[Pos].empty())
	{
		ItemName = MakeName(Items[Pos].front());
	}

	MenuItem.strName = string(msg(lng::MRightCtrl)) + L"+&" + str(Pos) + L" \x2502 " + ItemName;
	if(Items[Pos].size() > 1)
	{
		MenuItem.Flags |= MIF_SUBMENU;
	}
	else
	{
		MenuItem.Flags &= ~MIF_SUBMENU;
	}
}

void Shortcuts::EditItem(VMenu2& Menu, shortcut& Item, bool Root, bool raw)
{
	auto NewItem(Item.clone());

	DialogBuilder Builder(lng::MFolderShortcutsTitle, HelpFolderShortcuts);
	Builder.AddText(lng::MFSShortcutName);
	Builder.AddEditField(NewItem.strName, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(lng::MFSShortcutPath);
	Builder.AddEditField(NewItem.strFolder, 50, L"FS_Path", DIF_EDITPATH);
	if (Item.PluginGuid != FarGuid)
	{
		Builder.AddSeparator(Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid)->GetTitle().data());
		Builder.AddText(lng::MFSShortcutPluginFile);
		Builder.AddEditField(NewItem.strPluginFile, 50, L"FS_Path", DIF_EDITPATH);
		Builder.AddText(lng::MFSShortcutPluginData);
		Builder.AddEditField(NewItem.strPluginData, 50, L"FS_Path", DIF_EDITPATH);
	}
	Builder.SetId(FolderShortcutsDlgId);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		bool Save=true;
		if (Item.PluginGuid == FarGuid)
		{
			bool PathRoot = false;
			const auto Type = ParsePath(inplace::unquote(NewItem.strFolder), nullptr, &PathRoot);
			if(!(PathRoot && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID)))
			{
				DeleteEndSlash(NewItem.strFolder);
			}

			const auto strTemp = os::env::expand(NewItem.strFolder);

			if ((!raw || !strTemp.empty()) && !os::fs::exists(strTemp))
			{
				Global->CatchError();
				Save = Message(MSG_WARNING | MSG_ERRORTYPE,
					msg(lng::MError),
					{
						NewItem.strFolder,
						msg(lng::MSaveThisShortcut)
					},
					{ lng::MYes, lng::MNo }) == Message::first_button;
			}
		}

		if (Save && NewItem != Item)
		{
			Changed = true;
			Item = std::move(NewItem);

			auto& MenuItem = Menu.current();
			if(Root)
			{
				MakeItemName(Menu.GetSelectPos(), MenuItem);
			}
			else
				MenuItem.strName = MakeName(Item);
		}
	}
}

void Shortcuts::Configure()
{
	const auto FolderList = VMenu2::create(msg(lng::MFolderShortcutsTitle), nullptr, 0, ScrY - 4);
	FolderList->SetMenuFlags(VMENU_WRAPMODE);
	FolderList->SetHelp(HelpFolderShortcuts);
	FolderList->SetBottomTitle(msg(lng::MFolderShortcutBottom));
	FolderList->SetId(FolderShortcutsId);

	for (size_t i = 0; i < Items.size(); ++i)
	{
		MenuItemEx ListItem;
		MakeItemName(i, ListItem);
		FolderList->AddItem(ListItem);
	}

	bool raw_mode = false;

	int ExitCode=FolderList->Run([&](const Manager::Key& RawKey)
	{
		const auto Key=RawKey();
		int Pos = FolderList->GetSelectPos();
		auto ItemIterator = Items[Pos].begin();
		int KeyProcessed = 1;

		switch (Key)
		{
		case KEY_NUMPAD0:
		case KEY_INS:
		case KEY_SHIFTINS:
		case KEY_SHIFTNUMPAD0:
			if (!Accept())
				break;
			// fallthrough
		case KEY_NUMDEL:
		case KEY_DEL:
			{
				auto& MenuItem = FolderList->current();
				if (Key == KEY_INS || Key == KEY_NUMPAD0 || Key&KEY_SHIFT)
				{
					if(ItemIterator == Items[Pos].end() || !(Key&KEY_SHIFT))
					{
						Items[Pos].resize(Items[Pos].size()+1);
						ItemIterator = Items[Pos].end();
						--ItemIterator;
					}
					const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
					ItemIterator->strFolder = Global->CtrlObject->CmdLine()->GetCurDir();
					if (ActivePanel->GetMode() == panel_mode::PLUGIN_PANEL)
					{
						OpenPanelInfo Info;
						ActivePanel->GetOpenPanelInfo(&Info);
						const auto ph = ActivePanel->GetPluginHandle();
						ItemIterator->PluginGuid = ph->plugin()->GetGUID();
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
				FolderList->UpdateItemFlags(FolderList->GetSelectPos(), std::exchange(MenuItem.Flags, 0));
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
					FolderList->Close();
				}
				else
				{
					EditItem(*FolderList, *ItemIterator, true);
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
		Save();
		Global->CtrlObject->Cp()->ActivePanel()->ExecShortcutFolder(ExitCode, raw_mode);
	}
}

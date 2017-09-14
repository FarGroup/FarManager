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

static const wchar_t SeparatorToken[] = L"--";

class Shortcuts::shortcut: public rel_ops<shortcut>
{
public:
	NONCOPYABLE(shortcut);
	MOVABLE(shortcut);

	shortcut(): PluginGuid(FarGuid) {}

	shortcut(const string& Name, const string& Folder, const string& PluginFile, const string& PluginData, const GUID& PluginGuid):
		Name(Name),
		Folder(Folder),
		PluginFile(PluginFile),
		PluginData(PluginData),
		PluginGuid(PluginGuid)
	{
	}

	bool operator==(const shortcut& rhs) const
	{
		const auto& tie = [](const shortcut& s)
		{
			return std::tie(s.Name, s.Folder, s.PluginGuid, s.PluginFile, s.PluginData);
		};

		return tie(*this) == tie(rhs);
	}

	shortcut clone() const
	{
		return shortcut(Name, Folder, PluginFile, PluginData, PluginGuid);
	}

	string Name;
	string Folder;
	string PluginFile;
	string PluginData;
	GUID PluginGuid;
};

Shortcuts::Shortcuts()
{
	const auto cfg = ConfigProvider().CreateShortcutsConfig();

	const auto root = cfg->FindByName(cfg->root_key(), FolderShortcutsKey);
	if (!root)
		return;

	for_each_cnt(RANGE(m_Items, i, size_t index)
	{
		i.clear();
		if (const auto key = cfg->FindByName(root, str(index)))
		{
			for(size_t j=0; ; j++)
			{
				const auto sIndex = str(j);

				shortcut Item;
				if (!cfg->GetValue(key, RecTypeName[PSCR_RT_SHORTCUT] + sIndex, Item.Folder))
					break;

				cfg->GetValue(key, RecTypeName[PSCR_RT_NAME] + sIndex, Item.Name);

				string strPluginGuid;
				cfg->GetValue(key, RecTypeName[PSCR_RT_PLUGINGUID] + sIndex, strPluginGuid);
				if(!StrToGuid(strPluginGuid, Item.PluginGuid))
					Item.PluginGuid=FarGuid;

				cfg->GetValue(key, RecTypeName[PSCR_RT_PLUGINFILE] + sIndex, Item.PluginFile);
				cfg->GetValue(key, RecTypeName[PSCR_RT_PLUGINDATA] + sIndex, Item.PluginData);

				i.emplace_back(std::move(Item));
			}
		}
	});
}

Shortcuts::~Shortcuts()
{
	Save();
}

void Shortcuts::Save()
{
	if (!m_Changed)
		return;

	const auto cfg = ConfigProvider().CreateShortcutsConfig();
	auto root = cfg->FindByName(cfg->root_key(), FolderShortcutsKey);
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(cfg->root_key(), FolderShortcutsKey);
	if (!root)
		return;

	for_each_cnt(CONST_RANGE(m_Items, i, size_t OuterIndex)
	{
		if (const auto Key = cfg->CreateKey(root, str(OuterIndex)))
		{
			for_each_cnt(CONST_RANGE(i, j, size_t InnerIndex)
			{
				const auto sIndex = str(InnerIndex);

				cfg->SetValue(Key, RecTypeName[PSCR_RT_SHORTCUT] + sIndex, j.Folder);
				cfg->SetValue(Key, RecTypeName[PSCR_RT_NAME] + sIndex, j.Name);

				if(j.PluginGuid != FarGuid)
				{
					cfg->SetValue(Key, RecTypeName[PSCR_RT_PLUGINGUID] + sIndex, GuidToStr(j.PluginGuid));
				}

				if(!j.PluginFile.empty())
				{
					cfg->SetValue(Key, RecTypeName[PSCR_RT_PLUGINFILE] + sIndex, j.PluginFile);
				}

				if(!j.PluginData.empty())
				{
					cfg->SetValue(Key, RecTypeName[PSCR_RT_PLUGINDATA] + sIndex, j.PluginData);
				}
			});
		}
	});
}

static void Fill(const Shortcuts::shortcut& RetItem, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(Folder)
		*Folder = os::env::expand(RetItem.Folder);

	if(PluginGuid)
		*PluginGuid = RetItem.PluginGuid;

	if(PluginFile)
		*PluginFile = RetItem.PluginFile;

	if(PluginData)
		*PluginData = RetItem.PluginData;
}

static string MakeName(const Shortcuts::shortcut& Item)
{
	if (!Item.Name.empty())
	{
		return os::env::expand(Item.Name);
	}

	if (Item.PluginGuid == FarGuid)
	{
		return !Item.Folder.empty()? os::env::expand(Item.Folder) : msg(lng::MShortcutNone);
	}

	const auto plugin = Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid);
	if (!plugin)
		return GuidToStr(Item.PluginGuid);

	string TechInfo;

	if (!Item.PluginFile.empty())
		append(TechInfo, msg(lng::MFSShortcutPluginFile), L' ', Item.PluginFile, L", "_sv);

	if (!Item.Folder.empty())
		append(TechInfo, msg(lng::MFSShortcutPath), L' ', Item.Folder, L", "_sv);

	if (!Item.PluginData.empty())
	{
		const string PrintablePluginData(Item.PluginData.cbegin(), std::find_if(ALL_CONST_RANGE(Item.PluginData), [](const auto i) { return i < L' '; }));
		if (!PrintablePluginData.empty())
			append(TechInfo, msg(lng::MFSShortcutPluginData), L' ', PrintablePluginData, L", "_sv);
	}

	return plugin->GetTitle() + (TechInfo.empty()? TechInfo : concat(L" ("_sv, TechInfo.substr(0, TechInfo.size() - 2), L')'));
}

static void FillMenu(VMenu2& Menu, std::list<Shortcuts::shortcut>& List, bool raw_mode)
{
	Menu.clear();
	FOR_RANGE(List, i)
	{
		MenuItemEx ListItem(MakeName(*i));
		if (ListItem.strName.empty())
			continue;

		ListItem.UserData = i;
		if (!raw_mode && i->PluginGuid == FarGuid && i->Folder.empty())
		{
			if (ListItem.strName != SeparatorToken)
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
	if (ActivePanel->GetMode() == panel_mode::NORMAL_PANEL)
		return true;

	OpenPanelInfo Info{};
	ActivePanel->GetOpenPanelInfo(&Info);
	return (Info.Flags & OPIF_SHORTCUT) != 0;
}

auto CreateShortcutFromPanel()
{
	Shortcuts::shortcut Shortcut;

	const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	Shortcut.Folder = Global->CtrlObject->CmdLine()->GetCurDir();

	if (ActivePanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		OpenPanelInfo Info{};
		ActivePanel->GetOpenPanelInfo(&Info);
		Shortcut.PluginGuid = ActivePanel->GetPluginHandle()->plugin()->GetGUID();
		Shortcut.PluginFile = NullToEmpty(Info.HostFile);
		Shortcut.PluginData = NullToEmpty(Info.ShortcutData);
	}
	return Shortcut;
}

bool Shortcuts::Get(size_t Pos, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if (m_Items[Pos].empty())
		return false;

	if (m_Items[Pos].size() == 1)
	{
		Fill(m_Items[Pos].front(), Folder, PluginGuid, PluginFile, PluginData);
		return true;
	}

	const auto Iterator = Select(Pos, false);
	if (Iterator == m_Items[Pos].cend())
		return false;

	Fill(*Iterator, Folder, PluginGuid, PluginFile, PluginData);
	return true;
}

std::list<Shortcuts::shortcut>::const_iterator Shortcuts::Select(size_t Pos, bool Raw)
{
	const auto FolderList = VMenu2::create(msg(lng::MFolderShortcutsTitle), nullptr, 0, ScrY - 4);
	FolderList->SetMenuFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
	FolderList->SetHelp(HelpFolderShortcuts);
	FolderList->SetBottomTitle(msg(lng::MFolderShortcutBottomSub));
	FolderList->SetId(FolderShortcutsMoreId);
	FillMenu(*FolderList, m_Items[Pos], Raw);

	int ExitCode=FolderList->Run([&](const Manager::Key& RawKey)
	{
		const auto Key=RawKey();
		int ItemPos = FolderList->GetSelectPos();
		const auto Item = FolderList->GetUserDataPtr<ITERATOR(m_Items[Pos])>(ItemPos);
		int KeyProcessed = 1;

		switch (Key)
		{
		case KEY_NUMPAD0:
		case KEY_INS:
			if (Accept())
			{
				const auto newIter = m_Items[Pos].emplace(Item? *Item : m_Items[Pos].end(), CreateShortcutFromPanel());

				MenuItemEx NewMenuItem(newIter->Folder);
				NewMenuItem.UserData = newIter;
				FolderList->AddItem(NewMenuItem, ItemPos);
				FolderList->SetSelectPos(ItemPos, 1);
				m_Changed = true;
			}
			break;

		case KEY_NUMDEL:
		case KEY_DEL:
			if(Item)
			{
				m_Items[Pos].erase(*Item);
				FolderList->DeleteItem(FolderList->GetSelectPos());
				m_Changed = true;
			}
			break;

		case KEY_F4:
			if (Item && EditItem(*FolderList, **Item, false, Raw))
			{
				m_Changed = true;
			}
			break;

		case KEY_CTRLUP:
		case KEY_RCTRLUP:
			if (Item && *Item != m_Items[Pos].begin())
			{
				m_Items[Pos].splice(std::prev(*Item), m_Items[Pos], *Item);
				FillMenu(*FolderList, m_Items[Pos], Raw);
				FolderList->SetSelectPos(--ItemPos);
				m_Changed = true;
			}
			break;

		case KEY_CTRLDOWN:
		case KEY_RCTRLDOWN:
			if (Item && std::next(*Item) != m_Items[Pos].end())
			{
				m_Items[Pos].splice(*Item, m_Items[Pos], std::next(*Item));
				FillMenu(*FolderList, m_Items[Pos], Raw);
				FolderList->SetSelectPos(++ItemPos);
				m_Changed = true;
			}
			break;

		default:
			KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	return ExitCode < 0? m_Items[Pos].end() : *FolderList->GetUserDataPtr<ITERATOR(m_Items[Pos])>(ExitCode);
}

bool Shortcuts::GetOne(size_t Pos, size_t Index, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(m_Items[Pos].size() <= Index)
		return false;

	const auto RetItem = std::next(m_Items[Pos].begin(), Index);
	Fill(*RetItem,Folder,PluginGuid,PluginFile,PluginData);
	return true;
}

void Shortcuts::Set(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	if(m_Items[Pos].empty())
		return Add(Pos, Folder, PluginGuid, PluginFile, PluginData);

	auto& Item = m_Items[Pos].front();
	Item.Folder = Folder;
	Item.PluginGuid = PluginGuid;
	Item.PluginFile = PluginFile;
	Item.PluginData = PluginData;
	m_Changed = true;
}

void Shortcuts::Add(size_t Pos, const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	m_Items[Pos].emplace_back(string{}, Folder, PluginFile, PluginData, PluginGuid);
	m_Changed = true;
}

void Shortcuts::MakeItemName(size_t Pos, MenuItemEx& MenuItem)
{
	auto ItemName = m_Items[Pos].empty()? msg(lng::MShortcutNone) : MakeName(m_Items[Pos].front());
	MenuItem.strName = concat(msg(lng::MRightCtrl), L"+&"_sv, str(Pos), L" \x2502 "_sv, ItemName);
	if(m_Items[Pos].size() > 1)
	{
		MenuItem.Flags |= MIF_SUBMENU;
	}
	else
	{
		MenuItem.Flags &= ~MIF_SUBMENU;
	}
}

bool Shortcuts::EditItem(VMenu2& Menu, shortcut& Item, bool Root, bool raw)
{
	auto NewItem = Item.clone();

	DialogBuilder Builder(lng::MFolderShortcutsTitle, HelpFolderShortcuts);
	Builder.AddText(lng::MFSShortcutName);
	Builder.AddEditField(NewItem.Name, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(lng::MFSShortcutPath);
	Builder.AddEditField(NewItem.Folder, 50, L"FS_Path", DIF_EDITPATH);
	if (Item.PluginGuid != FarGuid)
	{
		const auto plugin = Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid);
		Builder.AddSeparator(plugin? plugin->GetTitle().data() : GuidToStr(Item.PluginGuid).data());
		Builder.AddText(lng::MFSShortcutPluginFile);
		Builder.AddEditField(NewItem.PluginFile, 50, L"FS_Path", DIF_EDITPATH);
		Builder.AddText(lng::MFSShortcutPluginData);
		Builder.AddEditField(NewItem.PluginData, 50, L"FS_Path", DIF_EDITPATH);
	}
	Builder.SetId(FolderShortcutsDlgId);
	Builder.AddOKCancel();

	if (!Builder.ShowDialog())
		return false;

	if (NewItem == Item)
		return false;

	if (Item.PluginGuid == FarGuid)
	{
		if (NewItem.Folder.empty())
		{
			if (NewItem.Name.empty())
				NewItem.Name = SeparatorToken;
		}
		else if (!raw)
		{
			if (!os::fs::exists(os::env::expand(NewItem.Folder)))
			{
				Global->CatchError();
				if (Message(MSG_WARNING | MSG_ERRORTYPE,
					msg(lng::MError),
					{
						NewItem.Folder,
						msg(lng::MSaveThisShortcut)
					},
					{ lng::MYes, lng::MNo }) != Message::first_button)
				{
					return false;
				}
			}
		}
	}

	Item = std::move(NewItem);

	auto& MenuItem = Menu.current();
	if(Root)
	{
		MakeItemName(Menu.GetSelectPos(), MenuItem);
	}
	else
		MenuItem.strName = MakeName(Item);
	
	return true;
}

void Shortcuts::Configure()
{
	const auto FolderList = VMenu2::create(msg(lng::MFolderShortcutsTitle), nullptr, 0, ScrY - 4);
	FolderList->SetMenuFlags(VMENU_WRAPMODE);
	FolderList->SetHelp(HelpFolderShortcuts);
	FolderList->SetBottomTitle(msg(lng::MFolderShortcutBottom));
	FolderList->SetId(FolderShortcutsId);

	for (size_t i = 0; i < m_Items.size(); ++i)
	{
		MenuItemEx ListItem;
		MakeItemName(i, ListItem);
		FolderList->AddItem(ListItem);
	}

	int ExitCode=FolderList->Run([&](const Manager::Key& RawKey)
	{
		const auto Key=RawKey();
		int Pos = FolderList->GetSelectPos();
		int KeyProcessed = 1;

		const auto& UpdateItem = [&]
		{
			auto& MenuItem = FolderList->current();
			MakeItemName(Pos, MenuItem);
			FolderList->UpdateItemFlags(FolderList->GetSelectPos(), std::exchange(MenuItem.Flags, 0));
		};

		const auto& EditSubmenu = [&]
		{
			// We don't care about the result here, just letting the user to edit the submenu
			Select(Pos, true);
			UpdateItem();
		};

		switch (Key)
		{
		case KEY_NUMPAD0:
		case KEY_INS:
			// Direct insertion only allowed if the list is empty. Otherwise do it in a submenu.
			if (m_Items[Pos].empty())
			{
				if (Accept())
				{
					m_Items[Pos].emplace_back(CreateShortcutFromPanel());
					UpdateItem();
					m_Changed = true;
				}
			}
			else
			{
				EditSubmenu();
			}
			break;

		case KEY_NUMDEL:
		case KEY_DEL:
			if (!m_Items[Pos].empty())
			{
				// Direct deletion only allowed if there's exactly one item in the list. Otherwise do it in a submenu.
				if (m_Items[Pos].size() == 1)
				{
					m_Items[Pos].pop_front();
					UpdateItem();
					m_Changed = true;
				}
				else
				{
					EditSubmenu();
				}
			}
			break;

		case KEY_F4:
			if (!m_Items[Pos].empty())
			{
				// Direct editing only allowed if there's exactly one item in the list. Otherwise do it in a submenu.
				if (m_Items[Pos].size() == 1)
				{
					if (EditItem(*FolderList, m_Items[Pos].front(), true))
					{
						UpdateItem();
						m_Changed = true;
					}
				}
				else
				{
					EditSubmenu();
				}
			}
			break;

		default:
			KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	if (ExitCode < 0)
		return;

	Save();
	Global->CtrlObject->Cp()->ActivePanel()->ExecShortcutFolder(ExitCode);
}

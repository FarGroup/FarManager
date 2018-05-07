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

#include "shortcuts.hpp"

#include "keys.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "dialog.hpp"
#include "FarDlgBuilder.hpp"
#include "plugins.hpp"
#include "configdb.hpp"
#include "FarGuid.hpp"
#include "DlgGuid.hpp"
#include "lang.hpp"
#include "global.hpp"

#include "platform.env.hpp"
#include "platform.fs.hpp"

#include "common/rel_ops.hpp"

#include "format.hpp"

static const auto
	FolderShortcutsKey = L"Shortcuts"_sv,
	FolderName = L"Shortcut"_sv,
	NameName = L"Name"_sv,
	PluginGuidName = L"PluginGuid"_sv,
	PluginFileName = L"PluginFile"_sv,
	PluginDataName = L"PluginData"_sv,

	HelpFolderShortcuts = L"FolderShortcuts"_sv,
	SeparatorToken = L"--"_sv;

class Shortcuts::shortcut: public data, public rel_ops<shortcut>
{
public:
	shortcut() = default;

	shortcut(string Name, string Folder, string PluginFile, string PluginData, const GUID& PluginGuid):
		Name(std::move(Name))
	{
		this->Folder = std::move(Folder);
		this->PluginFile = std::move(PluginFile);
		this->PluginData = std::move(PluginData);
		this->PluginGuid = PluginGuid;
	}

	bool operator==(const shortcut& rhs) const
	{
		const auto& tie = [](const shortcut& s)
		{
			return std::tie(s.Name, s.Folder, s.PluginGuid, s.PluginFile, s.PluginData);
		};

		return tie(*this) == tie(rhs);
	}

	string Name;
};

Shortcuts::Shortcuts(size_t Index):
	m_KeyName(str(Index))
{
	const auto Cfg = ConfigProvider().CreateShortcutsConfig();

	const auto Root = Cfg->FindByName(Cfg->root_key(), FolderShortcutsKey);
	if (!Root)
		return;

	const auto Key = Cfg->FindByName(Root, m_KeyName);
	if (!Key)
		return;

	for (size_t i = 0; ; ++i)
	{
		const auto sIndex = str(i);

		shortcut Item;
		if (!Cfg->GetValue(Key, FolderName + sIndex, Item.Folder))
			break;

		Cfg->GetValue(Key, NameName + sIndex, Item.Name);

		string PluginGuid;
		Cfg->GetValue(Key, PluginGuidName + sIndex, PluginGuid);
		if(!StrToGuid(PluginGuid, Item.PluginGuid))
			Item.PluginGuid=FarGuid;

		Cfg->GetValue(Key, PluginFileName + sIndex, Item.PluginFile);
		Cfg->GetValue(Key, PluginDataName + sIndex, Item.PluginData);

		m_Items.emplace_back(std::move(Item));
	}
}

Shortcuts::~Shortcuts()
{
	Save();
}

void Shortcuts::Save()
{
	if (!*m_Changed)
		return;

	const auto Cfg = ConfigProvider().CreateShortcutsConfig();
	auto Root = Cfg->FindByName(Cfg->root_key(), FolderShortcutsKey);
	if (!Root)
		Root = Cfg->CreateKey(Cfg->root_key(), FolderShortcutsKey);

	auto Key = Cfg->FindByName(Root, m_KeyName);
	if (Key)
		Cfg->DeleteKeyTree(Key);

	Key = Cfg->CreateKey(Root, m_KeyName);
	if (!Key)
		return;

	for_each_cnt(CONST_RANGE(m_Items, Item, size_t Index)
	{
		const auto sIndex = str(Index);

		Cfg->SetValue(Key, FolderName + sIndex, Item.Folder);
		Cfg->SetValue(Key, NameName + sIndex, Item.Name);

		if(Item.PluginGuid != FarGuid)
		{
			Cfg->SetValue(Key, PluginGuidName + sIndex, GuidToStr(Item.PluginGuid));
		}

		if(!Item.PluginFile.empty())
		{
			Cfg->SetValue(Key, PluginFileName + sIndex, Item.PluginFile);
		}

		if(!Item.PluginData.empty())
		{
			Cfg->SetValue(Key, PluginDataName + sIndex, Item.PluginData);
		}
	});
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

	return plugin->GetTitle() + (TechInfo.empty()? TechInfo : concat(L" ("_sv, string_view(TechInfo).substr(0, TechInfo.size() - 2), L')'));
}

// Don't listen to static analysers - List MUST NOT be const. We store non-const iterators in type-erased UserData for further usage.
static void FillMenu(VMenu2& Menu, std::list<Shortcuts::shortcut>& List, bool const raw_mode)
{
	Menu.clear();
	FOR_RANGE(List, i)
	{
		MenuItemEx ListItem(MakeName(*i));
		if (ListItem.Name.empty())
			continue;

		ListItem.UserData = i;
		if (!raw_mode && i->PluginGuid == FarGuid && i->Folder.empty())
		{
			if (ListItem.Name != SeparatorToken)
			{
				if (Menu.empty())
					Menu.SetTitle(ListItem.Name);

				continue;
			}

			ListItem.Name.clear();
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

static auto CreateShortcutFromPanel()
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

static bool EditItemImpl(Shortcuts::shortcut& Item, bool raw)
{
	auto NewItem = Item;

	DialogBuilder Builder(lng::MFolderShortcutsTitle, HelpFolderShortcuts);
	Builder.AddText(lng::MFSShortcutName);
	Builder.AddEditField(NewItem.Name, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(lng::MFSShortcutPath);
	Builder.AddEditField(NewItem.Folder, 50, L"FS_Path", DIF_EDITPATH);
	if (Item.PluginGuid != FarGuid)
	{
		const auto plugin = Global->CtrlObject->Plugins->FindPlugin(Item.PluginGuid);
		Builder.AddSeparator(plugin? plugin->GetTitle().c_str() : GuidToStr(Item.PluginGuid).c_str());
		Builder.AddText(lng::MFSShortcutPluginFile);
		Builder.AddEditField(NewItem.PluginFile, 50, L"FS_PluginFile", DIF_EDITPATH);
		Builder.AddText(lng::MFSShortcutPluginData);
		Builder.AddEditField(NewItem.PluginData, 50, L"FS_PluginData", DIF_EDITPATH);
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
				assign(NewItem.Name, SeparatorToken);
		}
		else if (!raw)
		{
			if (!os::fs::exists(os::env::expand(NewItem.Folder)))
			{
				const auto ErrorState = error_state::fetch();

				if (Message(MSG_WARNING, ErrorState,
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

	return true;
}

static bool EditItem(VMenu2& Menu, Shortcuts::shortcut& Item, bool raw)
{
	if (!EditItemImpl(Item, raw))
		return false;

	auto& MenuItem = Menu.current();
	MenuItem.Name = MakeName(Item);
	return true;
}

bool Shortcuts::Get(data& Data)
{
	if (m_Items.empty())
		return false;

	if (m_Items.size() == 1)
	{
		Data = static_cast<const data&>(m_Items.front());
		return true;
	}

	const auto Iterator = Select(false);
	if (Iterator == m_Items.cend())
		return false;

	Data = static_cast<const data&>(*Iterator);
	return true;
}

std::list<Shortcuts::shortcut>::const_iterator Shortcuts::Select(bool Raw)
{
	const auto FolderList = VMenu2::create(msg(lng::MFolderShortcutsTitle), {}, ScrY - 4);
	FolderList->SetMenuFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
	FolderList->SetHelp(HelpFolderShortcuts);
	FolderList->SetBottomTitle(msg(lng::MFolderShortcutBottomSub));
	FolderList->SetId(FolderShortcutsMoreId);
	FillMenu(*FolderList, m_Items, Raw);

	const auto ExitCode = FolderList->Run([&](const Manager::Key& RawKey)
	{
		const auto Key = RawKey();
		const auto ItemPos = FolderList->GetSelectPos();
		const auto Iterator = FolderList->GetUserDataPtr<ITERATOR(m_Items)>(ItemPos);

		switch (Key)
		{
		case KEY_NUMPAD0:
		case KEY_INS:
			if (Accept())
			{
				const auto newIter = m_Items.emplace(Iterator? *Iterator : m_Items.end(), CreateShortcutFromPanel());

				MenuItemEx NewMenuItem(newIter->Folder);
				NewMenuItem.UserData = newIter;
				FolderList->AddItem(NewMenuItem, ItemPos);
				FolderList->SetSelectPos(ItemPos, 1);
				m_Changed = true;
			}
			return true;

		case KEY_NUMDEL:
		case KEY_DEL:
			if(Iterator)
			{
				m_Items.erase(*Iterator);
				FolderList->DeleteItem(FolderList->GetSelectPos());
				m_Changed = true;
			}
			return true;

		case KEY_F4:
			if (Iterator && EditItem(*FolderList, **Iterator, Raw))
			{
				m_Changed = true;
			}
			return true;

		case KEY_CTRLUP:
		case KEY_RCTRLUP:
			if (Iterator && *Iterator != m_Items.begin())
			{
				m_Items.splice(std::prev(*Iterator), m_Items, *Iterator);
				FillMenu(*FolderList, m_Items, Raw);
				FolderList->SetSelectPos(ItemPos - 1);
				m_Changed = true;
			}
			return true;

		case KEY_CTRLDOWN:
		case KEY_RCTRLDOWN:
			if (Iterator && std::next(*Iterator) != m_Items.end())
			{
				m_Items.splice(*Iterator, m_Items, std::next(*Iterator));
				FillMenu(*FolderList, m_Items, Raw);
				FolderList->SetSelectPos(ItemPos + 1);
				m_Changed = true;
			}
			return true;

		default:
			return false;
		}
	});

	return ExitCode < 0? m_Items.end() : *FolderList->GetUserDataPtr<ITERATOR(m_Items)>(ExitCode);
}

bool Shortcuts::GetOne(size_t Index, data& Data) const
{
	if (Index >= m_Items.size())
		return false;

	Data = static_cast<const data&>(*std::next(m_Items.begin(), Index));
	return true;
}

void Shortcuts::Add(const string& Folder, const GUID& PluginGuid, const string& PluginFile, const string& PluginData)
{
	m_Items.emplace_back(string{}, Folder, PluginFile, PluginData, PluginGuid);
	m_Changed = true;
}

static void MakeListName(const std::list<Shortcuts::shortcut>& List, const string& Key, MenuItemEx& MenuItem)
{
	const auto ItemName = List.empty()? msg(lng::MShortcutNone) : MakeName(List.front());
	MenuItem.Name = concat(msg(lng::MRightCtrl), L"+&"_sv, Key, L" \x2502 "_sv, ItemName);
	if (List.size() > 1)
	{
		MenuItem.Flags |= MIF_SUBMENU;
	}
	else
	{
		MenuItem.Flags &= ~MIF_SUBMENU;
	}
}

static bool EditListItem(const std::list<Shortcuts::shortcut>& List, VMenu2& Menu, Shortcuts::shortcut& Item, bool raw)
{
	if (!EditItemImpl(Item, raw))
		return false;

	auto& MenuItem = Menu.current();
	MakeListName(List, str(Menu.GetSelectPos()), MenuItem);
	return true;
}

int Shortcuts::Configure()
{
	constexpr auto ShortcutsNumber = 10;

	std::vector<Shortcuts> AllShortcuts;
	AllShortcuts.reserve(ShortcutsNumber);
	for (auto i: make_irange(ShortcutsNumber))
	{
		AllShortcuts.emplace_back(i);
	}

	const auto FolderList = VMenu2::create(msg(lng::MFolderShortcutsTitle), {}, ScrY - 4);
	FolderList->SetMenuFlags(VMENU_WRAPMODE);
	FolderList->SetHelp(HelpFolderShortcuts);
	FolderList->SetBottomTitle(msg(lng::MFolderShortcutBottom));
	FolderList->SetId(FolderShortcutsId);

	for (auto& i: AllShortcuts)
	{
		MenuItemEx ListItem;
		MakeListName(i.m_Items, i.m_KeyName, ListItem);
		FolderList->AddItem(ListItem);
	}

	const auto ExitCode = FolderList->Run([&](const Manager::Key& RawKey)
	{
		const auto Key=RawKey();
		const auto Pos = FolderList->GetSelectPos();

		const auto& UpdateItem = [&]
		{
			auto& MenuItem = FolderList->current();
			MakeListName(AllShortcuts[Pos].m_Items, str(Pos), MenuItem);
			FolderList->UpdateItemFlags(FolderList->GetSelectPos(), std::exchange(MenuItem.Flags, 0));
		};

		const auto& EditSubmenu = [&]
		{
			// We don't care about the result here, just letting the user to edit the submenu
			AllShortcuts[Pos].Select(true);
			UpdateItem();
		};

		auto& CurrentList = AllShortcuts[Pos];

		switch (Key)
		{
		case KEY_NUMPAD0:
		case KEY_INS:
			// Direct insertion only allowed if the list is empty. Otherwise do it in a submenu.
			if (CurrentList.m_Items.empty())
			{
				if (Accept())
				{
					CurrentList.m_Items.emplace_back(CreateShortcutFromPanel());
					CurrentList.m_Changed = true;
					UpdateItem();
				}
			}
			else
			{
				EditSubmenu();
			}
			return true;

		case KEY_NUMDEL:
		case KEY_DEL:
			if (!CurrentList.m_Items.empty())
			{
				// Direct deletion only allowed if there's exactly one item in the list. Otherwise do it in a submenu.
				if (CurrentList.m_Items.size() == 1)
				{
					CurrentList.m_Items.pop_front();
					CurrentList.m_Changed = true;
					UpdateItem();
				}
				else
				{
					EditSubmenu();
				}
			}
			return true;

		case KEY_F4:
			if (!CurrentList.m_Items.empty())
			{
				// Direct editing only allowed if there's exactly one item in the list. Otherwise do it in a submenu.
				if (CurrentList.m_Items.size() == 1)
				{
					if (EditListItem(AllShortcuts[Pos].m_Items, *FolderList, CurrentList.m_Items.front(), true))
					{
						CurrentList.m_Changed = true;
						UpdateItem();
					}
				}
				else
				{
					EditSubmenu();
				}
			}
			return true;

		default:
			return false;
		}
	});

	return ExitCode < 0? -1 : ExitCode;
}

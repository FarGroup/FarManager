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


ShortcutItem::ShortcutItem()
{
	PluginGuid=FarGuid;
}

Shortcuts::Shortcuts()
{
	Changed = false;

	HierarchicalConfig *cfg = CreateShortcutsConfig();
	unsigned __int64 root = cfg->GetKeyID(0,FolderShortcutsKey);

	if (root)
	{
		for(size_t i = 0; i < KeyCount; i++)
		{
			unsigned __int64 key = cfg->GetKeyID(root, FormatString() << i);
			if (!key)
				continue;

			for(size_t j=0; ; j++)
			{
				FormatString ValueName;
				ValueName << RecTypeName[PSCR_RT_SHORTCUT] << j;
				string strValue;
				if (!cfg->GetValue(key, ValueName, strValue))
					break;
				ShortcutItem* Item = Items[i].Push();
				Item->strFolder = strValue;

				ValueName.Clear();
				ValueName << RecTypeName[PSCR_RT_NAME] << j;
				cfg->GetValue(key, ValueName, Item->strName);

				ValueName.Clear();
				ValueName << RecTypeName[PSCR_RT_PLUGINGUID] << j;
				string strPluginGuid;
				cfg->GetValue(key, ValueName, strPluginGuid);
				if(!StrToGuid(strPluginGuid,Item->PluginGuid)) Item->PluginGuid=FarGuid;

				ValueName.Clear();
				ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << j;
				cfg->GetValue(key, ValueName, Item->strPluginFile);

				ValueName.Clear();
				ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << j;
				cfg->GetValue(key, ValueName, Item->strPluginData);
			}
		}
	}

	delete cfg;
}

Shortcuts::~Shortcuts()
{
	if (!Changed)
		return;

	HierarchicalConfig *cfg = CreateShortcutsConfig();
	unsigned __int64 root = cfg->GetKeyID(0,FolderShortcutsKey);
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(0,FolderShortcutsKey);

	if (root)
	{
		for (size_t i = 0; i < KeyCount; i++)
		{
			unsigned __int64 key = cfg->CreateKey(root, FormatString() << i);
			if (!key)
				continue;

			int index = 0;
			for (ShortcutItem* j = Items[i].First(); j; j = Items[i].Next(j), index++)
			{
				FormatString ValueName;
				ValueName << RecTypeName[PSCR_RT_SHORTCUT] << index;
				cfg->SetValue(key, ValueName, j->strFolder);

				ValueName.Clear();
				ValueName << RecTypeName[PSCR_RT_NAME] << index;
				cfg->SetValue(key, ValueName, j->strName);

				if(!IsEqualGUID(FarGuid,j->PluginGuid))
				{
					ValueName.Clear();
					ValueName << RecTypeName[PSCR_RT_PLUGINGUID] << index;
					string strPluginGuid=GuidToStr(j->PluginGuid);
					cfg->SetValue(key, ValueName, strPluginGuid);
				}

				if(!j->strPluginFile.IsEmpty())
				{
					ValueName.Clear();
					ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << index;
					cfg->SetValue(key, ValueName, j->strPluginFile);
				}

				if(!j->strPluginData.IsEmpty())
				{
					ValueName.Clear();
					ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << index;
					cfg->SetValue(key, ValueName, j->strPluginData);
				}
			}
		}
	}

	delete cfg;
}

static void Fill(ShortcutItem* RetItem, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(Folder)
	{
		*Folder = RetItem->strFolder;
		apiExpandEnvironmentStrings(*Folder, *Folder);
	}
	if(PluginGuid)
	{
		*PluginGuid = RetItem->PluginGuid;
	}
	if(PluginFile)
	{
		*PluginFile = RetItem->strPluginFile;
	}
	if(PluginData)
	{
		*PluginData = RetItem->strPluginData;
	}
}

bool Shortcuts::Get(size_t Pos, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	bool Result = false;
	if(!Items[Pos].Empty())
	{
		ShortcutItem* RetItem = nullptr;
		if(Items[Pos].Count()>1)
		{
			VMenu FolderList(MSG(MFolderShortcutsTitle),nullptr,0,ScrY-4);
			FolderList.SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
			FolderList.SetHelp(HelpFolderShortcuts);
			FolderList.SetPosition(-1,-1,0,0);
			FolderList.SetBottomTitle(MSG(MFolderShortcutBottomSub));
			for(ShortcutItem* i = Items[Pos].First(); i; i = Items[Pos].Next(i))
			{
				MenuItemEx ListItem={};
				string strFolderName;
				if(!i->strName.IsEmpty())
				{
					strFolderName = i->strName;
				}
				else if(!i->strFolder.IsEmpty())
				{
					strFolderName = i->strFolder;
				}
				else
				{
					strFolderName = MSG(IsEqualGUID(FarGuid,i->PluginGuid)?MShortcutNone:MShortcutPlugin);
				}
				ListItem.strName = strFolderName;
				ListItem.UserData = &i;
				ListItem.UserDataSize = sizeof(i);
				FolderList.AddItem(&ListItem);
			}
			FolderList.Show();
			while (!FolderList.Done())
			{
				DWORD Key=FolderList.ReadInput();
				int ItemPos = FolderList.GetSelectPos();
				void* Data = FolderList.GetUserData(nullptr, 0, ItemPos);
				ShortcutItem* Item = Data?*static_cast<ShortcutItem**>(Data):nullptr;
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
							ShortcutItem* NewItem = Items[Pos].InsertBefore(Item);
							Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
							CtrlObject->CmdLine->GetCurDir(NewItem->strFolder);
							if (ActivePanel->GetMode() == PLUGIN_PANEL)
							{
								OpenPanelInfo Info;
								ActivePanel->GetOpenPanelInfo(&Info);
								string strTemp;
								PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();
								NewItem->PluginGuid = ph->pPlugin->GetGUID();
								NewItem->strPluginFile = Info.HostFile;
								NewItem->strPluginData = Info.ShortcutData;
							}
							else
							{
								Item->PluginGuid = FarGuid;
								Item->strPluginFile = L"";
								Item->strPluginData = L"";
							}
							MenuItemEx NewMenuItem = {};
							NewMenuItem.strName = NewItem->strFolder;
							NewMenuItem.UserData = &NewItem;
							NewMenuItem.UserDataSize = sizeof(NewItem);
							FolderList.AddItem(&NewMenuItem, ItemPos);
							FolderList.SetSelectPos(ItemPos, 1);
						}
						else
						{
							if(!Items[Pos].Empty())
							{
								Items[Pos].Delete(Item);
								FolderList.DeleteItem(FolderList.GetSelectPos());
							}
						}
						FolderList.SetPosition(-1, -1, -1, -1);
						FolderList.SetUpdateRequired(TRUE);
						FolderList.Show();
					}
					break;

				case KEY_F4:
					{
						EditItem(&FolderList, Item, false);
					}
					break;

				default:
					FolderList.ProcessInput();
					break;
				}
			}
			int ExitCode = FolderList.GetExitCode();
			if (ExitCode>=0)
			{
				void* Data = FolderList.GetUserData(nullptr, 0, ExitCode);
				RetItem = Data?*static_cast<ShortcutItem**>(Data):nullptr;
			}
		}
		else
		{
			RetItem = Items[Pos].First();
		}

		if(RetItem)
		{
			Fill(RetItem,Folder,PluginGuid,PluginFile,PluginData);
			Result = true;
		}
	}
	return Result;
}

bool Shortcuts::Get(size_t Pos, size_t Index, string* Folder, GUID* PluginGuid, string* PluginFile, string* PluginData)
{
	if(Items[Pos].Count()<=Index) return FALSE;
	ShortcutItem* RetItem = Items[Pos].First();
	while(Index--) RetItem = Items[Pos].Next(RetItem);
	Fill(RetItem,Folder,PluginGuid,PluginFile,PluginData);
	return TRUE;
}

void Shortcuts::Set(size_t Pos, const wchar_t* Folder, const GUID& PluginGuid, const wchar_t* PluginFile, const wchar_t* PluginData)
{
	Changed = true;
	ShortcutItem* Item = Items[Pos].Empty()?Items[Pos].Push():Items[Pos].First();
	Item->strFolder = Folder;
	Item->PluginGuid = PluginGuid;
	Item->strPluginFile = PluginFile;
	Item->strPluginData = PluginData;
}

void Shortcuts::Add(size_t Pos, const wchar_t* Folder, const GUID& PluginGuid, const wchar_t* PluginFile, const wchar_t* PluginData)
{
	Changed = true;
	ShortcutItem* Item = Items[Pos].Push();
	Item->strFolder = Folder;
	Item->PluginGuid = PluginGuid;
	Item->strPluginFile = PluginFile;
	Item->strPluginData = PluginData;
}

void Shortcuts::MakeItemName(size_t Pos, MenuItemEx* MenuItem)
{
	const wchar_t* Ptr = L"";
	if(!Items[Pos].Empty())
	{
		if(!Items[Pos].First()->strName.IsEmpty())
		{
			Ptr = Items[Pos].First()->strName;
		}
		else if(!Items[Pos].First()->strFolder.IsEmpty())
		{
			Ptr = Items[Pos].First()->strFolder;
		}
		else
		{
			Ptr = MSG(IsEqualGUID(FarGuid,Items[Pos].First()->PluginGuid)?MShortcutNone:MShortcutPlugin);
		}
	}

	MenuItem->strName = FormatString() << MSG(MRightCtrl) << L"+&" << Pos << L" \x2502 " << Ptr;
	if(Items[Pos].Count() > 1)
	{
		MenuItem->Flags|=MIF_SUBMENU;
	}
	else
	{
		MenuItem->Flags&=~MIF_SUBMENU;
	}
}

void Shortcuts::EditItem(VMenu* Menu, ShortcutItem* Item, bool Root)
{
	string strNewName = Item->strName;
	string strNewDir = Item->strFolder;

	DialogBuilder Builder(MFolderShortcutsTitle, HelpFolderShortcuts);
	Builder.AddText(MFSShortcutName);
	Builder.AddEditField(&strNewName, 50, L"FS_Name", DIF_EDITPATH);
	Builder.AddText(MFSShortcutPath);
	Builder.AddEditField(&strNewDir, 50, L"FS_Path", DIF_EDITPATH);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Unquote(strNewDir);

		bool Root = false;
		PATH_TYPE Type = ParsePath(strNewDir, nullptr, &Root);
		if(!(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID)))
		{
			DeleteEndSlash(strNewDir);
		}

		bool Save=true;
		string strTemp(strNewDir);
		apiExpandEnvironmentStrings(strNewDir,strTemp);

		if (apiGetFileAttributes(strTemp) == INVALID_FILE_ATTRIBUTES)
		{
			Save=!Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), strNewDir, MSG(MSaveThisShortcut), MSG(MYes), MSG(MNo));
		}

		if (Save && (Item->strFolder != strNewDir || Item->strName != strNewName))
		{
			Changed = true;
			Item->strPluginData.Clear();
			Item->strPluginFile.Clear();
			Item->PluginGuid=FarGuid;
			Item->strName = strNewName;
			Item->strFolder = strNewDir;

			MenuItemEx* MenuItem = Menu->GetItemPtr();
			MenuItem->strName = Item->strName.IsEmpty()? Item->strFolder : Item->strName;
			if(Root)
			{
				MakeItemName(Menu->GetSelectPos(), MenuItem);
			}
			Menu->SetPosition(-1, -1, -1, -1);
			Menu->SetUpdateRequired(TRUE);
			Menu->Show();
		}
	}
}

void Shortcuts::Configure()
{
	Changed = true;
	VMenu FolderList(MSG(MFolderShortcutsTitle),nullptr,0,ScrY-4);
	FolderList.SetFlags(VMENU_WRAPMODE);
	FolderList.SetHelp(HelpFolderShortcuts);
	FolderList.SetPosition(-1,-1,0,0);
	FolderList.SetBottomTitle(MSG(MFolderShortcutBottom));

	for (int I=0; I < 10; I++)
	{
		MenuItemEx ListItem={};
		MakeItemName(I, &ListItem);
		FolderList.AddItem(&ListItem);
	}

	FolderList.Show();

	while (!FolderList.Done())
	{
		DWORD Key=FolderList.ReadInput();
		int Pos = FolderList.GetSelectPos();
		ShortcutItem* Item = Items[Pos].First();

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
					if(!Item || !(Key&KEY_SHIFT))
					{
						Item = Items[Pos].Push();
					}
					Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
					CtrlObject->CmdLine->GetCurDir(Item->strFolder);
					if (ActivePanel->GetMode() == PLUGIN_PANEL)
					{
						OpenPanelInfo Info;
						ActivePanel->GetOpenPanelInfo(&Info);
						string strTemp;
						PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();
						Item->PluginGuid = ph->pPlugin->GetGUID();
						Item->strPluginFile = Info.HostFile;
						Item->strPluginData = Info.ShortcutData;
					}
					else
					{
						Item->PluginGuid = FarGuid;
						Item->strPluginFile = L"";
						Item->strPluginData = L"";
					}
					MakeItemName(Pos, MenuItem);
				}
				else
				{
					if(Item)
					{
						Items[Pos].Delete(Item);
						MakeItemName(Pos, MenuItem);
					}
				}
				INT64 Flags = MenuItem->Flags;
				MenuItem->Flags = 0;
				FolderList.UpdateItemFlags(FolderList.GetSelectPos(), Flags);
				FolderList.SetPosition(-1, -1, -1, -1);
				FolderList.SetUpdateRequired(TRUE);
				FolderList.Show();
			}
			break;

		case KEY_F4:
			{
				if(!Item)
				{
					Item = Items[Pos].Push();
				}
				if(Items[Pos].Count()>1)
				{
					FolderList.ProcessKey(KEY_ENTER);
				}
				else
				{
					EditItem(&FolderList, Item, true);
				}
			}
			break;

		default:
			FolderList.ProcessInput();
			break;
		}
	}

	FolderList.Hide();

	int ExitCode = FolderList.Modal::GetExitCode();

	if(ExitCode>=0)
	{
		CtrlObject->Cp()->ActivePanel->ExecShortcutFolder(ExitCode);
	}
}

bool Shortcuts::Accept(void)
{
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	if (ActivePanel->GetMode() == PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		ActivePanel->GetOpenPanelInfo(&Info);
		if(!(Info.Flags&OPIF_SHORTCUT)) return false;
	}
	return true;
}

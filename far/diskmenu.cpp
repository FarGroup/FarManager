/*
diskmenu.cpp
*/
/*
Copyright © 2016 Far Group
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

#include "diskmenu.hpp"

#include "global.hpp"
#include "config.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "strmix.hpp"
#include "ctrlobj.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "FarDlgBuilder.hpp"
#include "DlgGuid.hpp"
#include "interf.hpp"
#include "drivemix.hpp"
#include "network.hpp"
#include "elevation.hpp"
#include "cddrv.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "stddlg.hpp"
#include "eject.hpp"
#include "hotplug.hpp"
#include "setattr.hpp"
#include "panel.hpp"
#include "filepanels.hpp"
#include "execute.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"
#include "plugapi.hpp"
#include "message.hpp"
#include "keyboard.hpp"
#include "dirmix.hpp"
#include "lockscrn.hpp"
#include "string_sort.hpp"
#include "pathmix.hpp"

#include "platform.reg.hpp"
#include "platform.fs.hpp"

#include "format.hpp"

class ChDiskPluginItem
{
public:
	NONCOPYABLE(ChDiskPluginItem);
	MOVABLE(ChDiskPluginItem);

	ChDiskPluginItem():
		HotKey()
	{}

	bool operator <(const ChDiskPluginItem& rhs) const
	{
		return (Global->Opt->ChangeDriveMode & DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey != rhs.HotKey)?
			HotKey < rhs.HotKey :
			string_sort::less(Item.Name, rhs.Item.Name);
	}

	MenuItemEx& getItem() { return Item; }
	WCHAR& getHotKey() { return HotKey; }

private:
	MenuItemEx Item;
	WCHAR HotKey;
};

struct PanelMenuItem
{
	bool bIsPlugin;

	union
	{
		struct
		{
			Plugin *pPlugin;
			GUID Guid;
		};

		struct
		{
			wchar_t cDrive;
			int nDriveType;
		};
	};
};

static size_t AddPluginItems(VMenu2 &ChDisk, int Pos, int DiskCount, bool SetSelected)
{
	std::list<ChDiskPluginItem> MPItems;
	bool ItemPresent, Done = false;
	string strPluginText;

	for (const auto& pPlugin: *Global->CtrlObject->Plugins)
	{
		if (Done)
			break;
		for (int PluginItem = 0;; ++PluginItem)
		{
			WCHAR HotKey = 0;
			GUID guid;
			if (!Global->CtrlObject->Plugins->GetDiskMenuItem(
				pPlugin,
				PluginItem,
				ItemPresent,
				HotKey,
				strPluginText,
				guid
				))
			{
				Done = true;
				break;
			}

			if (!ItemPresent)
				break;

			if (!strPluginText.empty())
			{
				ChDiskPluginItem OneItem;
#ifndef NO_WRAPPER
				if (pPlugin->IsOemPlugin())
					OneItem.getItem().Flags = LIF_CHECKED | L'A';
#endif // NO_WRAPPER
				OneItem.getItem().Name = strPluginText;
				OneItem.getHotKey() = HotKey;

				PanelMenuItem item = {};
				item.bIsPlugin = true;
				item.pPlugin = pPlugin;
				item.Guid = guid;
				OneItem.getItem().ComplexUserData = item;

				MPItems.emplace_back(std::move(OneItem));
			}
		}
	}

	if (MPItems.empty())
		return 0;

	MPItems.sort();
	MenuItemEx ChDiskItem;
	ChDiskItem.Flags |= LIF_SEPARATOR;
	ChDisk.AddItem(ChDiskItem);

	for_each_cnt(RANGE(MPItems, i, size_t index)
	{
		if (Pos > DiskCount && !SetSelected)
		{
			i.getItem().SetSelect(DiskCount + static_cast<int>(index)+1 == Pos);

			if (!SetSelected)
				SetSelected = DiskCount + static_cast<int>(index)+1 == Pos;
		}
		const auto HotKey = i.getHotKey();
		i.getItem().Name = concat(HotKey ? concat(L'&', HotKey, L"  "sv) : L"   "s, i.getItem().Name);
		ChDisk.AddItem(i.getItem());
	});

	return MPItems.size();
}

static void ConfigureChangeDriveMode()
{
	DialogBuilder Builder(lng::MChangeDriveConfigure, L"ChangeDriveMode"sv);
	Builder.SetId(ChangeDriveModeId);
	Builder.AddCheckbox(lng::MChangeDriveShowDiskType, Global->Opt->ChangeDriveMode, DRIVE_SHOW_TYPE);
	Builder.AddCheckbox(lng::MChangeDriveShowLabel, Global->Opt->ChangeDriveMode, DRIVE_SHOW_LABEL);
	Builder.AddCheckbox(lng::MChangeDriveShowFileSystem, Global->Opt->ChangeDriveMode, DRIVE_SHOW_FILESYSTEM);

	BOOL ShowSizeAny = Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);

	DialogItemEx *ShowSize = Builder.AddCheckbox(lng::MChangeDriveShowSize, &ShowSizeAny);
	DialogItemEx *ShowSizeFloat = Builder.AddCheckbox(lng::MChangeDriveShowSizeFloat, Global->Opt->ChangeDriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat->Indent(4);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(lng::MChangeDriveShowPath, Global->Opt->ChangeDriveMode, DRIVE_SHOW_PATH);
	Builder.AddCheckbox(lng::MChangeDriveShowPlugins, Global->Opt->ChangeDriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(lng::MChangeDriveSortPluginsByHotkey, Global->Opt->ChangeDriveMode, DRIVE_SORT_PLUGINS_BY_HOTKEY)->Indent(4);
	Builder.AddCheckbox(lng::MChangeDriveShowRemovableDrive, Global->Opt->ChangeDriveMode, DRIVE_SHOW_REMOVABLE);
	Builder.AddCheckbox(lng::MChangeDriveShowCD, Global->Opt->ChangeDriveMode, DRIVE_SHOW_CDROM);
	Builder.AddCheckbox(lng::MChangeDriveShowNetworkDrive, Global->Opt->ChangeDriveMode, DRIVE_SHOW_REMOTE);

	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (ShowSizeAny)
		{
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT)
				Global->Opt->ChangeDriveMode &= ~DRIVE_SHOW_SIZE;
			else
				Global->Opt->ChangeDriveMode |= DRIVE_SHOW_SIZE;
		}
		else
			Global->Opt->ChangeDriveMode &= ~(DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);
	}
}

class separator
{
public:
	separator():m_value(L' '){}
	string Get()
	{
		if (m_value == L' ')
		{
			m_value = BoxSymbols[BS_V1];
			return { L' ' };
		}
		return { L' ', m_value, L' '};
	}
private:
	wchar_t m_value;
};

enum
{
	DRIVE_DEL_FAIL,
	DRIVE_DEL_SUCCESS,
	DRIVE_DEL_EJECT,
	DRIVE_DEL_NONE
};

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile)
{
	/*
	0         1         2         3         4         5         6         7
	0123456789012345678901234567890123456789012345678901234567890123456789012345
	0
	1   +-------- Отключение сетевого устройства --------+
	2   | Вы хотите удалить соединение с устройством C:? |
	3   | На устройство %c: отображен каталог            |
	4   | \\host\share                                   |
	6   +------------------------------------------------+
	7   | [ ] Восстанавливать при входе в систему        |
	8   +------------------------------------------------+
	9   |              [ Да ]   [ Отмена ]               |
	10  +------------------------------------------------+
	11
	*/
	FarDialogItem DCDlgData[] =
	{
		{ DI_DOUBLEBOX, 3, 1, 72, 9, 0, nullptr, nullptr, 0, msg(lng::MChangeDriveDisconnectTitle).c_str() },
		{ DI_TEXT, 5, 2, 0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND, L"" },
		{ DI_TEXT, 5, 3, 0, 3, 0, nullptr, nullptr, DIF_SHOWAMPERSAND, L"" },
		{ DI_TEXT, 5, 4, 0, 4, 0, nullptr, nullptr, DIF_SHOWAMPERSAND, L"" },
		{ DI_TEXT, -1, 5, 0, 5, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_CHECKBOX, 5, 6, 70, 6, 0, nullptr, nullptr, 0, msg(lng::MChangeDriveDisconnectReconnect).c_str() },
		{ DI_TEXT, -1, 7, 0, 7, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_BUTTON, 0, 8, 0, 8, 0, nullptr, nullptr, DIF_FOCUS | DIF_DEFAULTBUTTON | DIF_CENTERGROUP, msg(lng::MYes).c_str() },
		{ DI_BUTTON, 0, 8, 0, 8, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MCancel).c_str() },
	};
	auto DCDlg = MakeDialogItemsEx(DCDlgData);

	DCDlg[1].strData = format(msg(lng::MChangeDriveDisconnectQuestion), Letter);
	DCDlg[2].strData = format(msg(lng::MChangeDriveDisconnectMapped), Letter);

	string strMsgText;
	// TODO: check result
	DriveLocalToRemoteName(DRIVE_REMOTE, Letter, strMsgText);
	const auto Len = std::max({ DCDlg[0].strData.size(), DCDlg[1].strData.size(), DCDlg[2].strData.size(), DCDlg[5].strData.size() });
	DCDlg[3].strData = TruncPathStr(strMsgText, static_cast<int>(Len));
	// проверяем - это было постоянное соединение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.

	bool IsPersistent = true;
	if (os::reg::key::open(os::reg::key::current_user, concat(L"Network\\"sv, Letter), KEY_QUERY_VALUE))
	{
		DCDlg[5].Selected = Global->Opt->ChangeDriveDisconnectMode;
	}
	else
	{
		DCDlg[5].Flags |= DIF_DISABLE;
		DCDlg[5].Selected = 0;
		IsPersistent = false;
	}

	// скорректируем размеры диалога - для дизайнУ
	DCDlg[0].X2 = DCDlg[0].X1 + Len + 3;
	int ExitCode = 7;

	if (Global->Opt->Confirm.RemoveConnection)
	{
		const auto Dlg = Dialog::create(DCDlg);
		Dlg->SetPosition(-1, -1, DCDlg[0].X2 + 4, 11);
		Dlg->SetHelp(L"DisconnectDrive"sv);
		Dlg->SetId(DisconnectDriveId);
		Dlg->SetDialogMode(DMODE_WARNINGSTYLE);
		Dlg->Process();
		ExitCode = Dlg->GetExitCode();
	}

	UpdateProfile = DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Global->Opt->ChangeDriveDisconnectMode = DCDlg[5].Selected == BSTATE_CHECKED;

	return ExitCode == 7;
}

static int ProcessDelDisk(panel_ptr Owner, wchar_t Drive, int DriveType)
{
	const auto DiskLetter = os::fs::get_drive(Drive);

	switch (DriveType)
	{
	case DRIVE_SUBSTITUTE:
		{
			if (Global->Opt->Confirm.RemoveSUBST)
			{
				const auto Question = format(msg(lng::MChangeSUBSTDisconnectDriveQuestion), DiskLetter);
				const auto MappedTo = format(msg(lng::MChangeDriveDisconnectMapped), DiskLetter.front());
				string SubstitutedPath;
				GetSubstName(DriveType, DiskLetter, SubstitutedPath);
				if (Message(MSG_WARNING,
					msg(lng::MChangeSUBSTDisconnectDriveTitle),
					{
						Question,
						MappedTo,
						SubstitutedPath
					},
					{ lng::MYes, lng::MNo },
					{}, &SUBSTDisconnectDriveId) != Message::first_button)
				{
					return DRIVE_DEL_FAIL;
				}
			}

			if (DelSubstDrive(DiskLetter))
			{
				return DRIVE_DEL_SUCCESS;
			}

			const auto ErrorState = error_state::fetch();

			const auto LastError = ErrorState.Win32Error;
			const auto strMsgText = format(msg(lng::MChangeDriveCannotDelSubst), DiskLetter);
			if (LastError == ERROR_OPEN_FILES || LastError == ERROR_DEVICE_IN_USE)
			{
				if (Message(MSG_WARNING, ErrorState,
					msg(lng::MError),
					{
						strMsgText,
						L"\x1"s,
						msg(lng::MChangeDriveOpenFiles),
						msg(lng::MChangeDriveAskDisconnect)
					},
					{ lng::MOk, lng::MCancel },
					{}, &SUBSTDisconnectDriveError1Id) == Message::first_button)
				{
					if (DelSubstDrive(DiskLetter))
					{
						return DRIVE_DEL_SUCCESS;
					}
				}
				else
				{
					return DRIVE_DEL_FAIL;
				}
			}
			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					strMsgText
				},
				{ lng::MOk },
				{}, &SUBSTDisconnectDriveError2Id);
			return DRIVE_DEL_FAIL; // блин. в прошлый раз забыл про это дело...
		}

	case DRIVE_REMOTE:
	case DRIVE_REMOTE_NOT_CONNECTED:
		{
			int UpdateProfile = CONNECT_UPDATE_PROFILE;
			if (!MessageRemoveConnection(Drive, UpdateProfile))
				return DRIVE_DEL_FAIL;

			{
				// <КОСТЫЛЬ>
				SCOPED_ACTION(LockScreen);
				// если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
				// удалению
				Owner->IfGoHome(Drive);
				Global->WindowManager->ResizeAllWindows();
				Global->WindowManager->GetCurrentWindow()->Show();
				// </КОСТЫЛЬ>
			}

			if (WNetCancelConnection2(DiskLetter.c_str(), UpdateProfile, FALSE) == NO_ERROR)
				return DRIVE_DEL_SUCCESS;

			const auto ErrorState = error_state::fetch();

			const auto strMsgText = format(msg(lng::MChangeDriveCannotDisconnect), DiskLetter);
			const auto LastError = ErrorState.Win32Error;
			if (LastError == ERROR_OPEN_FILES || LastError == ERROR_DEVICE_IN_USE)
			{
				if (Message(MSG_WARNING, ErrorState,
					msg(lng::MError),
					{
						strMsgText,
						L"\x1"s,
						msg(lng::MChangeDriveOpenFiles),
						msg(lng::MChangeDriveAskDisconnect)
					},
					{ lng::MOk, lng::MCancel },
					{}, &RemoteDisconnectDriveError1Id) == Message::first_button)
				{
					if (WNetCancelConnection2(DiskLetter.c_str(), UpdateProfile, TRUE) == NO_ERROR)
					{
						return DRIVE_DEL_SUCCESS;
					}
				}
				else
				{
					return DRIVE_DEL_FAIL;
				}
			}

			if (FAR_GetDriveType(os::fs::get_root_directory(Drive)) == DRIVE_REMOTE)
			{
				Message(MSG_WARNING, ErrorState,
					msg(lng::MError),
					{
						strMsgText
					},
					{ lng::MOk },
					{}, &RemoteDisconnectDriveError2Id);
			}
			return DRIVE_DEL_FAIL;
		}

	case DRIVE_VIRTUAL:
		{
			if (Global->Opt->Confirm.DetachVHD)
			{
				const auto Question = format(msg(lng::MChangeVHDDisconnectDriveQuestion), DiskLetter);
				if (Message(MSG_WARNING,
					msg(lng::MChangeVHDDisconnectDriveTitle),
					{
						Question
					},
					{ lng::MYes, lng::MNo },
					{}, &VHDDisconnectDriveId) != Message::first_button)
				{
					return DRIVE_DEL_FAIL;
				}
			}

			string strVhdPath;
			VIRTUAL_STORAGE_TYPE VirtualStorageType;
			error_state ErrorState;

			if (GetVHDInfo(DiskLetter, strVhdPath, &VirtualStorageType) && !strVhdPath.empty())
			{
				if (os::fs::detach_virtual_disk(strVhdPath, VirtualStorageType))
				{
					return DRIVE_DEL_SUCCESS;
				}

				ErrorState = error_state::fetch();
			}
			else
			{
				ErrorState = error_state::fetch();
			}

			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					format(msg(lng::MChangeDriveCannotDetach), DiskLetter)
				},
				{ lng::MOk },
				{}, &VHDDisconnectDriveErrorId);
			return DRIVE_DEL_FAIL;
		}

	default:
		return DRIVE_DEL_FAIL;
	}
}

static int DisconnectDrive(panel_ptr Owner, const PanelMenuItem *item, VMenu2 &ChDisk)
{
	if ((item->nDriveType == DRIVE_REMOVABLE) || IsDriveTypeCDROM(item->nDriveType))
	{
		if ((item->nDriveType == DRIVE_REMOVABLE) && !IsEjectableMedia(item->cDrive))
			return -1;

		// первая попытка извлечь диск

		if (!EjectVolume(item->cDrive, EJECT_NO_MESSAGE))
		{
			// запоминаем состояние панелей
			const auto CMode = Owner->GetMode();
			const auto AMode = Owner->Parent()->GetAnotherPanel(Owner)->GetMode();
			string strTmpCDir(Owner->GetCurDir()), strTmpADir(Owner->Parent()->GetAnotherPanel(Owner)->GetCurDir());
			// "цикл до умопомрачения"
			bool DoneEject = false;

			while (!DoneEject)
			{
				// "освободим диск" - перейдем при необходимости в домашний каталог
				// TODO: А если домашний каталог - CD? ;-)
				Owner->IfGoHome(item->cDrive);
				// очередная попытка извлечения без вывода сообщения
				int ResEject = EjectVolume(item->cDrive, EJECT_NO_MESSAGE);

				if (!ResEject)
				{
					// восстановим пути - это избавит нас от левых данных в панели.
					if (AMode != panel_mode::PLUGIN_PANEL)
						Owner->Parent()->GetAnotherPanel(Owner)->SetCurDir(strTmpADir, false);

					if (CMode != panel_mode::PLUGIN_PANEL)
						Owner->SetCurDir(strTmpCDir, false);

					// ... и выведем месаг о...
					SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
					const auto ErrorState = error_state::fetch();

					DoneEject = OperationFailed(ErrorState, os::fs::get_drive(item->cDrive), lng::MError, format(msg(lng::MChangeCouldNotEjectMedia), item->cDrive), false) != operation::retry;
				}
				else
					DoneEject = true;
			}
		}
		return DRIVE_DEL_NONE;
	}
	else
	{
		return ProcessDelDisk(Owner, item->cDrive, item->nDriveType);
	}
}

static void RemoveHotplugDevice(panel_ptr Owner, const PanelMenuItem *item, VMenu2 &ChDisk)
{
	int Code = RemoveHotplugDisk(item->cDrive, EJECT_NOTIFY_AFTERREMOVE);

	if (!Code)
	{
		// запоминаем состояние панелей
		const auto CMode = Owner->GetMode();
		const auto AMode = Owner->Parent()->GetAnotherPanel(Owner)->GetMode();
		string strTmpCDir(Owner->GetCurDir()), strTmpADir(Owner->Parent()->GetAnotherPanel(Owner)->GetCurDir());
		// "цикл до умопомрачения"
		int DoneEject = FALSE;

		while (!DoneEject)
		{
			// "освободим диск" - перейдем при необходимости в домашний каталог
			// TODO: А если домашний каталог - USB? ;-)
			Owner->IfGoHome(item->cDrive);
			// очередная попытка извлечения без вывода сообщения
			Code = RemoveHotplugDisk(item->cDrive, EJECT_NO_MESSAGE | EJECT_NOTIFY_AFTERREMOVE);

			if (!Code)
			{
				// восстановим пути - это избавит нас от левых данных в панели.
				if (AMode != panel_mode::PLUGIN_PANEL)
					Owner->Parent()->GetAnotherPanel(Owner)->SetCurDir(strTmpADir, false);

				if (CMode != panel_mode::PLUGIN_PANEL)
					Owner->SetCurDir(strTmpCDir, false);

				// ... и выведем месаг о...
				SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
				const auto ErrorState = error_state::fetch();

				DoneEject = Message(MSG_WARNING, ErrorState,
					msg(lng::MError),
					{
						format(msg(lng::MChangeCouldNotEjectHotPlugMedia), item->cDrive)
					},
					{ lng::MHRetry, lng::MHCancel },
					{}, &EjectHotPlugMediaErrorId) != Message::first_button;
			}
			else
				DoneEject = TRUE;
		}
	}
}

static bool GetShellName(const string& Path, string& Name)
{
	// Q: Why not SHCreateItemFromParsingName + IShellItem::GetDisplayName?
	// A: Not available in WinXP.

	// Q: Why not SHParseDisplayName + SHCreateShellItem + IShellItem::GetDisplayName then?
	// A: Not available in Win2k.

	os::com::ptr<IShellFolder> ShellFolder;
	if (FAILED(SHGetDesktopFolder(&ptr_setter(ShellFolder))))
		return false;

	os::com::memory<PIDLIST_RELATIVE> IdList;
	if (FAILED(ShellFolder->ParseDisplayName(nullptr, nullptr, UNSAFE_CSTR(Path), nullptr, &ptr_setter(IdList), nullptr)))
		return false;

	STRRET StrRet;
	if (FAILED(ShellFolder->GetDisplayNameOf(IdList.get(), SHGDN_FOREDITING, &StrRet)))
		return false;

	if (StrRet.uType != STRRET_WSTR)
		return false;

	os::com::memory<wchar_t*> Str(StrRet.pOleStr);
	Name = Str.get();
	return true;
}

static int ChangeDiskMenu(panel_ptr Owner, int Pos, bool FirstCall)
{
	int Panel_X1, Panel_X2, Panel_Y1, Panel_Y2;
	Owner->GetPosition(Panel_X1, Panel_Y1, Panel_X2, Panel_Y2);

	class Guard_Macro_DskShowPosType  //фигня какая-то
	{
	public:
		explicit Guard_Macro_DskShowPosType(panel_ptr curPanel) { Global->Macro_DskShowPosType = curPanel->Parent()->IsLeft(curPanel)? 1 : 2; }
		~Guard_Macro_DskShowPosType() { Global->Macro_DskShowPosType = 0; }
	};
	SCOPED_ACTION(Guard_Macro_DskShowPosType)(Owner);

	const auto LogicalDrives = os::fs::get_logical_drives();
	const auto SavedNetworkDrives = GetSavedNetworkDrives();
	const auto DisconnectedNetworkDrives = SavedNetworkDrives & ~LogicalDrives;
	const auto AllDrives = (LogicalDrives | DisconnectedNetworkDrives) & allowed_drives_mask();

	const auto DiskCount = AllDrives.count();

	PanelMenuItem Item, *mitem = nullptr;
	{ // эта скобка надо, см. M#605
		const auto ChDisk = VMenu2::create(msg(lng::MChangeDriveTitle), {}, ScrY - Panel_Y1 - 3);
		ChDisk->SetBottomTitle(msg(lng::MChangeDriveMenuFooter));
		ChDisk->SetHelp(L"DriveDlg"sv);
		ChDisk->SetMenuFlags(VMENU_WRAPMODE);
		ChDisk->SetId(ChangeDiskMenuId);

		struct DiskMenuItem
		{
			string Letter;
			string Type;
			string Label;
			string Fs;
			string TotalSize;
			string FreeSize;
			string Path;

			int DriveType;
		};
		std::list<DiskMenuItem> Items;

		size_t TypeWidth = 0, LabelWidth = 0, FsWidth = 0, TotalSizeWidth = 0, FreeSizeWidth = 0, PathWidth = 0;


		auto DE = std::make_unique<elevation::suppress>();

		for (const auto& i: os::fs::enum_drives(AllDrives))
		{
			const auto LocalName = os::fs::get_drive(i);
			const auto strRootDir = os::fs::get_root_directory(i);

			DiskMenuItem NewItem;
			NewItem.Letter = L'&' + LocalName;

			// We have to determine at least the basic drive type (fixed/removable/remote) regardlessly of the DRIVE_SHOW_TYPE state,
			// as it affects the visibility of the other metrics
			NewItem.DriveType = FAR_GetDriveType(strRootDir, Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if (DisconnectedNetworkDrives[os::fs::get_drive_number(i)])
			{
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;
			}

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_TYPE | DRIVE_SHOW_PATH))
			{
				// These types don't affect other checks so we can retrieve them only if needed:
				if (GetSubstName(NewItem.DriveType, LocalName, NewItem.Path))
				{
					NewItem.DriveType = DRIVE_SUBSTITUTE;
				}
				else if (DriveCanBeVirtual(NewItem.DriveType) && GetVHDInfo(LocalName, NewItem.Path))
				{
					NewItem.DriveType = DRIVE_VIRTUAL;
				}

				if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_TYPE)
				{
					static const std::pair<int, lng> DriveTypes[] =
					{
						{ DRIVE_REMOVABLE, lng::MChangeDriveRemovable },
						{ DRIVE_FIXED, lng::MChangeDriveFixed },
						{ DRIVE_REMOTE, lng::MChangeDriveNetwork },
						{ DRIVE_REMOTE_NOT_CONNECTED, lng::MChangeDriveDisconnectedNetwork },
						{ DRIVE_CDROM, lng::MChangeDriveCDROM },
						{ DRIVE_CD_RW, lng::MChangeDriveCD_RW },
						{ DRIVE_CD_RWDVD, lng::MChangeDriveCD_RWDVD },
						{ DRIVE_DVD_ROM, lng::MChangeDriveDVD_ROM },
						{ DRIVE_DVD_RW, lng::MChangeDriveDVD_RW },
						{ DRIVE_DVD_RAM, lng::MChangeDriveDVD_RAM },
						{ DRIVE_BD_ROM, lng::MChangeDriveBD_ROM },
						{ DRIVE_BD_RW, lng::MChangeDriveBD_RW },
						{ DRIVE_HDDVD_ROM, lng::MChangeDriveHDDVD_ROM },
						{ DRIVE_HDDVD_RW, lng::MChangeDriveHDDVD_RW },
						{ DRIVE_RAMDISK, lng::MChangeDriveRAM },
						{ DRIVE_SUBSTITUTE, lng::MChangeDriveSUBST },
						{ DRIVE_VIRTUAL, lng::MChangeDriveVirtual },
						{ DRIVE_USBDRIVE, lng::MChangeDriveRemovable },
					};

					const auto ItemIterator = std::find_if(CONST_RANGE(DriveTypes, DriveTypeItem) { return DriveTypeItem.first == NewItem.DriveType; });
					if (ItemIterator != std::cend(DriveTypes))
						NewItem.Type = msg(ItemIterator->second);
				}
			}

			const auto ShowDiskInfo = (NewItem.DriveType != DRIVE_REMOVABLE || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
				(!IsDriveTypeCDROM(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
				(!IsDriveTypeRemote(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_LABEL | DRIVE_SHOW_FILESYSTEM))
			{
				const auto LabelRead = Global->Opt->ChangeDriveMode & DRIVE_SHOW_LABEL? GetShellName(strRootDir, NewItem.Label) : true;

				const auto LabelPtr = LabelRead? nullptr : &NewItem.Label;
				const auto FsPtr = Global->Opt->ChangeDriveMode & DRIVE_SHOW_FILESYSTEM? &NewItem.Fs : nullptr;

				if (ShowDiskInfo && (LabelPtr || FsPtr) && !os::fs::GetVolumeInformation(strRootDir, LabelPtr, nullptr, nullptr, nullptr, FsPtr))
				{
					if (LabelPtr)
						*LabelPtr = msg(lng::MChangeDriveLabelAbsent);

					// Should we set *FsPtr to something like "Absent" too?
				}
			}

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT))
			{
				unsigned long long TotalSize = 0, UserFree = 0;

				if (ShowDiskInfo && os::fs::get_disk_size(strRootDir, &TotalSize, nullptr, &UserFree))
				{
					if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						//размер как минимум в мегабайтах
						NewItem.TotalSize = FileSizeToStr(TotalSize, 9, COLUMN_GROUPDIGITS | COLUMN_UNIT_M);
						NewItem.FreeSize = FileSizeToStr(UserFree, 9, COLUMN_GROUPDIGITS | COLUMN_UNIT_M);
					}
					else
					{
						//размер с точкой и для 0 добавляем букву размера (B)
						NewItem.TotalSize = FileSizeToStr(TotalSize, 9, COLUMN_FLOATSIZE | COLUMN_SHOWUNIT);
						NewItem.FreeSize = FileSizeToStr(UserFree, 9, COLUMN_FLOATSIZE | COLUMN_SHOWUNIT);
					}
					inplace::trim(NewItem.TotalSize);
					inplace::trim(NewItem.FreeSize);
				}
			}

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_PATH)
			{
				switch (NewItem.DriveType)
				{
				case DRIVE_REMOTE:
				case DRIVE_REMOTE_NOT_CONNECTED:
					// TODO: check result
					DriveLocalToRemoteName(NewItem.DriveType, strRootDir[0], NewItem.Path);
					break;
				}
			}

			TypeWidth = std::max(TypeWidth, NewItem.Type.size());
			LabelWidth = std::max(LabelWidth, NewItem.Label.size());
			FsWidth = std::max(FsWidth, NewItem.Fs.size());
			TotalSizeWidth = std::max(TotalSizeWidth, NewItem.TotalSize.size());
			FreeSizeWidth = std::max(FreeSizeWidth, NewItem.FreeSize.size());
			PathWidth = std::max(PathWidth, NewItem.Path.size());

			Items.emplace_back(NewItem);
		}

		int MenuLine = 0;

		bool SetSelected = false;
		std::for_each(CONST_RANGE(Items, i)
		{
			MenuItemEx ChDiskItem;
			const auto DiskNumber = os::fs::get_drive_number(i.Letter[1]);
			if (FirstCall)
			{
				ChDiskItem.SetSelect(DiskNumber == Pos);

				if (!SetSelected)
					SetSelected = (DiskNumber == Pos);
			}
			else
			{
				if (Pos < static_cast<int>(DiskCount))
				{
					ChDiskItem.SetSelect(MenuLine == Pos);

					if (!SetSelected)
						SetSelected = (MenuLine == Pos);
				}
			}

			auto ItemName = i.Letter;

			separator Separator;

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_TYPE)
			{
				append(ItemName, Separator.Get(), fit_to_left(i.Type, TypeWidth));
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_LABEL)
			{
				append(ItemName, Separator.Get(), fit_to_left(i.Label, LabelWidth));
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
			{
				append(ItemName, Separator.Get(), fit_to_left(i.Fs, FsWidth));
			}
			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT))
			{
				append(ItemName, Separator.Get(), fit_to_right(i.TotalSize, TotalSizeWidth));
				append(ItemName, Separator.Get(), fit_to_right(i.FreeSize, FreeSizeWidth));
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_PATH && PathWidth)
			{
				append(ItemName, Separator.Get(), i.Path);
			}

			PanelMenuItem item;
			item.bIsPlugin = false;
			item.cDrive = L'A' + DiskNumber;
			item.nDriveType = i.DriveType;

			ChDiskItem.Name = ItemName;
			ChDiskItem.ComplexUserData = item;
			ChDisk->AddItem(ChDiskItem);

			MenuLine++;
		});

		size_t PluginMenuItemsCount = 0;

		if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(*ChDisk, Pos, static_cast<int>(DiskCount), SetSelected);
		}

		DE.reset();

		int X = Panel_X1 + 5;

		if ((Owner == Owner->Parent()->RightPanel()) && Owner->IsFullScreen() && (Panel_X2 - Panel_X1 > 40))
			X = (Panel_X2 - Panel_X1 + 1) / 2 + 5;

		ChDisk->SetPosition(X, -1, 0, 0);

		int Y = (ScrY + 1 - static_cast<int>((DiskCount + PluginMenuItemsCount) + 5)) / 2;
		if (Y < 3)
			ChDisk->SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk->SetMacroMode(MACROAREA_DISKS);
		int RetCode = -1;

		bool NeedRefresh = false;

		// TODO: position to a new drive?
		SCOPED_ACTION(listener)(update_devices, [&NeedRefresh] { NeedRefresh = true; });

		ChDisk->Run([&](const Manager::Key& RawKey)
		{
			auto Key = RawKey();
			if (Key == KEY_NONE && NeedRefresh)
			{
				Key = KEY_CTRLR;
				NeedRefresh = false;
			}

			int SelPos = ChDisk->GetSelectPos();
			const auto item = ChDisk->GetComplexUserDataPtr<PanelMenuItem>();

			int KeyProcessed = 1;

			switch (Key)
			{
				// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
			case KEY_SHIFTNUMENTER:
			case KEY_SHIFTENTER:
			{
				if (item && !item->bIsPlugin)
				{
					OpenFolderInShell(os::fs::get_root_directory(item->cDrive));
				}
			}
			break;
			case KEY_CTRLPGUP:
			case KEY_RCTRLPGUP:
			case KEY_CTRLNUMPAD9:
			case KEY_RCTRLNUMPAD9:
			{
				if (Global->Opt->PgUpChangeDisk != 0)
					ChDisk->Close(-1);
			}
			break;
			// Т.к. нет способа получить состояние "открытости" устройства,
			// то добавим обработку Ins для CD - "закрыть диск"
			case KEY_INS:
			case KEY_NUMPAD0:
			{
				if (item && !item->bIsPlugin)
				{
					if (IsDriveTypeCDROM(item->nDriveType) /* || DriveType == DRIVE_REMOVABLE*/)
					{
						SCOPED_ACTION(SaveScreen);
						EjectVolume(item->cDrive, EJECT_LOAD_MEDIA);
						RetCode = SelPos;
					}
				}
			}
			break;
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				if (item && !item->bIsPlugin)
				{
					int Code = DisconnectDrive(Owner, item, *ChDisk);
					if (Code != DRIVE_DEL_FAIL && Code != DRIVE_DEL_NONE)
					{
						Global->ScrBuf->Lock(); // отменяем всякую прорисовку
						Global->WindowManager->ResizeAllWindows();
						Global->WindowManager->PluginCommit(); // коммитим.
						Global->ScrBuf->Unlock(); // разрешаем прорисовку
						RetCode = (((DiskCount - SelPos) == 1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos - 1:SelPos;
					}
				}
			}
			break;
			case KEY_F3:
				if (item && item->bIsPlugin)
				{
					Global->CtrlObject->Plugins->ShowPluginInfo(item->pPlugin, item->Guid);
				}
				break;
			case KEY_CTRLA:
			case KEY_RCTRLA:
			case KEY_F4:
			{
				if (item)
				{
					if (!item->bIsPlugin)
					{
						const auto RootDirectory = os::fs::get_root_directory(item->cDrive);
						ShellSetFileAttributes(nullptr, &RootDirectory);
					}
					else
					{
						if (Global->CtrlObject->Plugins->SetHotKeyDialog(item->pPlugin, item->Guid, hotkey_type::drive_menu, trim(string_view(ChDisk->at(SelPos).Name).substr(3))))
							RetCode = SelPos;
					}
				}
				break;
			}

			case KEY_APPS:
			case KEY_SHIFTAPPS:
			case KEY_MSRCLICK:
			{
				//вызовем EMenu если он есть
				if (item && !item->bIsPlugin && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
				{
					const auto RootDirectory = os::fs::get_root_directory(item->cDrive);
					struct DiskMenuParam { const wchar_t* CmdLine; BOOL Apps; } p = { RootDirectory.c_str(), Key != KEY_MSRCLICK };
					Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, Owner->Parent()->IsLeft(Owner)? OPEN_LEFTDISKMENU : OPEN_RIGHTDISKMENU, &p); // EMenu Plugin :-)
				}
				break;
			}

			case KEY_SHIFTNUMDEL:
			case KEY_SHIFTDECIMAL:
			case KEY_SHIFTDEL:
			{
				if (item && !item->bIsPlugin)
				{
					RemoveHotplugDevice(Owner, item, *ChDisk);
					RetCode = SelPos;
				}
			}
			break;
			case KEY_CTRL1:
			case KEY_RCTRL1:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_TYPE;
				RetCode = SelPos;
				break;
			case KEY_CTRL2:
			case KEY_RCTRL2:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_PATH;
				RetCode = SelPos;
				break;
			case KEY_CTRL3:
			case KEY_RCTRL3:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_LABEL;
				RetCode = SelPos;
				break;
			case KEY_CTRL4:
			case KEY_RCTRL4:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_FILESYSTEM;
				RetCode = SelPos;
				break;
			case KEY_CTRL5:
			case KEY_RCTRL5:
			{
				if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE)
				{
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_SIZE;
					Global->Opt->ChangeDriveMode |= DRIVE_SHOW_SIZE_FLOAT;
				}
				else
				{
					if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT)
						Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_SIZE_FLOAT;
					else
						Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_SIZE;
				}

				RetCode = SelPos;
				break;
			}
			case KEY_CTRL6:
			case KEY_RCTRL6:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_REMOVABLE;
				RetCode = SelPos;
				break;
			case KEY_CTRL7:
			case KEY_RCTRL7:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_PLUGINS;
				RetCode = SelPos;
				break;
			case KEY_CTRL8:
			case KEY_RCTRL8:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_CDROM;
				RetCode = SelPos;
				break;
			case KEY_CTRL9:
			case KEY_RCTRL9:
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_REMOTE;
				RetCode = SelPos;
				break;
			case KEY_F9:
				ConfigureChangeDriveMode();
				RetCode = SelPos;
				break;
			case KEY_SHIFTF1:
			{
				if (item && item->bIsPlugin)
				{
					// Вызываем нужный топик, который передали в CommandsMenu()
					pluginapi::apiShowHelp(
						item->pPlugin->ModuleName().c_str(),
						nullptr,
						FHELP_SELFHELP | FHELP_NOSHOWERROR | FHELP_USECONTENTS
						);
				}

				break;
			}
			case KEY_ALTSHIFTF9:
			case KEY_RALTSHIFTF9:

				if (Global->Opt->ChangeDriveMode&DRIVE_SHOW_PLUGINS)
					Global->CtrlObject->Plugins->Configure();

				RetCode = SelPos;
				break;
			case KEY_SHIFTF9:

				if (item && item->bIsPlugin && item->pPlugin->has(iConfigure))
					Global->CtrlObject->Plugins->ConfigureCurrent(item->pPlugin, item->Guid);

				RetCode = SelPos;
				break;
			case KEY_CTRLR:
			case KEY_RCTRLR:
				RetCode = SelPos;
				break;

			default:
				KeyProcessed = 0;
			}

			if (RetCode >= 0)
				ChDisk->Close(-1);

			return KeyProcessed;
		});

		if (RetCode >= 0)
			return RetCode;

		const auto& CurDir = Owner->GetCurDir();

		if (ChDisk->GetExitCode() < 0 && CurDir.size() > 2 && !(IsSlash(CurDir[0]) && IsSlash(CurDir[1])))
		{
			if (FAR_GetDriveType(os::fs::get_root_directory(CurDir[0])) == DRIVE_NO_ROOT_DIR)
				return ChDisk->GetSelectPos();
		}

		if (ChDisk->GetExitCode()<0)
			return -1;

		mitem = ChDisk->GetComplexUserDataPtr<PanelMenuItem>();

		if (mitem)
		{
			Item = *mitem;
			mitem = &Item;
		}
	} // эта скобка надо, см. M#605

	if (mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		if (!os::fs::IsDiskInDrive(os::fs::get_drive(mitem->cDrive)))
		{
			if (!EjectVolume(mitem->cDrive, EJECT_READY | EJECT_NO_MESSAGE))
			{
				SCOPED_ACTION(SaveScreen);
				Message(0,
					{},
					{
						msg(lng::MChangeWaitingLoadDisk)
					},
					{});
				EjectVolume(mitem->cDrive, EJECT_LOAD_MEDIA | EJECT_NO_MESSAGE);
			}
		}
	}

	if (Owner->ProcessPluginEvent(FE_CLOSE, nullptr))
		return -1;

	Global->ScrBuf->Flush();
	INPUT_RECORD rec;
	PeekInputRecord(&rec);

	if (!mitem)
		return -1; //???

	if (!mitem->bIsPlugin)
	{
		for (;;)
		{
			if (FarChDir(os::fs::get_drive(mitem->cDrive)) || FarChDir(os::fs::get_root_directory(mitem->cDrive)))
			{
				break;
			}

			const auto ErrorState = error_state::fetch();

			DialogBuilder Builder(lng::MError);

			Builder.AddTextWrap(GetErrorString(ErrorState).c_str(), true);
			Builder.AddText(L"");

			string DriveLetter(1, mitem->cDrive);
			DialogItemEx *DriveLetterEdit = Builder.AddFixEditField(DriveLetter, 1);
			Builder.AddTextBefore(DriveLetterEdit, lng::MChangeDriveCannotReadDisk);
			Builder.AddTextAfter(DriveLetterEdit, L":", 0);

			Builder.AddOKCancel(lng::MRetry, lng::MCancel);
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);
			Builder.SetId(ChangeDriveCannotReadDiskErrorId);

			if (Builder.ShowDialog())
			{
				mitem->cDrive = upper(DriveLetter[0]);
			}
			else
			{
				return -1;
			}
		}

		const auto strNewCurDir = os::fs::GetCurrentDirectory();

		if ((Owner->GetMode() == panel_mode::NORMAL_PANEL) &&
			(Owner->GetType() == panel_type::FILE_PANEL) &&
			equal_icase(Owner->GetCurDir(), strNewCurDir) &&
			Owner->IsVisible())
		{
			// А нужно ли делать здесь Update????
			Owner->Update(UPDATE_KEEP_SELECTION);
		}
		else
		{
			const auto IsActive = Owner->IsFocused();
			const auto NewPanel = Owner->Parent()->ChangePanel(Owner, panel_type::FILE_PANEL, TRUE, FALSE);
			NewPanel->SetCurDir(strNewCurDir, true);
			NewPanel->Show();

			if (IsActive || !NewPanel->Parent()->GetAnotherPanel(NewPanel)->IsVisible())
				NewPanel->Parent()->SetActivePanel(NewPanel);

			if (!IsActive && NewPanel->Parent()->GetAnotherPanel(NewPanel)->GetType() == panel_type::INFO_PANEL)
				NewPanel->Parent()->GetAnotherPanel(NewPanel)->UpdateKeyBar();
		}
	}
	else //эта плагин, да
	{
		auto hPlugin = Global->CtrlObject->Plugins->Open(
			mitem->pPlugin,
			Owner->Parent()->IsLeft(Owner)? OPEN_LEFTDISKMENU : OPEN_RIGHTDISKMENU,
			mitem->Guid,
			0);

		if (hPlugin)
		{
			const auto IsActive = Owner->IsFocused();
			const auto NewPanel = Owner->Parent()->ChangePanel(Owner, panel_type::FILE_PANEL, TRUE, TRUE);
			NewPanel->SetPluginMode(std::move(hPlugin), {}, IsActive || !NewPanel->Parent()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!IsActive && NewPanel->Parent()->GetAnotherPanel(NewPanel)->GetType() == panel_type::INFO_PANEL)
				NewPanel->Parent()->GetAnotherPanel(NewPanel)->UpdateKeyBar();
		}
	}

	return -1;
}

void ChangeDisk(panel_ptr Owner)
{
	int Pos = 0;
	bool FirstCall = true;

	const auto& CurDir = Owner->GetCurDir();
	if (!CurDir.empty() && CurDir[1] == L':' && os::fs::is_standard_drive_letter(CurDir[0]))
	{
		Pos = os::fs::get_drive_number(CurDir[0]);
	}

	while (Pos != -1)
	{
		Pos = ChangeDiskMenu(Owner, Pos, FirstCall);
		FirstCall = false;
	}
}

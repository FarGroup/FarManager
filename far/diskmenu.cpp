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

// Self:
#include "diskmenu.hpp"

// Internal:
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
#include "scrbuf.hpp"
#include "plugapi.hpp"
#include "message.hpp"
#include "keyboard.hpp"
#include "dirmix.hpp"
#include "lockscrn.hpp"
#include "string_sort.hpp"
#include "pathmix.hpp"

// Platform:
#include "platform.reg.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

enum
{
	// DRIVE_UNKNOWN            = 0,
	// DRIVE_NO_ROOT_DIR        = 1,
	// DRIVE_REMOVABLE          = 2,
	// DRIVE_FIXED              = 3,
	// DRIVE_REMOTE             = 4,
	// DRIVE_CDROM              = 5,
	// DRIVE_RAMDISK            = 6,

	// BUGBUG ELIMINATE
	DRIVE_SUBSTITUTE            = 100,
	DRIVE_REMOTE_NOT_CONNECTED  = 101,
	DRIVE_VIRTUAL               = 102,
};

class ChDiskPluginItem
{
public:
	NONCOPYABLE(ChDiskPluginItem);
	MOVE_CONSTRUCTIBLE(ChDiskPluginItem);

	ChDiskPluginItem() = default;

	bool operator <(const ChDiskPluginItem& rhs) const
	{
		return (Global->Opt->ChangeDriveMode & DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey != rhs.HotKey)?
			HotKey < rhs.HotKey :
			string_sort::less(Item.Name, rhs.Item.Name);
	}

	auto& getItem() { return Item; }
	auto& getHotKey() { return HotKey; }

private:
	MenuItemEx Item;
	wchar_t HotKey{};
};

struct disk_item
{
	wchar_t cDrive;
	int nDriveType;
};

struct plugin_item
{
	Plugin* pPlugin;
	GUID Guid;
};

using disk_menu_item = std::variant<disk_item, plugin_item>;

[[nodiscard]]
static auto EjectFailed(error_state_ex const& ErrorState, wchar_t const Letter)
{
	return OperationFailed(ErrorState, os::fs::get_unc_drive(Letter), lng::MError, format(msg(lng::MChangeCouldNotEjectMedia), Letter), false);
}

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

				disk_menu_item item{ plugin_item{ pPlugin, guid } };
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

	for (const auto& [i, index]: enumerate(MPItems))
	{
		if (Pos > DiskCount && !SetSelected)
		{
			SetSelected = DiskCount + static_cast<int>(index) + 1 == Pos;
			i.getItem().SetSelect(SetSelected);
		}

		const auto HotKey = i.getHotKey();
		i.getItem().Name = concat(HotKey? concat(L'&', HotKey, L"  "sv) : L"   "sv, i.getItem().Name);
		ChDisk.AddItem(i.getItem());
	}

	return MPItems.size();
}

static void ConfigureChangeDriveMode()
{
	auto& DriveMode = Global->Opt->ChangeDriveMode;

	DialogBuilder Builder(lng::MChangeDriveConfigure, L"ChangeDriveMode"sv);
	Builder.SetId(ChangeDriveModeId);
	Builder.AddCheckbox(lng::MChangeDriveShowDiskType, DriveMode, DRIVE_SHOW_TYPE);
	const auto ShowLabel = Builder.AddCheckbox(lng::MChangeDriveShowLabel, DriveMode, DRIVE_SHOW_LABEL);
	const auto ShowLabelUseShell = Builder.AddCheckbox(lng::MChangeDriveShowLabelUseShell, DriveMode, DRIVE_SHOW_LABEL_USE_SHELL);
	ShowLabelUseShell->Indent(4);
	Builder.LinkFlags(ShowLabel, ShowLabelUseShell, DIF_DISABLE);
	Builder.AddCheckbox(lng::MChangeDriveShowFileSystem, DriveMode, DRIVE_SHOW_FILESYSTEM);

	int ShowSizeAny = DriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);

	const auto ShowSize = Builder.AddCheckbox(lng::MChangeDriveShowSize, ShowSizeAny);
	const auto ShowSizeFloat = Builder.AddCheckbox(lng::MChangeDriveShowSizeFloat, DriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat->Indent(4);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(lng::MChangeDriveShowPath, DriveMode, DRIVE_SHOW_PATH);
	Builder.AddCheckbox(lng::MChangeDriveShowPlugins, DriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(lng::MChangeDriveSortPluginsByHotkey, DriveMode, DRIVE_SORT_PLUGINS_BY_HOTKEY)->Indent(4);
	Builder.AddCheckbox(lng::MChangeDriveShowRemovableDrive, DriveMode, DRIVE_SHOW_REMOVABLE);
	Builder.AddCheckbox(lng::MChangeDriveShowCD, DriveMode, DRIVE_SHOW_CDROM);
	Builder.AddCheckbox(lng::MChangeDriveShowNetworkDrive, DriveMode, DRIVE_SHOW_REMOTE);
	Builder.AddCheckbox(lng::MChangeDriveShowVirtualDisk, DriveMode, DRIVE_SHOW_VIRTUAL);

	Builder.AddOKCancel();
	if (!Builder.ShowDialog())
		return;

	if (ShowSizeAny)
	{
		if (DriveMode & DRIVE_SHOW_SIZE_FLOAT)
			DriveMode &= ~DRIVE_SHOW_SIZE;
		else
			DriveMode |= DRIVE_SHOW_SIZE;
	}
	else
		DriveMode &= ~(DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);
}

class separator
{
public:
	string operator()()
	{
		if (m_value == L' ')
		{
			m_value = BoxSymbols[BS_V1];
			return { L' ' };
		}
		return { L' ', m_value, L' '};
	}

private:
	wchar_t m_value{L' '};
};

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile)
{
	/*
	          1         2         3         4         5
	   345678901234567890123456789012345678901234567890
	 1 ╔══════════ Disconnect network drive ══════════╗
	 2 ║ Do you want to disconnect from the drive Z:? ║
	 3 ║ The drive Z: is mapped to:                   ║
	 4 ║ \\host\share                                 ║
	 6 ╟──────────────────────────────────────────────╢
	 7 ║ [ ] Reconnect at logon                       ║
	 8 ╟──────────────────────────────────────────────╢
	 9 ║              { Yes } [ Cancel ]              ║
	10 ╚══════════════════════════════════════════════╝
	*/

	enum
	{
		rc_doublebox,
		rc_text_1,
		rc_text_2,
		rc_text_3,
		rc_separator_1,
		rc_checkbox,
		rc_separator_2,
		rc_button_yes,
		rc_button_cancel,

		rc_count
	};

	auto DCDlg = MakeDialogItems<rc_count>(
	{
		{ DI_DOUBLEBOX, {{3,  1}, {72, 9}}, DIF_NONE, msg(lng::MChangeDriveDisconnectTitle), },
		{ DI_TEXT,      {{5,  2}, {0,  2}}, DIF_SHOWAMPERSAND, },
		{ DI_TEXT,      {{5,  3}, {0,  3}}, DIF_SHOWAMPERSAND, },
		{ DI_TEXT,      {{5,  4}, {0,  4}}, DIF_SHOWAMPERSAND, },
		{ DI_TEXT,      {{-1, 5}, {0,  5}}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,  6}, {70, 6}}, DIF_FOCUS, msg(lng::MChangeDriveDisconnectReconnect), },
		{ DI_TEXT,      {{-1, 7}, {0,  7}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  8}, {0,  8}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MYes), },
		{ DI_BUTTON,    {{0,  8}, {0,  8}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	DCDlg[rc_text_1].strData = format(msg(lng::MChangeDriveDisconnectQuestion), Letter);
	DCDlg[rc_text_2].strData = format(msg(lng::MChangeDriveDisconnectMapped), Letter);

	const auto Len = std::max({ DCDlg[rc_doublebox].strData.size(), DCDlg[rc_text_1].strData.size(), DCDlg[rc_text_1].strData.size(), DCDlg[rc_checkbox].strData.size() });

	{
		string strMsgText;
		// TODO: check result
		DriveLocalToRemoteName(DRIVE_REMOTE, Letter, strMsgText);
		DCDlg[rc_text_3].strData = truncate_path(std::move(strMsgText), Len);
	}

	// проверяем - это было постоянное соединение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.

	bool IsPersistent = true;
	if (os::reg::key::open(os::reg::key::current_user, concat(L"Network\\"sv, Letter), KEY_QUERY_VALUE))
	{
		DCDlg[rc_checkbox].Selected = Global->Opt->ChangeDriveDisconnectMode;
	}
	else
	{
		DCDlg[rc_checkbox].Flags |= DIF_DISABLE;
		DCDlg[rc_checkbox].Selected = 0;
		IsPersistent = false;
	}

	// скорректируем размеры диалога - для дизайнУ
	DCDlg[rc_doublebox].X2 = DCDlg[rc_doublebox].X1 + Len + 3;
	int ExitCode = rc_button_yes;

	if (Global->Opt->Confirm.RemoveConnection)
	{
		const auto Dlg = Dialog::create(DCDlg);
		Dlg->SetPosition({ -1, -1, static_cast<int>(DCDlg[rc_doublebox].X2 + 4), 11 });
		Dlg->SetHelp(L"DisconnectDrive"sv);
		Dlg->SetId(DisconnectDriveId);
		Dlg->SetDialogMode(DMODE_WARNINGSTYLE);
		Dlg->Process();
		ExitCode = Dlg->GetExitCode();
	}

	UpdateProfile = DCDlg[rc_checkbox].Selected? 0 : CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Global->Opt->ChangeDriveDisconnectMode = DCDlg[rc_checkbox].Selected == BSTATE_CHECKED;

	return ExitCode == rc_button_yes;
}

static bool ProcessDelDisk(panel_ptr Owner, wchar_t Drive, int DriveType)
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
					return false;
				}
			}

			if (DelSubstDrive(DiskLetter))
			{
				return true;
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
						return true;
					}
				}
				else
				{
					return false;
				}
			}
			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					strMsgText
				},
				{ lng::MOk },
				{}, &SUBSTDisconnectDriveError2Id);
			return false;
		}

	case DRIVE_REMOTE:
	case DRIVE_REMOTE_NOT_CONNECTED:
		{
			int UpdateProfile = CONNECT_UPDATE_PROFILE;
			if (!MessageRemoveConnection(Drive, UpdateProfile))
				return false;

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
				return true;

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
						return true;
					}
				}
				else
				{
					return false;
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
			return false;
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
					return false;
				}
			}

			string strVhdPath;
			VIRTUAL_STORAGE_TYPE VirtualStorageType;
			error_state ErrorState;

			if (GetVHDInfo(DiskLetter, strVhdPath, &VirtualStorageType) && !strVhdPath.empty())
			{
				if (os::fs::detach_virtual_disk(strVhdPath, VirtualStorageType))
				{
					return true;
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
			return false;
		}

	default:
		return false;
	}
}

static bool DisconnectDrive(panel_ptr Owner, const disk_item& item, VMenu2 &ChDisk, bool& Cancelled)
{
	if (item.nDriveType != DRIVE_REMOVABLE && item.nDriveType != DRIVE_CDROM)
		return ProcessDelDisk(Owner, item.cDrive, item.nDriveType);

	if (item.nDriveType == DRIVE_REMOVABLE && !IsEjectableMedia(item.cDrive))
	{
		Cancelled = true;
		return false;
	}

	// первая попытка извлечь диск
	try
	{
		EjectVolume(item.cDrive);
		return true;
	}
	catch (const far_exception&)
	{
		// запоминаем состояние панелей
		const auto CMode = Owner->GetMode();
		const auto AMode = Owner->Parent()->GetAnotherPanel(Owner)->GetMode();
		const auto TmpCDir = Owner->GetCurDir();
		const auto TmpADir = Owner->Parent()->GetAnotherPanel(Owner)->GetCurDir();

		// "цикл до умопомрачения"
		for (;;)
		{
			// "освободим диск" - перейдем при необходимости в домашний каталог
			// TODO: А если домашний каталог - CD? ;-)
			Owner->IfGoHome(item.cDrive);
			// очередная попытка извлечения без вывода сообщения
			try
			{
				EjectVolume(item.cDrive);
				return true;
			}
			catch (const far_exception& e)
			{
				// восстановим пути - это избавит нас от левых данных в панели.
				if (AMode != panel_mode::PLUGIN_PANEL)
					Owner->Parent()->GetAnotherPanel(Owner)->SetCurDir(TmpADir, false);

				if (CMode != panel_mode::PLUGIN_PANEL)
					Owner->SetCurDir(TmpCDir, false);

				if (EjectFailed(e, item.cDrive) != operation::retry)
					return false;
			}
		}
	}
}

static void RemoveHotplugDevice(panel_ptr Owner, const disk_item& item, VMenu2 &ChDisk)
{
	bool Cancelled = false;
	if (RemoveHotplugDisk(item.cDrive, Global->Opt->Confirm.RemoveHotPlug, Cancelled) || Cancelled)
		return;


	// запоминаем состояние панелей
	const auto CMode = Owner->GetMode();
	const auto AMode = Owner->Parent()->GetAnotherPanel(Owner)->GetMode();
	const auto TmpCDir = Owner->GetCurDir();
	const auto TmpADir = Owner->Parent()->GetAnotherPanel(Owner)->GetCurDir();

	// "цикл до умопомрачения"
	for (;;)
	{
		// "освободим диск" - перейдем при необходимости в домашний каталог
		// TODO: А если домашний каталог - USB? ;-)
		Owner->IfGoHome(item.cDrive);
		// очередная попытка извлечения без вывода сообщения
		if (RemoveHotplugDisk(item.cDrive, false, Cancelled) || Cancelled)
			return;

		const auto ErrorState = error_state::fetch();

		// восстановим пути - это избавит нас от левых данных в панели.
		if (AMode != panel_mode::PLUGIN_PANEL)
			Owner->Parent()->GetAnotherPanel(Owner)->SetCurDir(TmpADir, false);

		if (CMode != panel_mode::PLUGIN_PANEL)
			Owner->SetCurDir(TmpCDir, false);

		if (Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			{
				format(msg(lng::MChangeCouldNotEjectHotPlugMedia), item.cDrive)
			},
			{ lng::MHRetry, lng::MHCancel },
			{}, &EjectHotPlugMediaErrorId) != Message::first_button)
			return;
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

	const os::com::memory<wchar_t*> Str(StrRet.pOleStr);
	Name = Str.get();
	return true;
}

static int ChangeDiskMenu(panel_ptr Owner, int Pos, bool FirstCall)
{
	const auto PanelRect = Owner->GetPosition();

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

	if (!FirstCall && Pos == static_cast<int>(DiskCount))
	{
		// Pos points to the separator - this is probably a redraw after a removal of the last disk.
		--Pos;
	}

	disk_menu_item Item, *mitem = nullptr;
	{ // эта скобка надо, см. M#605
		const auto ChDisk = VMenu2::create(msg(lng::MChangeDriveTitle), {}, ScrY - PanelRect.top - 3);
		ChDisk->SetBottomTitle(KeysToLocalizedText(KEY_DEL, KEY_SHIFTDEL, KEY_F3, KEY_F4, KEY_F9));
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


		std::optional<elevation::suppress> DE(std::in_place);
		auto& DriveMode = Global->Opt->ChangeDriveMode;

		for (const auto& i: os::fs::enum_drives(AllDrives))
		{
			const auto LocalName = os::fs::get_drive(i);
			const auto strRootDir = os::fs::get_root_directory(i);

			DiskMenuItem NewItem;
			NewItem.Letter = LocalName;

			// We have to determine at least the basic drive type (fixed/removable/remote) regardlessly of the DRIVE_SHOW_TYPE state,
			// as it affects the visibility of the other metrics
			NewItem.DriveType = FAR_GetDriveType(strRootDir);

			if (DisconnectedNetworkDrives[os::fs::get_drive_number(i)])
			{
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;
			}

			if (DriveMode & (DRIVE_SHOW_TYPE | DRIVE_SHOW_PATH))
			{
				// These types don't affect other checks so we can retrieve them only if needed:
				if (GetSubstName(NewItem.DriveType, LocalName, NewItem.Path))
				{
					NewItem.DriveType = DRIVE_SUBSTITUTE;
				}
				else if ((DriveMode & DRIVE_SHOW_VIRTUAL) && DriveCanBeVirtual(NewItem.DriveType) && GetVHDInfo(os::fs::get_unc_drive(i), NewItem.Path))
				{
					NewItem.DriveType = DRIVE_VIRTUAL;
				}

				if (DriveMode & DRIVE_SHOW_TYPE)
				{
					if (NewItem.DriveType == DRIVE_CDROM)
					{
						static_assert(as_underlying_type(lng::MChangeDriveHDDVDRAM) - as_underlying_type(lng::MChangeDriveCDROM) == as_underlying_type(cd_type::hddvdram) - as_underlying_type(cd_type::cdrom));

						NewItem.Type = msg(lng::MChangeDriveCDROM + ((DriveMode & DRIVE_SHOW_CDROM)? as_underlying_type(get_cdrom_type(strRootDir)) - as_underlying_type(cd_type::cdrom) : 0));
					}
					else
					{
						static const std::pair<int, lng> DriveTypes[]
						{
							{ DRIVE_REMOVABLE,            lng::MChangeDriveRemovable },
							{ DRIVE_FIXED,                lng::MChangeDriveFixed },
							{ DRIVE_REMOTE,               lng::MChangeDriveNetwork },
							{ DRIVE_REMOTE_NOT_CONNECTED, lng::MChangeDriveDisconnectedNetwork },
							{ DRIVE_RAMDISK,              lng::MChangeDriveRAM },
							{ DRIVE_SUBSTITUTE,           lng::MChangeDriveSUBST },
							{ DRIVE_VIRTUAL,              lng::MChangeDriveVirtual },
						};

						const auto ItemIterator = std::find_if(CONST_RANGE(DriveTypes, DriveTypeItem) { return DriveTypeItem.first == NewItem.DriveType; });
						if (ItemIterator != std::cend(DriveTypes))
							NewItem.Type = msg(ItemIterator->second);
					}
				}
			}

			const auto ShowDiskInfo =
				((DriveMode & DRIVE_SHOW_REMOVABLE) || NewItem.DriveType != DRIVE_REMOVABLE) &&
				((DriveMode & DRIVE_SHOW_CDROM) || NewItem.DriveType != DRIVE_CDROM) &&
				((DriveMode & DRIVE_SHOW_REMOTE) || !(NewItem.DriveType == DRIVE_REMOTE || NewItem.DriveType == DRIVE_REMOTE_NOT_CONNECTED));

			if (ShowDiskInfo)
			{
				if (DriveMode & (DRIVE_SHOW_LABEL | DRIVE_SHOW_FILESYSTEM))
				{
					bool TryReadLabel = (DriveMode & DRIVE_SHOW_LABEL) != 0;

					if (TryReadLabel && DriveMode & DRIVE_SHOW_LABEL_USE_SHELL)
					{
						TryReadLabel = !GetShellName(strRootDir, NewItem.Label);
					}

					const auto LabelPtr = TryReadLabel? &NewItem.Label : nullptr;
					const auto FsPtr = DriveMode & DRIVE_SHOW_FILESYSTEM? &NewItem.Fs : nullptr;

					if ((LabelPtr || FsPtr) && !os::fs::GetVolumeInformation(strRootDir, LabelPtr, nullptr, nullptr, nullptr, FsPtr))
					{
						if (LabelPtr)
							*LabelPtr = msg(lng::MChangeDriveLabelAbsent);

						// Should we set *FsPtr to something like "Absent" too?
					}
				}

				if (DriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT))
				{
					unsigned long long TotalSize = 0, UserFree = 0;
					if (os::fs::get_disk_size(strRootDir, &TotalSize, nullptr, &UserFree))
					{
						const auto SizeFlags = DriveMode & DRIVE_SHOW_SIZE?
							//размер как минимум в мегабайтах
							COLFLAGS_GROUPDIGITS | COLFLAGS_MULTIPLIER_M :
							//размер с точкой и для 0 добавляем букву размера (B)
							COLFLAGS_FLOATSIZE | COLFLAGS_SHOW_MULTIPLIER;

						const auto FormatSize = [SizeFlags](unsigned long long const Size)
						{
							return trim(FileSizeToStr(Size, 9, SizeFlags));
						};

						NewItem.TotalSize = FormatSize(TotalSize);
						NewItem.FreeSize = FormatSize(UserFree);
					}
				}
			}

			if (DriveMode & DRIVE_SHOW_PATH)
			{
				switch (NewItem.DriveType)
				{
				case DRIVE_REMOTE:
				case DRIVE_REMOTE_NOT_CONNECTED:
					// TODO: check result
					DriveLocalToRemoteName(DRIVE_REMOTE, strRootDir[0], NewItem.Path);
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

		for (const auto& i: Items)
		{
			MenuItemEx ChDiskItem;
			const auto DiskNumber = os::fs::get_drive_number(i.Letter.front());
			if (FirstCall)
			{
				ChDiskItem.SetSelect(static_cast<int>(DiskNumber) == Pos);

				if (!SetSelected)
					SetSelected = (static_cast<int>(DiskNumber) == Pos);
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

			if (DriveMode & DRIVE_SHOW_TYPE)
			{
				append(ItemName, Separator(), fit_to_left(i.Type, TypeWidth));
			}
			if (DriveMode & DRIVE_SHOW_LABEL)
			{
				append(ItemName, Separator(), fit_to_left(i.Label, LabelWidth));
			}
			if (DriveMode & DRIVE_SHOW_FILESYSTEM)
			{
				append(ItemName, Separator(), fit_to_left(i.Fs, FsWidth));
			}
			if (DriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT))
			{
				append(ItemName, Separator(), fit_to_right(i.TotalSize, TotalSizeWidth));
				append(ItemName, Separator(), fit_to_right(i.FreeSize, FreeSizeWidth));
			}
			if (DriveMode & DRIVE_SHOW_PATH && PathWidth)
			{
				append(ItemName, Separator(), i.Path);
			}

			disk_menu_item item{ disk_item{os::fs::get_drive_letter(DiskNumber), i.DriveType} };

			inplace::escape_ampersands(ItemName);
			ItemName.insert(0, 1, L'&');

			ChDiskItem.Name = std::move(ItemName);
			ChDiskItem.ComplexUserData = item;
			ChDisk->AddItem(ChDiskItem);

			MenuLine++;
		}

		size_t PluginMenuItemsCount = 0;

		if (DriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(*ChDisk, Pos, static_cast<int>(DiskCount), SetSelected);
		}

		DE.reset();

		int X = PanelRect.left + 5;

		if ((Owner == Owner->Parent()->RightPanel()) && Owner->IsFullScreen() && (PanelRect.width() > 40))
			X = PanelRect.width() / 2 + 5;

		ChDisk->SetPosition({ X, -1, 0, 0 });

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

			const auto SelPos = ChDisk->GetSelectPos();
			const auto MenuItem = ChDisk->GetComplexUserDataPtr<disk_menu_item>();

			int KeyProcessed = 1;

			switch (Key)
			{
			// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
			case KEY_SHIFTNUMENTER:
			case KEY_SHIFTENTER:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)
				{
					OpenFolderInShell(os::fs::get_root_directory(item.cDrive));
				},
				[](plugin_item const&){}}, *MenuItem);
				break;

			case KEY_CTRLPGUP:
			case KEY_RCTRLPGUP:
			case KEY_CTRLNUMPAD9:
			case KEY_RCTRLNUMPAD9:
				if (Global->Opt->PgUpChangeDisk != 0)
					ChDisk->Close(-1);
				break;

			// Т.к. нет способа получить состояние "открытости" устройства,
			// то добавим обработку Ins для CD - "закрыть диск"
			case KEY_INS:
			case KEY_NUMPAD0:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)
				{
					if (item.nDriveType != DRIVE_REMOVABLE && item.nDriveType != DRIVE_CDROM)
						return;

					for (;;)
					{
						Message(0,
							{},
							{
								msg(lng::MChangeWaitingLoadDisk)
							},
							{});

						try
						{
							LoadVolume(item.cDrive);
							break;
						}
						catch (far_exception const& e)
						{
							if (EjectFailed(e, item.cDrive) != operation::retry)
								break;
						}
					}

					RetCode = SelPos;
				},
				[](plugin_item const&){}}, *MenuItem);
				break;

			case KEY_NUMDEL:
			case KEY_DEL:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)

				{
					bool Cancelled = false;
					if (DisconnectDrive(Owner, item, *ChDisk, Cancelled))
					{
						RetCode = SelPos;
					}
				},
				[](plugin_item const&){}}, *MenuItem);
				break;

			case KEY_F3:
				if (!MenuItem)
					break;

				std::visit(overload{[&](plugin_item const& item)
				{
					Global->CtrlObject->Plugins->ShowPluginInfo(item.pPlugin, item.Guid);
				},
				[](disk_item const&){}}, *MenuItem);
				break;

			case KEY_CTRLA:
			case KEY_RCTRLA:
			case KEY_F4:
				if (!MenuItem)
					break;

				std::visit(overload
				{
					[&](plugin_item const& item)
					{
						if (Global->CtrlObject->Plugins->SetHotKeyDialog(item.pPlugin, item.Guid, hotkey_type::drive_menu, trim(string_view(ChDisk->at(SelPos).Name).substr(3))))
							RetCode = SelPos;
					},
					[](disk_item const& item)
					{
						const auto RootDirectory = os::fs::get_root_directory(item.cDrive);
						ShellSetFileAttributes(nullptr, &RootDirectory);
					}
				},
				*MenuItem);
				break;

			case KEY_APPS:
			case KEY_SHIFTAPPS:
			case KEY_MSRCLICK:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)
				{
					if (!Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
						return;

					//вызовем EMenu если он есть
					const auto RootDirectory = os::fs::get_root_directory(item.cDrive);
					struct DiskMenuParam { const wchar_t* CmdLine; BOOL Apps; } p = { RootDirectory.c_str(), Key != KEY_MSRCLICK };
					Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, Owner->Parent()->IsLeft(Owner)? OPEN_LEFTDISKMENU : OPEN_RIGHTDISKMENU, &p); // EMenu Plugin :-)
				},
				[](plugin_item const&){}}, *MenuItem);
				break;

			case KEY_SHIFTNUMDEL:
			case KEY_SHIFTDECIMAL:
			case KEY_SHIFTDEL:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)
				{
					RemoveHotplugDevice(Owner, item, *ChDisk);
					RetCode = SelPos;
				},
				[](plugin_item const&){}}, *MenuItem);
				break;

			case KEY_CTRL1:
			case KEY_RCTRL1:
				DriveMode ^= DRIVE_SHOW_TYPE;
				RetCode = SelPos;
				break;

			case KEY_CTRL2:
			case KEY_RCTRL2:
				DriveMode ^= DRIVE_SHOW_PATH;
				RetCode = SelPos;
				break;

			case KEY_CTRL3:
			case KEY_RCTRL3:
				if (DriveMode & DRIVE_SHOW_LABEL)
				{
					if (DriveMode & DRIVE_SHOW_LABEL_USE_SHELL)
						DriveMode &= ~(DRIVE_SHOW_LABEL | DRIVE_SHOW_LABEL_USE_SHELL);
					else
						DriveMode |= DRIVE_SHOW_LABEL_USE_SHELL;
				}
				else
				{
					DriveMode |= DRIVE_SHOW_LABEL;
				}
				RetCode = SelPos;
				break;

			case KEY_CTRL4:
			case KEY_RCTRL4:
				DriveMode ^= DRIVE_SHOW_FILESYSTEM;
				RetCode = SelPos;
				break;

			case KEY_CTRL5:
			case KEY_RCTRL5:
				if (DriveMode & DRIVE_SHOW_SIZE)
				{
					DriveMode ^= DRIVE_SHOW_SIZE;
					DriveMode |= DRIVE_SHOW_SIZE_FLOAT;
				}
				else
				{
					if (DriveMode & DRIVE_SHOW_SIZE_FLOAT)
						DriveMode ^= DRIVE_SHOW_SIZE_FLOAT;
					else
						DriveMode ^= DRIVE_SHOW_SIZE;
				}

				RetCode = SelPos;
				break;

			case KEY_CTRL6:
			case KEY_RCTRL6:
				DriveMode ^= DRIVE_SHOW_REMOVABLE;
				RetCode = SelPos;
				break;

			case KEY_CTRL7:
			case KEY_RCTRL7:
				DriveMode ^= DRIVE_SHOW_PLUGINS;
				RetCode = SelPos;
				break;

			case KEY_CTRL8:
			case KEY_RCTRL8:
				DriveMode ^= DRIVE_SHOW_CDROM;
				RetCode = SelPos;
				break;

			case KEY_CTRL9:
			case KEY_RCTRL9:
				DriveMode ^= DRIVE_SHOW_REMOTE;
				RetCode = SelPos;
				break;

			case KEY_F9:
				ConfigureChangeDriveMode();
				RetCode = SelPos;
				break;

			case KEY_SHIFTF1:
				if (!MenuItem)
					break;

				std::visit(overload{[&](plugin_item const& item)
				{
					// Вызываем нужный топик, который передали в CommandsMenu()
					pluginapi::apiShowHelp(
						item.pPlugin->ModuleName().c_str(),
						nullptr,
						FHELP_SELFHELP | FHELP_NOSHOWERROR | FHELP_USECONTENTS
					);
				},
				[](disk_item const&){}}, *MenuItem);
				break;

			case KEY_ALTSHIFTF9:
			case KEY_RALTSHIFTF9:
				Global->CtrlObject->Plugins->Configure();
				RetCode = SelPos;
				break;

			case KEY_SHIFTF9:
				if (!MenuItem)
					break;

				std::visit(overload{[&](plugin_item const& item)
				{
					if (item.pPlugin->has(iConfigure))
						Global->CtrlObject->Plugins->ConfigureCurrent(item.pPlugin, item.Guid);
				},
				[](disk_item const&){}}, *MenuItem);
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

		mitem = ChDisk->GetComplexUserDataPtr<disk_menu_item>();

		if (mitem)
		{
			Item = *mitem;
			mitem = &Item;
		}
	} // эта скобка надо, см. M#605

	if (mitem)
	{
		std::visit(overload{[&](disk_item const& item)
		{
			if (item.nDriveType != DRIVE_REMOVABLE && item.nDriveType != DRIVE_CDROM)
				return;

			if (os::fs::IsDiskInDrive(os::fs::get_drive(item.cDrive)))
				return;

			Message(0,
				{},
				{
					msg(lng::MChangeWaitingLoadDisk)
				},
				{});

			try
			{
				LoadVolume(item.cDrive);
			}
			catch (far_exception const&)
			{
				// TODO: Message & retry? It conflicts with the CD retry dialog.
			}
		},
		[](plugin_item const&){}}, *mitem);
	}

	if (Owner->ProcessPluginEvent(FE_CLOSE, nullptr))
		return -1;

	Global->ScrBuf->Flush();
	INPUT_RECORD rec;
	PeekInputRecord(&rec);

	if (!mitem)
		return -1; //???

	if (mitem->index() == 0)
	{
		auto& item = std::get<disk_item>(*mitem);

		for (;;)
		{
			if (FarChDir(os::fs::get_drive(item.cDrive)) || FarChDir(os::fs::get_root_directory(item.cDrive)))
				break;

			error_state_ex const ErrorState = error_state::fetch();

			DialogBuilder Builder(lng::MError);

			Builder.AddTextWrap(ErrorState.format_error().c_str(), true);
			Builder.AddText(L"");

			string DriveLetter(1, item.cDrive);
			DialogItemEx *DriveLetterEdit = Builder.AddFixEditField(DriveLetter, 1);
			Builder.AddTextBefore(DriveLetterEdit, lng::MChangeDriveCannotReadDisk);
			Builder.AddTextAfter(DriveLetterEdit, L":", 0);

			Builder.AddOKCancel(lng::MRetry, lng::MCancel);
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);
			Builder.SetId(ChangeDriveCannotReadDiskErrorId);

			if (Builder.ShowDialog())
			{
				item.cDrive = upper(DriveLetter[0]);
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
		const auto& item = std::get<plugin_item>(*mitem);

		auto hPlugin = Global->CtrlObject->Plugins->Open(
			item.pPlugin,
			Owner->Parent()->IsLeft(Owner)? OPEN_LEFTDISKMENU : OPEN_RIGHTDISKMENU,
			item.Guid,
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
		Pos = static_cast<int>(os::fs::get_drive_number(CurDir[0]));
	}

	while (Pos != -1)
	{
		Pos = ChangeDiskMenu(Owner, Pos, FirstCall);
		FirstCall = false;
	}
}

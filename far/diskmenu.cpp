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

// BUGBUG
#include "platform.headers.hpp"

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
#include "uuids.far.dialogs.hpp"
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
#include "notification.hpp"
#include "keyboard.hpp"
#include "dirmix.hpp"
#include "lockscrn.hpp"
#include "string_sort.hpp"
#include "pathmix.hpp"
#include "cvtname.hpp"

// Platform:
#include "platform.hpp"
#include "platform.com.hpp"
#include "platform.fs.hpp"
#include "platform.reg.hpp"

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

struct disk_item
{
	string Path;
	int nDriveType;
};

struct plugin_item
{
	Plugin* pPlugin;
	UUID Uuid;
};

using disk_menu_item = std::variant<disk_item, plugin_item>;

[[nodiscard]]
static bool is_disk(string_view const RootDirectory)
{
	return RootDirectory.size() == L"\\\\?\\C:\\"sv.size();
}

[[nodiscard]]
static string_view dos_drive_name(string_view const RootDirectory)
{
	if (is_disk(RootDirectory))
		return RootDirectory.substr(L"\\\\?\\"sv.size(), L"C:"sv.size());

	return RootDirectory;
}

[[nodiscard]]
static string_view dos_drive_root_directory(string_view const RootDirectory)
{
	if (is_disk(RootDirectory))
		return RootDirectory.substr(L"\\\\?\\"sv.size(), L"C:\\"sv.size());

	return RootDirectory;
}

[[nodiscard]]
static auto EjectFailed(error_state_ex const& ErrorState, string_view const Path)
{
	// BUGBUG load uses the same error message
	return OperationFailed(ErrorState, extract_root_device(Path), lng::MError, msg(lng::MChangeCouldNotEjectMedia), false);
}

static void AddPluginItems(VMenu2 &ChDisk, int Pos, int DiskCount, bool SetSelected)
{
	struct menu_init_item
	{
		string Str;
		LISTITEMFLAGS Flags;
		wchar_t Hotkey;
		plugin_item PluginData;
	};

	std::vector<menu_init_item> MenuInitItems;
	MenuInitItems.reserve(Global->CtrlObject->Plugins->size());

	bool ItemPresent, Done = false;

	for (const auto& pPlugin: *Global->CtrlObject->Plugins)
	{
		if (Done)
			break;

		for (int PluginItem = 0;; ++PluginItem)
		{
			wchar_t HotKey = 0;
			UUID Uuid;
			string strPluginText;

			if (!Global->CtrlObject->Plugins->GetDiskMenuItem(
				pPlugin,
				PluginItem,
				ItemPresent,
				HotKey,
				strPluginText,
				Uuid
				))
			{
				Done = true;
				break;
			}

			if (!ItemPresent)
				break;

			if (!strPluginText.empty())
			{
				const auto Flags =
#ifndef NO_WRAPPER
					pPlugin->IsOemPlugin()? LIF_CHECKED | L'A' :
#endif // NO_WRAPPER
					LIF_NONE;

				MenuInitItems.push_back({ std::move(strPluginText), Flags, HotKey, { pPlugin, Uuid } });
			}
		}
	}

	if (MenuInitItems.empty())
		return;

	std::ranges::sort(MenuInitItems, [SortByHotkey = (Global->Opt->ChangeDriveMode & DRIVE_SORT_PLUGINS_BY_HOTKEY) != 0](menu_init_item const& a, menu_init_item const& b)
	{
		if (!SortByHotkey || a.Hotkey == b.Hotkey)
			return string_sort::less(a.Str, b.Str);

		if (!a.Hotkey)
			return false;

		if (!b.Hotkey)
			return true;

		return a.Hotkey < b.Hotkey;
	});

	MenuItemEx ChDiskItem;
	ChDiskItem.Flags |= LIF_SEPARATOR;
	ChDisk.AddItem(ChDiskItem);

	for (const auto& [i, index]: enumerate(MenuInitItems))
	{
		MenuItemEx MenuItem;
		MenuItem.Name = concat(i.Hotkey? concat(L'&', i.Hotkey, L"  "sv) : L"   "sv, i.Str);
		MenuItem.Flags = i.Flags;
		MenuItem.ComplexUserData = disk_menu_item{ i.PluginData };
		if (Pos > DiskCount && !SetSelected && DiskCount + static_cast<int>(index) + 1 == Pos)
		{
			SetSelected = true;
			MenuItem.SetSelect(true);
		}

		ChDisk.AddItem(MenuItem);
	}
}

static void ConfigureChangeDriveMode()
{
	auto& DriveMode = Global->Opt->ChangeDriveMode;

	DialogBuilder Builder(lng::MChangeDriveConfigure, L"ChangeDriveMode"sv);
	Builder.SetId(ChangeDriveModeId);
	Builder.AddCheckbox(lng::MChangeDriveShowDiskType, DriveMode, DRIVE_SHOW_TYPE);
	auto& ShowLabel = Builder.AddCheckbox(lng::MChangeDriveShowLabel, DriveMode, DRIVE_SHOW_LABEL);
	auto& ShowLabelUseShell = Builder.AddCheckbox(lng::MChangeDriveShowLabelUseShell, DriveMode, DRIVE_SHOW_LABEL_USE_SHELL);
	ShowLabelUseShell.Indent(4);
	Builder.LinkFlags(ShowLabel, ShowLabelUseShell, DIF_DISABLE);
	Builder.AddCheckbox(lng::MChangeDriveShowFileSystem, DriveMode, DRIVE_SHOW_FILESYSTEM);

	int ShowSizeAny = DriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);

	auto& ShowSize = Builder.AddCheckbox(lng::MChangeDriveShowSize, ShowSizeAny);
	auto& ShowSizeFloat = Builder.AddCheckbox(lng::MChangeDriveShowSizeFloat, DriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat.Indent(4);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(lng::MChangeDriveShowPath, DriveMode, DRIVE_SHOW_ASSOCIATED_PATH);
	Builder.AddCheckbox(lng::MChangeDriveShowPlugins, DriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(lng::MChangeDriveSortPluginsByHotkey, DriveMode, DRIVE_SORT_PLUGINS_BY_HOTKEY).Indent(4);
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

static int MessageRemoveConnection(string_view const Drive, int &UpdateProfile)
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

	DCDlg[rc_text_1].strData = far::vformat(msg(lng::MChangeDriveDisconnectQuestion), Drive);
	DCDlg[rc_text_2].strData = msg(lng::MChangeDriveDisconnectMapped);

	const auto Len = std::max({ DCDlg[rc_doublebox].strData.size(), DCDlg[rc_text_1].strData.size(), DCDlg[rc_text_1].strData.size(), DCDlg[rc_checkbox].strData.size() });

	{
		string strMsgText;
		// TODO: check result
		DriveLocalToRemoteName(false, Drive, strMsgText);
		DCDlg[rc_text_3].strData = truncate_path(std::move(strMsgText), Len);
	}

	// проверяем - это было постоянное соединение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.

	bool IsPersistent = true;
	if (os::reg::key::current_user.open(concat(L"Network\\"sv, Drive.front()), KEY_QUERY_VALUE))
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

static bool ProcessDelDisk(panel_ptr Owner, string_view const Path, int DriveType)
{
	const auto DosDriveName = dos_drive_name(Path);

	switch (DriveType)
	{
	case DRIVE_SUBSTITUTE:
		{
			if (Global->Opt->Confirm.RemoveSUBST)
			{
				const auto Question = far::vformat(msg(lng::MChangeSUBSTDisconnectDriveQuestion), DosDriveName);
				string SubstitutedPath;
				GetSubstName(DriveType, DosDriveName, SubstitutedPath);
				if (Message(MSG_WARNING,
					msg(lng::MChangeSUBSTDisconnectDriveTitle),
					{
						Question,
						msg(lng::MChangeDriveDisconnectMapped),
						SubstitutedPath
					},
					{ lng::MYes, lng::MNo },
					{}, &SUBSTDisconnectDriveId) != message_result::first_button)
				{
					return false;
				}
			}

			if (DelSubstDrive(DosDriveName))
			{
				return true;
			}

			const auto ErrorState = os::last_error();

			const auto LastError = ErrorState.Win32Error;
			const auto strMsgText = far::vformat(msg(lng::MChangeDriveCannotDelSubst), DosDriveName);
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
					{}, &SUBSTDisconnectDriveError1Id) == message_result::first_button)
				{
					if (DelSubstDrive(DosDriveName))
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
			if (!MessageRemoveConnection(DosDriveName, UpdateProfile))
				return false;

			{
				// <КОСТЫЛЬ>
				SCOPED_ACTION(LockScreen);
				// если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
				// удалению
				Owner->GoHome(DosDriveName);
				Global->WindowManager->ResizeAllWindows();
				Global->WindowManager->GetCurrentWindow()->Show();
				// </КОСТЫЛЬ>
			}

			null_terminated const C_DosDriveName(DosDriveName);

			if (WNetCancelConnection2(C_DosDriveName.c_str(), UpdateProfile, FALSE) == NO_ERROR)
				return true;

			const auto ErrorState = os::last_error();

			const auto strMsgText = far::vformat(msg(lng::MChangeDriveCannotDisconnect), DosDriveName);
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
					{}, &RemoteDisconnectDriveError1Id) == message_result::first_button)
				{
					if (WNetCancelConnection2(C_DosDriveName.c_str(), UpdateProfile, TRUE) == NO_ERROR)
					{
						return true;
					}
				}
				else
				{
					return false;
				}
			}

			if (os::fs::drive::get_type(Path) == DRIVE_REMOTE)
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
				const auto Question = far::vformat(msg(lng::MChangeVHDDisconnectDriveQuestion), DosDriveName);
				if (Message(MSG_WARNING,
					msg(lng::MChangeVHDDisconnectDriveTitle),
					{
						Question
					},
					{ lng::MYes, lng::MNo },
					{}, &VHDDisconnectDriveId) != message_result::first_button)
				{
					return false;
				}
			}

			if (auto Dummy = false; detach_vhd(Path, Dummy))
				return true;

			const auto ErrorState = os::last_error();

			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					far::vformat(msg(lng::MChangeDriveCannotDetach), DosDriveName)
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
		return ProcessDelDisk(Owner, item.Path, item.nDriveType);

	if (item.nDriveType == DRIVE_REMOVABLE && !IsEjectableMedia(item.Path))
	{
		Cancelled = true;
		return false;
	}

	// первая попытка извлечь диск
	try
	{
		EjectVolume(item.Path);
		return true;
	}
	catch (far_exception const&)
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
			Owner->GoHome(dos_drive_name(item.Path));
			// очередная попытка извлечения без вывода сообщения
			try
			{
				EjectVolume(item.Path);
				return true;
			}
			catch (far_exception const& e)
			{
				// восстановим пути - это избавит нас от левых данных в панели.
				if (AMode != panel_mode::PLUGIN_PANEL)
					Owner->Parent()->GetAnotherPanel(Owner)->SetCurDir(TmpADir, false);

				if (CMode != panel_mode::PLUGIN_PANEL)
					Owner->SetCurDir(TmpCDir, false);

				if (EjectFailed(e, item.Path) != operation::retry)
					return false;
			}
		}
	}
}

static void RemoveHotplugDevice(panel_ptr Owner, const disk_item& item, VMenu2 &ChDisk)
{
	bool Cancelled = false;
	if (RemoveHotplugDrive(item.Path, Global->Opt->Confirm.RemoveHotPlug, Cancelled) || Cancelled)
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
		Owner->GoHome(dos_drive_name(item.Path));
		// очередная попытка извлечения без вывода сообщения
		if (RemoveHotplugDrive(item.Path, false, Cancelled) || Cancelled)
			return;

		const auto ErrorState = os::last_error();

		// восстановим пути - это избавит нас от левых данных в панели.
		if (AMode != panel_mode::PLUGIN_PANEL)
			Owner->Parent()->GetAnotherPanel(Owner)->SetCurDir(TmpADir, false);

		if (CMode != panel_mode::PLUGIN_PANEL)
			Owner->SetCurDir(TmpCDir, false);

		if (Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			{
				far::vformat(msg(lng::MChangeCouldNotEjectHotPlugMedia), dos_drive_name(item.Path))
			},
			{ lng::MHRetry, lng::MHCancel },
			{}, &EjectHotPlugMediaErrorId) != message_result::first_button)
			return;
	}
}

static string GetShellName(string_view const RootDirectory)
{
	return is_disk(RootDirectory)?
		os::com::get_shell_name(dos_drive_root_directory(RootDirectory)) :
		L""s;
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

	disk_menu_item Item, *mitem = nullptr;
	{ // эта скобка надо, см. M#605
		const auto ChDisk = VMenu2::create(msg(lng::MChangeDriveTitle), {}, ScrY - PanelRect.top - 3);
		ChDisk->SetBottomTitle(KeysToLocalizedText(KEY_DEL, KEY_SHIFTDEL, KEY_F3, KEY_F4, KEY_F9));
		ChDisk->SetHelp(L"DriveDlg"sv);
		ChDisk->SetMenuFlags(VMENU_WRAPMODE);
		ChDisk->SetId(ChangeDiskMenuId);

		struct DiskMenuItem
		{
			string RootDirectory;
			string Type;
			string Label;
			string Fs;
			string TotalSize;
			string FreeSize;
			string AssociatedPath;

			int DriveType;
		};

		std::vector<DiskMenuItem> Items;
		Items.reserve(AllDrives.size() * 2);

		size_t TypeWidth = 0, LabelWidth = 0, FsWidth = 0, TotalSizeWidth = 0, FreeSizeWidth = 0, PathWidth = 0;


		std::optional<elevation::suppress> DE(std::in_place);
		auto& DriveMode = Global->Opt->ChangeDriveMode;

		const auto process_location = [&](string_view const RootDirectory)
		{
			const auto IsDisk = is_disk(RootDirectory);

			DiskMenuItem NewItem;
			NewItem.RootDirectory = RootDirectory;

			// We have to determine at least the basic drive type (fixed/removable/remote) regardless of the DRIVE_SHOW_TYPE state,
			// as it affects the visibility of the other metrics
			NewItem.DriveType = os::fs::drive::get_type(RootDirectory);

			if (IsDisk && DisconnectedNetworkDrives[os::fs::drive::get_number(dos_drive_name(RootDirectory).front())])
			{
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;
			}

			if (DriveMode & (DRIVE_SHOW_TYPE | DRIVE_SHOW_ASSOCIATED_PATH))
			{
				// These types don't affect other checks so we can retrieve them only if needed:
				if (GetSubstName(NewItem.DriveType, dos_drive_name(RootDirectory), NewItem.AssociatedPath))
				{
					NewItem.DriveType = DRIVE_SUBSTITUTE;
				}
				else if ((DriveMode & DRIVE_SHOW_VIRTUAL) && DriveCanBeVirtual(NewItem.DriveType) && GetVHDInfo(RootDirectory, NewItem.AssociatedPath))
				{
					NewItem.DriveType = DRIVE_VIRTUAL;
				}

				if (DriveMode & DRIVE_SHOW_TYPE)
				{
					if (NewItem.DriveType == DRIVE_CDROM)
					{
						static_assert(std::to_underlying(lng::MChangeDriveHDDVDRAM) - std::to_underlying(lng::MChangeDriveCDROM) == std::to_underlying(cd_type::hddvdram) - std::to_underlying(cd_type::cdrom));

						NewItem.Type = msg(lng::MChangeDriveCDROM + ((DriveMode & DRIVE_SHOW_CDROM)? std::to_underlying(get_cdrom_type(RootDirectory)) - std::to_underlying(cd_type::cdrom) : 0));
					}
					else
					{
						if (const auto TypeId = [](int const Type) -> std::optional<lng>
							{
								switch (Type)
								{
								case DRIVE_REMOVABLE:                 return lng::MChangeDriveRemovable;
								case DRIVE_FIXED:                     return lng::MChangeDriveFixed;
								case DRIVE_REMOTE:                    return lng::MChangeDriveNetwork;
								case DRIVE_REMOTE_NOT_CONNECTED:      return lng::MChangeDriveDisconnectedNetwork;
								case DRIVE_RAMDISK:                   return lng::MChangeDriveRAM;
								case DRIVE_SUBSTITUTE:                return lng::MChangeDriveSUBST;
								case DRIVE_VIRTUAL:                   return lng::MChangeDriveVirtual;
								default:                              return {};
								}
							}(NewItem.DriveType))
						{
							NewItem.Type = msg(*TypeId);
						}
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
						NewItem.Label = GetShellName(RootDirectory);
						TryReadLabel = NewItem.Label.empty();
					}

					const auto LabelPtr = TryReadLabel? &NewItem.Label : nullptr;
					const auto FsPtr = DriveMode & DRIVE_SHOW_FILESYSTEM? &NewItem.Fs : nullptr;

					if ((LabelPtr || FsPtr) && !os::fs::GetVolumeInformation(RootDirectory, LabelPtr, nullptr, nullptr, nullptr, FsPtr))
					{
						if (LabelPtr)
							*LabelPtr = msg(lng::MChangeDriveLabelAbsent);

						// Should we set *FsPtr to something like "Absent" too?
					}
				}

				if (DriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT))
				{
					unsigned long long UserTotal = 0, UserFree = 0;
					if (os::fs::get_disk_size(RootDirectory, &UserTotal, {}, &UserFree))
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

						NewItem.TotalSize = FormatSize(UserTotal);
						NewItem.FreeSize = FormatSize(UserFree);
					}
				}
			}

			if (DriveMode & DRIVE_SHOW_ASSOCIATED_PATH)
			{
				switch (NewItem.DriveType)
				{
				case DRIVE_REMOTE:
				case DRIVE_REMOTE_NOT_CONNECTED:
					// TODO: check result
					DriveLocalToRemoteName(false, RootDirectory, NewItem.AssociatedPath);
					break;
				}
			}

			TypeWidth = std::max(TypeWidth, visual_string_length(NewItem.Type));
			LabelWidth = std::max(LabelWidth, visual_string_length(NewItem.Label));
			FsWidth = std::max(FsWidth, visual_string_length(NewItem.Fs));
			TotalSizeWidth = std::max(TotalSizeWidth, visual_string_length(NewItem.TotalSize));
			FreeSizeWidth = std::max(FreeSizeWidth, visual_string_length(NewItem.FreeSize));
			PathWidth = std::max(PathWidth, visual_string_length(NewItem.AssociatedPath));

			Items.emplace_back(NewItem);
		};

		for (const auto& i: os::fs::enum_drives(AllDrives))
		{
			process_location(os::fs::drive::get_win32nt_root_directory(i));
		}

		if (DriveMode & DRIVE_SHOW_UNMOUNTED_VOLUMES)
		{
			for (const auto& i : os::fs::enum_volumes())
			{
				if (const auto DriveLetter = get_volume_drive(i); DriveLetter && AllDrives[os::fs::drive::get_number(*DriveLetter)])
					continue;

				process_location(i);
			}
		}

		int MenuLine = 0;

		bool SetSelected = false;

		if (!FirstCall && Pos == static_cast<int>(Items.size()))
		{
			// Pos points to the separator - this is probably a redraw after a removal of the last disk.
			--Pos;
		}

		for (const auto& i: Items)
		{
			MenuItemEx ChDiskItem;

			const auto IsDisk = is_disk(i.RootDirectory);

			if (FirstCall)
			{
				if (IsDisk)
				{
					const auto DiskNumber = os::fs::drive::get_number(i.RootDirectory[L"\\\\?\\"sv.size()]);

					ChDiskItem.SetSelect(static_cast<int>(DiskNumber) == Pos);

					if (!SetSelected)
						SetSelected = (static_cast<int>(DiskNumber) == Pos);
				}
			}
			else
			{
				if (Pos < static_cast<int>(Items.size()))
				{
					ChDiskItem.SetSelect(MenuLine == Pos);

					if (!SetSelected)
						SetSelected = (MenuLine == Pos);
				}
			}

			string ItemName{ IsDisk? dos_drive_name(i.RootDirectory) : L"  "sv };

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
			if (DriveMode & DRIVE_SHOW_ASSOCIATED_PATH && PathWidth)
			{
				append(ItemName, Separator(), i.AssociatedPath);
			}

			disk_menu_item item{ disk_item{i.RootDirectory, i.DriveType} };

			inplace::escape_ampersands(ItemName);
			ItemName.insert(0, 1, L'&');

			ChDiskItem.Name = std::move(ItemName);
			ChDiskItem.ComplexUserData = item;
			ChDisk->AddItem(ChDiskItem);

			MenuLine++;
		}

		if (DriveMode & DRIVE_SHOW_PLUGINS)
		{
			AddPluginItems(*ChDisk, Pos, static_cast<int>(Items.size()), SetSelected);
		}

		DE.reset();

		int X = PanelRect.left + 5;

		if ((Owner == Owner->Parent()->RightPanel()) && Owner->IsFullScreen() && (PanelRect.width() > 40))
			X = PanelRect.width() / 2 + 5;

		ChDisk->SetPosition({ X, -1, 0, 0 });

		int Y = (ScrY + 1 - static_cast<int>(Items.size() + 5)) / 2;
		if (Y < 3)
			ChDisk->SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk->SetMacroMode(MACROAREA_DISKS);
		int RetCode = -1;

		// TODO: position to a new drive?
		SCOPED_ACTION(listener)(update_devices, [&]
		{
			ChDisk->ProcessKey(Manager::Key(KEY_CTRLR));
		});

		ChDisk->Run([&](const Manager::Key& RawKey)
		{
			const auto SelPos = ChDisk->GetSelectPos();
			const auto MenuItem = ChDisk->GetComplexUserDataPtr<disk_menu_item>();

			int KeyProcessed = 1;

			switch (const auto& Key = RawKey())
			{
			// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
			case KEY_SHIFTNUMENTER:
			case KEY_SHIFTENTER:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)
				{
					OpenFolderInShell(dos_drive_root_directory(item.Path));
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
							LoadVolume(item.Path);
							break;
						}
						catch (far_exception const& e)
						{
							if (EjectFailed(e, item.Path) != operation::retry)
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
					Global->CtrlObject->Plugins->ShowPluginInfo(item.pPlugin, item.Uuid);
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
						if (Global->CtrlObject->Plugins->SetHotKeyDialog(item.pPlugin, item.Uuid, hotkey_type::drive_menu, trim(string_view(ChDisk->at(SelPos).Name).substr(3))))
							RetCode = SelPos;
					},
					[](disk_item const& item)
					{
						ShellSetFileAttributes(nullptr, &item.Path);
					}
				},
				*MenuItem);
				break;

			case KEY_APPS:
			case KEY_SHIFTAPPS:
			case KEY_MSRCLICK:
			case KEY_SHIFT|KEY_MSRCLICK:
				if (!MenuItem)
					break;

				std::visit(overload{[&](disk_item const& item)
				{
					if (!Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
						return;

					//вызовем EMenu если он есть
					null_terminated const RootDirectory(dos_drive_root_directory(item.Path));
					struct DiskMenuParam
					{
						const wchar_t* CmdLine; BOOL Apps; COORD MousePos;
					}
					p
					{
						RootDirectory.c_str(),
						any_of(Key, KEY_APPS, KEY_SHIFTAPPS),
						{
							static_cast<short>(IntKeyState.MousePos.x),
							static_cast<short>(IntKeyState.MousePos.y)
						},
					};
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
				DriveMode ^= DRIVE_SHOW_ASSOCIATED_PATH;
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

			case KEY_CTRLH:
			case KEY_RCTRLH:
				DriveMode ^= DRIVE_SHOW_UNMOUNTED_VOLUMES;
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
						Global->CtrlObject->Plugins->ConfigureCurrent(item.pPlugin, item.Uuid);
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

		if (ChDisk->GetExitCode() < 0)
		{
			if (os::fs::drive::get_type(CurDir) == DRIVE_NO_ROOT_DIR)
			{
				// get_type can return DRIVE_NO_ROOT_DIR when we can't access the path
				if (!ElevationRequired(ELEVATION_READ_REQUEST))
					return ChDisk->GetSelectPos();
			}

			return -1;
		}

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

			if (os::fs::IsDiskInDrive(item.Path))
				return;

			Message(0,
				{},
				{
					msg(lng::MChangeWaitingLoadDisk)
				},
				{});

			try
			{
				LoadVolume(item.Path);
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

	if (const auto DiskItemPtr = std::get_if<disk_item>(mitem))
	{
		auto& item = *DiskItemPtr;

		const auto IsDisk = is_disk(item.Path);

		while (!(FarChDir(dos_drive_name(item.Path)) || (IsDisk && FarChDir(dos_drive_root_directory(item.Path)))))
		{
			error_state_ex const ErrorState = os::last_error();

			DialogBuilder Builder(lng::MError);

			string DriveLetter;
			if (IsDisk)
			{
				DriveLetter = dos_drive_name(item.Path).front();
				auto& DriveLetterEdit = Builder.AddFixEditField(DriveLetter, 1);
				Builder.AddTextBefore(DriveLetterEdit, lng::MChangeDriveCannotReadDisk);
				Builder.AddTextAfter(DriveLetterEdit, L":", 0);
			}
			else
			{
				Builder.AddText(far::format(L"{} {}"sv, msg(lng::MChangeDriveCannotReadDisk), item.Path));
			}

			Builder.AddSeparator();

			if (!ErrorState.What.empty())
				Builder.AddTextWrap(ErrorState.What, true);

			if (ErrorState.any())
				Builder.AddTextWrap(ErrorState.system_error(), true);

			Builder.AddOKCancel(lng::MRetry, lng::MCancel);
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);
			Builder.SetId(ChangeDriveCannotReadDiskErrorId);

			if (Builder.ShowDialog())
			{
				if (IsDisk)
					item.Path = os::fs::drive::get_win32nt_root_directory(upper(DriveLetter[0]));
			}
			else
			{
				return -1;
			}
		}

		const auto strNewCurDir = os::fs::get_current_directory();

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
			item.Uuid,
			0);

		if (!hPlugin)
			return -1;

		const auto IsActive = Owner->IsFocused();
		const auto NewPanel = Owner->Parent()->ChangePanel(Owner, panel_type::FILE_PANEL, TRUE, TRUE);
		NewPanel->SetPluginMode(std::move(hPlugin), {}, IsActive || !NewPanel->Parent()->GetAnotherPanel(NewPanel)->IsVisible());
		NewPanel->Update(0);
		NewPanel->Show();

		if (!IsActive && NewPanel->Parent()->GetAnotherPanel(NewPanel)->GetType() == panel_type::INFO_PANEL)
			NewPanel->Parent()->GetAnotherPanel(NewPanel)->UpdateKeyBar();
	}

	return -1;
}

void ChangeDisk(panel_ptr Owner)
{
	int Pos = 0;
	bool FirstCall = true;

	const auto& CurDir = Owner->GetCurDir();
	if (!CurDir.empty() && CurDir[1] == L':' && os::fs::drive::is_standard_letter(CurDir[0]))
	{
		Pos = static_cast<int>(os::fs::drive::get_number(CurDir[0]));
	}

	while (Pos != -1)
	{
		Pos = ChangeDiskMenu(Owner, Pos, FirstCall);
		FirstCall = false;
	}
}

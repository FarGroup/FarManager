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

#include "headers.hpp"
#pragma hdrstop

#include "diskmenu.hpp"
#include "global.hpp"
#include "config.hpp"
#include "vmenu2.hpp"
#include "strmix.hpp"
#include "ctrlobj.hpp"
#include "plugins.hpp"
#include "language.hpp"
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

class ChDiskPluginItem
{
public:
	NONCOPYABLE(ChDiskPluginItem);
	TRIVIALLY_MOVABLE(ChDiskPluginItem);

	ChDiskPluginItem():
		HotKey()
	{}

	bool operator <(const ChDiskPluginItem& rhs) const
	{
		return (Global->Opt->ChangeDriveMode & DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey != rhs.HotKey)?
			HotKey < rhs.HotKey :
			StrCmpI(Item.strName, rhs.Item.strName) < 0;
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
	int PluginItem;
	bool ItemPresent, Done = false;
	string strMenuText;
	string strPluginText;
	size_t PluginMenuItemsCount = 0;

	for (const auto& i: *Global->CtrlObject->Plugins)
	{
		if (Done)
			break;
		for (PluginItem = 0;; ++PluginItem)
		{
			Plugin *pPlugin = i;

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

			strMenuText = strPluginText;

			if (!strMenuText.empty())
			{
				ChDiskPluginItem OneItem;
#ifndef NO_WRAPPER
				if (pPlugin->IsOemPlugin())
					OneItem.getItem().Flags = LIF_CHECKED | L'A';
#endif // NO_WRAPPER
				OneItem.getItem().strName = strMenuText;
				OneItem.getHotKey() = HotKey;

				PanelMenuItem item = {};
				item.bIsPlugin = true;
				item.pPlugin = pPlugin;
				item.Guid = guid;
				OneItem.getItem().UserData = item;

				MPItems.emplace_back(std::move(OneItem));
			}
		}
	}

	MPItems.sort();

	PluginMenuItemsCount = MPItems.size();

	if (PluginMenuItemsCount)
	{
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
			wchar_t HotKey = i.getHotKey();
			const wchar_t HotKeyStr[] = { HotKey? L'&' : L' ', HotKey? HotKey : L' ', L' ', HotKey? L' ' : L'\0', L'\0' };
			i.getItem().strName = string(HotKeyStr) + i.getItem().strName;
			ChDisk.AddItem(i.getItem());
		});
	}
	return PluginMenuItemsCount;
}

static void ConfigureChangeDriveMode()
{
	DialogBuilder Builder(MChangeDriveConfigure, L"ChangeDriveMode");
	Builder.SetId(ChangeDriveModeId);
	Builder.AddCheckbox(MChangeDriveShowDiskType, Global->Opt->ChangeDriveMode, DRIVE_SHOW_TYPE);
	Builder.AddCheckbox(MChangeDriveShowLabel, Global->Opt->ChangeDriveMode, DRIVE_SHOW_LABEL);
	Builder.AddCheckbox(MChangeDriveShowFileSystem, Global->Opt->ChangeDriveMode, DRIVE_SHOW_FILESYSTEM);

	BOOL ShowSizeAny = Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);

	DialogItemEx *ShowSize = Builder.AddCheckbox(MChangeDriveShowSize, &ShowSizeAny);
	DialogItemEx *ShowSizeFloat = Builder.AddCheckbox(MChangeDriveShowSizeFloat, Global->Opt->ChangeDriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat->Indent(4);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(MChangeDriveShowNetworkName, Global->Opt->ChangeDriveMode, DRIVE_SHOW_NETNAME);
	Builder.AddCheckbox(MChangeDriveShowPlugins, Global->Opt->ChangeDriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(MChangeDriveSortPluginsByHotkey, Global->Opt->ChangeDriveMode, DRIVE_SORT_PLUGINS_BY_HOTKEY)->Indent(4);
	Builder.AddCheckbox(MChangeDriveShowRemovableDrive, Global->Opt->ChangeDriveMode, DRIVE_SHOW_REMOVABLE);
	Builder.AddCheckbox(MChangeDriveShowCD, Global->Opt->ChangeDriveMode, DRIVE_SHOW_CDROM);
	Builder.AddCheckbox(MChangeDriveShowNetworkDrive, Global->Opt->ChangeDriveMode, DRIVE_SHOW_REMOTE);

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
			return{ L' ' };
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
		{ DI_DOUBLEBOX, 3, 1, 72, 9, 0, nullptr, nullptr, 0, MSG(MChangeDriveDisconnectTitle) },
		{ DI_TEXT, 5, 2, 0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND, L"" },
		{ DI_TEXT, 5, 3, 0, 3, 0, nullptr, nullptr, DIF_SHOWAMPERSAND, L"" },
		{ DI_TEXT, 5, 4, 0, 4, 0, nullptr, nullptr, DIF_SHOWAMPERSAND, L"" },
		{ DI_TEXT, -1, 5, 0, 5, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_CHECKBOX, 5, 6, 70, 6, 0, nullptr, nullptr, 0, MSG(MChangeDriveDisconnectReconnect) },
		{ DI_TEXT, -1, 7, 0, 7, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_BUTTON, 0, 8, 0, 8, 0, nullptr, nullptr, DIF_FOCUS | DIF_DEFAULTBUTTON | DIF_CENTERGROUP, MSG(MYes) },
		{ DI_BUTTON, 0, 8, 0, 8, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel) },
	};
	auto DCDlg = MakeDialogItemsEx(DCDlgData);

	DCDlg[1].strData = format(MChangeDriveDisconnectQuestion, Letter);
	DCDlg[2].strData = format(MChangeDriveDisconnectMapped, Letter);

	size_t Len1 = DCDlg[0].strData.size();
	size_t Len2 = DCDlg[1].strData.size();
	size_t Len3 = DCDlg[2].strData.size();
	size_t Len4 = DCDlg[5].strData.size();
	Len1 = std::max(Len1, std::max(Len2, std::max(Len3, Len4)));
	string strMsgText;
	// TODO: check result
	DriveLocalToRemoteName(DRIVE_REMOTE, Letter, strMsgText);
	DCDlg[3].strData = TruncPathStr(strMsgText, static_cast<int>(Len1));
	// проверяем - это было постоянное соединение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.

	bool IsPersistent = true;
	const wchar_t KeyName[] = { L'N', L'e', L't', L'w', L'o', L'r', L'k', L'\\', Letter, L'\0' };

	if (os::reg::open_key(HKEY_CURRENT_USER, KeyName, KEY_QUERY_VALUE))
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
	DCDlg[0].X2 = DCDlg[0].X1 + Len1 + 3;
	int ExitCode = 7;

	if (Global->Opt->Confirm.RemoveConnection)
	{
		const auto Dlg = Dialog::create(DCDlg);
		Dlg->SetPosition(-1, -1, DCDlg[0].X2 + 4, 11);
		Dlg->SetHelp(L"DisconnectDrive");
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
	const string DiskLetter{ Drive, L':' };

	switch (DriveType)
	{
	case DRIVE_SUBSTITUTE:
	{
		if (Global->Opt->Confirm.RemoveSUBST)
		{
			const auto Question = format(MChangeSUBSTDisconnectDriveQuestion, DiskLetter);
			const auto MappedTo = format(MChangeDriveDisconnectMapped, DiskLetter.front());
			string SubstitutedPath;
			GetSubstName(DriveType, DiskLetter, SubstitutedPath);
			if (Message(MSG_WARNING, MSG(MChangeSUBSTDisconnectDriveTitle),
				{ Question, MappedTo, SubstitutedPath },
				{ MSG(MYes), MSG(MNo) },
				nullptr, nullptr, &SUBSTDisconnectDriveId) != Message::first_button)
			{
				return DRIVE_DEL_FAIL;
			}
		}
		if (DelSubstDrive(DiskLetter))
		{
			return DRIVE_DEL_SUCCESS;
		}
		else
		{
			Global->CatchError();
			DWORD LastError = Global->CaughtError();
			const auto strMsgText = format(MChangeDriveCannotDelSubst, DiskLetter);
			if (LastError == ERROR_OPEN_FILES || LastError == ERROR_DEVICE_IN_USE)
			{
				if (Message(MSG_WARNING | MSG_ERRORTYPE, MSG(MError),
					{ strMsgText, L"\x1", MSG(MChangeDriveOpenFiles), MSG(MChangeDriveAskDisconnect) },
					{ MSG(MOk), MSG(MCancel) },
					nullptr, nullptr, &SUBSTDisconnectDriveError1Id) == Message::first_button)
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
			Message(MSG_WARNING | MSG_ERRORTYPE, MSG(MError),
				{ strMsgText },
				{ MSG(MOk) },
				nullptr, nullptr, &SUBSTDisconnectDriveError2Id);
		}
		return DRIVE_DEL_FAIL; // блин. в прошлый раз забыл про это дело...
	}
	break;

	case DRIVE_REMOTE:
	case DRIVE_REMOTE_NOT_CONNECTED:
	{
		int UpdateProfile = CONNECT_UPDATE_PROFILE;
		if (MessageRemoveConnection(Drive, UpdateProfile))
		{
			// <КОСТЫЛЬ>
			SCOPED_ACTION(LockScreen);
			// если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
			// удалению
			Owner->IfGoHome(Drive);
			Global->WindowManager->ResizeAllWindows();
			Global->WindowManager->GetCurrentWindow()->Show();
			// </КОСТЫЛЬ>

			if (WNetCancelConnection2(DiskLetter.data(), UpdateProfile, FALSE) == NO_ERROR)
			{
				return DRIVE_DEL_SUCCESS;
			}
			else
			{
				Global->CatchError();
				const auto strMsgText = format(MChangeDriveCannotDisconnect, DiskLetter);
				DWORD LastError = Global->CaughtError();
				if (LastError == ERROR_OPEN_FILES || LastError == ERROR_DEVICE_IN_USE)
				{
					if (Message(MSG_WARNING | MSG_ERRORTYPE, MSG(MError),
						{ strMsgText, L"\x1", MSG(MChangeDriveOpenFiles), MSG(MChangeDriveAskDisconnect) },
						{ MSG(MOk), MSG(MCancel) },
						nullptr, nullptr, &RemoteDisconnectDriveError1Id) == Message::first_button)
					{
						if (WNetCancelConnection2(DiskLetter.data(), UpdateProfile, TRUE) == NO_ERROR)
						{
							return DRIVE_DEL_SUCCESS;
						}
					}
					else
					{
						return DRIVE_DEL_FAIL;
					}
				}
				const wchar_t RootDir[] = { DiskLetter[0], L':', L'\\', L'\0' };
				if (FAR_GetDriveType(RootDir) == DRIVE_REMOTE)
				{
					Message(MSG_WARNING | MSG_ERRORTYPE, MSG(MError),
						{ strMsgText },
						{ MSG(MOk) },
						nullptr, nullptr, &RemoteDisconnectDriveError2Id);
				}
			}
			return DRIVE_DEL_FAIL;
		}
	}
	break;

	case DRIVE_VIRTUAL:
	{
		if (Global->Opt->Confirm.DetachVHD)
		{
			const auto Question = format(MChangeVHDDisconnectDriveQuestion, DiskLetter);
			if (Message(MSG_WARNING, MSG(MChangeVHDDisconnectDriveTitle),
				{ Question },
				{ MSG(MYes), MSG(MNo) },
				nullptr, nullptr, &VHDDisconnectDriveId) != Message::first_button)
			{
				return DRIVE_DEL_FAIL;
			}
		}
		string strVhdPath;
		VIRTUAL_STORAGE_TYPE VirtualStorageType;
		int Result = DRIVE_DEL_FAIL;
		if (GetVHDInfo(DiskLetter, strVhdPath, &VirtualStorageType) && !strVhdPath.empty())
		{
			if (os::DetachVirtualDisk(strVhdPath, VirtualStorageType))
			{
				Result = DRIVE_DEL_SUCCESS;
			}
			else
			{
				Global->CatchError();
			}
		}
		else
		{
			Global->CatchError();
		}

		if (Result != DRIVE_DEL_SUCCESS)
		{
			Message(MSG_WARNING | MSG_ERRORTYPE, MSG(MError),
				{ format(MChangeDriveCannotDetach, DiskLetter) },
				{ MSG(MOk) },
				nullptr, nullptr, &VHDDisconnectDriveErrorId);
		}
		return Result;
	}
	break;

	}
	return DRIVE_DEL_FAIL;
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
			int DoneEject = FALSE;

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
					Global->CatchError();
					wchar_t Drive[] = { item->cDrive, L':', L'\\', 0 };
					DoneEject = OperationFailed(Drive, MError, format(MChangeCouldNotEjectMedia, item->cDrive), false);
				}
				else
					DoneEject = TRUE;
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
				Global->CatchError();
				DoneEject = Message(MSG_WARNING | MSG_ERRORTYPE,
					MSG(MError),
					{ format(MChangeCouldNotEjectHotPlugMedia, item->cDrive) },
					{ MSG(MHRetry), MSG(MHCancel) },
					nullptr, nullptr, &EjectHotPlugMediaErrorId) != Message::first_button;
			}
			else
				DoneEject = TRUE;
		}
	}
}

static int ChangeDiskMenu(panel_ptr Owner, int Pos, bool FirstCall)
{
	int Panel_X1, Panel_X2, Panel_Y1, Panel_Y2;
	Owner->GetPosition(Panel_X1, Panel_Y1, Panel_X2, Panel_Y2);

	class Guard_Macro_DskShowPosType  //фигня какая-то
	{
	public:
		Guard_Macro_DskShowPosType(panel_ptr curPanel) { Global->Macro_DskShowPosType = curPanel->Parent()->IsLeft(curPanel)? 1 : 2; }
		~Guard_Macro_DskShowPosType() { Global->Macro_DskShowPosType = 0; }
	};
	SCOPED_ACTION(Guard_Macro_DskShowPosType)(Owner);

	auto Mask = FarGetLogicalDrives();
	const auto NetworkMask = AddSavedNetworkDisks(Mask);
	const auto DiskCount = Mask.count();

	PanelMenuItem Item, *mitem = nullptr;
	{ // эта скобка надо, см. M#605
		const auto ChDisk = VMenu2::create(MSG(MChangeDriveTitle), nullptr, 0, ScrY - Panel_Y1 - 3);
		ChDisk->SetBottomTitle(MSG(MChangeDriveMenuFooter));
		ChDisk->SetHelp(L"DriveDlg");
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
		/* $ 02.04.2001 VVM
		! Попытка не будить спящие диски... */
		for (size_t i = 0; i < Mask.size(); ++i)
		{
			if (!Mask[i])   //нету диска
				continue;

			DiskMenuItem NewItem;

			const wchar_t Drv[] = { static_cast<wchar_t>(L'A' + i), L':', L'\0' };
			const string LocalName = Drv;
			const auto strRootDir = LocalName + L"\\";
			NewItem.Letter = L"&" + LocalName;

			NewItem.DriveType = FAR_GetDriveType(strRootDir, Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if (NetworkMask[i])
			{
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;
			}
			else if (GetSubstName(NewItem.DriveType, LocalName, NewItem.Path))
			{
				NewItem.DriveType = DRIVE_SUBSTITUTE;
			}
			else if (DriveCanBeVirtual(NewItem.DriveType) && GetVHDInfo(LocalName, NewItem.Path))
			{
				NewItem.DriveType = DRIVE_VIRTUAL;
			}

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_TYPE | DRIVE_SHOW_NETNAME))
			{
				static constexpr std::pair<int, LNGID> DrTMsg[] =
				{
					{ DRIVE_REMOVABLE, MChangeDriveRemovable },
					{ DRIVE_FIXED, MChangeDriveFixed },
					{ DRIVE_REMOTE, MChangeDriveNetwork },
					{ DRIVE_REMOTE_NOT_CONNECTED, MChangeDriveDisconnectedNetwork },
					{ DRIVE_CDROM, MChangeDriveCDROM },
					{ DRIVE_CD_RW, MChangeDriveCD_RW },
					{ DRIVE_CD_RWDVD, MChangeDriveCD_RWDVD },
					{ DRIVE_DVD_ROM, MChangeDriveDVD_ROM },
					{ DRIVE_DVD_RW, MChangeDriveDVD_RW },
					{ DRIVE_DVD_RAM, MChangeDriveDVD_RAM },
					{ DRIVE_BD_ROM, MChangeDriveBD_ROM },
					{ DRIVE_BD_RW, MChangeDriveBD_RW },
					{ DRIVE_HDDVD_ROM, MChangeDriveHDDVD_ROM },
					{ DRIVE_HDDVD_RW, MChangeDriveHDDVD_RW },
					{ DRIVE_RAMDISK, MChangeDriveRAM },
					{ DRIVE_SUBSTITUTE, MChangeDriveSUBST },
					{ DRIVE_VIRTUAL, MChangeDriveVirtual },
					{ DRIVE_USBDRIVE, MChangeDriveRemovable },
				};

				const auto ItemIterator = std::find_if(CONST_RANGE(DrTMsg, i) { return i.first == NewItem.DriveType; });
				if (ItemIterator != std::cend(DrTMsg))
					NewItem.Type = MSG(ItemIterator->second);
			}

			int ShowDisk = (NewItem.DriveType != DRIVE_REMOVABLE || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
				(!IsDriveTypeCDROM(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
				(!IsDriveTypeRemote(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_LABEL | DRIVE_SHOW_FILESYSTEM))
			{
				bool Absent = false;
				if (ShowDisk && !os::GetVolumeInformation(strRootDir, &NewItem.Label, nullptr, nullptr, nullptr, &NewItem.Fs))
				{
					Absent = true;
					ShowDisk = FALSE;
				}

				if (NewItem.Label.empty())
				{
					static const HKEY Roots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
					std::any_of(CONST_RANGE(Roots, i)
					{
						return os::reg::GetValue(i, string(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DriveIcons\\") + NewItem.Letter[1] + L"\\DefaultLabel", L"", NewItem.Label);
					});
				}

				if (Absent && NewItem.Label.empty())
				{
					NewItem.Label = MSG(MChangeDriveLabelAbsent);
				}
			}

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT))
			{
				unsigned long long TotalSize = 0, UserFree = 0;

				if (ShowDisk && os::GetDiskSize(strRootDir, &TotalSize, nullptr, &UserFree))
				{
					if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						//размер как минимум в мегабайтах
						NewItem.TotalSize = FileSizeToStr(TotalSize, 9, COLUMN_COMMAS | COLUMN_MINSIZEINDEX | 1);
						NewItem.FreeSize = FileSizeToStr(UserFree, 9, COLUMN_COMMAS | COLUMN_MINSIZEINDEX | 1);
					}
					else
					{
						//размер с точкой и для 0 добавляем букву размера (B)
						NewItem.TotalSize = FileSizeToStr(TotalSize, 9, COLUMN_FLOATSIZE | COLUMN_SHOWBYTESINDEX);
						NewItem.FreeSize = FileSizeToStr(UserFree, 9, COLUMN_FLOATSIZE | COLUMN_SHOWBYTESINDEX);
					}
					RemoveExternalSpaces(NewItem.TotalSize);
					RemoveExternalSpaces(NewItem.FreeSize);
				}
			}

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_NETNAME)
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
			const auto DiskNumber = os::get_drive_number(i.Letter[1]);
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
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_NETNAME && PathWidth)
			{
				append(ItemName, Separator.Get(), i.Path);
			}

			PanelMenuItem item;
			item.bIsPlugin = false;
			item.cDrive = L'A' + DiskNumber;
			item.nDriveType = i.DriveType;

			ChDiskItem.strName = ItemName;
			ChDiskItem.UserData = item;
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
			const auto item = ChDisk->GetUserDataPtr<PanelMenuItem>();

			int KeyProcessed = 1;

			switch (Key)
			{
				// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
			case KEY_SHIFTNUMENTER:
			case KEY_SHIFTENTER:
			{
				if (item && !item->bIsPlugin)
				{
					OpenFolderInShell({ item->cDrive, L':', L'\\' });
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
						string DeviceName{ item->cDrive, L':', L'\\' };
						ShellSetFileAttributes(nullptr, &DeviceName);
					}
					else
					{
						string strName = ChDisk->at(SelPos).strName.substr(3);
						RemoveExternalSpaces(strName);
						if (Global->CtrlObject->Plugins->SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::hotkey_type::drive_menu, strName))
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
					const wchar_t DeviceName[] = { item->cDrive, L':', L'\\', 0 };
					struct DiskMenuParam { const wchar_t* CmdLine; BOOL Apps; } p = { DeviceName, Key != KEY_MSRCLICK };
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
				Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_NETNAME;
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
						item->pPlugin->GetModuleName().data(),
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
			const wchar_t RootDir[] = { CurDir[0], L':', L'\\', L'\0' };

			if (FAR_GetDriveType(RootDir) == DRIVE_NO_ROOT_DIR)
				return ChDisk->GetSelectPos();
		}

		if (ChDisk->GetExitCode()<0)
			return -1;

		mitem = ChDisk->GetUserDataPtr<PanelMenuItem>();

		if (mitem)
		{
			Item = *mitem;
			mitem = &Item;
		}
	} // эта скобка надо, см. M#605

	if (Global->Opt->CloseCDGate && mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		if (!os::IsDiskInDrive({ mitem->cDrive, L':' }))
		{
			if (!EjectVolume(mitem->cDrive, EJECT_READY | EJECT_NO_MESSAGE))
			{
				SCOPED_ACTION(SaveScreen);
				Message(0, 0, L"", MSG(MChangeWaitingLoadDisk));
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
			wchar_t NewDir[] = { mitem->cDrive, L':', 0, 0 };

			if (FarChDir(NewDir))
			{
				break;
			}
			else
			{
				NewDir[2] = L'\\';

				if (FarChDir(NewDir))
				{
					break;
				}
			}
			Global->CatchError();

			DialogBuilder Builder(MError, nullptr);

			Builder.AddTextWrap(GetErrorString().data(), true);
			Builder.AddText(L"");

			const wchar_t Drive[] = { mitem->cDrive, L'\0' };
			string DriveLetter = Drive;
			DialogItemEx *DriveLetterEdit = Builder.AddFixEditField(DriveLetter, 1);
			Builder.AddTextBefore(DriveLetterEdit, MChangeDriveCannotReadDisk);
			Builder.AddTextAfter(DriveLetterEdit, L":", 0);

			Builder.AddOKCancel(MRetry, MCancel);
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);
			Builder.SetId(ChangeDriveCannotReadDiskErrorId);

			if (Builder.ShowDialog())
			{
				mitem->cDrive = Upper(DriveLetter[0]);
			}
			else
			{
				return -1;
			}
		}

		const auto strNewCurDir = os::GetCurrentDirectory();

		if ((Owner->GetMode() == panel_mode::NORMAL_PANEL) &&
			(Owner->GetType() == panel_type::FILE_PANEL) &&
			!StrCmpI(Owner->GetCurDir(), strNewCurDir) &&
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
		const auto hPlugin = Global->CtrlObject->Plugins->Open(
			mitem->pPlugin,
			Owner->Parent()->IsLeft(Owner)? OPEN_LEFTDISKMENU : OPEN_RIGHTDISKMENU,
			mitem->Guid,
			0);

		if (hPlugin)
		{
			const auto IsActive = Owner->IsFocused();
			const auto NewPanel = Owner->Parent()->ChangePanel(Owner, panel_type::FILE_PANEL, TRUE, TRUE);
			NewPanel->SetPluginMode(hPlugin, L"", IsActive || !NewPanel->Parent()->GetAnotherPanel(NewPanel)->IsVisible());
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
	if (!CurDir.empty() && CurDir[1] == L':' && os::is_standard_drive_letter(CurDir[0]))
	{
		Pos = os::get_drive_number(CurDir[0]);
	}

	while (Pos != -1)
	{
		Pos = ChangeDiskMenu(Owner, Pos, FirstCall);
		FirstCall = false;
	}
}

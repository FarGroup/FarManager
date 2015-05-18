/*
panel.cpp

Parent class для панелей
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

#include "panel.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "vmenu2.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "edit.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "lockscrn.hpp"
#include "help.hpp"
#include "syslog.hpp"
#include "plugapi.hpp"
#include "network.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "hotplug.hpp"
#include "eject.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "execute.hpp"
#include "shortcuts.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "constitle.hpp"
#include "FarDlgBuilder.hpp"
#include "setattr.hpp"
#include "colormix.hpp"
#include "FarGuid.hpp"
#include "elevation.hpp"
#include "stddlg.hpp"
#include "language.hpp"
#include "plugins.hpp"
#include "notification.hpp"
#include "DlgGuid.hpp"
#include "keybar.hpp"
#include "menubar.hpp"
#include "strmix.hpp"
#include "drivemix.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=nullptr;

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile);

/* $ 21.08.2002 IS
   Класс для хранения пункта плагина в меню выбора дисков
*/

class ChDiskPluginItem:noncopyable
{
public:
	ChDiskPluginItem():
		HotKey()
	{}

	ChDiskPluginItem(ChDiskPluginItem&& rhs) noexcept:
		HotKey()
	{
		*this = std::move(rhs);
	}

	MOVE_OPERATOR_BY_SWAP(ChDiskPluginItem);

	void swap(ChDiskPluginItem& rhs) noexcept
	{
		using std::swap;
		Item.swap(rhs.Item);
		swap(HotKey, rhs.HotKey);
	}

	FREE_SWAP(ChDiskPluginItem);

	bool operator ==(const ChDiskPluginItem& rhs) const
	{
		return HotKey==rhs.HotKey && !StrCmpI(Item.strName, rhs.Item.strName) && Item.UserData==rhs.Item.UserData;
	}

	bool operator <(const ChDiskPluginItem& rhs) const
	{
		return (Global->Opt->ChangeDriveMode&DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey!=rhs.HotKey)?
			HotKey < rhs.HotKey :
			StrCmpI(Item.strName, rhs.Item.strName) < 0;
	}

	MenuItemEx& getItem() { return Item; }
	WCHAR& getHotKey() { return HotKey; }

private:
	MenuItemEx Item;
	WCHAR HotKey;
};

Panel::Panel(window_ptr Owner):
	ScreenObject(Owner),
	ProcessingPluginCommand(0),
	m_Focus(false),
	m_Type(0),
	m_EnableUpdate(TRUE),
	m_PanelMode(NORMAL_PANEL),
	m_SortMode(UNSORTED),
	m_ReverseSortOrder(false),
	m_SortGroups(0),
	m_PrevViewMode(VIEW_3),
	m_ViewMode(0),
	m_CurTopFile(0),
	m_CurFile(0),
	m_ShowShortNames(0),
	m_NumericSort(0),
	m_CaseSensitiveSort(0),
	m_DirectoriesFirst(1),
	m_ModalMode(0),
	m_PluginCommand(0)
{
	_OT(SysLog(L"[%p] Panel::Panel()", this));
	SrcDragPanel=nullptr;
	DragX=DragY=-1;
};


Panel::~Panel()
{
	_OT(SysLog(L"[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	m_PrevViewMode=ViewMode;
	m_ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	string strNewDir;
	os::GetCurrentDirectory(strNewDir);
	SetCurDir(strNewDir,true);
}


void Panel::ChangeDisk()
{
	int Pos=0,FirstCall=TRUE;

	if (!m_CurDir.empty() && m_CurDir[1]==L':')
	{
		Pos=std::max(0, ToUpper(m_CurDir[0])-L'A');
	}

	while (Pos!=-1)
	{
		Pos=ChangeDiskMenu(Pos,FirstCall);
		FirstCall=FALSE;
	}
}

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
	bool ItemPresent,Done=false;
	string strMenuText;
	string strPluginText;
	size_t PluginMenuItemsCount = 0;

	FOR(const auto& i, *Global->CtrlObject->Plugins)
	{
		if(Done)
			break;
		for (PluginItem=0;; ++PluginItem)
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
				Done=true;
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
					OneItem.getItem().Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
				OneItem.getItem().strName = strMenuText;
				OneItem.getHotKey()=HotKey;

				PanelMenuItem *item = new PanelMenuItem;
				item->bIsPlugin = true;
				item->pPlugin = pPlugin;
				item->Guid = guid;
				OneItem.getItem().UserData=item;
				OneItem.getItem().UserDataSize=sizeof(*item);

				MPItems.emplace_back(std::move(OneItem));
			}
		}
	}

	MPItems.sort();
	MPItems.unique(); // выкинем дубли
	PluginMenuItemsCount=MPItems.size();

	if (PluginMenuItemsCount)
	{
		MenuItemEx ChDiskItem;
		ChDiskItem.Flags|=LIF_SEPARATOR;
		ChDisk.AddItem(ChDiskItem);

		for_each_cnt(RANGE(MPItems, i, size_t index)
		{
			if (Pos > DiskCount && !SetSelected)
			{
				i.getItem().SetSelect(DiskCount + static_cast<int>(index) + 1 == Pos);

				if (!SetSelected)
					SetSelected = DiskCount + static_cast<int>(index) + 1 == Pos;
			}
			wchar_t HotKey = i.getHotKey();
			const wchar_t HotKeyStr[]={HotKey? L'&' : L' ', HotKey? HotKey : L' ', L' ', HotKey? L' ' : L'\0', L'\0'};
			i.getItem().strName = string(HotKeyStr) + i.getItem().strName;
			ChDisk.AddItem(i.getItem());

			delete(PanelMenuItem*)i.getItem().UserData;  //ммда...
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
	const string Get()
	{
		wchar_t c = m_value;
		m_value = BoxSymbols[BS_V1];
		const wchar_t value[] = {L' ', c, L' ', 0};
		return value[1] == L' '? L" " : value;
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

int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
	class Guard_Macro_DskShowPosType  //фигня какая-то
	{
	public:
		Guard_Macro_DskShowPosType(Panel *curPanel) { Global->Macro_DskShowPosType = (curPanel == curPanel->Parent()->LeftPanel) ? 1 : 2; }
		~Guard_Macro_DskShowPosType() {Global->Macro_DskShowPosType=0;}
	};
	SCOPED_ACTION(Guard_Macro_DskShowPosType)(this);

	auto Mask = FarGetLogicalDrives();
	const auto NetworkMask = AddSavedNetworkDisks(Mask);
	const auto DiskCount = Mask.count();

	PanelMenuItem Item, *mitem=0;
	{ // эта скобка надо, см. M#605
		auto ChDisk = VMenu2::create(MSG(MChangeDriveTitle), nullptr, 0, ScrY - m_Y1 - 3);
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

			wchar_t Drv[]={L'&',static_cast<wchar_t>(L'A'+i),L':',L'\\',L'\0'};
			string strRootDir=Drv+1;
			Drv[3] = 0;
			NewItem.Letter = Drv;
			NewItem.DriveType = FAR_GetDriveType(strRootDir, Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if (NetworkMask[i])
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_TYPE|DRIVE_SHOW_NETNAME))
			{
				string LocalName(L"?:");
				LocalName[0] = strRootDir[0];

				if (GetSubstName(NewItem.DriveType, LocalName, NewItem.Path))
				{
					NewItem.DriveType=DRIVE_SUBSTITUTE;
				}
				else if(DriveCanBeVirtual(NewItem.DriveType) && GetVHDInfo(LocalName, NewItem.Path))
				{
					NewItem.DriveType=DRIVE_VIRTUAL;
				}

				static const simple_pair<int, LNGID> DrTMsg[]=
				{
					{DRIVE_REMOVABLE,MChangeDriveRemovable},
					{DRIVE_FIXED,MChangeDriveFixed},
					{DRIVE_REMOTE,MChangeDriveNetwork},
					{DRIVE_REMOTE_NOT_CONNECTED,MChangeDriveDisconnectedNetwork},
					{DRIVE_CDROM,MChangeDriveCDROM},
					{DRIVE_CD_RW,MChangeDriveCD_RW},
					{DRIVE_CD_RWDVD,MChangeDriveCD_RWDVD},
					{DRIVE_DVD_ROM,MChangeDriveDVD_ROM},
					{DRIVE_DVD_RW,MChangeDriveDVD_RW},
					{DRIVE_DVD_RAM,MChangeDriveDVD_RAM},
					{DRIVE_BD_ROM,MChangeDriveBD_ROM},
					{DRIVE_BD_RW,MChangeDriveBD_RW},
					{DRIVE_HDDVD_ROM,MChangeDriveHDDVD_ROM},
					{DRIVE_HDDVD_RW,MChangeDriveHDDVD_RW},
					{DRIVE_RAMDISK,MChangeDriveRAM},
					{DRIVE_SUBSTITUTE,MChangeDriveSUBST},
					{DRIVE_VIRTUAL,MChangeDriveVirtual},
					{DRIVE_USBDRIVE,MChangeDriveRemovable},
				};

				auto ItemIterator = std::find_if(CONST_RANGE(DrTMsg, i) {return i.first == NewItem.DriveType;});
				if (ItemIterator != std::cend(DrTMsg))
					NewItem.Type = MSG(ItemIterator->second);
			}

			int ShowDisk = (NewItem.DriveType!=DRIVE_REMOVABLE || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
			               (!IsDriveTypeCDROM(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
			               (!IsDriveTypeRemote(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
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

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				unsigned __int64 TotalSize = 0, UserFree = 0;

				if (ShowDisk && os::GetDiskSize(strRootDir,&TotalSize, nullptr, &UserFree))
				{
					if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						//размер как минимум в мегабайтах
						FileSizeToStr(NewItem.TotalSize,TotalSize,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
						FileSizeToStr(NewItem.FreeSize,UserFree,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
					}
					else
					{
						//размер с точкой и для 0 добавляем букву размера (B)
						FileSizeToStr(NewItem.TotalSize,TotalSize,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
						FileSizeToStr(NewItem.FreeSize,UserFree,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
					}
					RemoveExternalSpaces(NewItem.TotalSize);
					RemoveExternalSpaces(NewItem.FreeSize);
				}
			}

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_NETNAME)
			{
				switch(NewItem.DriveType)
				{
				case DRIVE_REMOTE:
				case DRIVE_REMOTE_NOT_CONNECTED:
					DriveLocalToRemoteName(NewItem.DriveType,strRootDir[0],NewItem.Path);
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

		bool SetSelected=false;
		std::for_each(CONST_RANGE(Items, i)
		{
			MenuItemEx ChDiskItem;
			int DiskNumber = i.Letter[1] - L'A';
			if (FirstCall)
			{
				ChDiskItem.SetSelect(DiskNumber==Pos);

				if (!SetSelected)
					SetSelected=(DiskNumber==Pos);
			}
			else
			{
				if (Pos < static_cast<int>(DiskCount))
				{
					ChDiskItem.SetSelect(MenuLine==Pos);

					if (!SetSelected)
						SetSelected=(MenuLine==Pos);
				}
			}
			FormatString ItemName;
			ItemName << i.Letter;

			separator Separator;

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_TYPE)
			{
				ItemName << Separator.Get() << fmt::LeftAlign() << fmt::ExactWidth(TypeWidth) << i.Type;
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_LABEL)
			{
				ItemName << Separator.Get() << fmt::LeftAlign() << fmt::ExactWidth(LabelWidth) << i.Label;
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
			{
				ItemName << Separator.Get() << fmt::LeftAlign() << fmt::ExactWidth(FsWidth) << i.Fs;
			}
			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				ItemName << Separator.Get() << fmt::ExactWidth(TotalSizeWidth) << i.TotalSize;
				ItemName << Separator.Get() << fmt::ExactWidth(FreeSizeWidth) << i.FreeSize;
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_NETNAME && PathWidth)
			{
				ItemName << Separator.Get() << i.Path;
			}

			ChDiskItem.strName = ItemName;
			PanelMenuItem item;
			item.bIsPlugin = false;
			item.cDrive = L'A' + DiskNumber;
			item.nDriveType = i.DriveType;
			ChDisk->SetUserData(&item, sizeof(item), ChDisk->AddItem(ChDiskItem));
			MenuLine++;
		});

		size_t PluginMenuItemsCount=0;

		if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(*ChDisk, Pos, static_cast<int>(DiskCount), SetSelected);
		}

		DE.reset();

		int X=m_X1+5;

		if ((this == Parent()->RightPanel) && IsFullScreen() && (m_X2 - m_X1 > 40))
			X = (m_X2-m_X1+1)/2+5;

		ChDisk->SetPosition(X,-1,0,0);

		int Y = (ScrY+1-static_cast<int>((DiskCount+PluginMenuItemsCount)+5))/2;
		if (Y < 3)
			ChDisk->SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk->SetMacroMode(MACROAREA_DISKS);
		int RetCode=-1;

		bool NeedRefresh = false;

		SCOPED_ACTION(listener)(update_devices, [&NeedRefresh] { NeedRefresh = true; });

		ChDisk->Run([&](const Manager::Key& RawKey)->int
		{
			auto Key=RawKey.FarKey();
			if(Key==KEY_NONE && NeedRefresh)
			{
				Key=KEY_CTRLR;
				NeedRefresh = false;
			}

			int SelPos=ChDisk->GetSelectPos();
			PanelMenuItem *item = (PanelMenuItem*)ChDisk->GetUserData(nullptr,0);

			int KeyProcessed = 1;

			switch (Key)
			{
				// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				{
					if (item && !item->bIsPlugin)
					{
						string DosDeviceName(L"?:\\");
						DosDeviceName[0] = item->cDrive;
						Execute(DosDeviceName, false, true, true, true);
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
							RetCode=SelPos;
						}
					}
				}
				break;
				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (item && !item->bIsPlugin)
					{
						int Code = DisconnectDrive(item, *ChDisk);
						if (Code != DRIVE_DEL_FAIL && Code != DRIVE_DEL_NONE)
						{
							Global->ScrBuf->Lock(); // отменяем всякую прорисовку
							Global->WindowManager->ResizeAllWindows();
							Global->WindowManager->PluginCommit(); // коммитим.
							Global->ScrBuf->Unlock(); // разрешаем прорисовку
							RetCode=(((DiskCount-SelPos)==1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos-1:SelPos;
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
							string DeviceName(L"?:\\");
							DeviceName[0] = item->cDrive;
							ShellSetFileAttributes(nullptr, &DeviceName);
						}
						else
						{
							string strName = ChDisk->GetItemPtr(SelPos)->strName.data() + 3;
							RemoveExternalSpaces(strName);
							if(Global->CtrlObject->Plugins->SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::DRIVE_MENU, strName))
							RetCode=SelPos;
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
						const wchar_t DeviceName[] = {item->cDrive, L':', L'\\', 0};
						struct DiskMenuParam {const wchar_t* CmdLine; BOOL Apps;} p = {DeviceName, Key!=KEY_MSRCLICK};
						Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, (Parent()->LeftPanel == this) ? OPEN_LEFTDISKMENU : OPEN_RIGHTDISKMENU, &p); // EMenu Plugin :-)
					}
					break;
				}

				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDECIMAL:
				case KEY_SHIFTDEL:
				{
					if (item && !item->bIsPlugin)
					{
						RemoveHotplugDevice(item, *ChDisk);
						RetCode=SelPos;
					}
				}
				break;
				case KEY_CTRL1:
				case KEY_RCTRL1:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_TYPE;
					RetCode=SelPos;
					break;
				case KEY_CTRL2:
				case KEY_RCTRL2:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_NETNAME;
					RetCode=SelPos;
					break;
				case KEY_CTRL3:
				case KEY_RCTRL3:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_LABEL;
					RetCode=SelPos;
					break;
				case KEY_CTRL4:
				case KEY_RCTRL4:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_FILESYSTEM;
					RetCode=SelPos;
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

					RetCode=SelPos;
					break;
				}
				case KEY_CTRL6:
				case KEY_RCTRL6:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_REMOVABLE;
					RetCode=SelPos;
					break;
				case KEY_CTRL7:
				case KEY_RCTRL7:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_PLUGINS;
					RetCode=SelPos;
					break;
				case KEY_CTRL8:
				case KEY_RCTRL8:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_CDROM;
					RetCode=SelPos;
					break;
				case KEY_CTRL9:
				case KEY_RCTRL9:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_REMOTE;
					RetCode=SelPos;
					break;
				case KEY_F9:
					ConfigureChangeDriveMode();
					RetCode=SelPos;
					break;
				case KEY_SHIFTF1:
				{
					if (item && item->bIsPlugin)
					{
						// Вызываем нужный топик, который передали в CommandsMenu()
						pluginapi::apiShowHelp(
						    item->pPlugin->GetModuleName().data(),
						    nullptr,
						    FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS
						);
					}

					break;
				}
				case KEY_ALTSHIFTF9:
				case KEY_RALTSHIFTF9:

					if (Global->Opt->ChangeDriveMode&DRIVE_SHOW_PLUGINS)
						Global->CtrlObject->Plugins->Configure();

					RetCode=SelPos;
					break;
				case KEY_SHIFTF9:

					if (item && item->bIsPlugin && item->pPlugin->has<iConfigure>())
						Global->CtrlObject->Plugins->ConfigureCurrent(item->pPlugin, item->Guid);

					RetCode=SelPos;
					break;
				case KEY_CTRLR:
				case KEY_RCTRLR:
					RetCode=SelPos;
					break;

				 default:
				 	KeyProcessed = 0;
			}

			if (RetCode>=0)
				ChDisk->Close(-1);

			return KeyProcessed;
		});

		if (RetCode>=0)
			return RetCode;

		if (ChDisk->GetExitCode()<0 &&
			        !m_CurDir.empty() &&
			        (StrCmpN(m_CurDir.data(),L"\\\\",2) ))
			{
				const wchar_t RootDir[4] = {m_CurDir[0],L':',L'\\',L'\0'};

				if (FAR_GetDriveType(RootDir) == DRIVE_NO_ROOT_DIR)
				return ChDisk->GetSelectPos();
			}

		if (ChDisk->GetExitCode()<0)
			return -1;

		mitem=(PanelMenuItem*)ChDisk->GetUserData(nullptr,0);

		if (mitem)
		{
			Item=*mitem;
			mitem=&Item;
		}
	} // эта скобка надо, см. M#605

	if (Global->Opt->CloseCDGate && mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		string RootDir(L"?:");
		RootDir[0] = mitem->cDrive;

		if (!os::IsDiskInDrive(RootDir))
		{
			if (!EjectVolume(mitem->cDrive, EJECT_READY|EJECT_NO_MESSAGE))
			{
				SCOPED_ACTION(SaveScreen);
				Message(0,0,L"",MSG(MChangeWaitingLoadDisk));
				EjectVolume(mitem->cDrive, EJECT_LOAD_MEDIA|EJECT_NO_MESSAGE);
			}
		}
	}

	if (ProcessPluginEvent(FE_CLOSE,nullptr))
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
			wchar_t NewDir[]={mitem->cDrive,L':',0,0};

			// In general, mitem->cDrive can contain any unicode character
			if (mitem->cDrive >= L'A' && mitem->cDrive <= L'Z' && NetworkMask[mitem->cDrive-L'A'])
			{
				ConnectToNetworkDrive(NewDir);
			}

			if (FarChDir(NewDir))
			{
				break;
			}
			else
			{
				NewDir[2]=L'\\';

				if (FarChDir(NewDir))
				{
					break;
				}
			}
			Global->CatchError();

			DialogBuilder Builder(MError, nullptr);

			Builder.AddTextWrap(GetErrorString().data(), true);
			Builder.AddText(L"");

			const wchar_t Drive[] = {mitem->cDrive,L'\0'};
			string DriveLetter = Drive;
			DialogItemEx *DriveLetterEdit = Builder.AddFixEditField(DriveLetter, 1);
			Builder.AddTextBefore(DriveLetterEdit, MChangeDriveCannotReadDisk);
			Builder.AddTextAfter(DriveLetterEdit, L":", 0);

			Builder.AddOKCancel(MRetry, MCancel);
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);

			if (Builder.ShowDialog())
			{
				mitem->cDrive = ToUpper(DriveLetter[0]);
			}
			else
			{
				return -1;
			}
		}

		string strNewCurDir;
		os::GetCurrentDirectory(strNewCurDir);

		if ((m_PanelMode == NORMAL_PANEL) &&
		        (GetType() == FILE_PANEL) &&
		        !StrCmpI(m_CurDir, strNewCurDir) &&
		        IsVisible())
		{
			// А нужно ли делать здесь Update????
			Update(UPDATE_KEEP_SELECTION);
		}
		else
		{
			int Focus=GetFocus();
			auto NewPanel = Parent()->ChangePanel(this, FILE_PANEL, TRUE, FALSE);
			NewPanel->SetCurDir(strNewCurDir,true);
			NewPanel->Show();

			if (Focus || !NewPanel->Parent()->GetAnotherPanel(NewPanel)->IsVisible())
				NewPanel->SetFocus();

			if (!Focus && NewPanel->Parent()->GetAnotherPanel(NewPanel)->GetType() == INFO_PANEL)
				NewPanel->Parent()->GetAnotherPanel(NewPanel)->UpdateKeyBar();
		}
	}
	else //эта плагин, да
	{
		auto hPlugin = Global->CtrlObject->Plugins->Open(
		                     mitem->pPlugin,
		                     (Parent()->LeftPanel == this)?OPEN_LEFTDISKMENU:OPEN_RIGHTDISKMENU,
		                     mitem->Guid,
		                     0
		                 );

		if (hPlugin)
		{
			int Focus=GetFocus();
			auto NewPanel = Parent()->ChangePanel(this, FILE_PANEL, TRUE, TRUE);
			NewPanel->SetPluginMode(hPlugin, L"", Focus || !NewPanel->Parent()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!Focus && NewPanel->Parent()->GetAnotherPanel(NewPanel)->GetType() == INFO_PANEL)
				NewPanel->Parent()->GetAnotherPanel(NewPanel)->UpdateKeyBar();
		}
	}

	return -1;
}

int Panel::DisconnectDrive(const PanelMenuItem *item, VMenu2 &ChDisk)
{
	if ((item->nDriveType == DRIVE_REMOVABLE) || IsDriveTypeCDROM(item->nDriveType))
	{
		if ((item->nDriveType == DRIVE_REMOVABLE) && !IsEjectableMedia(item->cDrive))
			return -1;

		// первая попытка извлечь диск

		if (!EjectVolume(item->cDrive, EJECT_NO_MESSAGE))
		{
			// запоминаем состояние панелей
			int CMode=GetMode();
			int AMode = Parent()->GetAnotherPanel(this)->GetMode();
			string strTmpCDir(GetCurDir()), strTmpADir(Parent()->GetAnotherPanel(this)->GetCurDir());
			// "цикл до умопомрачения"
			int DoneEject=FALSE;

			while (!DoneEject)
			{
				// "освободим диск" - перейдем при необходимости в домашний каталог
				// TODO: А если домашний каталог - CD? ;-)
				IfGoHome(item->cDrive);
				// очередная попытка извлечения без вывода сообщения
				int ResEject = EjectVolume(item->cDrive, EJECT_NO_MESSAGE);

				if (!ResEject)
				{
					// восстановим пути - это избавит нас от левых данных в панели.
					if (AMode != PLUGIN_PANEL)
						Parent()->GetAnotherPanel(this)->SetCurDir(strTmpADir, false);

					if (CMode != PLUGIN_PANEL)
						SetCurDir(strTmpCDir, false);

					// ... и выведем месаг о...
					SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
					Global->CatchError();
					wchar_t Drive[] = {item->cDrive, L':', L'\\', 0};
					DoneEject = OperationFailed(Drive, MError, LangString(MChangeCouldNotEjectMedia) << item->cDrive, false);
				}
				else
					DoneEject=TRUE;
			}
		}
		return DRIVE_DEL_NONE;
	}
	else
	{
		return ProcessDelDisk(item->cDrive, item->nDriveType, &ChDisk);
	}
}

void Panel::RemoveHotplugDevice(const PanelMenuItem *item, VMenu2 &ChDisk)
{
	int Code = RemoveHotplugDisk(item->cDrive, EJECT_NOTIFY_AFTERREMOVE);

	if (!Code)
	{
		// запоминаем состояние панелей
		int CMode=GetMode();
		int AMode = Parent()->GetAnotherPanel(this)->GetMode();
		string strTmpCDir(GetCurDir()), strTmpADir(Parent()->GetAnotherPanel(this)->GetCurDir());
		// "цикл до умопомрачения"
		int DoneEject=FALSE;

		while (!DoneEject)
		{
			// "освободим диск" - перейдем при необходимости в домашний каталог
			// TODO: А если домашний каталог - USB? ;-)
			IfGoHome(item->cDrive);
			// очередная попытка извлечения без вывода сообщения
			Code = RemoveHotplugDisk(item->cDrive, EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);

			if (!Code)
			{
				// восстановим пути - это избавит нас от левых данных в панели.
				if (AMode != PLUGIN_PANEL)
					Parent()->GetAnotherPanel(this)->SetCurDir(strTmpADir, false);

				if (CMode != PLUGIN_PANEL)
					SetCurDir(strTmpCDir, false);

				// ... и выведем месаг о...
				SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
				Global->CatchError();
				DoneEject = Message(MSG_WARNING|MSG_ERRORTYPE, 2,
				                MSG(MError),
				                (LangString(MChangeCouldNotEjectHotPlugMedia) << item->cDrive).data(),
				                MSG(MHRetry), MSG(MHCancel));
			}
			else
				DoneEject=TRUE;
		}
	}
}

/* $ 28.12.2001 DJ
   обработка Del в меню дисков
*/

int Panel::ProcessDelDisk(wchar_t Drive, int DriveType,VMenu2 *ChDiskMenu)
{
	string DiskLetter(L"?:");
	DiskLetter[0] = Drive;

	switch(DriveType)
	{
	case DRIVE_SUBSTITUTE:
		{
			if (Global->Opt->Confirm.RemoveSUBST)
			{
				LangString Question(MChangeSUBSTDisconnectDriveQuestion);
				Question << DiskLetter;
				if (Message(MSG_WARNING, MSG(MChangeSUBSTDisconnectDriveTitle),
					make_vector<string>(Question.data()),
					make_vector<string>(MSG(MYes), MSG(MNo)),
					nullptr, nullptr, &SUBSTDisconnectDriveId))
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
				LangString strMsgText(MChangeDriveCannotDelSubst);
				strMsgText << DiskLetter;
				if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
				{
					if (!Message(MSG_WARNING|MSG_ERRORTYPE, 2,
						MSG(MError),
						strMsgText.data(),
						L"\x1",
						MSG(MChangeDriveOpenFiles),
						MSG(MChangeDriveAskDisconnect),
						MSG(MOk),MSG(MCancel)))
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
				Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText.data(),MSG(MOk));
			}
			return DRIVE_DEL_FAIL; // блин. в прошлый раз забыл про это дело...
		}
		break;

	case DRIVE_REMOTE:
	case DRIVE_REMOTE_NOT_CONNECTED:
		{
			int UpdateProfile=CONNECT_UPDATE_PROFILE;
			if (MessageRemoveConnection(Drive,UpdateProfile))
			{
				// <КОСТЫЛЬ>
				SCOPED_ACTION(LockScreen);
				// если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
				// удалению
				IfGoHome(Drive);
				Global->WindowManager->ResizeAllWindows();
				Global->WindowManager->GetCurrentWindow()->Show();
				// </КОСТЫЛЬ>

				if (WNetCancelConnection2(DiskLetter.data(),UpdateProfile,FALSE)==NO_ERROR)
				{
					return DRIVE_DEL_SUCCESS;
				}
				else
				{
					Global->CatchError();
					LangString strMsgText(MChangeDriveCannotDisconnect);
					strMsgText << DiskLetter;
					DWORD LastError = Global->CaughtError();
					if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
					{
						if (!Message(MSG_WARNING|MSG_ERRORTYPE, 2,
							MSG(MError),
							strMsgText.data(),
							L"\x1",
							MSG(MChangeDriveOpenFiles),
							MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel)))
						{
							if (WNetCancelConnection2(DiskLetter.data(),UpdateProfile,TRUE)==NO_ERROR)
							{
								return DRIVE_DEL_SUCCESS;
							}
						}
						else
						{
							return DRIVE_DEL_FAIL;
						}
					}
					const wchar_t RootDir[]={DiskLetter[0],L':',L'\\',L'\0'};
					if (FAR_GetDriveType(RootDir)==DRIVE_REMOTE)
					{
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText.data(),MSG(MOk));
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
				LangString Question(MChangeVHDDisconnectDriveQuestion);
				Question << DiskLetter;
				if (Message(MSG_WARNING, MSG(MChangeVHDDisconnectDriveTitle),
					make_vector<string>(Question.data()),
					make_vector<string>(MSG(MYes), MSG(MNo)),
					nullptr, nullptr, &VHDDisconnectDriveId))
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
				Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), (LangString(MChangeDriveCannotDetach) << DiskLetter).data(), MSG(MOk));
			}
			return Result;
		}
		break;

	}
	return DRIVE_DEL_FAIL;
}


__int64 Panel::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return 0;
}

// корректировка букв
static DWORD _CorrectFastFindKbdLayout(const INPUT_RECORD& rec,DWORD Key)
{
	if ((Key&(KEY_ALT|KEY_RALT)))// && Key!=(KEY_ALT|0x3C))
	{
		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
		if (rec.Event.KeyEvent.uChar.UnicodeChar && (Key&KEY_MASKF) != rec.Event.KeyEvent.uChar.UnicodeChar) //???
			Key = (Key & 0xFFF10000) | rec.Event.KeyEvent.uChar.UnicodeChar;   //???

		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
	}

	return Key;
}

class Search: public Modal
{
public:
	static search_ptr create(Panel* Owner, int FirstKey);

	void Process(void);
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual int GetType() const override { return windowtype_search; }
	virtual int GetTypeAndName(string &, string &) override { return windowtype_search; }
	virtual void ResizeConsole(void) override;
	int KeyToProcess(void) const { return m_KeyToProcess; }

private:
	Search(Panel* Owner, int FirstKey);
	void InitPositionAndSize(void);
	void init(void);

	Panel* m_Owner;
	int m_FirstKey;
	std::unique_ptr<EditControl> m_FindEdit;
	int m_KeyToProcess;
	virtual void DisplayObject(void) override;
	virtual string GetTitle() const override { return string(); }
	void ProcessName(const string& Src, string &strName);
	void ShowBorder(void);
	void Close(void);
};

Search::Search(Panel* Owner, int FirstKey):
	m_Owner(Owner),
	m_FirstKey(FirstKey),
	m_FindEdit(),
	m_KeyToProcess(0)
{
}

void Search::InitPositionAndSize(void)
{
	int X1, Y1, X2, Y2;
	m_Owner->GetPosition(X1, Y1, X2, Y2);
	int FindX=std::min(X1+9,ScrX-22);
	int FindY=std::min(Y2,ScrY-2);
	SetPosition(FindX,FindY,FindX+21,FindY+2);
	m_FindEdit->SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
}

search_ptr Search::create(Panel* Owner, int FirstKey)
{
	search_ptr SearchPtr(new Search(Owner, FirstKey));
	SearchPtr->init();
	return SearchPtr;
}

void Search::init(void)
{
	SetMacroMode(MACROAREA_SEARCH);
	SetRestoreScreenMode(true);

	m_FindEdit = std::make_unique<EditControl>(shared_from_this(), this);
	m_FindEdit->SetEditBeyondEnd(false);
	m_FindEdit->SetObjectColor(COL_DIALOGEDIT);

	InitPositionAndSize();
}

void Search::Process(void)
{
	Global->WindowManager->ExecuteWindow(shared_from_this());
	if(m_FirstKey) Global->WindowManager->CallbackWindow([this](){this->ProcessKey(Manager::Key(m_FirstKey));});
	Global->WindowManager->ExecuteModal(shared_from_this());
}

int Search::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key.FarKey();
	string strName;

	// для вставки воспользуемся макродвижком...
	if (LocalKey==KEY_CTRLV || LocalKey==KEY_RCTRLV || LocalKey==KEY_SHIFTINS || LocalKey==KEY_SHIFTNUMPAD0)
	{
		string ClipText;
		if (GetClipboard(ClipText))
		{
			if (!ClipText.empty())
			{
				ProcessName(ClipText, strName);
				ShowBorder();
			}
		}

		return TRUE;
	}
	else if (LocalKey == KEY_OP_XLAT)
	{
		string strTempName;
		m_FindEdit->Xlat();
		m_FindEdit->GetString(strTempName);
		m_FindEdit->SetString(L"");
		ProcessName(strTempName, strName);
		Redraw();
		return TRUE;
	}
	else if (LocalKey == KEY_OP_PLAINTEXT)
	{
		string strTempName;
		m_FindEdit->ProcessKey(Manager::Key(LocalKey));
		m_FindEdit->GetString(strTempName);
		m_FindEdit->SetString(L"");
		ProcessName(strTempName, strName);
		Redraw();
		return TRUE;
	}
	else
		LocalKey=_CorrectFastFindKbdLayout(Key.Event(),LocalKey);

	if (LocalKey==KEY_ESC || LocalKey==KEY_F10)
	{
		m_KeyToProcess=KEY_NONE;
		Close();
		return TRUE;
	}

	// // _SVS(if (!FirstKey) SysLog(L"Panel::FastFind  Key=%s  %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(&rec)));
	if (LocalKey>=KEY_ALT_BASE+0x01 && LocalKey<=KEY_ALT_BASE+65535)
		LocalKey=ToLower(static_cast<WCHAR>(LocalKey-KEY_ALT_BASE));
	else if (LocalKey>=KEY_RALT_BASE+0x01 && LocalKey<=KEY_RALT_BASE+65535)
		LocalKey=ToLower(static_cast<WCHAR>(LocalKey-KEY_RALT_BASE));

	if (LocalKey>=KEY_ALTSHIFT_BASE+0x01 && LocalKey<=KEY_ALTSHIFT_BASE+65535)
		LocalKey=ToLower(static_cast<WCHAR>(LocalKey-KEY_ALTSHIFT_BASE));
	else if (LocalKey>=KEY_RALTSHIFT_BASE+0x01 && LocalKey<=KEY_RALTSHIFT_BASE+65535)
		LocalKey=ToLower(static_cast<WCHAR>(LocalKey-KEY_RALTSHIFT_BASE));

	if (LocalKey==KEY_MULTIPLY)
		LocalKey=L'*';

	switch (LocalKey)
	{
		case KEY_F1:
		{
			Hide();
			{
				Help::create(L"FastFind");
			}
			Show();
			break;
		}
		case KEY_CTRLNUMENTER:   case KEY_RCTRLNUMENTER:
		case KEY_CTRLENTER:      case KEY_RCTRLENTER:
			m_FindEdit->GetString(strName);
			m_Owner->FindPartName(strName, TRUE, 1);
			Redraw();
			break;
		case KEY_CTRLSHIFTNUMENTER:  case KEY_RCTRLSHIFTNUMENTER:
		case KEY_CTRLSHIFTENTER:     case KEY_RCTRLSHIFTENTER:
			m_FindEdit->GetString(strName);
			m_Owner->FindPartName(strName, TRUE, -1);
			Redraw();
			break;
		case KEY_NONE:
		case KEY_IDLE:
			break;
		default:

			if ((LocalKey<32 || LocalKey>=65536) && LocalKey!=KEY_BS && LocalKey!=KEY_CTRLY && LocalKey!=KEY_RCTRLY &&
			        LocalKey!=KEY_CTRLBS && LocalKey!=KEY_RCTRLBS && LocalKey!=KEY_ALT && LocalKey!=KEY_SHIFT &&
			        LocalKey!=KEY_CTRL && LocalKey!=KEY_RALT && LocalKey!=KEY_RCTRL &&
			        !(LocalKey==KEY_CTRLINS||LocalKey==KEY_CTRLNUMPAD0) && // KEY_RCTRLINS/NUMPAD0 passed to panels
			        !(LocalKey==KEY_SHIFTINS||LocalKey==KEY_SHIFTNUMPAD0) &&
			        !((LocalKey == KEY_KILLFOCUS || LocalKey == KEY_GOTFOCUS) && IsWindowsVistaOrGreater()) // Mantis #2903
			        )
			{
				m_KeyToProcess=LocalKey;
				Close();
				return TRUE;
			}

			string strLastName;
			m_FindEdit->GetString(strLastName);
			if (m_FindEdit->ProcessKey(Manager::Key(LocalKey)))
			{
				m_FindEdit->GetString(strName);

				// уберем двойные '**'
				if (strName.size() > 1
				        && strName.back() == L'*'
				        && strName[strName.size()-2] == L'*')
				{
					strName.pop_back();
					m_FindEdit->SetString(strName.data());
				}

				/* $ 09.04.2001 SVS
				   проблемы с быстрым поиском.
				   Подробнее в 00573.ChangeDirCrash.txt
				*/
				if (!strName.empty() && strName.front() == L'"')
				{
					strName.erase(0, 1);
					m_FindEdit->SetString(strName.data());
				}

				if (m_Owner->FindPartName(strName, FALSE, 1))
				{
					strLastName = strName;
				}
				else
				{
					if (Global->CtrlObject->Macro.IsExecuting())// && Global->CtrlObject->Macro.GetLevelState() > 0) // если вставка макросом...
					{
						//Global->CtrlObject->Macro.DropProcess(); // ... то дропнем макропроцесс
						//Global->CtrlObject->Macro.PopState();
						;
					}

					m_FindEdit->SetString(strLastName.data());
				}

				Redraw();
			}

			break;
	}
	return TRUE;
}

int Search::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		;
	else
		Close();
	return TRUE;
}

void Search::ShowBorder(void)
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(m_X1+1,m_Y1+1);
	Text(L' ');
	GotoXY(m_X1+20,m_Y1+1);
	Text(L' ');
	Box(m_X1,m_Y1,m_X1+21,m_Y1+2,colors::PaletteColorToFarColor(COL_DIALOGBOX),DOUBLE_BOX);
	GotoXY(m_X1+7,m_Y1);
	SetColor(COL_DIALOGBOXTITLE);
	Text(MSearchFileTitle);
}

void Search::DisplayObject(void)
{
	ShowBorder();
	m_FindEdit->Show();
}

void Search::ProcessName(const string& Src, string &strName)
{
	string Buffer;
	m_FindEdit->GetString(Buffer);
	Buffer = Unquote(Buffer + Src);

	for (; !Buffer.empty() && !m_Owner->FindPartName(Buffer, FALSE, 1); Buffer.pop_back())
		;

	if (!Buffer.empty())
	{
		m_FindEdit->SetString(Buffer.data());
		m_FindEdit->Show();
		strName = Buffer;
	}
}

void Search::ResizeConsole(void)
{
	InitPositionAndSize();
}

void Search::Close(void)
{
	Hide();
	Global->WindowManager->DeleteWindow(shared_from_this());
}

void Panel::FastFind(int FirstKey)
{
	// // _SVS(CleverSysLog Clev(L"Panel::FastFind"));
	int KeyToProcess=0;
	Global->WaitInFastFind++;
	{
		auto search = Search::create(this, FirstKey);
		search->Process();
		KeyToProcess=search->KeyToProcess();
	}
	Global->WaitInFastFind--;
	Show();
	Parent()->GetKeybar().Redraw();
	Global->ScrBuf->Flush();

	auto TreePanel = dynamic_cast<TreeList*>(Parent()->ActivePanel());
	if (TreePanel && (KeyToProcess == KEY_ENTER || KeyToProcess == KEY_NUMENTER))
		TreePanel->ProcessEnter();
	else
		Parent()->ProcessKey(Manager::Key(KeyToProcess));
}

void Panel::SetFocus()
{
	if (Parent()->ActivePanel() != this)
	{
		Parent()->ActivePanel()->KillFocus();
		Parent()->SetActivePanel(this);
	}

	Global->WindowManager->UpdateMacroArea();
	ProcessPluginEvent(FE_GOTFOCUS,nullptr);

	if (!GetFocus())
	{
		Parent()->RedrawKeyBar();
		m_Focus = true;
		Redraw();
		FarChDir(m_CurDir);
	}
}


void Panel::KillFocus()
{
	m_Focus = false;
	ProcessPluginEvent(FE_KILLFOCUS,nullptr);
	Redraw();
}


int  Panel::PanelProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
{
	RetCode=TRUE;

	if (!m_ModalMode && !MouseEvent->dwMousePosition.Y)
	{
		if (MouseEvent->dwMousePosition.X==ScrX)
		{
			if (Global->Opt->ScreenSaver && !(MouseEvent->dwButtonState & 3))
			{
				EndDrag();
				ScreenSaver();
				return TRUE;
			}
		}
		else
		{
			if ((MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
			{
				EndDrag();

				if (!MouseEvent->dwMousePosition.X)
					Parent()->ProcessKey(Manager::Key(KEY_CTRLO));
				else
					Global->Opt->ShellOptions(false,MouseEvent);

				return TRUE;
			}
		}
	}

	if (!IsVisible() ||
	        (MouseEvent->dwMousePosition.X<m_X1 || MouseEvent->dwMousePosition.X>m_X2 ||
	         MouseEvent->dwMousePosition.Y<m_Y1 || MouseEvent->dwMousePosition.Y>m_Y2))
	{
		RetCode=FALSE;
		return TRUE;
	}

	if (DragX!=-1)
	{
		if (!(MouseEvent->dwButtonState & 3))
		{
			EndDrag();

			if (!MouseEvent->dwEventFlags && SrcDragPanel!=this)
			{
				MoveToMouse(MouseEvent);
				Redraw();
				SrcDragPanel->ProcessKey(Manager::Key(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY));
			}

			return TRUE;
		}

		if (MouseEvent->dwMousePosition.Y<=m_Y1 || MouseEvent->dwMousePosition.Y>=m_Y2 ||
			!Parent()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return TRUE;
		}

		if ((MouseEvent->dwButtonState & 2) && !MouseEvent->dwEventFlags)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & 1)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
			        !m_ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && !DragSaveScr)
				{
					SrcDragPanel->GoToFile(strDragName);
					SrcDragPanel->Show();
				}

				DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
				return TRUE;
			}
			else
			{
				delete DragSaveScr;
				DragSaveScr=nullptr;
			}
		}
	}

	if (!(MouseEvent->dwButtonState & 3))
		return TRUE;

	if ((MouseEvent->dwButtonState & 1) && !MouseEvent->dwEventFlags &&
	        m_X2-m_X1<ScrX)
	{
		DWORD FileAttr;
		MoveToMouse(MouseEvent);
		GetSelName(nullptr,FileAttr);

		if (GetSelName(&strDragName,FileAttr) && !TestParentFolderName(strDragName))
		{
			SrcDragPanel=this;
			DragX=MouseEvent->dwMousePosition.X;
			DragY=MouseEvent->dwMousePosition.Y;
			DragMove=IntKeyState.ShiftPressed;
		}
	}

	return FALSE;
}


bool Panel::IsDragging()
{
	return DragSaveScr!=nullptr;
}


void Panel::EndDrag()
{
	delete DragSaveScr;
	DragSaveScr=nullptr;
	DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{

	string strSelName;
	int MsgX,Length;

	const auto SelCount = SrcDragPanel->GetSelCount();

	if (!SelCount)
	{
		return;
	}
	else if (SelCount == 1)
	{
		string strCvtName;
		DWORD FileAttr;
		SrcDragPanel->GetSelName(nullptr,FileAttr);
		SrcDragPanel->GetSelName(&strSelName,FileAttr);
		strCvtName = PointToName(strSelName);
		QuoteSpace(strCvtName);
		strSelName = strCvtName;
	}
	else
	{
		strSelName = LangString(MDragFiles) << SelCount;
	}

	LangString strDragMsg(Move? MDragMove : MDragCopy);
	strDragMsg << strSelName;


	if ((Length=(int)strDragMsg.size())+X>ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			TruncStrFromEnd(strDragMsg,ScrX);
			Length=(int)strDragMsg.size();
		}
	}
	else
		MsgX=X;

	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	delete DragSaveScr;
	DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(strDragMsg);
}


const string& Panel::GetCurDir() const
{
	return m_CurDir;
}


bool Panel::SetCurDir(const string& CurDir,bool ClosePanel,bool /*IsUpdated*/)
{
	InitCurDir(CurDir);
	return true;
}


void Panel::InitCurDir(const string& CurDir)
{
	if (StrCmpI(m_CurDir, CurDir) || !TestCurrentDirectory(CurDir))
	{
		m_CurDir = CurDir;

		if (m_PanelMode!=PLUGIN_PANEL)
		{
			PrepareDiskPath(m_CurDir);
			if(!IsRootPath(m_CurDir))
			{
				DeleteEndSlash(m_CurDir);
			}
		}
	}
}


/* $ 14.06.2001 KM
   + Добавлена установка переменных окружения, определяющих
     текущие директории дисков как для активной, так и для
     пассивной панели. Это необходимо программам запускаемым
     из FAR.
*/
/* $ 05.10.2001 SVS
   ! Давайте для начала выставим нужные значения для пассивной панели,
     а уж потом...
     А то фигня какая-то получается...
*/
/* $ 14.01.2002 IS
   ! Убрал установку переменных окружения, потому что она производится
     в FarChDir, которая теперь используется у нас для установления
     текущего каталога.
*/
int Panel::SetCurPath()
{
	if (GetMode()==PLUGIN_PANEL)
		return TRUE;

	auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=PLUGIN_PANEL)
	{
		if (AnotherPanel->m_CurDir.size() > 1 && AnotherPanel->m_CurDir[1]==L':' &&
		        (m_CurDir.empty() || ToUpper(AnotherPanel->m_CurDir[0])!=ToUpper(m_CurDir[0])))
		{
			// сначала установим переменные окружения для пассивной панели
			// (без реальной смены пути, чтобы лишний раз пассивный каталог
			// не перечитывать)
			FarChDir(AnotherPanel->m_CurDir,FALSE);
		}
	}

	if (!FarChDir(m_CurDir))
	{
		while (!FarChDir(m_CurDir))
		{
			string strRoot;
			GetPathRoot(m_CurDir, strRoot);

			if (FAR_GetDriveType(strRoot) != DRIVE_REMOVABLE || os::IsDiskInDrive(strRoot))
			{
				if (!os::fs::is_directory(m_CurDir))
				{
					if (CheckShortcutFolder(m_CurDir, true, true) && FarChDir(m_CurDir))
					{
						SetCurDir(m_CurDir,true);
						return TRUE;
					}
				}
				else
					break;
			}

			if (Global->WindowManager->ManagerStarted()) // сначала проверим - а запущен ли менеджер
			{
				SetCurDir(Global->g_strFarPath,true);                    // если запущен - выставим путь который мы точно знаем что существует
				ChangeDisk();                                    // и вызовем меню выбора дисков
			}
			else                                               // оппа...
			{
				string strTemp(m_CurDir);
				CutToFolderNameIfFolder(m_CurDir);             // подымаемся вверх, для очередной порции ChDir

				if (strTemp.size()==m_CurDir.size())  // здесь проблема - видимо диск недоступен
				{
					SetCurDir(Global->g_strFarPath,true);                 // тогда просто сваливаем в каталог, откуда стартанул FAR.
					break;
				}
				else
				{
					if (FarChDir(m_CurDir))
					{
						SetCurDir(m_CurDir,true);
						break;
					}
				}
			}
		}
		return FALSE;
	}

	return TRUE;
}


void Panel::Hide()
{
	ScreenObject::Hide();
	auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible())
	{
		if (AnotherPanel->GetFocus())
			if ((AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen()) ||
			        (GetType()==FILE_PANEL && IsFullScreen()))
				AnotherPanel->Show();
	}
}


void Panel::Show()
{
	if (Locked())
		return;

	SCOPED_ACTION(DelayDestroy)(this);

	/* $ 03.10.2001 IS перерисуем строчку меню */
	if (Global->Opt->ShowMenuBar)
		Parent()->GetTopMenuBar()->Show();

	if (!GetModalMode())
	{
		auto AnotherPanel = Parent()->GetAnotherPanel(this);
		if (AnotherPanel->IsVisible())
		{
			if (SaveScr)
			{
				SaveScr->AppendArea(AnotherPanel->SaveScr.get());
			}

			if (AnotherPanel->GetFocus())
			{
				if (AnotherPanel->IsFullScreen())
				{
					SetVisible(true);
					return;
				}

				if (GetType() == FILE_PANEL && IsFullScreen())
				{
					ScreenObject::Show();
					AnotherPanel->Show();
					return;
				}
			}
		}
	}

	ScreenObject::Show();
	if (!this->Destroyed())
		ShowScreensCount();
}


void Panel::DrawSeparator(int Y)
{
	if (Y<m_Y2)
	{
		SetColor(COL_PANELBOX);
		GotoXY(m_X1,Y);
		ShowSeparator(m_X2-m_X1+1,1);
	}
}


void Panel::ShowScreensCount()
{
	if (Global->Opt->ShowScreensNumber && !m_X1)
	{
		int Viewers = Global->WindowManager->GetWindowCountByType(windowtype_viewer);
		int Editors = Global->WindowManager->GetWindowCountByType(windowtype_editor);
		int Dialogs = Global->WindowManager->GetWindowCountByType(windowtype_dialog);

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			GotoXY(Global->Opt->ShowColumnTitles ? m_X1:m_X1+2,m_Y1);
			SetColor(COL_PANELSCREENSNUMBER);

			Global->FS << L"[" << Viewers;
			if (Editors > 0)
			{
				Global->FS << L"+" << Editors;
			}

			if (Dialogs > 0)
			{
				Global->FS << L"," << Dialogs;
			}

			Global->FS << L"]";
		}
	}
}


void Panel::SetTitle()
{
	if (GetFocus())
	{
		ConsoleTitle::SetFarTitle(L"{" + (m_CurDir.empty()? Parent()->GetCmdLine()->GetCurDir() : m_CurDir) + L"}");
	}
}

string Panel::GetTitle() const
{
	string strTitle;
	if (m_PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		GetOpenPanelInfo(&Info);
		strTitle = NullToEmpty(Info.PanelTitle);
		RemoveExternalSpaces(strTitle);
	}
	else
	{
		if (m_ShowShortNames)
			ConvertNameToShort(m_CurDir,strTitle);
		else
			strTitle = m_CurDir;

	}

	return strTitle;
}

int Panel::SetPluginCommand(int Command,int Param1,void* Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result = Parent()->ChangePanelViewMode(this, Param1, Parent()->IsTopWindow());
			break;

		case FCTL_SETSORTMODE:
		{
			int Mode=Param1;

			if ((Mode>SM_DEFAULT) && (Mode<=SM_CHTIME))
			{
				SetSortMode(--Mode); // Уменьшим на 1 из-за SM_DEFAULT
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETNUMERICSORT:
		{
			ChangeNumericSort(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETCASESENSITIVESORT:
		{
			ChangeCaseSensitiveSort(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETSORTORDER:
		{
			ChangeSortOrder(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETDIRECTORIESFIRST:
		{
			ChangeDirectoriesFirst(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_CLOSEPANEL:
			m_PluginCommand=Command;
			m_PluginParam = NullToEmpty((const wchar_t *)Param2);
			Result=TRUE;
			break;

		case FCTL_GETPANELINFO:
		{
			PanelInfo *Info=(PanelInfo *)Param2;

			if(!CheckStructSize(Info))
				break;

			ClearStruct(*Info);
			Info->StructSize = sizeof(PanelInfo);

			UpdateIfRequired();
			Info->OwnerGuid=FarGuid;
			Info->PluginHandle=nullptr;

			switch (GetType())
			{
				case FILE_PANEL:
					Info->PanelType=PTYPE_FILEPANEL;
					break;
				case TREE_PANEL:
					Info->PanelType=PTYPE_TREEPANEL;
					break;
				case QVIEW_PANEL:
					Info->PanelType=PTYPE_QVIEWPANEL;
					break;
				case INFO_PANEL:
					Info->PanelType=PTYPE_INFOPANEL;
					break;
			}

			int X1,Y1,X2,Y2;
			GetPosition(X1,Y1,X2,Y2);
			Info->PanelRect.left=X1;
			Info->PanelRect.top=Y1;
			Info->PanelRect.right=X2;
			Info->PanelRect.bottom=Y2;
			Info->ViewMode=GetViewMode();
			Info->SortMode=static_cast<OPENPANELINFO_SORTMODES>(SM_UNSORTED-UNSORTED+GetSortMode());

			Info->Flags |= Global->Opt->ShowHidden? PFLAGS_SHOWHIDDEN : 0;
			Info->Flags |= Global->Opt->Highlight? PFLAGS_HIGHLIGHT : 0;
			Info->Flags |= GetSortOrder()? PFLAGS_REVERSESORTORDER : 0;
			Info->Flags |= GetSortGroups()? PFLAGS_USESORTGROUPS : 0;
			Info->Flags |= GetSelectedFirstMode()? PFLAGS_SELECTEDFIRST : 0;
			Info->Flags |= GetDirectoriesFirst()? PFLAGS_DIRECTORIESFIRST : 0;
			Info->Flags |= GetNumericSort()? PFLAGS_NUMERICSORT : 0;
			Info->Flags |= GetCaseSensitiveSort()? PFLAGS_CASESENSITIVESORT : 0;
			Info->Flags |= (GetMode()==PLUGIN_PANEL)? PFLAGS_PLUGIN : 0;
			Info->Flags |= IsVisible()? PFLAGS_VISIBLE : 0;
			Info->Flags |= GetFocus()? PFLAGS_FOCUS : 0;
			Info->Flags |= this == Parent()->LeftPanel ? PFLAGS_PANELLEFT : 0;

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;

				if (Info->Flags&PFLAGS_PLUGIN)
				{
					Info->OwnerGuid = DestFilePanel->GetPluginHandle()->pPlugin->GetGUID();
					Info->PluginHandle = DestFilePanel->GetPluginHandle()->hPlugin;
					static int Reenter=0;
					if (!Reenter)
					{
						Reenter++;
						OpenPanelInfo PInfo;
						DestFilePanel->GetOpenPanelInfo(&PInfo);

						if (PInfo.Flags & OPIF_REALNAMES)
							Info->Flags |= PFLAGS_REALNAMES;

						if (PInfo.Flags & OPIF_DISABLEHIGHLIGHTING)
							Info->Flags &= ~PFLAGS_HIGHLIGHT;

						if (PInfo.Flags & OPIF_USECRC32)
							Info->Flags |= PFLAGS_USECRC32;

						if (PInfo.Flags & OPIF_SHORTCUT)
							Info->Flags |= PFLAGS_SHORTCUT;

						Reenter--;
					}
				}

				DestFilePanel->PluginGetPanelInfo(*Info);
			}

			if (!(Info->Flags&PFLAGS_PLUGIN)) // $ 12.12.2001 DJ - на неплагиновой панели - всегда реальные имена
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}

		case FCTL_GETPANELPREFIX:
		{
			string strTemp;

			if (GetType()==FILE_PANEL && GetMode() == PLUGIN_PANEL)
			{
				PluginInfo PInfo = {sizeof(PInfo)};
				FileList *DestPanel = ((FileList*)this);
				if (DestPanel->GetPluginInfo(&PInfo))
					strTemp = NullToEmpty(PInfo.CommandPrefix);
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp.data(),Param1);

			Result=(int)strTemp.size()+1;
			break;
		}

		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELFORMAT:
		{
			string strTemp;

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && GetMode()==PLUGIN_PANEL)
				{
					Reenter++;

					OpenPanelInfo PInfo;
					DestFilePanel->GetOpenPanelInfo(&PInfo);

					switch (Command)
					{
						case FCTL_GETPANELHOSTFILE:
							strTemp=NullToEmpty(PInfo.HostFile);
							break;
						case FCTL_GETPANELFORMAT:
							strTemp=NullToEmpty(PInfo.Format);
							break;
					}

					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp.data(),Param1);

			Result=(int)strTemp.size()+1;
			break;
		}
		case FCTL_GETPANELDIRECTORY:
		{
			static int Reenter=0;
			if(!Reenter)
			{
				Reenter++;
				ShortcutInfo Info;
				GetShortcutInfo(Info);
				Result=ALIGN(sizeof(FarPanelDirectory));
				size_t folderOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.ShortcutFolder.size()+1));
				size_t pluginFileOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginFile.size()+1));
				size_t pluginDataOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginData.size()+1));
				FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
				if(Param1>=Result && CheckStructSize(dirInfo))
				{
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					dirInfo->PluginId=Info.PluginGuid;
					dirInfo->Name=(wchar_t*)((char*)Param2+folderOffset);
					dirInfo->Param=(wchar_t*)((char*)Param2+pluginDataOffset);
					dirInfo->File=(wchar_t*)((char*)Param2+pluginFileOffset);
					std::copy_n(Info.ShortcutFolder.data(), Info.ShortcutFolder.size() + 1, const_cast<wchar_t*>(dirInfo->Name));
					std::copy_n(Info.PluginData.data(), Info.PluginData.size() + 1, const_cast<wchar_t*>(dirInfo->Param));
					std::copy_n(Info.PluginFile.data(), Info.PluginFile.size() + 1, const_cast<wchar_t*>(dirInfo->File));
				}
				Reenter--;
			}
			break;
		}

		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:

			if (GetType()==FILE_PANEL)
			{
				string strColumnTypes,strColumnWidths;
				((FileList *)this)->PluginGetColumnTypesAndWidths(strColumnTypes,strColumnWidths);

				if (Command==FCTL_GETCOLUMNTYPES)
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnTypes.data(),Param1);

					Result=(int)strColumnTypes.size()+1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnWidths.data(),Param1);

					Result=(int)strColumnWidths.size()+1;
				}
			}
			break;

		case FCTL_GETPANELITEM:
		{
			if (GetType()==FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
				Result = static_cast<int>(static_cast<FileList*>(this)->PluginGetPanelItem(Param1, static_cast<FarGetPluginPanelItem*>(Param2)));
			break;
		}

		case FCTL_GETSELECTEDPANELITEM:
		{
			if (GetType() == FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
				Result = static_cast<int>(static_cast<FileList*>(this)->PluginGetSelectedPanelItem(Param1, static_cast<FarGetPluginPanelItem*>(Param2)));
			break;
		}

		case FCTL_GETCURRENTPANELITEM:
		{
			if (GetType() == FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
			{
				PanelInfo Info;
				auto DestPanel = static_cast<FileList*>(this);
				DestPanel->PluginGetPanelInfo(Info);
				Result = static_cast<int>(DestPanel->PluginGetPanelItem(static_cast<int>(Info.CurrentItem), static_cast<FarGetPluginPanelItem*>(Param2)));
			}
			break;
		}

		case FCTL_BEGINSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginBeginSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginSetSelection(Param1, Param2 != nullptr);
				Result=TRUE;
			}
			break;
		}

		case FCTL_CLEARSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginClearSelection(Param1);
				Result=TRUE;
			}
			break;
		}

		case FCTL_ENDSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginEndSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_UPDATEPANEL:
			Update(Param1?UPDATE_KEEP_SELECTION:0);

			if (GetType() == QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;

		case FCTL_REDRAWPANEL:
		{
			PanelRedrawInfo *Info=(PanelRedrawInfo *)Param2;

			if (CheckStructSize(Info))
			{
				m_CurFile=static_cast<int>(Info->CurrentItem);
				m_CurTopFile=static_cast<int>(Info->TopPanelItem);
			}

			// $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущее окно
			if (Parent()->IsTopWindow())
				Redraw();

			Result=TRUE;
			break;
		}

		case FCTL_SETPANELDIRECTORY:
		{
			FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
			if (CheckStructSize(dirInfo))
			{
				string strName(NullToEmpty(dirInfo->Name)), strFile(NullToEmpty(dirInfo->File)), strParam(NullToEmpty(dirInfo->Param));
				Result = ExecShortcutFolder(strName, dirInfo->PluginId, strFile, strParam, false, false, true);
				// restore current directory to active panel path
				if (Result)
				{
					auto ActivePanel = Parent()->ActivePanel();
					if (this != ActivePanel)
					{
						ActivePanel->SetCurPath();
					}
				}
			}
			break;
		}

		case FCTL_SETACTIVEPANEL:
		{
			if (IsVisible())
			{
				SetFocus();
				Result=TRUE;
			}
			break;
		}
	}

	ProcessingPluginCommand--;
	return Result;
}


int Panel::GetCurName(string &strName, string &strShortName) const
{
	strName.clear();
	strShortName.clear();
	return FALSE;
}


int Panel::GetCurBaseName(string &strName, string &strShortName) const
{
	strName.clear();
	strShortName.clear();
	return FALSE;
}

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
	FarDialogItem DCDlgData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 72, 9, 0, nullptr, nullptr, 0,                MSG(MChangeDriveDisconnectTitle)},
		{DI_TEXT,      5, 2,  0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,L""},
		{DI_TEXT,      5, 3,  0, 3, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,L""},
		{DI_TEXT,      5, 4,  0, 4, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,L""},
		{DI_TEXT,     -1, 5,  0, 5, 0, nullptr, nullptr, DIF_SEPARATOR,    L""},
		{DI_CHECKBOX,  5, 6, 70, 6, 0, nullptr, nullptr, 0,                MSG(MChangeDriveDisconnectReconnect)},
		{DI_TEXT,     -1, 7,  0, 7, 0, nullptr, nullptr, DIF_SEPARATOR,    L""},
		{DI_BUTTON,    0, 8,  0, 8, 0, nullptr, nullptr, DIF_FOCUS|DIF_DEFAULTBUTTON|DIF_CENTERGROUP, MSG(MYes)},
		{DI_BUTTON,    0, 8,  0, 8, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel)},
	};
	auto DCDlg = MakeDialogItemsEx(DCDlgData);

	LangString strMsgText;

	strMsgText = MChangeDriveDisconnectQuestion;
	strMsgText << Letter;
	DCDlg[1].strData = strMsgText;

	strMsgText = MChangeDriveDisconnectMapped;
	strMsgText << Letter;
	DCDlg[2].strData = strMsgText;

	size_t Len1 = DCDlg[0].strData.size();
	size_t Len2 = DCDlg[1].strData.size();
	size_t Len3 = DCDlg[2].strData.size();
	size_t Len4 = DCDlg[5].strData.size();
	Len1 = std::max(Len1,std::max(Len2,std::max(Len3,Len4)));
	DriveLocalToRemoteName(DRIVE_REMOTE,Letter,strMsgText);
	DCDlg[3].strData = TruncPathStr(strMsgText, static_cast<int>(Len1));
	// проверяем - это было постоянное соединение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.

	bool IsPersistent = true;
	const wchar_t KeyName[] = {L'N', L'e', L't', L'w', L'o', L'r', L'k', L'\\', Letter, L'\0'};

	if (os::reg::key(HKEY_CURRENT_USER, KeyName, KEY_QUERY_VALUE))
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
	int ExitCode=7;

	if (Global->Opt->Confirm.RemoveConnection)
	{
		auto Dlg = Dialog::create(DCDlg);
		Dlg->SetPosition(-1,-1,DCDlg[0].X2+4,11);
		Dlg->SetHelp(L"DisconnectDrive");
		Dlg->SetId(DisconnectDriveId);
		Dlg->SetDialogMode(DMODE_WARNINGSTYLE);
		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Global->Opt->ChangeDriveDisconnectMode=DCDlg[5].Selected == BSTATE_CHECKED;

	return ExitCode == 7;
}

BOOL Panel::NeedUpdatePanel(const Panel *AnotherPanel)
{
	/* Обновить, если обновление разрешено и пути совпадают */
	if ((!Global->Opt->AutoUpdateLimit || static_cast<unsigned>(GetFileCount()) <= static_cast<unsigned>(Global->Opt->AutoUpdateLimit)) &&
	        !StrCmpI(AnotherPanel->m_CurDir, m_CurDir))
		return TRUE;

	return FALSE;
}

bool Panel::GetShortcutInfo(ShortcutInfo& ShortcutInfo) const
{
	bool result=true;
	if (m_PanelMode==PLUGIN_PANEL)
	{
		auto ph = GetPluginHandle();
		ShortcutInfo.PluginGuid = ph->pPlugin->GetGUID();
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(ph, &Info);
		ShortcutInfo.PluginFile = NullToEmpty(Info.HostFile);
		ShortcutInfo.ShortcutFolder = NullToEmpty(Info.CurDir);
		ShortcutInfo.PluginData = NullToEmpty(Info.ShortcutData);
		if(!(Info.Flags&OPIF_SHORTCUT)) result=false;
	}
	else
	{
		ShortcutInfo.PluginGuid=FarGuid;
		ShortcutInfo.PluginFile.clear();
		ShortcutInfo.PluginData.clear();
		ShortcutInfo.ShortcutFolder = m_CurDir;
	}
	return result;
}

bool Panel::SaveShortcutFolder(int Pos, bool Add) const
{
	ShortcutInfo Info;
	if(GetShortcutInfo(Info))
	{
		auto Function = Add? &Shortcuts::Add : &Shortcuts::Set;
		(Shortcuts().*Function)(Pos, Info.ShortcutFolder, Info.PluginGuid, Info.PluginFile, Info.PluginData);
		return true;
	}
	return false;
}

/*
int Panel::ProcessShortcutFolder(int Key,BOOL ProcTreePanel)
{
	string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

	if (GetShortcutFolder(Key-KEY_RCTRL0,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (ProcTreePanel)
		{
			if (AnotherPanel->GetType()==FILE_PANEL)
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
			else
			{
				SetCurDir(strShortcutFolder,true);
				ProcessKey(KEY_ENTER);
			}
		}
		else
		{
			if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.empty())
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
		}

		return TRUE;
	}

	return FALSE;
}
*/

bool Panel::ExecShortcutFolder(int Pos, bool raw)
{
	string strShortcutFolder,strPluginFile,strPluginData;
	GUID PluginGuid;

	if (Shortcuts().Get(Pos,&strShortcutFolder, &PluginGuid, &strPluginFile, &strPluginData, raw))
	{
		return ExecShortcutFolder(strShortcutFolder,PluginGuid,strPluginFile,strPluginData,true);
	}
	return false;
}

bool Panel::ExecShortcutFolder(string& strShortcutFolder, const GUID& PluginGuid, const string& strPluginFile, const string& strPluginData, bool CheckType, bool TryClosest, bool Silent)
{
	auto SrcPanel=this;
	auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if(CheckType)
	{
		switch (GetType())
		{
			case TREE_PANEL:
				if (AnotherPanel->GetType()==FILE_PANEL)
					SrcPanel=AnotherPanel;
				break;

			case QVIEW_PANEL:
			case INFO_PANEL:
			{
				if (AnotherPanel->GetType()==FILE_PANEL)
					SrcPanel=AnotherPanel;
				break;
			}
		}
	}

	bool CheckFullScreen=SrcPanel->IsFullScreen();

	if (PluginGuid != FarGuid)
	{
		if (ProcessPluginEvent(FE_CLOSE, nullptr))
		{
			return true;
		}

		if (auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(PluginGuid))
		{
			if (pPlugin->has<iOpen>())
			{
				if (!strPluginFile.empty())
				{
					string strRealDir;
					strRealDir = strPluginFile;

					if (CutToSlash(strRealDir))
					{
						SrcPanel->SetCurDir(strRealDir,true);
						SrcPanel->GoToFile(PointToName(strPluginFile));

						SrcPanel->ClearAllItem();
					}
				}

				OpenShortcutInfo info=
				{
					sizeof(OpenShortcutInfo),
					strPluginFile.empty()?nullptr:strPluginFile.data(),
					strPluginData.empty()?nullptr:strPluginData.data(),
					(SrcPanel == Parent()->ActivePanel()) ? FOSF_ACTIVE : FOSF_NONE
				};

				if (auto hNewPlugin = Global->CtrlObject->Plugins->Open(pPlugin, OPEN_SHORTCUT, FarGuid, (intptr_t)&info))
				{
					int CurFocus=SrcPanel->GetFocus();

					auto NewPanel = Parent()->ChangePanel(SrcPanel, FILE_PANEL, TRUE, TRUE);
					NewPanel->SetPluginMode(hNewPlugin, L"", CurFocus || !Parent()->GetAnotherPanel(NewPanel)->IsVisible());

					if (!strShortcutFolder.empty())
					{
						UserDataItem UserData = {}; //????
						Global->CtrlObject->Plugins->SetDirectory(hNewPlugin,strShortcutFolder,0,&UserData);
					}

					NewPanel->Update(0);
					NewPanel->Show();
				}
			}
		}

		return true;
	}

	if (!CheckShortcutFolder(strShortcutFolder, TryClosest, Silent) || ProcessPluginEvent(FE_CLOSE, nullptr))
	{
		return false;
	}

    /*
	if (SrcPanel->GetType()!=FILE_PANEL)
	{
		SrcPanel = Parent()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
	}
    */

	SrcPanel->SetCurDir(strShortcutFolder,true);

	if (CheckFullScreen!=SrcPanel->IsFullScreen())
		Parent()->GetAnotherPanel(SrcPanel)->Show();

	SrcPanel->Refresh();
	return true;
}

bool Panel::CreateFullPathName(const string& Name, const string& ShortName,DWORD FileAttr, string &strDest, int UNC,int ShortNameAsIs) const
{
	string strFileName = strDest;
	const wchar_t *ShortNameLastSlash=LastSlash(ShortName.data());
	const wchar_t *NameLastSlash=LastSlash(Name.data());

	if (nullptr==ShortNameLastSlash && nullptr==NameLastSlash)
	{
		ConvertNameToFull(strFileName, strFileName);
	}

	/* BUGBUG весь этот if какая то чушь
	else if (ShowShortNames)
	{
	  string strTemp = Name;

	  if (NameLastSlash)
	    strTemp.SetLength(1+NameLastSlash-Name);

	  const wchar_t *NamePtr = wcsrchr(strFileName, L'\\');

	  if(NamePtr )
	    NamePtr++;
	  else
	    NamePtr=strFileName;

	  strTemp += NameLastSlash?NameLastSlash+1:Name; //??? NamePtr??? BUGBUG
	  strFileName = strTemp;
	}
	*/

	if (m_ShowShortNames && ShortNameAsIs)
		ConvertNameToShort(strFileName,strFileName);

	/* $ 29.01.2001 VVM
	  + По CTRL+ALT+F в командную строку сбрасывается UNC-имя текущего файла. */
	if (UNC)
		ConvertNameToUNC(strFileName);

	// $ 20.10.2000 SVS Сделаем фичу Ctrl-F опциональной!
	if (Global->Opt->PanelCtrlFRule)
	{
		/* $ 13.10.2000 tran
		  по Ctrl-f имя должно отвечать условиям на панели */
		if (m_ViewSettings.Flags&PVS_FOLDERUPPERCASE)
		{
			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				ToUpper(strFileName);
			}
			else
			{
				size_t pos;
				ToUpper(strFileName, 0, FindLastSlash(pos,strFileName)? pos : string::npos);
			}
		}

		if ((m_ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			size_t pos;

			if (FindLastSlash(pos,strFileName) && !IsCaseMixed(strFileName.data()+pos))
				ToLower(strFileName, pos);
		}

		if ((m_ViewSettings.Flags&PVS_FILELOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			size_t pos;

			if (FindLastSlash(pos,strFileName))
				ToLower(strFileName, pos);
		}
	}

	strDest = strFileName;
	return !strDest.empty();
}

void Panel::exclude_sets(string& mask)
{
	ReplaceStrings(mask, L"[", L"<[%>", true);
	ReplaceStrings(mask, L"]", L"[]]", true);
	ReplaceStrings(mask, L"<[%>", L"[[]", true);
}

FilePanels* Panel::Parent(void)const
{
	return dynamic_cast<FilePanels*>(GetOwner().get());
}

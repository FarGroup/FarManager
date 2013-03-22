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
#include "macroopcode.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "vmenu2.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "chgmmode.hpp"
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
#include "options.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "imports.hpp"
#include "constitle.hpp"
#include "FarDlgBuilder.hpp"
#include "setattr.hpp"
#include "window.hpp"
#include "palette.hpp"
#include "FarGuid.hpp"
#include "elevation.hpp"
#include "stddlg.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=nullptr;

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile);

/* $ 21.08.2002 IS
   Класс для хранения пункта плагина в меню выбора дисков
*/

class ChDiskPluginItem
{
	public:
		MenuItemEx Item;
		WCHAR HotKey;

		ChDiskPluginItem() { Clear(); }
		~ChDiskPluginItem() {}

		void Clear() { HotKey = 0; Item.Clear(); }
		bool operator==(const ChDiskPluginItem &rhs) const { return HotKey==rhs.HotKey && !StrCmpI(Item.strName,rhs.Item.strName) && Item.UserData==rhs.Item.UserData; }
		int operator<(const ChDiskPluginItem &rhs) const {return (Global->Opt->ChangeDriveMode&DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey!=rhs.HotKey)?unsigned(HotKey-1)<unsigned(rhs.HotKey-1):StrCmpI(Item.strName,rhs.Item.strName)<0;}
		ChDiskPluginItem& operator=(const ChDiskPluginItem &rhs);
};

ChDiskPluginItem& ChDiskPluginItem::operator=(const ChDiskPluginItem &rhs)
{
	if (this != &rhs)
	{
		Item=rhs.Item;
		HotKey=rhs.HotKey;
	}

	return *this;
}


Panel::Panel():
	Focus(false),
	Type(0),
	EnableUpdate(TRUE),
	PanelMode(NORMAL_PANEL),
	SortMode(UNSORTED),
	SortOrder(0),
	SortGroups(0),
	PrevViewMode(VIEW_3),
	ViewMode(0),
	CurTopFile(0),
	CurFile(0),
	ShowShortNames(0),
	NumericSort(0),
	CaseSensitiveSort(0),
	DirectoriesFirst(1),
	ModalMode(0),
	PluginCommand(0),
	ViewSettings(),
	ProcessingPluginCommand(0)
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
	PrevViewMode=ViewMode;
	this->ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	string strNewDir;
	apiGetCurrentDirectory(strNewDir);
	SetCurDir(strNewDir,TRUE);
}


void Panel::ChangeDisk()
{
	int Pos=0,FirstCall=TRUE;

	if (!strCurDir.IsEmpty() && strCurDir.At(1)==L':')
	{
		Pos=std::max(0, Upper(strCurDir.At(0))-L'A');
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

struct TypeMessage
{
	int DrvType;
	LNGID FarMsg;
};

const TypeMessage DrTMsg[]=
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

static size_t AddPluginItems(VMenu2 &ChDisk, int Pos, int DiskCount, bool SetSelected)
{
	std::list<ChDiskPluginItem> MPItems;
	int PluginItem;
	bool ItemPresent,Done=false;
	string strMenuText;
	string strPluginText;
	size_t PluginMenuItemsCount = 0;

	FOR_CONST_RANGE(*Global->CtrlObject->Plugins, i)
	{
		if(Done)
			break;
		for (PluginItem=0;; ++PluginItem)
		{
			Plugin *pPlugin = *i;

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

			if (!strMenuText.IsEmpty())
			{
				ChDiskPluginItem OneItem;
				OneItem.Clear();
				PanelMenuItem *item = new PanelMenuItem;
				item->bIsPlugin = true;
				item->pPlugin = pPlugin;
				item->Guid = guid;
#ifndef NO_WRAPPER
				if (pPlugin->IsOemPlugin())
					OneItem.Item.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
				OneItem.Item.strName = strMenuText;
				OneItem.Item.UserDataSize=sizeof(PanelMenuItem);
				OneItem.Item.UserData=item;
				OneItem.HotKey=HotKey;
				MPItems.push_back(OneItem);
				/*
				else BUGBUG, а вот это, похоже, лишнее
				{
					Done=TRUE;
					break;
				}
				*/
			}
		} // END: for (PluginItem=0;;++PluginItem)
	}

	MPItems.sort();
	MPItems.unique(); // выкинем дубли
	PluginMenuItemsCount=MPItems.size();

	if (PluginMenuItemsCount)
	{
		MenuItemEx ChDiskItem;

		ChDiskItem.Clear();
		ChDiskItem.Flags|=LIF_SEPARATOR;
		ChDiskItem.UserDataSize=0;
		ChDisk.AddItem(&ChDiskItem);
		ChDiskItem.Flags&=~LIF_SEPARATOR;

		int index = 0;
		std::for_each(RANGE(MPItems, i)
		{
			if (Pos > DiskCount && !SetSelected)
			{
				i.Item.SetSelect(DiskCount+index+1==Pos);

				if (!SetSelected)
					SetSelected=DiskCount+index+1==Pos;
			}
			const wchar_t HotKeyStr[]={i.HotKey? L'&' : L' ', i.HotKey? i.HotKey : L' ', L' ', i.HotKey? L' ' : L'\0', L'\0'};
			i.Item.strName = string(HotKeyStr) + i.Item.strName;
			ChDisk.AddItem(&i.Item);

			delete(PanelMenuItem*)i.Item.UserData;  //ммда...
			++index;
		});
	}
	return PluginMenuItemsCount;
}

static void ConfigureChangeDriveMode()
{
	DialogBuilder Builder(MChangeDriveConfigure, L"ChangeDriveMode");
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


intptr_t ChDiskDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
		{
			if (Param1 == 1) // BUGBUG, magic number
			{
				FarColor Color = ColorIndexToColor(COL_WARNDIALOGTEXT);
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;
	default:
		break;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

class separator
{
public:
	separator():m_value(L' '){}
	wchar_t Get()
	{
		wchar_t value = m_value;
		m_value = BoxSymbols[BS_V1];
		return value;
	}
private:
	wchar_t m_value;
};

int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
	Global->Window->DeviceArivalEvent().Reset();
	Global->Window->DeviceRemoveEvent().Reset();
	Global->Window->MediaArivalEvent().Reset();
	Global->Window->MediaRemoveEvent().Reset();

	class Guard_Macro_DskShowPosType  //фигня какая-то
	{
		public:
			Guard_Macro_DskShowPosType(Panel *curPanel) {Global->Macro_DskShowPosType=(curPanel==Global->CtrlObject->Cp()->LeftPanel)?1:2;};
			~Guard_Macro_DskShowPosType() {Global->Macro_DskShowPosType=0;};
	};
	Guard_Macro_DskShowPosType _guard_Macro_DskShowPosType(this);
	MenuItemEx ChDiskItem;
	string strRootDir;
	DWORD Mask,DiskMask;
	int DiskCount,Focus;
	bool SetSelected=false;
	Mask = FarGetLogicalDrives();
	DWORD NetworkMask = 0;
	AddSavedNetworkDisks(Mask, NetworkMask);

	for (DiskMask=Mask,DiskCount=0; DiskMask; DiskMask>>=1)
		DiskCount+=DiskMask & 1;

	PanelMenuItem Item, *mitem=0;
	{ // эта скобка надо, см. M#605
		VMenu2 ChDisk(MSG(MChangeDriveTitle),nullptr,0,ScrY-Y1-3);
		ChDisk.SetBottomTitle(MSG(MChangeDriveMenuFooter));

		ChDisk.SetHelp(L"DriveDlg");
		ChDisk.SetFlags(VMENU_WRAPMODE);

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


		DisableElevation* DE = new DisableElevation;
		/* $ 02.04.2001 VVM
		! Попытка не будить спящие диски... */
		WCHAR I;
		for (DiskMask=Mask,I=0; DiskMask; DiskMask>>=1,I++)
		{
			if (!(DiskMask & 1))   //нету диска
				continue;

			DiskMenuItem NewItem;

			wchar_t Drv[]={L'&',static_cast<wchar_t>(L'A'+I),L':',L'\\',L'\0'};
			strRootDir=Drv+1;
			Drv[3] = 0;
			NewItem.Letter = Drv;
			NewItem.DriveType = FAR_GetDriveType(strRootDir, Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if ((1<<I)&NetworkMask)
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_TYPE|DRIVE_SHOW_NETNAME))
			{
				string LocalName("?:");
				LocalName.Replace(0, strRootDir.At(0));

				if (GetSubstName(NewItem.DriveType, LocalName, NewItem.Path))
				{
					NewItem.DriveType=DRIVE_SUBSTITUTE;
				}
				else if(DriveCanBeVirtual(NewItem.DriveType) && GetVHDName(LocalName, NewItem.Path))
				{
					NewItem.DriveType=DRIVE_VIRTUAL;
				}

				for (size_t J=0; J < ARRAYSIZE(DrTMsg); ++J)
				{
					if (DrTMsg[J].DrvType == NewItem.DriveType)
					{
						NewItem.Type = MSG(DrTMsg[J].FarMsg);
						break;
					}
				}
			}

			int ShowDisk = (NewItem.DriveType!=DRIVE_REMOVABLE || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
			               (!IsDriveTypeCDROM(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
			               (!IsDriveTypeRemote(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
			{
				if (ShowDisk && !apiGetVolumeInformation(
				            strRootDir,
				            &NewItem.Label,
				            nullptr,
				            nullptr,
				            nullptr,
				            &NewItem.Fs
				        ))
				{
					NewItem.Label = MSG(MChangeDriveLabelAbsent);
					ShowDisk = FALSE;
				}
			}

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				unsigned __int64 TotalSize = 0, UserFree = 0;

				if (ShowDisk && apiGetDiskSize(strRootDir,&TotalSize, nullptr, &UserFree))
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
					DriveLocalToRemoteName(NewItem.DriveType,strRootDir.At(0),NewItem.Path);
					break;
				}
			}

			TypeWidth = std::max(TypeWidth, NewItem.Type.GetLength());
			LabelWidth = std::max(LabelWidth, NewItem.Label.GetLength());
			FsWidth = std::max(FsWidth, NewItem.Fs.GetLength());
			TotalSizeWidth = std::max(TotalSizeWidth, NewItem.TotalSize.GetLength());
			FreeSizeWidth = std::max(FreeSizeWidth, NewItem.FreeSize.GetLength());
			PathWidth = std::max(PathWidth, NewItem.Path.GetLength());

			Items.push_back(NewItem);
		}

		int MenuLine = 0;

		std::for_each(CONST_RANGE(Items, i)
		{
			ChDiskItem.Clear();
			I = i.Letter[1] - L'A';
			if (FirstCall)
			{
				ChDiskItem.SetSelect(I==Pos);

				if (!SetSelected)
					SetSelected=(I==Pos);
			}
			else
			{
				if (Pos < DiskCount)
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
			item.cDrive = L'A'+I;
			item.nDriveType = i.DriveType;
			ChDisk.SetUserData(&item, sizeof(item), ChDisk.AddItem(&ChDiskItem));
			MenuLine++;
		});

		size_t PluginMenuItemsCount=0;

		if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(ChDisk, Pos, DiskCount, SetSelected);
		}

		delete DE;

		int X=X1+5;

		if ((this == Global->CtrlObject->Cp()->RightPanel) && IsFullScreen() && (X2-X1 > 40))
			X = (X2-X1+1)/2+5;

		ChDisk.SetPosition(X,-1,0,0);

		int Y = (ScrY+1-(DiskCount+static_cast<int>(PluginMenuItemsCount)+5))/2;
		if (Y < 3)
			ChDisk.SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk.SetMacroMode(MACRO_DISKS);
		int RetCode=-1;
		ChDisk.Run([&](int Key)->int
		{
			if(Key==KEY_NONE && (Global->Window->DeviceArivalEvent().Signaled() || Global->Window->DeviceRemoveEvent().Signaled() || Global->Window->MediaArivalEvent().Signaled() || Global->Window->MediaRemoveEvent().Signaled()))
				Key=KEY_CTRLR;

			int SelPos=ChDisk.GetSelectPos();
			PanelMenuItem *item = (PanelMenuItem*)ChDisk.GetUserData(nullptr,0);

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
						DosDeviceName.Replace(0, item->cDrive);
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
						ChDisk.Close(-1);
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
							SaveScreen SvScrn;
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
						int Code = DisconnectDrive(item, ChDisk);
						if (Code != DRIVE_DEL_FAIL && Code != DRIVE_DEL_NONE)
						{
							Global->ScrBuf->Lock(); // отменяем всякую прорисовку
							FrameManager->ResizeAllFrame();
							FrameManager->PluginCommit(); // коммитим.
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
							string DeviceName("?:\\");
							DeviceName.Replace(0, item->cDrive);
							ShellSetFileAttributes(nullptr, &DeviceName);
						}
						else
						{
							string strName = ChDisk.GetItemPtr(SelPos)->strName + 3;
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
					if (item && !item->bIsPlugin && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu))
					{
						const wchar_t DeviceName[] = {item->cDrive, L':', L'\\', 0};
						struct DiskMenuParam {const wchar_t* CmdLine; BOOL Apps;} p = {DeviceName, Key!=KEY_MSRCLICK};
						Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu, OPEN_LEFTDISKMENU, &p); // EMenu Plugin :-)
					}
					break;
				}

				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDECIMAL:
				case KEY_SHIFTDEL:
				{
					if (item && !item->bIsPlugin)
					{
						RemoveHotplugDevice(item, ChDisk);
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
						    item->pPlugin->GetModuleName(),
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

					if (item && item->bIsPlugin && item->pPlugin->HasConfigure())
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
				ChDisk.Close(-1);

			return KeyProcessed;
		});

		if (RetCode>=0)
			return RetCode;

		if (ChDisk.GetExitCode()<0 &&
			        !strCurDir.IsEmpty() &&
			        (StrCmpN(strCurDir,L"\\\\",2) ))
			{
				const wchar_t RootDir[4] = {strCurDir.At(0),L':',L'\\',L'\0'};

				if (FAR_GetDriveType(RootDir) == DRIVE_NO_ROOT_DIR)
				return ChDisk.GetSelectPos();
			}

		if (ChDisk.GetExitCode()<0)
			return -1;

		mitem=(PanelMenuItem*)ChDisk.GetUserData(nullptr,0);

		if (mitem)
		{
			Item=*mitem;
			mitem=&Item;
		}
	} // эта скобка надо, см. M#605

	if (Global->Opt->CloseCDGate && mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		string RootDir(L"?:");
		RootDir.Replace(0, mitem->cDrive);

		if (!apiIsDiskInDrive(RootDir))
		{
			if (!EjectVolume(mitem->cDrive, EJECT_READY|EJECT_NO_MESSAGE))
			{
				SaveScreen SvScrn;
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

			if (NetworkMask & (1<<(mitem->cDrive-L'A')))
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

			enum
			{
				CHDISKERROR_DOUBLEBOX,
				CHDISKERROR_TEXT0,
				CHDISKERROR_TEXT1,
				CHDISKERROR_FIXEDIT,
				CHDISKERROR_TEXT2,
				CHDISKERROR_SEPARATOR,
				CHDISKERROR_BUTTON_OK,
				CHDISKERROR_BUTTON_CANCEL,
			};
			const wchar_t Drive[]={mitem->cDrive,L'\0'};
			string strError;
			GetErrorString(strError);
			int Len1=static_cast<int>(strError.GetLength());
			int Len2=StrLength(MSG(MChangeDriveCannotReadDisk));

			const int DX=std::min(std::max(Len1,Len2 + 3)+11, ScrX-1), DY=8;
			const FarDialogItem ChDiskData[]=
			{
				{DI_DOUBLEBOX,3,1,DX-4,DY-2,0,nullptr,nullptr,0,MSG(MError)},
				{DI_EDIT,5,2,DX-6,2,0,nullptr,nullptr,DIF_READONLY,strError.CPtr()},
				{DI_TEXT,5,3,DX-9,3,0,nullptr,nullptr,0,MSG(MChangeDriveCannotReadDisk)},
				{DI_FIXEDIT,5+Len2+1,3,5+Len2+1,3,0,nullptr,nullptr,DIF_FOCUS,Drive},
				{DI_TEXT,5+Len2+2,3,5+Len2+2,3,0,nullptr,nullptr,0,L":"},
				{DI_TEXT,-1,DY-4,0,DY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
				{DI_BUTTON,0,DY-3,0,DY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MRetry)},
				{DI_BUTTON,0,DY-3,0,DY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
			};
			MakeDialogItemsEx(ChDiskData,ChDiskDlg);
			Dialog Dlg(ChDiskDlg, ARRAYSIZE(ChDiskData), ChDiskDlgProc, 0);
			Dlg.SetPosition(-1,-1,DX,DY);
			Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
			Dlg.Process();
			if(Dlg.GetExitCode()==CHDISKERROR_BUTTON_OK)
			{
				mitem->cDrive=ChDiskDlg[CHDISKERROR_FIXEDIT].strData.At(0);
			}
			else
			{
				return -1;
			}
		}

		string strNewCurDir;
		apiGetCurrentDirectory(strNewCurDir);

		if ((PanelMode == NORMAL_PANEL) &&
		        (GetType() == FILE_PANEL) &&
		        !StrCmpI(strCurDir,strNewCurDir) &&
		        IsVisible())
		{
			// А нужно ли делать здесь Update????
			Update(UPDATE_KEEP_SELECTION);
		}
		else
		{
			Focus=GetFocus();
			Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(this, FILE_PANEL, TRUE, FALSE);
			NewPanel->SetCurDir(strNewCurDir,TRUE);
			NewPanel->Show();

			if (Focus || !Global->CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
				NewPanel->SetFocus();

			if (!Focus && Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}
	else //эта плагин, да
	{
		HANDLE hPlugin = Global->CtrlObject->Plugins->Open(
		                     mitem->pPlugin,
		                     (Global->CtrlObject->Cp()->LeftPanel == this)?OPEN_LEFTDISKMENU:OPEN_RIGHTDISKMENU,
		                     mitem->Guid,
		                     0
		                 );

		if (hPlugin)
		{
			Focus=GetFocus();
			Panel *NewPanel = Global->CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
			NewPanel->SetPluginMode(hPlugin,L"",Focus || !Global->CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!Focus && Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}

	return -1;
}

int Panel::DisconnectDrive(PanelMenuItem *item, VMenu2 &ChDisk)
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
			int AMode=Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
			string strTmpCDir(GetCurDir()), strTmpADir(Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir());
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
						Global->CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, FALSE);

					if (CMode != PLUGIN_PANEL)
						SetCurDir(strTmpCDir, FALSE);

					// ... и выведем месаг о...
					SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
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

void Panel::RemoveHotplugDevice(PanelMenuItem *item, VMenu2 &ChDisk)
{
	int Code = ProcessRemoveHotplugDevice(item->cDrive, EJECT_NOTIFY_AFTERREMOVE);

	if (!Code)
	{
		// запоминаем состояние панелей
		int CMode=GetMode();
		int AMode=Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
		string strTmpCDir(GetCurDir()), strTmpADir(Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir());
		// "цикл до умопомрачения"
		int DoneEject=FALSE;

		while (!DoneEject)
		{
			// "освободим диск" - перейдем при необходимости в домашний каталог
			// TODO: А если домашний каталог - USB? ;-)
			IfGoHome(item->cDrive);
			// очередная попытка извлечения без вывода сообщения
			Code = ProcessRemoveHotplugDevice(item->cDrive, EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);

			if (!Code)
			{
				// восстановим пути - это избавит нас от левых данных в панели.
				if (AMode != PLUGIN_PANEL)
					Global->CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, FALSE);

				if (CMode != PLUGIN_PANEL)
					SetCurDir(strTmpCDir, FALSE);

				// ... и выведем месаг о...
				SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
				DoneEject = Message(MSG_WARNING|MSG_ERRORTYPE, 2,
				                MSG(MError),
				                LangString(MChangeCouldNotEjectHotPlugMedia) << item->cDrive,
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
	DiskLetter.Replace(0, Drive);

	switch(DriveType)
	{
	case DRIVE_SUBSTITUTE:
		{
			if (Global->Opt->Confirm.RemoveSUBST)
			{
				if (Message(MSG_WARNING,2,
					MSG(MChangeSUBSTDisconnectDriveTitle),
					LangString(MChangeSUBSTDisconnectDriveQuestion) << DiskLetter,
					MSG(MYes),MSG(MNo)))
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
				LangString strMsgText(MChangeDriveCannotDelSubst);
				strMsgText << DiskLetter;
				DWORD LastError=GetLastError();
				if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
				{
					if (!Message(MSG_WARNING|MSG_ERRORTYPE, 2,
						MSG(MError),
						strMsgText,
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
				Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText,MSG(MOk));
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
				LockScreen LckScr;
				// если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
				// удалению
				IfGoHome(Drive);
				FrameManager->ResizeAllFrame();
				FrameManager->GetCurrentFrame()->Show();
				// </КОСТЫЛЬ>

				if (WNetCancelConnection2(DiskLetter,UpdateProfile,FALSE)==NO_ERROR)
				{
					return DRIVE_DEL_SUCCESS;
				}
				else
				{
					LangString strMsgText(MChangeDriveCannotDisconnect);
					strMsgText << DiskLetter;
					DWORD LastError=GetLastError();
					if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
					{
						if (!Message(MSG_WARNING|MSG_ERRORTYPE, 2,
							MSG(MError),
							strMsgText,
							L"\x1",
							MSG(MChangeDriveOpenFiles),
							MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel)))
						{
							if (WNetCancelConnection2(DiskLetter,UpdateProfile,TRUE)==NO_ERROR)
							{
								return DRIVE_DEL_SUCCESS;
							}
						}
						else
						{
							return DRIVE_DEL_FAIL;
						}
					}
					const wchar_t RootDir[]={*DiskLetter,L':',L'\\',L'\0'};
					if (FAR_GetDriveType(RootDir)==DRIVE_REMOTE)
					{
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText,MSG(MOk));
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
				if (Message(MSG_WARNING, 2,
					MSG(MChangeVHDDisconnectDriveTitle),
					LangString(MChangeVHDDisconnectDriveQuestion) << DiskLetter,
					MSG(MYes),MSG(MNo)))
				{
					return DRIVE_DEL_FAIL;
				}
			}
			string strVhdPath;
			if(GetVHDName(DiskLetter, strVhdPath) && !strVhdPath.IsEmpty())
			{
				VIRTUAL_STORAGE_TYPE vst = {VIRTUAL_STORAGE_TYPE_DEVICE_VHD, VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT};
				OPEN_VIRTUAL_DISK_PARAMETERS ovdp = {OPEN_VIRTUAL_DISK_VERSION_1, 0};
				HANDLE Handle;
				if(apiOpenVirtualDisk(vst, strVhdPath, VIRTUAL_DISK_ACCESS_DETACH, OPEN_VIRTUAL_DISK_FLAG_NONE, ovdp, Handle))
				{
					int Result = Global->ifn->DetachVirtualDisk(Handle, DETACH_VIRTUAL_DISK_FLAG_NONE, 0) == ERROR_SUCCESS? DRIVE_DEL_SUCCESS : DRIVE_DEL_FAIL;
					CloseHandle(Handle);
					return Result;
				}
			}
		}
		break;

	}
	return DRIVE_DEL_FAIL;
}


void Panel::FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName,string &strName)
{
	wchar_t_ptr Buffer(StrLength(Src)+StrLength(FindEdit->GetStringAddr()) + 1);

	if (Buffer)
	{
		auto Ptr = Buffer.get();
		wcscpy(Ptr,FindEdit->GetStringAddr());
		wchar_t *EndPtr=Ptr+StrLength(Ptr);
		wcscat(Ptr,Src);
		Unquote(EndPtr);
		EndPtr=Ptr+StrLength(Ptr);
		for (;;)
		{
			if (EndPtr == Ptr)
			{
				break;
			}

			if (FindPartName(Ptr,FALSE,1,1))
			{
				*EndPtr=0;
				FindEdit->SetString(Ptr);
				strLastName = Ptr;
				strName = Ptr;
				FindEdit->Show();
				break;
			}

			*--EndPtr=0;
		}
	}
}

__int64 Panel::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return 0;
}

// корректировка букв
static DWORD _CorrectFastFindKbdLayout(INPUT_RECORD *rec,DWORD Key)
{
	if ((Key&(KEY_ALT|KEY_RALT)))// && Key!=(KEY_ALT|0x3C))
	{
		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
		if (rec->Event.KeyEvent.uChar.UnicodeChar && (Key&KEY_MASKF) != rec->Event.KeyEvent.uChar.UnicodeChar) //???
			Key=(Key&0xFFF10000)|rec->Event.KeyEvent.uChar.UnicodeChar;   //???

		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
	}

	return Key;
}

void Panel::FastFind(int FirstKey)
{
	// // _SVS(CleverSysLog Clev(L"Panel::FastFind"));
	INPUT_RECORD rec;
	string strLastName, strName;
	int KeyToProcess=0;
	Global->WaitInFastFind++;
	{
		int FindX=std::min(X1+9,ScrX-22);
		int FindY=std::min(Y2,static_cast<SHORT>(ScrY-2));
		ChangeMacroMode MacroMode(MACRO_SEARCH);
		SaveScreen SaveScr(FindX,FindY,FindX+21,FindY+2);
		FastFindShow(FindX,FindY);
		EditControl FindEdit(this);
		FindEdit.SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
		FindEdit.SetEditBeyondEnd(FALSE);
		FindEdit.SetObjectColor(COL_DIALOGEDIT);
		FindEdit.Show();

		while (!KeyToProcess)
		{
			int Key;
			if (FirstKey)
			{
				FirstKey=_CorrectFastFindKbdLayout(FrameManager->GetLastInputRecord(),FirstKey);
				// // _SVS(SysLog(L"Panel::FastFind  FirstKey=%s  %s",_FARKEY_ToName(FirstKey),_INPUT_RECORD_Dump(FrameManager->GetLastInputRecord())));
				// // _SVS(SysLog(L"if (FirstKey)"));
				Key=FirstKey;
			}
			else
			{
				// // _SVS(SysLog(L"else if (FirstKey)"));
				Key=GetInputRecord(&rec);

				if (rec.EventType==MOUSE_EVENT)
				{
					if (!(rec.Event.MouseEvent.dwButtonState & 3))
						continue;
					else
						Key=KEY_ESC;
				}
				else if (!rec.EventType || rec.EventType==KEY_EVENT || rec.EventType==FARMACRO_KEY_EVENT)
				{
					// для вставки воспользуемся макродвижком...
					if (Key==KEY_CTRLV || Key==KEY_RCTRLV || Key==KEY_SHIFTINS || Key==KEY_SHIFTNUMPAD0)
					{
						wchar_t *ClipText=PasteFromClipboard();

						if (ClipText)
						{
							if (*ClipText)
							{
								FastFindProcessName(&FindEdit,ClipText,strLastName,strName);
								FastFindShow(FindX,FindY);
							}

							delete[] ClipText;
						}

						continue;
					}
					else if (Key == KEY_OP_XLAT)
					{
						string strTempName;
						FindEdit.Xlat();
						FindEdit.GetString(strTempName);
						FindEdit.SetString(L"");
						FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
						FastFindShow(FindX,FindY);
						continue;
					}
					else if (Key == KEY_OP_PLAINTEXT)
					{
						string strTempName;
						FindEdit.ProcessKey(Key);
						FindEdit.GetString(strTempName);
						FindEdit.SetString(L"");
						FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
						FastFindShow(FindX,FindY);
						continue;
					}
					else
						Key=_CorrectFastFindKbdLayout(&rec,Key);
				}
			}

			if (Key==KEY_ESC || Key==KEY_F10)
			{
				KeyToProcess=KEY_NONE;
				break;
			}

			// // _SVS(if (!FirstKey) SysLog(L"Panel::FastFind  Key=%s  %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(&rec)));
			if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALT_BASE));
			else if (Key>=KEY_RALT_BASE+0x01 && Key<=KEY_RALT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_RALT_BASE));

			if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALTSHIFT_BASE));
			else if (Key>=KEY_RALTSHIFT_BASE+0x01 && Key<=KEY_RALTSHIFT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_RALTSHIFT_BASE));

			if (Key==KEY_MULTIPLY)
				Key=L'*';

			switch (Key)
			{
				case KEY_F1:
				{
					FindEdit.Hide();
					SaveScr.RestoreArea();
					{
						Help Hlp(L"FastFind");
					}
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				}
				case KEY_CTRLNUMENTER:   case KEY_RCTRLNUMENTER:
				case KEY_CTRLENTER:      case KEY_RCTRLENTER:
					FindPartName(strName,TRUE,1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_CTRLSHIFTNUMENTER:  case KEY_RCTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:     case KEY_RCTRLSHIFTENTER:
					FindPartName(strName,TRUE,-1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_NONE:
				case KEY_IDLE:
					break;
				default:

					if ((Key<32 || Key>=65536) && Key!=KEY_BS && Key!=KEY_CTRLY && Key!=KEY_RCTRLY &&
					        Key!=KEY_CTRLBS && Key!=KEY_RCTRLBS && Key!=KEY_ALT && Key!=KEY_SHIFT &&
					        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
					        !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && // KEY_RCTRLINS/NUMPAD0 passed to panels
							!(Key==KEY_SHIFTINS||Key==KEY_SHIFTNUMPAD0))
					{
						KeyToProcess=Key;
						break;
					}

					if (FindEdit.ProcessKey(Key))
					{
						FindEdit.GetString(strName);

						// уберем двойные '**'
						if (strName.GetLength() > 1
						        && strName.At(strName.GetLength()-1) == L'*'
						        && strName.At(strName.GetLength()-2) == L'*')
						{
							strName.SetLength(strName.GetLength()-1);
							FindEdit.SetString(strName);
						}

						/* $ 09.04.2001 SVS
						   проблемы с быстрым поиском.
						   Подробнее в 00573.ChangeDirCrash.txt
						*/
						if (strName.At(0) == L'"')
						{
							strName.LShift(1);
							FindEdit.SetString(strName);
						}

						if (FindPartName(strName,FALSE,1,1))
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

							FindEdit.SetString(strLastName);
							strName = strLastName;
						}

						FindEdit.Show();
						FastFindShow(FindX,FindY);
					}

					break;
			}

			FirstKey=0;
		}
	}
	Global->WaitInFastFind--;
	Show();
	Global->CtrlObject->MainKeyBar->Redraw();
	Global->ScrBuf->Flush();
	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;

	if ((KeyToProcess==KEY_ENTER||KeyToProcess==KEY_NUMENTER) && ActivePanel->GetType()==TREE_PANEL)
		((TreeList *)ActivePanel)->ProcessEnter();
	else
		Global->CtrlObject->Cp()->ProcessKey(KeyToProcess);
}


void Panel::FastFindShow(int FindX,int FindY)
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(FindX+1,FindY+1);
	Text(L" ");
	GotoXY(FindX+20,FindY+1);
	Text(L" ");
	Box(FindX,FindY,FindX+21,FindY+2,ColorIndexToColor(COL_DIALOGBOX),DOUBLE_BOX);
	GotoXY(FindX+7,FindY);
	SetColor(COL_DIALOGBOXTITLE);
	Text(MSearchFileTitle);
}


void Panel::SetFocus()
{
	if (Global->CtrlObject->Cp()->ActivePanel!=this)
	{
		Global->CtrlObject->Cp()->ActivePanel->KillFocus();
		Global->CtrlObject->Cp()->ActivePanel=this;
	}

	ProcessPluginEvent(FE_GOTFOCUS,nullptr);

	if (!GetFocus())
	{
		Global->CtrlObject->Cp()->RedrawKeyBar();
		Focus = true;
		Redraw();
		FarChDir(strCurDir);
	}
}


void Panel::KillFocus()
{
	Focus = false;
	ProcessPluginEvent(FE_KILLFOCUS,nullptr);
	Redraw();
}


int  Panel::PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
{
	RetCode=TRUE;

	if (!ModalMode && !MouseEvent->dwMousePosition.Y)
	{
		if (MouseEvent->dwMousePosition.X==ScrX)
		{
			if (Global->Opt->ScreenSaver && !(MouseEvent->dwButtonState & 3))
			{
				EndDrag();
				ScreenSaver(TRUE);
				return TRUE;
			}
		}
		else
		{
			if ((MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
			{
				EndDrag();

				if (!MouseEvent->dwMousePosition.X)
					Global->CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
				else
					ShellOptions(0,MouseEvent);

				return TRUE;
			}
		}
	}

	if (!IsVisible() ||
	        (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	         MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2))
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
				SrcDragPanel->ProcessKey(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY);
			}

			return TRUE;
		}

		if (MouseEvent->dwMousePosition.Y<=Y1 || MouseEvent->dwMousePosition.Y>=Y2 ||
		        !Global->CtrlObject->Cp()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return TRUE;
		}

		if ((MouseEvent->dwButtonState & 2) && !MouseEvent->dwEventFlags)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & 1)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
			        !ModalMode)
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
	        X2-X1<ScrX)
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


int  Panel::IsDragging()
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
	size_t SelCount;
	int MsgX,Length;

	if (!(SelCount=SrcDragPanel->GetSelCount()))
		return;

	if (SelCount==1)
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


	if ((Length=(int)strDragMsg.GetLength())+X>ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			TruncStrFromEnd(strDragMsg,ScrX);
			Length=(int)strDragMsg.GetLength();
		}
	}
	else
		MsgX=X;

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	delete DragSaveScr;
	DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(strDragMsg);
}


const string& Panel::GetCurDir()
{
	return strCurDir; // TODO: ОПАСНО!!!
}


BOOL Panel::SetCurDir(const string& CurDir,int ClosePanel,BOOL /*IsUpdated*/)
{
	InitCurDir(CurDir);
	return TRUE;
}


void Panel::InitCurDir(const wchar_t *CurDir)
{
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (PanelMode!=PLUGIN_PANEL)
		{
			PrepareDiskPath(strCurDir);
			if(!IsRootPath(strCurDir))
			{
				DeleteEndSlash(strCurDir);
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

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=PLUGIN_PANEL)
	{
		if (IsAlpha(AnotherPanel->strCurDir.At(0)) && AnotherPanel->strCurDir.At(1)==L':' &&
		        Upper(AnotherPanel->strCurDir.At(0))!=Upper(strCurDir.At(0)))
		{
			// сначала установим переменные окружения для пассивной панели
			// (без реальной смены пути, чтобы лишний раз пассивный каталог
			// не перечитывать)
			FarChDir(AnotherPanel->strCurDir,FALSE);
		}
	}

	if (!FarChDir(strCurDir))
	{
		// здесь на выбор :-)
#if 1

		while (!FarChDir(strCurDir))
		{
			string strRoot;
			GetPathRoot(strCurDir, strRoot);

			if (FAR_GetDriveType(strRoot) != DRIVE_REMOVABLE || apiIsDiskInDrive(strRoot))
			{
				int Result=TestFolder(strCurDir);

				if (Result == TSTFLD_NOTFOUND)
				{
					if (CheckShortcutFolder(&strCurDir, FALSE, TRUE) && FarChDir(strCurDir))
					{
						SetCurDir(strCurDir,TRUE);
						return TRUE;
					}
				}
				else
					break;
			}

			if (FrameManager && FrameManager->ManagerStarted()) // сначала проверим - а запущен ли менеджер
			{
				SetCurDir(Global->g_strFarPath,TRUE);                    // если запущен - выставим путь который мы точно знаем что существует
				ChangeDisk();                                    // и вызовем меню выбора дисков
			}
			else                                               // оппа...
			{
				string strTemp(strCurDir);
				CutToFolderNameIfFolder(strCurDir);             // подымаемся вверх, для очередной порции ChDir

				if (strTemp.GetLength()==strCurDir.GetLength())  // здесь проблема - видимо диск недоступен
				{
					SetCurDir(Global->g_strFarPath,TRUE);                 // тогда просто сваливаем в каталог, откуда стартанул FAR.
					break;
				}
				else
				{
					if (FarChDir(strCurDir))
					{
						SetCurDir(strCurDir,TRUE);
						break;
					}
				}
			}
		}

#else

		do
		{
			BOOL IsChangeDisk=FALSE;
			char Root[1024];
			GetPathRoot(CurDir,Root);

			if (FAR_GetDriveType(Root) == DRIVE_REMOVABLE && !apiIsDiskInDrive(Root))
				IsChangeDisk=TRUE;
			else if (TestFolder(CurDir) == TSTFLD_NOTACCESS)
			{
				if (FarChDir(Root))
					SetCurDir(Root,TRUE);
				else
					IsChangeDisk=TRUE;
			}

			if (IsChangeDisk)
				ChangeDisk();
		}
		while (!FarChDir(CurDir));

#endif
		return FALSE;
	}

	return TRUE;
}


void Panel::Hide()
{
	ScreenObject::Hide();
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

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

	DelayDestroy dd(this);

	/* $ 03.10.2001 IS перерисуем строчку меню */
	if (Global->Opt->ShowMenuBar)
		Global->CtrlObject->TopMenuBar->Show();

	/* $ 09.05.2001 OT */
//  SavePrevScreen();
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible() && !GetModalMode())
	{
		if (SaveScr)
		{
			SaveScr->AppendArea(AnotherPanel->SaveScr);
		}

		if (AnotherPanel->GetFocus())
		{
			if (AnotherPanel->IsFullScreen())
			{
				SetVisible(TRUE);
				return;
			}

			if (GetType()==FILE_PANEL && IsFullScreen())
			{
				ScreenObject::Show();
				AnotherPanel->Show();
				return;
			}
		}
	}

	ScreenObject::Show();
	if (!this->Destroyed())
		ShowScreensCount();
}


void Panel::DrawSeparator(int Y)
{
	if (Y<Y2)
	{
		SetColor(COL_PANELBOX);
		GotoXY(X1,Y);
		ShowSeparator(X2-X1+1,1);
	}
}


void Panel::ShowScreensCount()
{
	if (Global->Opt->ShowScreensNumber && !X1)
	{
		int Viewers=FrameManager->GetFrameCountByType(MODALTYPE_VIEWER);
		int Editors=FrameManager->GetFrameCountByType(MODALTYPE_EDITOR);
		int Dialogs=FrameManager->GetFrameCountByType(MODALTYPE_DIALOG);

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			GotoXY(Global->Opt->ShowColumnTitles ? X1:X1+2,Y1);
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
		string strTitleDir(L"{");

		if (!strCurDir.IsEmpty())
		{
			strTitleDir += strCurDir;
		}
		else
		{
			string strCmdText;
			Global->CtrlObject->CmdLine->GetCurDir(strCmdText);
			strTitleDir += strCmdText;
		}

		strTitleDir += L"}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

string &Panel::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
	string strTitleDir;
	bool truncTitle = (SubLen==-1 || TruncSize==0)?false:true;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		GetOpenPanelInfo(&Info);
		strTitleDir = Info.PanelTitle;
		RemoveExternalSpaces(strTitleDir);
		if (truncTitle)
			TruncStr(strTitleDir,SubLen-TruncSize);
	}
	else
	{
		if (ShowShortNames)
			ConvertNameToShort(strCurDir,strTitleDir);
		else
			strTitleDir = strCurDir;

		if (truncTitle)
			TruncPathStr(strTitleDir,SubLen-TruncSize);
	}

	strTitle = L" "+strTitleDir+L" ";
	return strTitle;
}

int Panel::SetPluginCommand(int Command,int Param1,void* Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;
	FilePanels *FPanels=Global->CtrlObject->Cp();

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result=FPanels->ChangePanelViewMode(this,Param1,FPanels->IsTopFrame());
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
			ChangeSortOrder(Param1?-1:1);
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
			PluginCommand=Command;
			strPluginParam = (const wchar_t *)Param2;
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
			{
				static struct
				{
					BoolOption *Opt;
					DWORD Flags;
				} PFLAGS[]=
				{
					{&Global->Opt->ShowHidden,PFLAGS_SHOWHIDDEN},
					{&Global->Opt->Highlight,PFLAGS_HIGHLIGHT},
				};
				unsigned __int64 Flags=0;

				for (size_t I=0; I < ARRAYSIZE(PFLAGS); ++I)
					if (*PFLAGS[I].Opt)
						Flags|=PFLAGS[I].Flags;

				Flags|=GetSortOrder()<0?PFLAGS_REVERSESORTORDER:0;
				Flags|=GetSortGroups()?PFLAGS_USESORTGROUPS:0;
				Flags|=GetSelectedFirstMode()?PFLAGS_SELECTEDFIRST:0;
				Flags|=GetDirectoriesFirst()?PFLAGS_DIRECTORIESFIRST:0;
				Flags|=GetNumericSort()?PFLAGS_NUMERICSORT:0;
				Flags|=GetCaseSensitiveSort()?PFLAGS_CASESENSITIVESORT:0;
				Flags|=(GetMode()==PLUGIN_PANEL)?PFLAGS_PLUGIN:0;
				Flags|=IsVisible()?PFLAGS_VISIBLE:0;
				Flags|=GetFocus()?PFLAGS_FOCUS:0;

				if (Global->CtrlObject->Cp()->LeftPanel == this)
					Flags|=PFLAGS_PANELLEFT;

				Info->Flags=Flags;
			}

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (Info->Flags&PFLAGS_PLUGIN)
				{
					Info->OwnerGuid = static_cast<PluginHandle*>(DestFilePanel->GetPluginHandle())->pPlugin->GetGUID();
					Info->PluginHandle = static_cast<PluginHandle*>(DestFilePanel->GetPluginHandle())->hPlugin;
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
					strTemp = PInfo.CommandPrefix;
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp,Param1);

			Result=(int)strTemp.GetLength()+1;
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
							strTemp=PInfo.HostFile;
							break;
						case FCTL_GETPANELFORMAT:
							strTemp=PInfo.Format;
							break;
					}

					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp,Param1);

			Result=(int)strTemp.GetLength()+1;
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
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.ShortcutFolder.GetLength()+1));
				size_t pluginFileOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginFile.GetLength()+1));
				size_t pluginDataOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginData.GetLength()+1));
				FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
				if(Param1>=Result && CheckStructSize(dirInfo))
				{
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					dirInfo->PluginId=Info.PluginGuid;
					dirInfo->Name=(wchar_t*)((char*)Param2+folderOffset);
					dirInfo->Param=(wchar_t*)((char*)Param2+pluginDataOffset);
					dirInfo->File=(wchar_t*)((char*)Param2+pluginFileOffset);
					wmemcpy((wchar_t*)dirInfo->Name,Info.ShortcutFolder,Info.ShortcutFolder.GetLength()+1);
					wmemcpy((wchar_t*)dirInfo->Param,Info.PluginData,Info.PluginData.GetLength()+1);
					wmemcpy((wchar_t*)dirInfo->File,Info.PluginFile,Info.PluginFile.GetLength()+1);
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
						xwcsncpy((wchar_t*)Param2,strColumnTypes,Param1);

					Result=(int)strColumnTypes.GetLength()+1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnWidths,Param1);

					Result=(int)strColumnWidths.GetLength()+1;
				}
			}
			break;

		case FCTL_GETPANELITEM:
		{
			Result=CheckNullOrStructSize((FarGetPluginPanelItem*)Param2)?(int)((FileList*)this)->PluginGetPanelItem(Param1,(FarGetPluginPanelItem*)Param2):0;
			break;
		}

		case FCTL_GETSELECTEDPANELITEM:
		{
			Result=CheckNullOrStructSize((FarGetPluginPanelItem*)Param2)?(int)((FileList*)this)->PluginGetSelectedPanelItem(Param1,(FarGetPluginPanelItem*)Param2):0;
			break;
		}

		case FCTL_GETCURRENTPANELITEM:
		{
			PanelInfo Info;
			FileList *DestPanel = ((FileList*)this);
			DestPanel->PluginGetPanelInfo(Info);
			Result = CheckNullOrStructSize((FarGetPluginPanelItem*)Param2)?(int)DestPanel->PluginGetPanelItem(static_cast<int>(Info.CurrentItem),(FarGetPluginPanelItem*)Param2):0;
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
				((FileList *)this)->PluginSetSelection(Param1,Param2?true:false);
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
				CurFile=static_cast<int>(Info->CurrentItem);
				CurTopFile=static_cast<int>(Info->TopPanelItem);
			}

			// $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущий фрейм
			if (FPanels->IsTopFrame())
				Redraw();

			Result=TRUE;
			break;
		}

		case FCTL_SETPANELDIRECTORY:
		{
			FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
			if (CheckStructSize(dirInfo))
			{
				string strName(dirInfo->Name),strFile(dirInfo->File),strParam(dirInfo->Param);
				Result = ExecShortcutFolder(strName,dirInfo->PluginId,strFile,strParam,false);
				// restore current directory to active panel path
				Panel* ActivePanel = Global->CtrlObject->Cp()->ActivePanel;
				if (Result && this != ActivePanel)
				{
					ActivePanel->SetCurPath();
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


int Panel::GetCurName(string &strName, string &strShortName)
{
	strName.Clear();
	strShortName.Clear();
	return FALSE;
}


int Panel::GetCurBaseName(string &strName, string &strShortName)
{
	strName.Clear();
	strShortName.Clear();
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
	MakeDialogItemsEx(DCDlgData,DCDlg);

	LangString strMsgText;

	strMsgText = MChangeDriveDisconnectQuestion;
	strMsgText << Letter;
	DCDlg[1].strData = strMsgText;

	strMsgText = MChangeDriveDisconnectMapped;
	strMsgText << Letter;
	DCDlg[2].strData = strMsgText;

	size_t Len1 = DCDlg[0].strData.GetLength();
	size_t Len2 = DCDlg[1].strData.GetLength();
	size_t Len3 = DCDlg[2].strData.GetLength();
	size_t Len4 = DCDlg[5].strData.GetLength();
	Len1 = std::max(Len1,std::max(Len2,std::max(Len3,Len4)));
	DriveLocalToRemoteName(DRIVE_REMOTE,Letter,strMsgText);
	DCDlg[3].strData = TruncPathStr(strMsgText, static_cast<int>(Len1));
	// проверяем - это было постоянное соедение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.
	BOOL IsPersistent = FALSE;
	{
		HKEY hKey;
		IsPersistent=TRUE;
		string KeyName(L"Network\\");
		KeyName+=Letter;

		if (RegOpenKeyEx(HKEY_CURRENT_USER,KeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		{
			DCDlg[5].Flags|=DIF_DISABLE;
			DCDlg[5].Selected=0;
			IsPersistent=FALSE;
			RegCloseKey(hKey);
		}
		else
			DCDlg[5].Selected=Global->Opt->ChangeDriveDisconnectMode;
	}
	// скорректируем размеры диалога - для дизайнУ
	DCDlg[0].X2=DCDlg[0].X1+static_cast<int>(Len1)+3;
	int ExitCode=7;

	if (Global->Opt->Confirm.RemoveConnection)
	{
		Dialog Dlg(DCDlg,ARRAYSIZE(DCDlg));
		Dlg.SetPosition(-1,-1,DCDlg[0].X2+4,11);
		Dlg.SetHelp(L"DisconnectDrive");
		Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Global->Opt->ChangeDriveDisconnectMode=DCDlg[5].Selected == BSTATE_CHECKED;

	return ExitCode == 7;
}

BOOL Panel::NeedUpdatePanel(Panel *AnotherPanel)
{
	/* Обновить, если обновление разрешено и пути совпадают */
	if ((!Global->Opt->AutoUpdateLimit || static_cast<unsigned>(GetFileCount()) <= static_cast<unsigned>(Global->Opt->AutoUpdateLimit)) &&
	        !StrCmpI(AnotherPanel->strCurDir, strCurDir))
		return TRUE;

	return FALSE;
}

bool Panel::GetShortcutInfo(ShortcutInfo& ShortcutInfo)
{
	bool result=true;
	if (PanelMode==PLUGIN_PANEL)
	{
		HANDLE hPlugin=GetPluginHandle();
		PluginHandle *ph = (PluginHandle*)hPlugin;
		ShortcutInfo.PluginGuid = ph->pPlugin->GetGUID();
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		ShortcutInfo.PluginFile = Info.HostFile;
		ShortcutInfo.ShortcutFolder = Info.CurDir;
		ShortcutInfo.PluginData = Info.ShortcutData;
		if(!(Info.Flags&OPIF_SHORTCUT)) result=false;
	}
	else
	{
		ShortcutInfo.PluginGuid=FarGuid;
		ShortcutInfo.PluginFile.Clear();
		ShortcutInfo.PluginData.Clear();
		ShortcutInfo.ShortcutFolder = strCurDir;
	}
	return result;
}

bool Panel::SaveShortcutFolder(int Pos, bool Add)
{
	ShortcutInfo Info;
	if(GetShortcutInfo(Info))
	{
		if(Add)
		{
			Global->CtrlObject->FolderShortcuts->Add(Pos, Info.ShortcutFolder, Info.PluginGuid, Info.PluginFile, Info.PluginData);
		}
		else
		{
			Global->CtrlObject->FolderShortcuts->Set(Pos, Info.ShortcutFolder, Info.PluginGuid, Info.PluginFile, Info.PluginData);
		}
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
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (ProcTreePanel)
		{
			if (AnotherPanel->GetType()==FILE_PANEL)
			{
				AnotherPanel->SetCurDir(strShortcutFolder,TRUE);
				AnotherPanel->Redraw();
			}
			else
			{
				SetCurDir(strShortcutFolder,TRUE);
				ProcessKey(KEY_ENTER);
			}
		}
		else
		{
			if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.IsEmpty())
			{
				AnotherPanel->SetCurDir(strShortcutFolder,TRUE);
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

	if (Global->CtrlObject->FolderShortcuts->Get(Pos,&strShortcutFolder, &PluginGuid, &strPluginFile, &strPluginData, raw))
	{
		return ExecShortcutFolder(strShortcutFolder,PluginGuid,strPluginFile,strPluginData,true);
	}
	return false;
}

bool Panel::ExecShortcutFolder(string& strShortcutFolder,const GUID& PluginGuid,string& strPluginFile,const string& strPluginData,bool CheckType)
{
	Panel *SrcPanel=this;
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

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
		switch (CheckShortcutFolder(nullptr,TRUE))
		{
			case 0:
				//              return FALSE;
			case -1:
				return true;
		}

		Plugin *pPlugin = Global->CtrlObject->Plugins->FindPlugin(PluginGuid);

		if (pPlugin)
		{
			if (pPlugin->HasOpen())
			{
				if (!strPluginFile.IsEmpty())
				{
					string strRealDir;
					strRealDir = strPluginFile;

					if (CutToSlash(strRealDir))
					{
						SrcPanel->SetCurDir(strRealDir,TRUE);
						SrcPanel->GoToFile(PointToName(strPluginFile));

						SrcPanel->ClearAllItem();
					}
				}

				OpenShortcutInfo info=
				{
					sizeof(OpenShortcutInfo),
					strPluginFile.IsEmpty()?nullptr:strPluginFile.CPtr(),
					strPluginData.IsEmpty()?nullptr:strPluginData.CPtr(),
					(SrcPanel==Global->CtrlObject->Cp()->ActivePanel)?FOSF_ACTIVE:FOSF_NONE
				};
				HANDLE hNewPlugin=Global->CtrlObject->Plugins->Open(pPlugin,OPEN_SHORTCUT,FarGuid,(intptr_t)&info);

				if (hNewPlugin)
				{
					int CurFocus=SrcPanel->GetFocus();

					Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
					NewPanel->SetPluginMode(hNewPlugin,L"",CurFocus || !Global->CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());

					if (!strShortcutFolder.IsEmpty())
					{
						struct UserDataItem UserData={0}; //????
						Global->CtrlObject->Plugins->SetDirectory(hNewPlugin,strShortcutFolder,0,&UserData);
					}

					NewPanel->Update(0);
					NewPanel->Show();
				}
			}
		}

		/*
		if(I == Global->CtrlObject->Plugins->PluginsCount)
		{
		  char Target[NM*2];
		  xstrncpy(Target, PluginModule, sizeof(Target));
		  TruncPathStr(Target, ScrX-16);
		  Message (MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), Target, MSG (MNeedNearPath), MSG(MOk))
		}
		*/

		return true;
	}

	switch (CheckShortcutFolder(&strShortcutFolder,FALSE))
	{
		case 0:
			//          return FALSE;
		case -1:
			return true;
	}

    /*
	if (SrcPanel->GetType()!=FILE_PANEL)
	{
		SrcPanel=Global->CtrlObject->Cp()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
	}
    */

	SrcPanel->SetCurDir(strShortcutFolder,TRUE);

	if (CheckFullScreen!=SrcPanel->IsFullScreen())
		Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->Show();

	SrcPanel->Redraw();
	return true;
}

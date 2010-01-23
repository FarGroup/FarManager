/*
panel.cpp

Parent class для панелей
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
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
#include "array.hpp"
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
#include "ffolders.hpp"
#include "options.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "imports.hpp"
#include "constitle.hpp"
#include "FarDlgBuilder.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=NULL;
static string strDragName;

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
		int operator<(const ChDiskPluginItem &rhs) const;
		const ChDiskPluginItem& operator=(const ChDiskPluginItem &rhs);
};

int ChDiskPluginItem::operator<(const ChDiskPluginItem &rhs) const
{
	if (HotKey==rhs.HotKey)
		return StrCmpI(Item.strName,rhs.Item.strName)<0;

	if (HotKey && rhs.HotKey)
		return HotKey < rhs.HotKey;

	return HotKey && !rhs.HotKey;
}

const ChDiskPluginItem& ChDiskPluginItem::operator=(const ChDiskPluginItem &rhs)
{
	if (this != &rhs)
	{
		Item=rhs.Item;
		HotKey=rhs.HotKey;
	}

	return *this;
}


Panel::Panel()
{
	_OT(SysLog(L"[%p] Panel::Panel()", this));
	Focus=0;
	NumericSort=0;
	PanelMode=NORMAL_PANEL;
	PrevViewMode=VIEW_3;
	EnableUpdate=TRUE;
	DragX=DragY=-1;
	SrcDragPanel=NULL;
	ModalMode=0;
	ViewSettings.ColumnCount=0;
	ViewSettings.FullScreen=0;
	ProcessingPluginCommand=0;
};


Panel::~Panel()
{
	_OT(SysLog(L"[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	PrevViewMode=ViewMode;
	Panel::ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	string strNewDir;
	apiGetCurrentDirectory(strNewDir);
	SetCurDir(strNewDir,TRUE);
}


void Panel::ChangeDisk()
{
	int Pos,FirstCall=TRUE;

	if (!strCurDir.IsEmpty() && strCurDir.At(1)==L':')
		Pos=Upper(strCurDir.At(0))-L'A';
	else
		Pos=getdisk();

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
			int nItem;
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
	int FarMsg;
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
	{DRIVE_RAMDISK,MChangeDriveRAM},
};

static int AddPluginItems(VMenu &ChDisk, int Pos, int DiskCount, bool ShowSpecial, bool SetSelected)
{
	int UsedNumbers[10]={0};
	TArray<ChDiskPluginItem> MPItems, MPItemsNoHotkey;
	ChDiskPluginItem OneItem;
	// Список дополнительных хоткеев, для случая, когда плагинов, добавляющих пункт в меню, больше 9.
	const wchar_t *AdditionalHotKey=MSG(MAdditionalHotKey);
	int AHKPos = 0; // индекс в списке хоткеев
	int AHKSize = StrLength(AdditionalHotKey); // для предотвращения выхода за границу массива
	int PluginItem, PluginNumber = 0; // IS: счетчики - плагинов и пунктов плагина
	int PluginTextNumber;
	WCHAR HotKey;
	bool ItemPresent,Done=false;
	string strMenuText;
	string strPluginText;
	int PluginMenuItemsCount = 0;

	while (!Done)
	{
		for (PluginItem=0;; ++PluginItem)
		{
			if (PluginNumber >= CtrlObject->Plugins.GetPluginsCount())
			{
				Done=true;
				break;
			}

			Plugin *pPlugin = CtrlObject->Plugins.GetPlugin(PluginNumber);

			if (!CtrlObject->Plugins.GetDiskMenuItem(
			            pPlugin,
			            PluginItem,
			            ItemPresent,
			            PluginTextNumber,
			            strPluginText
			        ))
			{
				Done=true;
				break;
			}

			if (!ItemPresent)
				break;

			if (!PluginTextNumber)   // IS: автохоткей, назначим потом
				HotKey = WCHAR_MAX; // "-1" -  признак автохоткея
			else
			{
				if (PluginTextNumber < 10)   // IS: хотей указан явно
				{
					// IS: проверим, а не занят ли хоткей
					// IS: если занят, то будем искать его с самого начала - нуля,
					// IS: а не со следующего
					if (UsedNumbers[PluginTextNumber])
					{
						PluginTextNumber=0;

						while (PluginTextNumber<10 && UsedNumbers[PluginTextNumber])
							PluginTextNumber++;
					}

					UsedNumbers[PluginTextNumber%10]=1;
				}

				if (PluginTextNumber < 10)
					HotKey = PluginTextNumber+L'0';
				else
				{
					if (AHKPos < AHKSize)
						HotKey = AdditionalHotKey[AHKPos];
					else
						HotKey = 0;
				}
			}

			strMenuText.Clear();

			if (HotKey==WCHAR_MAX)
				strMenuText = ShowSpecial?strPluginText:L"";
			else
			{
				const wchar_t HotKeyStr[]={L'&',HotKey,L':',L' ',L'\0'};

				if (PluginTextNumber < 10)
				{
					strMenuText=HotKeyStr;

					if (ShowSpecial)
					{
						strMenuText+=strPluginText;
					}
				}
				else
				{
					if (AHKPos<AHKSize)
					{
						strMenuText=HotKeyStr;

						if (ShowSpecial)
						{
							strMenuText+=strPluginText;
						}

						++AHKPos;
					}
					else
					{
						if (ShowSpecial)   // IS: не добавляем пустые строки!
						{
							HotKey=0;
							strMenuText=L"   ";
							strMenuText+=strPluginText;
						}
					}
				}
			}

			if (!strMenuText.IsEmpty() || (HotKey==WCHAR_MAX))
			{
				OneItem.Clear();
				PanelMenuItem *item = new PanelMenuItem;
				item->bIsPlugin = true;
				item->pPlugin = pPlugin;
				item->nItem = PluginItem;

				if (pPlugin->IsOemPlugin())
					OneItem.Item.Flags=LIF_CHECKED|L'A';

				OneItem.Item.strName = strMenuText;
				OneItem.Item.UserDataSize=sizeof(PanelMenuItem);
				OneItem.Item.UserData=(char*)item;
				OneItem.HotKey=HotKey;
				ChDiskPluginItem *pResult = (HotKey==WCHAR_MAX)?MPItemsNoHotkey.addItem(OneItem):MPItems.addItem(OneItem);

				if (pResult)
				{
					pResult->Item.UserData = (char*)item; //BUGBUG, это фантастика просто. Исправить!!!! связано с работой TArray
					pResult->Item.UserDataSize = sizeof(PanelMenuItem);
				}

				/*
				else BUGBUG, а вот это, похоже, лишнее
				{
					Done=TRUE;
					break;
				}
				*/
			}
		} // END: for (PluginItem=0;;++PluginItem)

		++PluginNumber;
	}

	// IS: теперь произведем назначение автохоткеев
	PluginTextNumber=0;

	for (int i=0;; ++i)
	{
		ChDiskPluginItem *item = MPItemsNoHotkey.getItem(i);

		if (!item)
			break;

		if (UsedNumbers[PluginTextNumber])
		{
			while (PluginTextNumber < 10 && UsedNumbers[PluginTextNumber])
				PluginTextNumber++;
		}

		UsedNumbers[PluginTextNumber%10]=1;
		strMenuText.Clear();
		wchar_t HotKeyStr[]={L'&',L' ',L':',L' ',L'\0'};

		if (PluginTextNumber<10)
		{
			item->HotKey=static_cast<WCHAR>(PluginTextNumber+'0');
			HotKeyStr[1]=item->HotKey;
			strMenuText=HotKeyStr;
			strMenuText+=item->Item.strName;
		}
		else
		{
			if (AHKPos < AHKSize)
			{
				item->HotKey=AdditionalHotKey[AHKPos];
				HotKeyStr[1]=item->HotKey;
				strMenuText=HotKeyStr;
				strMenuText+=item->Item.strName;
				++AHKPos;
			}
			else
			{
				if (ShowSpecial)   // IS: не добавляем пустые строки!
				{
					item->HotKey=0;
					strMenuText="   ";
					strMenuText+=item->Item.strName;
				}
			}
		}

		item->Item.strName = strMenuText;
		ChDiskPluginItem *pResult = NULL;

		if (!item->Item.strName.IsEmpty() && ((pResult = MPItems.addItem(*item)) != NULL))
		{
			pResult->Item.UserData = (char*)item->Item.UserData; //BUGBUG, это фантастика просто. Исправить!!!! связано с работой TArray
			pResult->Item.UserDataSize = item->Item.UserDataSize;
		}
	}

	MPItems.Sort();
	MPItems.Pack(); // выкинем дубли
	PluginMenuItemsCount=MPItems.getSize();

	if (PluginMenuItemsCount)
	{
		MenuItemEx ChDiskItem;

		ChDiskItem.Clear();
		ChDiskItem.Flags|=LIF_SEPARATOR;
		ChDiskItem.UserDataSize=0;
		ChDisk.AddItem(&ChDiskItem);
		ChDiskItem.Flags&=~LIF_SEPARATOR;

		for (int I=0; I < PluginMenuItemsCount; ++I)
		{
			if (Pos > DiskCount && !SetSelected)
			{
				MPItems.getItem(I)->Item.SetSelect(DiskCount+I+1==Pos);

				if (!SetSelected)
					SetSelected=DiskCount+I+1==Pos;
			}

			ChDisk.AddItem(&MPItems.getItem(I)->Item);
			delete(PanelMenuItem*)MPItems.getItem(I)->Item.UserData;  //ммда...
		}
	}
	return PluginMenuItemsCount;
}

static void ConfigureChangeDriveMode()
{
	DialogBuilder Builder(MChangeDriveConfigure, L"");
	Builder.AddCheckbox(MChangeDriveShowDiskType, &Opt.ChangeDriveMode, DRIVE_SHOW_TYPE);
	Builder.AddCheckbox(MChangeDriveShowNetworkName, &Opt.ChangeDriveMode, DRIVE_SHOW_NETNAME);
	Builder.AddCheckbox(MChangeDriveShowLabel, &Opt.ChangeDriveMode, DRIVE_SHOW_LABEL);
	Builder.AddCheckbox(MChangeDriveShowFileSystem, &Opt.ChangeDriveMode, DRIVE_SHOW_FILESYSTEM);

	BOOL ShowSizeAny = Opt.ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);
	
	DialogItemEx *ShowSize = Builder.AddCheckbox(MChangeDriveShowSize, &ShowSizeAny);
	DialogItemEx *ShowSizeFloat = Builder.AddCheckbox(MChangeDriveShowSizeFloat, &Opt.ChangeDriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat->Indent(3);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(MChangeDriveShowRemovableDrive, &Opt.ChangeDriveMode, DRIVE_SHOW_REMOVABLE);
	Builder.AddCheckbox(MChangeDriveShowPlugins, &Opt.ChangeDriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(MChangeDriveShowCD, &Opt.ChangeDriveMode, DRIVE_SHOW_CDROM);
	Builder.AddCheckbox(MChangeDriveShowNetworkDrive, &Opt.ChangeDriveMode, DRIVE_SHOW_REMOTE);
		
	Builder.AddOKCancel();
	if (Builder.ShowDialog()) 
	{
		if (ShowSizeAny)
		{
			bool ShowSizeFloat = (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT) ? true : false;
			if (ShowSizeFloat)
				Opt.ChangeDriveMode &= ~DRIVE_SHOW_SIZE;
			else
				Opt.ChangeDriveMode |= DRIVE_SHOW_SIZE;
		}
		else
			Opt.ChangeDriveMode &= ~(DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);
	}
}


int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
	class Guard_Macro_DskShowPosType  //фигня какая-то
	{
		public:
			Guard_Macro_DskShowPosType(Panel *curPanel) {Macro_DskShowPosType=(curPanel==CtrlObject->Cp()->LeftPanel)?1:2;};
			~Guard_Macro_DskShowPosType() {Macro_DskShowPosType=0;};
	};
	Guard_Macro_DskShowPosType _guard_Macro_DskShowPosType(this);
	MenuItemEx ChDiskItem;
	string strDiskType, strRootDir, strDiskLetter;
	DWORD Mask,DiskMask;
	int DiskCount,Focus;
	WCHAR I;
	bool ShowSpecial=false, SetSelected=false;
	Mask = FarGetLogicalDrives();
	DWORD NetworkMask = 0;
	AddSavedNetworkDisks(Mask, NetworkMask);

	for (DiskMask=Mask,DiskCount=0; DiskMask!=0; DiskMask>>=1)
		DiskCount+=DiskMask & 1;

	PanelMenuItem Item, *mitem=0;
	{ // эта скобка надо, см. M#605
		VMenu ChDisk(MSG(MChangeDriveTitle),NULL,0,ScrY-Y1-3);
		ChDisk.SetBottomTitle(MSG(MChangeDriveMenuFooter));
		ChDisk.SetFlags(VMENU_NOTCENTER);

		if (this == CtrlObject->Cp()->LeftPanel)
			ChDisk.SetFlags(VMENU_LEFTMOST);

		ChDisk.SetHelp(L"DriveDlg");
		ChDisk.SetFlags(VMENU_WRAPMODE);
		string strMenuText;
		int DriveType,MenuLine;
		int LabelWidth = Max(11,StrLength(MSG(MChangeDriveLabelAbsent)));
		int DiskTypeWidth=0;

		for (size_t J=0; J < countof(DrTMsg); ++J)
		{
			DiskTypeWidth = Max(DiskTypeWidth,StrLength(MSG(DrTMsg[J].FarMsg)));
		}

		/* $ 02.04.2001 VVM
		! Попытка не будить спящие диски... */
		for (DiskMask=Mask,MenuLine=I=0; DiskMask!=0; DiskMask>>=1,I++)
		{
			if ((DiskMask & 1) == 0)   //нету диска
				continue;

			wchar_t Drv[]={L'&',L'A'+I,L':',L'\\',L'\0'};
			strRootDir=Drv+1;
			Drv[3]=L' ';
			strMenuText=Drv;
			DriveType = FAR_GetDriveType(strRootDir,NULL,Opt.ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if ((1<<I)&NetworkMask)
				DriveType = DRIVE_REMOTE_NOT_CONNECTED;

			if (Opt.ChangeDriveMode & DRIVE_SHOW_TYPE)
			{
				strDiskType.Format(L"%*s",StrLength(MSG(MChangeDriveFixed)),L"");

				for (size_t J=0; J < countof(DrTMsg); ++J)
				{
					if (DrTMsg[J].DrvType == DriveType)
					{
						strDiskType = MSG(DrTMsg[J].FarMsg);
						break;
					}
				}

				const wchar_t LocalName[]={strRootDir.At(0),L':',L'\0'};

				string strSubstName;

				if (GetSubstName(DriveType,LocalName,strSubstName))
				{
					strDiskType = MSG(MChangeDriveSUBST);
					DriveType=DRIVE_SUBSTITUTE;
				}

				strDiskType.Format(L"%-*s",DiskTypeWidth,strDiskType.CPtr());
				strMenuText += strDiskType;
			}

			int ShowDisk = (DriveType!=DRIVE_REMOVABLE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
			               (!IsDriveTypeCDROM(DriveType) || (Opt.ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
			               (!IsDriveTypeRemote(DriveType) || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Opt.ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
			{
				string strVolumeName, strFileSystemName;

				if (ShowDisk && !apiGetVolumeInformation(
				            strRootDir,
				            &strVolumeName,
				            NULL,
				            NULL,
				            NULL,
				            &strFileSystemName
				        ))
				{
					strVolumeName = MSG(MChangeDriveLabelAbsent);
					ShowDisk = FALSE;
				}

				string strTemp;

				if (Opt.ChangeDriveMode & DRIVE_SHOW_LABEL)
				{
					TruncStrFromEnd(strVolumeName,LabelWidth);
					strTemp.Format(L"%c%-*s",BoxSymbols[BS_V1],LabelWidth,(const wchar_t*)strVolumeName);
					strMenuText += strTemp;
				}

				if (Opt.ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
				{
					strTemp.Format(L"%c%-8.8s",BoxSymbols[BS_V1],(const wchar_t*)strFileSystemName);
					strMenuText += strTemp;
				}
			}

			if (Opt.ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				string strTotalText, strFreeText;
				unsigned __int64 TotalSize,TotalFree,UserFree;

				if (ShowDisk && apiGetDiskSize(strRootDir,&TotalSize,&TotalFree,&UserFree))
				{
					if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						//размер как минимум в мегабайтах
						FileSizeToStr(strTotalText,TotalSize,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
						FileSizeToStr(strFreeText,UserFree,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
					}
					else
					{
						//размер с точкой и для 0 добавляем букву размера (B)
						FileSizeToStr(strTotalText,TotalSize,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
						FileSizeToStr(strFreeText,UserFree,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
					}
				}

				string strTemp;
				strTemp.Format(L"%c%-9s%c%-9s",BoxSymbols[BS_V1],(const wchar_t*)strTotalText,BoxSymbols[BS_V1],(const wchar_t*)strFreeText);
				strMenuText += strTemp;
			}

			if (Opt.ChangeDriveMode & DRIVE_SHOW_NETNAME)
			{
				string strRemoteName;
				DriveLocalToRemoteName(DriveType,strRootDir.At(0),strRemoteName);
				//TruncPathStr(strRemoteName,ScrX-(int)strMenuText.GetLength()-12);

				if (!strRemoteName.IsEmpty())
				{
					strMenuText += L"  ";
					strMenuText += strRemoteName;
				}

				ShowSpecial=true;
			}

			ChDiskItem.Clear();

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

			ChDiskItem.strName = strMenuText;

			if (strMenuText.GetLength()>4)
				ShowSpecial=true;

			PanelMenuItem item;
			item.bIsPlugin = false;
			item.cDrive = L'A'+I;
			item.nDriveType = DriveType;
			ChDisk.SetUserData(&item, sizeof(item), ChDisk.AddItem(&ChDiskItem));
			MenuLine++;
		}

		int PluginMenuItemsCount=0;

		if (Opt.ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(ChDisk, Pos, DiskCount, ShowSpecial, SetSelected);
		}

		int X=X1+5;

		if ((this == CtrlObject->Cp()->RightPanel) && IsFullScreen() && (X2-X1 > 40))
			X = (X2-X1+1)/2+5;

		int Y = (ScrY+1-(DiskCount+PluginMenuItemsCount+5))/2;

		if (Y < 1)
			Y=1;

		ChDisk.SetPosition(X,Y,0,0);

		if (Y < 3)
			ChDisk.SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk.Show();

		while (!ChDisk.Done())
		{
			int Key;
			{ //очередная фигня
				ChangeMacroMode MacroMode(MACRO_DISKS);
				Key=ChDisk.ReadInput();
			}
			int SelPos=ChDisk.GetSelectPos();
			PanelMenuItem *item = (PanelMenuItem*)ChDisk.GetUserData(NULL,0);

			switch (Key)
			{
					// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				{
					if (item && !item->bIsPlugin)
					{
						wchar_t DosDeviceName[]={item->cDrive,L':',L'\\',L'\0'};
						Execute(DosDeviceName,FALSE,TRUE,TRUE);
					}
				}
				break;
				case KEY_CTRLPGUP:
				case KEY_CTRLNUMPAD9:
				{
					if (Opt.PgUpChangeDisk)
						return -1;
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
							return SelPos;
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
							ScrBuf.Lock(); // отменяем всякую прорисовку
							FrameManager->ResizeAllFrame();
							FrameManager->PluginCommit(); // коммитим.
							ScrBuf.Unlock(); // разрешаем прорисовку
							return (((DiskCount-SelPos)==1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos-1:SelPos;
						}
					}
				}
				break;
				case KEY_CTRLA:
				case KEY_F4:
				{
					if (item && !item->bIsPlugin)
					{
						wchar_t DosDeviceName[]={item->cDrive,L':',L'\\',L'\0'};
						SHELLEXECUTEINFOW seInfo={sizeof(seInfo)};
						seInfo.nShow = SW_SHOW;
						seInfo.fMask = SEE_MASK_INVOKEIDLIST;
						seInfo.lpFile = DosDeviceName;
						seInfo.lpVerb = L"properties";
						string strCurDir;
						apiGetCurrentDirectory(strCurDir);
						seInfo.lpDirectory=strCurDir;
						ShellExecuteExW(&seInfo);
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
						return SelPos;
					}
				}
				break;
				case KEY_CTRL1:
				case KEY_RCTRL1:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_TYPE;
					return SelPos ;
				case KEY_CTRL2:
				case KEY_RCTRL2:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_NETNAME;
					return SelPos;
				case KEY_CTRL3:
				case KEY_RCTRL3:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_LABEL;
					return SelPos;
				case KEY_CTRL4:
				case KEY_RCTRL4:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_FILESYSTEM;
					return SelPos;
				case KEY_CTRL5:
				case KEY_RCTRL5:
				{
					if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						Opt.ChangeDriveMode ^= DRIVE_SHOW_SIZE;
						Opt.ChangeDriveMode |= DRIVE_SHOW_SIZE_FLOAT;
					}
					else
					{
						if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT)
							Opt.ChangeDriveMode ^= DRIVE_SHOW_SIZE_FLOAT;
						else
							Opt.ChangeDriveMode ^= DRIVE_SHOW_SIZE;
					}

					return SelPos;
				}
				case KEY_CTRL6:
				case KEY_RCTRL6:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_REMOVABLE;
					return SelPos;
				case KEY_CTRL7:
				case KEY_RCTRL7:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_PLUGINS;
					return SelPos;
				case KEY_CTRL8:
				case KEY_RCTRL8:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_CDROM;
					return SelPos;
				case KEY_CTRL9:
				case KEY_RCTRL9:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_REMOTE;
					return SelPos;
				case KEY_F9:
					ConfigureChangeDriveMode();
					return SelPos;
				case KEY_SHIFTF1:
				{
					if (item && item->bIsPlugin)
					{
						// Вызываем нужный топик, который передали в CommandsMenu()
						FarShowHelp(
						    item->pPlugin->GetModuleName(),
						    NULL,
						    FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS
						);
					}

					break;
				}
				case KEY_ALTSHIFTF9:

					if (Opt.ChangeDriveMode&DRIVE_SHOW_PLUGINS)
					{
						ChDisk.Hide();
						CtrlObject->Plugins.Configure();
					}

					return SelPos;
				case KEY_SHIFTF9:

					if (item && item->bIsPlugin && item->pPlugin->HasConfigure())
						CtrlObject->Plugins.ConfigureCurrent(item->pPlugin, item->nItem);

					return SelPos;
				case KEY_CTRLR:
					return SelPos;
				default:
					ChDisk.ProcessInput();
					break;
			}

			if (ChDisk.Done() &&
			        (ChDisk.Modal::GetExitCode() < 0) &&
			        !strCurDir.IsEmpty() &&
			        (StrCmpN(strCurDir,L"\\\\",2) != 0))
			{
				const wchar_t RootDir[4] = {strCurDir.At(0),L':',L'\\',L'\0'};

				if (FAR_GetDriveType(RootDir) == DRIVE_NO_ROOT_DIR)
					ChDisk.ClearDone();
			}
		} // while (!Done)

		if (ChDisk.Modal::GetExitCode()<0)
			return -1;

		mitem=(PanelMenuItem*)ChDisk.GetUserData(NULL,0);

		if (mitem)
		{
			Item=*mitem;
			mitem=&Item;
		}
	} // эта скобка надо, см. M#605

	if (Opt.CloseCDGate && mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		const wchar_t RootDir[]={mitem->cDrive,L':',L'\0'};

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

	if (ProcessPluginEvent(FE_CLOSE,NULL))
		return -1;

	ScrBuf.Flush();
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

			string strMsgStr;
			strMsgStr.Format(MSG(MChangeDriveCannotReadDisk), mitem->cDrive);

			if (Message(
			            MSG_WARNING|MSG_ERRORTYPE,
			            2,
			            MSG(MError),
			            strMsgStr,
			            MSG(MRetry),
			            MSG(MCancel)
			        ) != 0)
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
			Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this, FILE_PANEL, TRUE, FALSE);
			NewPanel->SetCurDir(strNewCurDir,TRUE);
			NewPanel->Show();

			if (Focus || !CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
				NewPanel->SetFocus();

			if (!Focus && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}
	else //эта плагин, да
	{
		HANDLE hPlugin = CtrlObject->Plugins.OpenPlugin(
		                     mitem->pPlugin,
		                     OPEN_DISKMENU,
		                     mitem->nItem
		                 );

		if (hPlugin != INVALID_HANDLE_VALUE)
		{
			Focus=GetFocus();
			Panel *NewPanel = CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
			NewPanel->SetPluginMode(hPlugin,L"",Focus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!Focus && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}

	return -1;
}

int Panel::DisconnectDrive(PanelMenuItem *item, VMenu &ChDisk)
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
			int AMode=CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
			string strTmpCDir, strTmpADir;
			GetCurDir(strTmpCDir);
			CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(strTmpADir);
			// отключим меню, иначе бага с прорисовкой этой самой меню
			// (если меню поболее высоты экрана)
			ChDisk.Hide();
			ChDisk.Lock(); // ... и запретим ее перерисовку.
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
						CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, FALSE);

					if (CMode != PLUGIN_PANEL)
						SetCurDir(strTmpCDir, FALSE);

					// ... и выведем месаг о...
					string strMsgText;
					strMsgText.Format(MSG(MChangeCouldNotEjectMedia), item->cDrive);
					SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
					DoneEject = Message(
					                MSG_WARNING|MSG_ERRORTYPE,
					                2,
					                MSG(MError),
					                strMsgText,
					                MSG(MRetry),
					                MSG(MCancel)
					            ) != 0;
				}
				else
					DoneEject=TRUE;
			}

			// "отпустим" менюху выбора дисков
			ChDisk.Unlock();
			ChDisk.Show();
		}
		return DRIVE_DEL_NONE;
	}
	else
	{
		return ProcessDelDisk(item->cDrive, item->nDriveType, &ChDisk);
	}
}

void Panel::RemoveHotplugDevice(PanelMenuItem *item, VMenu &ChDisk)
{
	int Code = ProcessRemoveHotplugDevice(item->cDrive, EJECT_NOTIFY_AFTERREMOVE);

	if (Code == 0)
	{
		// запоминаем состояние панелей
		int CMode=GetMode();
		int AMode=CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
		string strTmpCDir, strTmpADir;
		GetCurDir(strTmpCDir);
		CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(strTmpADir);
		// отключим меню, иначе бага с прорисовкой этой самой меню
		// (если меню поболее высоты экрана)
		ChDisk.Hide();
		ChDisk.Lock(); // ... и запретим ее перерисовку.
		// "цикл до умопомрачения"
		int DoneEject=FALSE;

		while (!DoneEject)
		{
			// "освободим диск" - перейдем при необходимости в домашний каталог
			// TODO: А если домашний каталог - USB? ;-)
			IfGoHome(item->cDrive);
			// очередная попытка извлечения без вывода сообщения
			Code = ProcessRemoveHotplugDevice(item->cDrive, EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);

			if (Code == 0)
			{
				// восстановим пути - это избавит нас от левых данных в панели.
				if (AMode != PLUGIN_PANEL)
					CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, FALSE);

				if (CMode != PLUGIN_PANEL)
					SetCurDir(strTmpCDir, FALSE);

				// ... и выведем месаг о...
				string strMsgText;
				strMsgText.Format(MSG(MChangeCouldNotEjectHotPlugMedia), item->cDrive);
				SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
				DoneEject = Message(
				                MSG_WARNING|MSG_ERRORTYPE,
				                2,
				                MSG(MError),
				                strMsgText,
				                MSG(MHRetry),
				                MSG(MHCancel)
				            ) != 0;
			}
			else
				DoneEject=TRUE;
		}

		// "отпустим" менюху выбора дисков
		ChDisk.Unlock();
		ChDisk.Show();
	}
}

/* $ 28.12.2001 DJ
   обработка Del в меню дисков
*/

int Panel::ProcessDelDisk(wchar_t Drive, int DriveType,VMenu *ChDiskMenu)
{
	string strMsgText;
	int UpdateProfile=CONNECT_UPDATE_PROFILE;
	BOOL Processed=FALSE;
	wchar_t DiskLetter[]={Drive,L':',0};

	if (DriveType == DRIVE_REMOTE && MessageRemoveConnection(Drive,UpdateProfile))
		Processed=TRUE;

	// <КОСТЫЛЬ>
	if (Processed)
	{
		LockScreen LckScr;
		// если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
		// удалению
		IfGoHome(Drive);
		FrameManager->ResizeAllFrame();
		FrameManager->GetCurrentFrame()->Show();
		ChDiskMenu->Show();
	}

	// </КОСТЫЛЬ>

	/* $ 05.01.2001 SVS
	   Пробуем удалить SUBST-драйв.
	*/
	if (DriveType == DRIVE_SUBSTITUTE)
	{
		if (Opt.Confirm.RemoveSUBST)
		{
			strMsgText.Format(MSG(MChangeSUBSTDisconnectDriveQuestion),Drive);

			if (Message(MSG_WARNING,2,MSG(MChangeSUBSTDisconnectDriveTitle),strMsgText,MSG(MYes),MSG(MNo))!=0)
				return DRIVE_DEL_FAIL;
		}

		if (DelSubstDrive(DiskLetter))
			return DRIVE_DEL_SUCCESS;
		else
		{
			int LastError=GetLastError();
			strMsgText.Format(MSG(MChangeDriveCannotDelSubst),DiskLetter);

			if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
			{
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),strMsgText,
				            L"\x1",MSG(MChangeDriveOpenFiles),
				            MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
				{
					if (DelSubstDrive(DiskLetter))
						return DRIVE_DEL_SUCCESS;
				}
				else
					return DRIVE_DEL_FAIL;
			}

			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText,MSG(MOk));
		}

		return DRIVE_DEL_FAIL; // блин. в прошлый раз забыл про это дело...
	}

	if (Processed)
	{
		if (WNetCancelConnection2(DiskLetter,UpdateProfile,FALSE)==NO_ERROR)
			return DRIVE_DEL_SUCCESS;
		else
		{
			int LastError=GetLastError();
			strMsgText.Format(MSG(MChangeDriveCannotDisconnect),DiskLetter);

			if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
			{
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),strMsgText,
				            L"\x1",MSG(MChangeDriveOpenFiles),
				            MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
				{
					if (WNetCancelConnection2(DiskLetter,UpdateProfile,TRUE)==NO_ERROR)
						return DRIVE_DEL_SUCCESS;
				}
				else
					return DRIVE_DEL_FAIL;
			}

			const wchar_t RootDir[]={*DiskLetter,L':',L'\\',L'\0'};

			if (FAR_GetDriveType(RootDir)==DRIVE_REMOTE)
				Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText,MSG(MOk));
		}

		return DRIVE_DEL_FAIL;
	}

	return DRIVE_DEL_FAIL;
}


void Panel::FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName,string &strName)
{
	wchar_t *Ptr=(wchar_t *)xf_malloc((StrLength(Src)+StrLength(FindEdit->GetStringAddr())+32)*sizeof(wchar_t));

	if (Ptr)
	{
		wcscpy(Ptr,FindEdit->GetStringAddr());
		wchar_t *EndPtr=Ptr+StrLength(Ptr);
		wcscat(Ptr,Src);
		Unquote(EndPtr);
		EndPtr=Ptr+StrLength(Ptr);
		DWORD Key;

		for (;;)
		{
			if (EndPtr == Ptr)
			{
				Key=KEY_NONE;
				break;
			}

			if (FindPartName(Ptr,FALSE,1,1))
			{
				Key=*(EndPtr-1);
				*EndPtr=0;
				FindEdit->SetString(Ptr);
				strLastName = Ptr;
				strName = Ptr;
				FindEdit->Show();
				break;
			}

			*--EndPtr=0;
		}

		xf_free(Ptr);
	}
}

__int64 Panel::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return 0;
}

// корректировка букв
static DWORD _CorrectFastFindKbdLayout(INPUT_RECORD *rec,DWORD Key)
{
	if ((Key&KEY_ALT))// && Key!=(KEY_ALT|0x3C))
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
	int Key,KeyToProcess=0;
	WaitInFastFind++;
	{
		int FindX=Min(X1+9,ScrX-22);
		int FindY=Min(Y2,ScrY-2);
		ChangeMacroMode MacroMode(MACRO_SEARCH);
		SaveScreen SaveScr(FindX,FindY,FindX+21,FindY+2);
		FastFindShow(FindX,FindY);
		Edit FindEdit;
		FindEdit.SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
		FindEdit.SetEditBeyondEnd(FALSE);
		FindEdit.SetObjectColor(COL_DIALOGEDIT);
		FindEdit.Show();

		while (!KeyToProcess)
		{
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
					if ((rec.Event.MouseEvent.dwButtonState & 3)==0)
						continue;
					else
						Key=KEY_ESC;
				}
				else if (!rec.EventType || rec.EventType==KEY_EVENT || rec.EventType==FARMACRO_KEY_EVENT)
				{
					// для вставки воспользуемся макродвижком...
					if (Key==KEY_CTRLV || Key==KEY_SHIFTINS || Key==KEY_SHIFTNUMPAD0)
					{
						wchar_t *ClipText=PasteFromClipboard();

						if (ClipText)
						{
							if (*ClipText)
							{
								FastFindProcessName(&FindEdit,ClipText,strLastName,strName);
								FastFindShow(FindX,FindY);
							}

							xf_free(ClipText);
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
					else if (Key == KEY_OP_DATE || Key == KEY_OP_PLAINTEXT)
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

			if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALTSHIFT_BASE));

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
				case KEY_CTRLNUMENTER:
				case KEY_CTRLENTER:
					FindPartName(strName,TRUE,1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
					FindPartName(strName,TRUE,-1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_NONE:
				case KEY_IDLE:
					break;
				default:

					if ((Key<32 || Key>=65536) && Key!=KEY_BS && Key!=KEY_CTRLY &&
					        Key!=KEY_CTRLBS && Key!=KEY_ALT && Key!=KEY_SHIFT &&
					        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
					        !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && !(Key==KEY_SHIFTINS||Key==KEY_SHIFTNUMPAD0))
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
							if (CtrlObject->Macro.IsExecuting())// && CtrlObject->Macro.GetLevelState() > 0) // если вставка макросом...
							{
								//CtrlObject->Macro.DropProcess(); // ... то дропнем макропроцесс
								//CtrlObject->Macro.PopState();
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
	WaitInFastFind--;
	Show();
	CtrlObject->MainKeyBar->Redraw();
	ScrBuf.Flush();
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

	if ((KeyToProcess==KEY_ENTER||KeyToProcess==KEY_NUMENTER) && ActivePanel->GetType()==TREE_PANEL)
		((TreeList *)ActivePanel)->ProcessEnter();
	else
		CtrlObject->Cp()->ProcessKey(KeyToProcess);
}


void Panel::FastFindShow(int FindX,int FindY)
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(FindX+1,FindY+1);
	Text(L" ");
	GotoXY(FindX+20,FindY+1);
	Text(L" ");
	Box(FindX,FindY,FindX+21,FindY+2,COL_DIALOGBOX,DOUBLE_BOX);
	GotoXY(FindX+7,FindY);
	SetColor(COL_DIALOGBOXTITLE);
	Text(MSearchFileTitle);
}


void Panel::SetFocus()
{
	if (CtrlObject->Cp()->ActivePanel!=this)
	{
		CtrlObject->Cp()->ActivePanel->KillFocus();
		CtrlObject->Cp()->ActivePanel=this;
	}

	ProcessPluginEvent(FE_GOTFOCUS,NULL);

	if (!GetFocus())
	{
		CtrlObject->Cp()->RedrawKeyBar();
		Focus=TRUE;
		Redraw();
		FarChDir(strCurDir);
	}
}


void Panel::KillFocus()
{
	Focus=FALSE;
	ProcessPluginEvent(FE_KILLFOCUS,NULL);
	Redraw();
}


int  Panel::PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
{
	RetCode=TRUE;

	if (!ModalMode && MouseEvent->dwMousePosition.Y==0)
	{
		if (MouseEvent->dwMousePosition.X==ScrX)
		{
			if (Opt.ScreenSaver && (MouseEvent->dwButtonState & 3)==0)
			{
				EndDrag();
				ScreenSaver(TRUE);
				return(TRUE);
			}
		}
		else
		{
			if ((MouseEvent->dwButtonState & 3)!=0 && MouseEvent->dwEventFlags==0)
			{
				EndDrag();

				if (MouseEvent->dwMousePosition.X==0)
					CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
				else
					ShellOptions(0,MouseEvent);

				return(TRUE);
			}
		}
	}

	if (!IsVisible() ||
	        (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	         MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2))
	{
		RetCode=FALSE;
		return(TRUE);
	}

	if (DragX!=-1)
	{
		if ((MouseEvent->dwButtonState & 3)==0)
		{
			EndDrag();

			if (MouseEvent->dwEventFlags==0 && SrcDragPanel!=this)
			{
				MoveToMouse(MouseEvent);
				Redraw();
				SrcDragPanel->ProcessKey(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY);
			}

			return(TRUE);
		}

		if (MouseEvent->dwMousePosition.Y<=Y1 || MouseEvent->dwMousePosition.Y>=Y2 ||
		        !CtrlObject->Cp()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return(TRUE);
		}

		if ((MouseEvent->dwButtonState & 2) && MouseEvent->dwEventFlags==0)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & 1)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
			        !ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && DragSaveScr==NULL)
				{
					SrcDragPanel->GoToFile(strDragName);
					SrcDragPanel->Show();
				}

				DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
				return(TRUE);
			}
			else
			{
				delete DragSaveScr;
				DragSaveScr=NULL;
			}
		}
	}

	if ((MouseEvent->dwButtonState & 3)==0)
		return(TRUE);

	if ((MouseEvent->dwButtonState & 1) && MouseEvent->dwEventFlags==0 &&
	        X2-X1<ScrX)
	{
		DWORD FileAttr;
		MoveToMouse(MouseEvent);
		GetSelName(NULL,FileAttr);

		if (GetSelName(&strDragName,FileAttr) && !TestParentFolderName(strDragName))
		{
			SrcDragPanel=this;
			DragX=MouseEvent->dwMousePosition.X;
			DragY=MouseEvent->dwMousePosition.Y;
			DragMove=ShiftPressed;
		}
	}

	return(FALSE);
}


int  Panel::IsDragging()
{
	return(DragSaveScr!=NULL);
}


void Panel::EndDrag()
{
	delete DragSaveScr;
	DragSaveScr=NULL;
	DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{
	string strDragMsg, strSelName;
	int SelCount,MsgX,Length;

	if ((SelCount=SrcDragPanel->GetSelCount())==0)
		return;

	if (SelCount==1)
	{
		string strCvtName;
		DWORD FileAttr;
		SrcDragPanel->GetSelName(NULL,FileAttr);
		SrcDragPanel->GetSelName(&strSelName,FileAttr);
		strCvtName = PointToName(strSelName);
		QuoteSpace(strCvtName);
		strSelName = strCvtName;
	}
	else
		strSelName.Format(MSG(MDragFiles), SelCount);

	if (Move)
		strDragMsg.Format(MSG(MDragMove), (const wchar_t*)strSelName);
	else
		strDragMsg.Format(MSG(MDragCopy), (const wchar_t*)strSelName);

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



int Panel::GetCurDir(string &strCurDir)
{
	strCurDir = Panel::strCurDir; // TODO: ОПАСНО!!!
	return (int)strCurDir.GetLength();
}



BOOL Panel::SetCurDir(const wchar_t *CurDir,int ClosePlugin)
{
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (PanelMode!=PLUGIN_PANEL)
			PrepareDiskPath(strCurDir);
	}

	return TRUE;
}


void Panel::InitCurDir(const wchar_t *CurDir)
{
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (PanelMode!=PLUGIN_PANEL)
			PrepareDiskPath(strCurDir);
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

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

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

	if (!FarChDir(strCurDir)||(!(HasPathPrefix(strCurDir)&&(!StrCmpNI(&strCurDir[4],L"pipe",4)||!StrCmpNI(&strCurDir[4],L"nul",3)))&&apiGetFileAttributes(strCurDir)==INVALID_FILE_ATTRIBUTES))
	{
		// здесь на выбор :-)
#if 1

		while (!FarChDir(strCurDir))
		{
			string strRoot;
			GetPathRoot(strCurDir, strRoot);

			if (FAR_GetDriveType(strRoot) != DRIVE_REMOVABLE || apiIsDiskInDrive(strRoot))
			{
				int Result=CheckFolder(strCurDir);

				if (Result == CHKFLD_NOTFOUND)
				{
					if (CheckShortcutFolder(&strCurDir,FALSE,TRUE) && FarChDir(strCurDir))
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
				SetCurDir(g_strFarPath,TRUE);                    // если запущен - выставим путь который мы точно знаем что существует
				ChangeDisk();                                    // и вызовем меню выбора дисков
			}
			else                                               // оппа...
			{
				string strTemp=strCurDir;
				CutToFolderNameIfFolder(strCurDir);             // подымаемся вверх, для очередной порции ChDir

				if (strTemp.GetLength()==strCurDir.GetLength())  // здесь проблема - видимо диск недоступен
				{
					SetCurDir(g_strFarPath,TRUE);                 // тогда просто сваливаем в каталог, откуда стартанул FAR.
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
			else if (CheckFolder(CurDir) == CHKFLD_NOTACCESS)
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
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

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

	/* $ 03.10.2001 IS перерисуем строчку меню */
	if (Opt.ShowMenuBar)
		CtrlObject->TopMenuBar->Show();

	/* $ 09.05.2001 OT */
//  SavePrevScreen();
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

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
	if (Opt.ShowScreensNumber && X1==0)
	{
		int Viewers=FrameManager->GetFrameCountByType(MODALTYPE_VIEWER);
		int Editors=FrameManager->GetFrameCountByType(MODALTYPE_EDITOR);
		int Dialogs=FrameManager->GetFrameCountByType(MODALTYPE_DIALOG);

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			string strScreensText;
			string strAdd;
			strScreensText.Format(L"[%d", Viewers);

			if (Editors > 0)
			{
				strAdd.Format(L"+%d", Editors);
				strScreensText += strAdd;
			}

			if (Dialogs > 0)
			{
				strAdd.Format(L",%d", Dialogs);
				strScreensText += strAdd;
			}

			strScreensText += L"]";
			GotoXY(Opt.ShowColumnTitles ? X1:X1+2,Y1);
			SetColor(COL_PANELSCREENSNUMBER);
			Text(strScreensText);
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
			CtrlObject->CmdLine->GetCurDir(strCmdText);
			strTitleDir += strCmdText;
		}

		strTitleDir += L"}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

string &Panel::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
	string strTitleDir;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPluginInfo Info;
		GetOpenPluginInfo(&Info);
		strTitleDir = Info.PanelTitle;
		RemoveExternalSpaces(strTitleDir);
		TruncStr(strTitleDir,SubLen-TruncSize);
	}
	else
	{
		if (ShowShortNames)
			ConvertNameToShort(strCurDir,strTitleDir);
		else
			strTitleDir = strCurDir;

		TruncPathStr(strTitleDir,SubLen-TruncSize);
	}

	strTitle = L" "+strTitleDir+L" ";
	return strTitle;
}

int Panel::SetPluginCommand(int Command,int Param1,LONG_PTR Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;
	FilePanels *FPanels=CtrlObject->Cp();
	PluginCommand=Command;

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result=FPanels->ChangePanelViewMode(this,Param1,FPanels->IsTopFrame());
			break;
		case FCTL_SETSORTMODE:
		{
			int Mode=Param1;

			if ((Mode>SM_DEFAULT) && (Mode<=SM_NUMLINKS))
			{
				SetSortMode(--Mode); // Уменьшим на 1 из-за SM_DEFAULT
				Result=TRUE;
			}
		}
		break;
		case FCTL_SETNUMERICSORT:
		{
			SetNumericSort(Param1);
			Result=TRUE;
		}
		break;
		case FCTL_SETSORTORDER:
		{
			ChangeSortOrder(Param1?-1:1);
			Result=TRUE;
		}
		break;
		case FCTL_CLOSEPLUGIN:
			strPluginParam = (const wchar_t *)Param2;
			Result=TRUE;
			//if(Opt.CPAJHefuayor)
			//  CtrlObject->Plugins.ProcessCommandLine((char *)PluginParam);
			break;
		case FCTL_GETPANELINFO:
		{
			if (!Param2)
				break;

			PanelInfo *Info=(PanelInfo *)Param2;
			memset(Info,0,sizeof(*Info));
			UpdateIfRequired();

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

			Info->Plugin=(GetMode()==PLUGIN_PANEL);
			int X1,Y1,X2,Y2;
			GetPosition(X1,Y1,X2,Y2);
			Info->PanelRect.left=X1;
			Info->PanelRect.top=Y1;
			Info->PanelRect.right=X2;
			Info->PanelRect.bottom=Y2;
			Info->Visible=IsVisible();
			Info->Focus=GetFocus();
			Info->ViewMode=GetViewMode();
			Info->SortMode=SM_UNSORTED-UNSORTED+GetSortMode();
			{
				static struct
				{
					int *Opt;
					DWORD Flags;
				} PFLAGS[]=
				{
					{&Opt.ShowHidden,PFLAGS_SHOWHIDDEN},
					{&Opt.Highlight,PFLAGS_HIGHLIGHT},
				};
				DWORD Flags=0;

				for (size_t I=0; I < countof(PFLAGS); ++I)
					if (*(PFLAGS[I].Opt) != 0)
						Flags|=PFLAGS[I].Flags;

				Flags|=GetSortOrder()<0?PFLAGS_REVERSESORTORDER:0;
				Flags|=GetSortGroups()?PFLAGS_USESORTGROUPS:0;
				Flags|=GetSelectedFirstMode()?PFLAGS_SELECTEDFIRST:0;
				Flags|=GetNumericSort()?PFLAGS_NUMERICSORT:0;

				if (CtrlObject->Cp()->LeftPanel == this)
					Flags|=PFLAGS_PANELLEFT;

				Info->Flags=Flags;
			}

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && Info->Plugin)
				{
					Reenter++;
					OpenPluginInfo PInfo;
					DestFilePanel->GetOpenPluginInfo(&PInfo);

					if (PInfo.Flags & OPIF_REALNAMES)
						Info->Flags |= PFLAGS_REALNAMES;

					if (!(PInfo.Flags & OPIF_USEHIGHLIGHTING))
						Info->Flags &= ~PFLAGS_HIGHLIGHT;

					Reenter--;
				}

				DestFilePanel->PluginGetPanelInfo(*Info);
			}

			if (!Info->Plugin) // $ 12.12.2001 DJ - на неплагиновой панели - всегда реальные имена
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}
		case FCTL_GETPANELDIR:
		{
			string strInfoCurDir;
			GetCurDir(strInfoCurDir);

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && GetMode()==PLUGIN_PANEL)
				{
					Reenter++;
					OpenPluginInfo PInfo;
					DestFilePanel->GetOpenPluginInfo(&PInfo);
					strInfoCurDir=PInfo.CurDir;
					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strInfoCurDir,Param1-1);

			Result=(int)strInfoCurDir.GetLength()+1;
		}
		break;
		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:

			if (GetType()==FILE_PANEL)
			{
				string strColumnTypes,strColumnWidths;
				((FileList *)this)->PluginGetColumnTypesAndWidths(strColumnTypes,strColumnWidths);

				if (Command==FCTL_GETCOLUMNTYPES)
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnTypes,Param1-1);

					Result=(int)strColumnTypes.GetLength()+1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnWidths,Param1-1);

					Result=(int)strColumnWidths.GetLength()+1;
				}
			}

			break;
		case FCTL_GETPANELITEM:
		{
			Result=(int)((FileList*)this)->PluginGetPanelItem(Param1,(PluginPanelItem*)Param2);
		}
		break;
		case FCTL_GETSELECTEDPANELITEM:
		{
			Result=(int)((FileList*)this)->PluginGetSelectedPanelItem(Param1,(PluginPanelItem*)Param2);
		}
		break;
		case FCTL_GETCURRENTPANELITEM:
		{
			PanelInfo Info;
			FileList *DestPanel = ((FileList*)this);
			DestPanel->PluginGetPanelInfo(Info);
			Result = (int)DestPanel->PluginGetPanelItem(Info.CurrentItem,(PluginPanelItem*)Param2);
		}
		break;
		case FCTL_BEGINSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginBeginSelection();
				Result=TRUE;
			}
		}
		break;
		case FCTL_SETSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginSetSelection(Param1,Param2?true:false);
				Result=TRUE;
			}
		}
		break;
		case FCTL_ENDSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginEndSelection();
				Result=TRUE;
			}
		}
		break;
		case FCTL_UPDATEPANEL:
			Update(Param1?UPDATE_KEEP_SELECTION:0);

			if (GetType() == QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;
		case FCTL_REDRAWPANEL:
		{
			PanelRedrawInfo *Info=(PanelRedrawInfo *)Param2;

			if (Info)
			{
				CurFile=Info->CurrentItem;
				CurTopFile=Info->TopPanelItem;
			}

			// $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущий фрейм
			if (FPanels->IsTopFrame())
				Redraw();

			Result=TRUE;
			break;
		}
		case FCTL_SETPANELDIR:
		{
			if (Param2)
			{
				Result = SetCurDir((const wchar_t *)Param2,TRUE);
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
	return(FALSE);
}


int Panel::GetCurBaseName(string &strName, string &strShortName)
{
	strName.Clear();
	strShortName.Clear();
	return(FALSE);
}

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile)
{
	int Len1, Len2, Len3,Len4;
	BOOL IsPersistent;
	string strMsgText;
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
	static DialogDataEx DCDlgData[]=
	{
		/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
		                                      Selected               Data
		*/
		/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 9, 0, 0, 0,                0,L"",
		/* 1 */ DI_TEXT,      5, 2,  0, 2, 0, 0, DIF_SHOWAMPERSAND,0,L"",
		/* 2 */ DI_TEXT,      5, 3,  0, 3, 0, 0, DIF_SHOWAMPERSAND,0,L"",
		/* 3 */ DI_TEXT,      5, 4,  0, 4, 0, 0, DIF_SHOWAMPERSAND,0,L"",
		/* 4 */ DI_TEXT,      0, 5,  0, 5, 0, 0, DIF_SEPARATOR,    0,L"",
		/* 5 */ DI_CHECKBOX,  5, 6, 70, 6, 0, 0, 0,                0,L"",
		/* 6 */ DI_TEXT,      0, 7,  0, 7, 0, 0, DIF_SEPARATOR,    0,L"",
		/* 7 */ DI_BUTTON,    0, 8,  0, 8, 1, 0, DIF_CENTERGROUP,  1,L"",
		/* 8 */ DI_BUTTON,    0, 8,  0, 8, 0, 0, DIF_CENTERGROUP,  0,L""
	};
	MakeDialogItemsEx(DCDlgData,DCDlg);
	DCDlg[0].strData = MSG(MChangeDriveDisconnectTitle);
	Len1 = (int)DCDlg[0].strData.GetLength();
	strMsgText.Format(MSG(MChangeDriveDisconnectQuestion),Letter);
	DCDlg[1].strData = strMsgText;
	Len2 = (int)DCDlg[1].strData.GetLength();
	strMsgText.Format(MSG(MChangeDriveDisconnectMapped),Letter);
	DCDlg[2].strData = strMsgText;
	Len4 = (int)DCDlg[2].strData.GetLength();
	DCDlg[5].strData = MSG(MChangeDriveDisconnectReconnect);
	Len3 = (int)DCDlg[5].strData.GetLength();
	Len1=Max(Len1,Max(Len2,Max(Len3,Len4)));
	DCDlg[3].strData = TruncPathStr(DriveLocalToRemoteName(DRIVE_REMOTE,Letter,strMsgText),Len1);
	DCDlg[7].strData = MSG(MYes);
	DCDlg[8].strData = MSG(MCancel);
	// проверяем - это было постоянное соедение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.
	{
		HKEY hKey;
		IsPersistent=TRUE;
		const wchar_t KeyName[]={L'N',L'e',L't',L'w',L'o',L'r',L'k',L'\\',Letter,L'\0'};

		if (RegOpenKeyEx(HKEY_CURRENT_USER,KeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		{
			DCDlg[5].Flags|=DIF_DISABLE;
			DCDlg[5].Selected=0;
			IsPersistent=FALSE;
			RegCloseKey(hKey);
		}
		else
			DCDlg[5].Selected=Opt.ChangeDriveDisconnetMode;
	}
	// скорректируем размеры диалога - для дизайнУ
	DCDlg[0].X2=DCDlg[0].X1+Len1+3;
	int ExitCode=7;

	if (Opt.Confirm.RemoveConnection)
	{
		Dialog Dlg(DCDlg,countof(DCDlg));
		Dlg.SetPosition(-1,-1,DCDlg[0].X2+4,11);
		Dlg.SetHelp(L"DisconnectDrive");
		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Opt.ChangeDriveDisconnetMode=DCDlg[5].Selected;

	return ExitCode == 7;
}

BOOL Panel::NeedUpdatePanel(Panel *AnotherPanel)
{
	/* Обновить, если обновление разрешено и пути совпадают */
	if ((!Opt.AutoUpdateLimit || static_cast<DWORD>(GetFileCount()) <= Opt.AutoUpdateLimit) &&
	        StrCmpI(AnotherPanel->strCurDir,strCurDir)==0)
		return TRUE;

	return FALSE;
}

int Panel::ProcessShortcutFolder(int Key,BOOL ProcTreePanel)
{
	string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

	if (GetShortcutFolder(Key,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

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

		return(TRUE);
	}

	return FALSE;
}

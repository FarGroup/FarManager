/*
panel.cpp

Parent class для панелей

*/

#include "headers.hpp"
#pragma hdrstop

#include "panel.hpp"
#include "plugin.hpp"
#include "macroopcode.hpp"
#include "global.hpp"
#include "fn.hpp"
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

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=NULL;
static char DragName[NM];

static unsigned char VerticalLine=0x0B3;

static int MessageRemoveConnection(char Letter, int &UpdateProfile);

/* $ 21.08.2002 IS
   Класс для хранения пункта плагина в меню выбора дисков
*/
class ChDiskPluginItem
{
	public:
		MenuItem Item;
		unsigned int HotKey;
		ChDiskPluginItem():HotKey(0)
		{
			memset(&Item,0,sizeof(Item));
		}
		bool operator==(const ChDiskPluginItem &rhs) const;
		int operator<(const ChDiskPluginItem &rhs) const;
		const ChDiskPluginItem& operator=(const ChDiskPluginItem &rhs);
		~ChDiskPluginItem()
		{
		}
};

bool ChDiskPluginItem::operator==(const ChDiskPluginItem &rhs) const
{
	return HotKey==rhs.HotKey &&
	       !LocalStricmp(Item.Name,rhs.Item.Name) &&
	       Item.UserData==rhs.Item.UserData;
}

int ChDiskPluginItem::operator<(const ChDiskPluginItem &rhs) const
{
	if (HotKey==rhs.HotKey)
		return LocalStricmp(Item.Name,rhs.Item.Name)<0;
	else if (HotKey && rhs.HotKey)
		return HotKey < rhs.HotKey;
	else
		return HotKey && !rhs.HotKey;
}

const ChDiskPluginItem& ChDiskPluginItem::operator=(const ChDiskPluginItem &rhs)
{
	Item=rhs.Item;
	HotKey=rhs.HotKey;
	return *this;
}
/* IS $ */


Panel::Panel()
{
	_OT(SysLog("[%p] Panel::Panel()", this));
	Focus=0;
	*CurDir=0;
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
	_OT(SysLog("[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	PrevViewMode=ViewMode;
	Panel::ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	char NewDir[NM];
	FarGetCurDir(sizeof(NewDir),NewDir);
	SetCurDir(NewDir,TRUE);
}


void Panel::ChangeDisk()
{
	int Pos,FirstCall=TRUE;

	if (CurDir[0]!=0 && CurDir[1]==':')
		Pos=toupper(CurDir[0])-'A';
	else
		Pos=getdisk();

	while (Pos!=-1)
	{
		Pos=ChangeDiskMenu(Pos,FirstCall);
		FirstCall=FALSE;
	}
}


int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
	class Guard_Macro_DskShowPosType
	{
		public:
			Guard_Macro_DskShowPosType(Panel *curPanel) {Macro_DskShowPosType=(curPanel==CtrlObject->Cp()->LeftPanel)?1:2;};
			~Guard_Macro_DskShowPosType() {Macro_DskShowPosType=0;};
	};
	Guard_Macro_DskShowPosType _guard_Macro_DskShowPosType(this);
	struct MenuItem ChDiskItem;
	char DiskType[100],RootDir[10],DiskLetter[50];
	DWORD Mask,DiskMask;
	int DiskCount,Focus,I,J;
	int ShowSpecial=FALSE, SetSelected=FALSE;
	memset(&ChDiskItem,0,sizeof(ChDiskItem));
	*DiskLetter=0;
	_tran(SysLog("Panel::ChangeDiskMenu(), Pos=%i, FirstCall=%i",Pos,FirstCall));
	Mask=FarGetLogicalDrives();

	for (DiskMask=Mask,DiskCount=0; DiskMask!=0; DiskMask>>=1)
		DiskCount+=DiskMask & 1;

	int UserDataSize=0;
	DWORD UserData=0;
	{
		_tran(SysLog("create VMenu ChDisk"));
		VMenu ChDisk(MSG(MChangeDriveTitle),NULL,0,ScrY-Y1-3);
		ChDisk.SetFlags(VMENU_NOTCENTER);

		if (this==CtrlObject->Cp()->LeftPanel)
		{
			ChDisk.SetFlags(VMENU_LEFTMOST);
		}

		ChDisk.SetHelp("DriveDlg");
		/* $ 17.06.2001 KM
		   ! Добавление WRAPMODE в меню.
		*/
		ChDisk.SetFlags(VMENU_WRAPMODE);
		/* KM $ */
		char MenuText[NM];
		int DriveType,MenuLine;
		int LabelWidth=Max(11,(int)strlen(MSG(MChangeDriveLabelAbsent)));

		/* $ 02.04.2001 VVM
		  ! Попытка не будить спящие диски... */
		for (DiskMask=Mask,MenuLine=I=0; DiskMask!=0; DiskMask>>=1,I++)
		{
			if (DiskMask & 1)
			{
				sprintf(MenuText,"&%c: ",'A'+I);
				sprintf(RootDir,"%c:\\",'A'+I);
				DriveType = FAR_GetDriveType(RootDir,NULL,Opt.ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

				if (Opt.ChangeDriveMode & DRIVE_SHOW_TYPE)
				{
					static struct TypeMessage
					{
						int DrvType;
						int FarMsg;
					} DrTMsg[]=
					{
						{DRIVE_REMOVABLE,MChangeDriveRemovable},
						{DRIVE_FIXED,MChangeDriveFixed},
						{DRIVE_REMOTE,MChangeDriveNetwork},
						{DRIVE_CDROM,MChangeDriveCDROM},
						{DRIVE_CD_RW,MChangeDriveCD_RW},
						{DRIVE_CD_RWDVD,MChangeDriveCD_RWDVD},
						{DRIVE_DVD_ROM,MChangeDriveDVD_ROM},
						{DRIVE_DVD_RW,MChangeDriveDVD_RW},
						{DRIVE_DVD_RAM,MChangeDriveDVD_RAM},
						{DRIVE_RAMDISK,MChangeDriveRAM},
					};

					for (J=0; J < sizeof(DrTMsg)/sizeof(DrTMsg[1]); ++J)
						if (DrTMsg[J].DrvType == DriveType)
						{
							strcpy(DiskType,MSG(DrTMsg[J].FarMsg));
							_SVS(SysLog("DriveType=%d, DiskType='%s'",DriveType,DiskType));
							break;
						}

					if (J >= sizeof(DrTMsg)/sizeof(DrTMsg[1]))
						sprintf(DiskType,"%*s",strlen(MSG(MChangeDriveFixed)),"");

					/* 05.01.2001 SVS
					   + Информация про Subst-тип диска
					*/
					{
						char LocalName[8],SubstName[NM];
						sprintf(LocalName,"%c:",*RootDir);

						if (GetSubstName(DriveType,LocalName,SubstName,sizeof(SubstName)))
						{
							strcpy(DiskType,MSG(MChangeDriveSUBST));
							DriveType=DRIVE_SUBSTITUTE;
						}
					}
					/* SVS $ */
					strcat(MenuText,DiskType);
				}

				int ShowDisk = (DriveType!=DRIVE_REMOVABLE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
				               (!IsDriveTypeCDROM(DriveType) || (Opt.ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
				               (DriveType!=DRIVE_REMOTE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOTE));

				if (Opt.ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
				{
					char VolumeName[NM],FileSystemName[NM];
					*VolumeName=*FileSystemName=0;

					if (ShowDisk && !GetVolumeInformation(RootDir,VolumeName,sizeof(VolumeName),NULL,NULL,NULL,FileSystemName,sizeof(FileSystemName)))
					{
						strcpy(VolumeName,MSG(MChangeDriveLabelAbsent));
						ShowDisk=FALSE;
					}

					if (Opt.ChangeDriveMode & DRIVE_SHOW_LABEL)
					{
						/* $ 01.10.2001 IS метку усекаем с конца */
						TruncStrFromEnd(VolumeName,LabelWidth);
						/* IS $ */
						sprintf(MenuText+strlen(MenuText),"%c%-*s",VerticalLine,LabelWidth,VolumeName);
					}

					if (Opt.ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
						sprintf(MenuText+strlen(MenuText),"%c%-8.8s",VerticalLine,FileSystemName);
				}

				if (Opt.ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
				{
					char TotalText[NM],FreeText[NM];
					*TotalText=*FreeText=0;
					unsigned __int64 TotalSize,TotalFree,UserFree;

					if (ShowDisk && GetDiskSize(RootDir,&TotalSize,&TotalFree,&UserFree))
					{
						if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
						{
							//размер как минимум в мегабайтах
							FileSizeToStr(TotalText,TotalSize,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
							FileSizeToStr(FreeText,UserFree,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
						}
						else
						{
							//размер с точкой и для 0 добавляем букву размера (B)
							FileSizeToStr(TotalText,TotalSize,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
							FileSizeToStr(FreeText,UserFree,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
						}
					}

					sprintf(MenuText+strlen(MenuText),"%c%-9s%c%-9s",VerticalLine,TotalText,VerticalLine,FreeText);
				}

				if (Opt.ChangeDriveMode & DRIVE_SHOW_NETNAME)
				{
					char RemoteName[NM];
					DriveLocalToRemoteName(DriveType,*RootDir,RemoteName);
					TruncPathStr(RemoteName,ScrX-(int)strlen(MenuText)-12);

					if (*RemoteName)
					{
						strcat(MenuText,"  ");
						strcat(MenuText,RemoteName);
					}

					ShowSpecial=TRUE;
				}

				if (FirstCall)
				{
					ChDiskItem.SetSelect(I==Pos);

					if (!SetSelected)
						SetSelected=(I==Pos);
				}
				else if (Pos < DiskCount)
				{
					ChDiskItem.SetSelect(MenuLine==Pos);

					if (!SetSelected)
						SetSelected=(MenuLine==Pos);
				}

				xstrncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name)-1);

				if (strlen(MenuText)>4)
					ShowSpecial=TRUE;

				UserData=MAKELONG(MAKEWORD('A'+I,0),DriveType);
				ChDisk.SetUserData((char*)(DWORD_PTR)UserData,sizeof(UserData),
				                   ChDisk.AddItem(&ChDiskItem));
				MenuLine++;
			} // if (DiskMask & 1)
		} // for

		/* VVM $ */
		/* $ 21.01.2002 IS
		   Снимем ограничение на количество пунктов плагинов в меню вообще(!)
		*/
		/* $ 22.01.2002 IS
		   Автохоткеи назначаем после основных
		*/
		int PluginMenuItemsCount=0;

		if (Opt.ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			int UsedNumbers[10];
			memset(UsedNumbers,0,sizeof(UsedNumbers));
			TArray<ChDiskPluginItem> MPItems, MPItemsNoHotkey;
			ChDiskPluginItem OneItem;
			/* $ 15.03.2001 IS
			     Список дополнительных хоткеев, для случая, когда плагинов,
			     добавляющих пункт в меню, больше 9.
			*/
			char *AdditionalHotKey=MSG(MAdditionalHotKey);
			int AHKPos=0,                             // индекс в списке хоткеев
			           AHKSize=(int)strlen(AdditionalHotKey);/* для предотвращения выхода за
                                                   границу массива */
			/* IS $ */
			int PluginNumber=0, PluginItem; // IS: счетчики - плагинов и пунктов плагина
			int PluginTextNumber, ItemPresent, HotKey, Done=FALSE;
			char PluginText[100];

			while (!Done)
			{
				for (PluginItem=0;; ++PluginItem)
				{
					if (!CtrlObject->Plugins.GetDiskMenuItem(PluginNumber,PluginItem,
					        ItemPresent,PluginTextNumber,PluginText,sizeof(PluginText)))
					{
						Done=TRUE;
						break;
					}

					if (!ItemPresent)
						break;

					if (!PluginTextNumber) // IS: автохоткей, назначим потом
						HotKey=-1; // "-1" -  признак автохоткея
					else
					{
						if (PluginTextNumber<10) // IS: хотей указан явно
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

						if (PluginTextNumber<10)
							HotKey=PluginTextNumber+'0';
						else if (AHKPos<AHKSize)
							HotKey=AdditionalHotKey[AHKPos];
						else
							HotKey=0;
					}

					/* $ 22.08.2002 IS
					     Используем дополнительные хоткеи, а не просто '#', как раньше.
					*/
					*MenuText=0;

					if (HotKey<0)
						xstrncpy(MenuText,ShowSpecial?PluginText:"",
						         sizeof(MenuText)-1-4); // -4 - добавка для хоткея
					else if (PluginTextNumber<10)
					{
						sprintf(MenuText,"&%c: %s", HotKey,
						        ShowSpecial ? PluginText:"");
					}
					else if (AHKPos<AHKSize)
					{
						sprintf(MenuText,"&%c: %s", HotKey,
						        ShowSpecial ? PluginText:"");
						++AHKPos;
					}
					else if (ShowSpecial) // IS: не добавляем пустые строки!
					{
						HotKey=0;
						sprintf(MenuText,"   %s", PluginText);
					}

					/* IS $ */
					if (HotKey>-1 && *MenuText) // IS: не добавляем пустые строки!
					{
						xstrncpy(OneItem.Item.Name,MenuText,sizeof(ChDiskItem.Name)-1);
						OneItem.Item.UserDataSize=0;
						OneItem.Item.UserData=(char*)(LONG_PTR)MAKELONG(PluginNumber,PluginItem);
						OneItem.HotKey=HotKey;

						if (!MPItems.addItem(OneItem))
						{
							Done=TRUE;
							break;
						}
					}
					else if (HotKey<0) // IS: назначение автохоткеей отложим на потом
					{
						xstrncpy(OneItem.Item.Name,MenuText,sizeof(ChDiskItem.Name)-1);
						OneItem.Item.UserDataSize=0;
						OneItem.Item.UserData=(char*)(LONG_PTR)MAKELONG(PluginNumber,PluginItem);
						OneItem.HotKey=HotKey;

						if (!MPItemsNoHotkey.addItem(OneItem))
						{
							Done=TRUE;
							break;
						}
					}
				} // END: for (PluginItem=0;;++PluginItem)

				++PluginNumber;
			}

			// IS: теперь произведем назначение автохоткеев
			PluginTextNumber=0;

			for (int i=0;; ++i)
			{
				ChDiskPluginItem *item=MPItemsNoHotkey.getItem(i);

				if (item)
				{
					if (UsedNumbers[PluginTextNumber])
					{
						while (PluginTextNumber<10 && UsedNumbers[PluginTextNumber])
							PluginTextNumber++;
					}

					UsedNumbers[PluginTextNumber%10]=1;
					*MenuText=0;

					if (PluginTextNumber<10)
					{
						item->HotKey=PluginTextNumber+'0';
						sprintf(MenuText,"&%c: %s", item->HotKey, item->Item.Name);
					}
					else if (AHKPos<AHKSize)
					{
						item->HotKey=AdditionalHotKey[AHKPos];
						sprintf(MenuText,"&%c: %s", item->HotKey, item->Item.Name);
						++AHKPos;
					}
					else if (ShowSpecial) // IS: не добавляем пустые строки!
					{
						item->HotKey=0;
						sprintf(MenuText,"   %s", item->Item.Name);
					}

					xstrncpy(item->Item.Name, MenuText, sizeof(item->Item.Name)-1);

					if (*item->Item.Name && !MPItems.addItem(*item))
						break;
				}
				else
					break;
			}

			MPItems.Sort();
			MPItems.Pack(); // выкинем дубли
			PluginMenuItemsCount=MPItems.getSize();

			if (PluginMenuItemsCount)
			{
				memset(&ChDiskItem,0,sizeof(ChDiskItem));
				ChDiskItem.Flags|=LIF_SEPARATOR;
				ChDiskItem.UserDataSize=0;
				ChDisk.AddItem(&ChDiskItem);
				ChDiskItem.Flags&=~LIF_SEPARATOR;

				for (int I=0; I<PluginMenuItemsCount; ++I)
				{
					if (Pos > DiskCount && !SetSelected)
					{
						MPItems.getItem(I)->Item.SetSelect(DiskCount+I+1==Pos);

						if (!SetSelected)
							SetSelected=DiskCount+I+1==Pos;
					}

					ChDisk.AddItem(&MPItems.getItem(I)->Item);
				}
			}
		}

		/* IS 22.08.2002 $ */
		/* IS 21.08.2002 $ */
		int X=X1+5;

		if (this==CtrlObject->Cp()->RightPanel && IsFullScreen() && X2-X1>40)
			X=(X2-X1+1)/2+5;

		int Y=(ScrY+1-(DiskCount+PluginMenuItemsCount+5))/2;

		if (Y<1) Y=1;

		ChDisk.SetPosition(X,Y,0,0);

		if (Y<3)
			ChDisk.SetBoxType(SHORT_DOUBLE_BOX);

		_tran(SysLog(" call ChDisk.Show"));
		ChDisk.Show();

		while (!ChDisk.Done())
		{
			//_D(SysLog("ExitCode=%i",ChDisk.GetExitCode()));
			int Key;
			{
				ChangeMacroMode MacroMode(MACRO_DISKS);
				Key=ChDisk.ReadInput();
			}
			int SelPos=ChDisk.GetSelectPos();

			switch (Key)
			{
					// Shift-Enter в меню выбора дисков вызывает проводник для данного диска
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:

					if (SelPos<DiskCount)
					{
						if ((UserData=(DWORD)(DWORD_PTR)ChDisk.GetUserData(NULL,0)) != 0)
						{
							char DosDeviceName[16];
							sprintf(DosDeviceName,"%c:\\",LOBYTE(LOWORD(UserData)));
							Execute(DosDeviceName,FALSE,TRUE,TRUE);
						}
					}

					break;
				case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:

					if (Opt.PgUpChangeDisk)
						return -1;

					break;
					/* $ 27.04.2001 SVS
					   Т.к. нет способа получить состояние "открытости" устройства,
					   то добавим обработку Ins для CD - "закрыть диск"
					*/
				case KEY_INS:       case KEY_NUMPAD0:

					if (SelPos<DiskCount)// && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
					{
//            char MsgText[200], LocalName[50];
						if ((UserData=(DWORD)(DWORD_PTR)ChDisk.GetUserData(NULL,0)) != 0)
						{
							DriveType=HIWORD(UserData);

							if (IsDriveTypeCDROM(DriveType) /* || DriveType == DRIVE_REMOVABLE*/)
							{
								SaveScreen SvScrn;
								EjectVolume(LOBYTE(LOWORD(UserData)),EJECT_LOAD_MEDIA);
								return(SelPos);
							}
						}
					}

					break;
					/* SVS $ */
				case KEY_NUMDEL:
				case KEY_DEL:

					if (SelPos<DiskCount)
					{
						/* $ 28.12.2001 DJ
						   обработка Del вынесена в отдельную функцию
						*/
						if ((UserData=(DWORD)(DWORD_PTR)ChDisk.GetUserData(NULL,0)) != 0)
						{
							if (HIWORD(UserData) == DRIVE_REMOVABLE || IsDriveTypeCDROM(HIWORD(UserData)))
							{
								if (HIWORD(UserData) == DRIVE_REMOVABLE && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && !IsEjectableMedia(LOBYTE(LOWORD(UserData))))
									break;

								// первая попытка извлеч диск
								if (!EjectVolume(LOBYTE(LOWORD(UserData)),EJECT_NO_MESSAGE))
								{
									// запоминаем состояние панелей
									int CMode=GetMode();
									int AMode=CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
									char TmpCDir[NM], TmpADir[NM];
									GetCurDir(TmpCDir);
									CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(TmpADir);
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
										IfGoHome(LOBYTE(LOWORD(UserData)));
										// очередная попытка извлечения без вывода сообщения
										int ResEject=EjectVolume(LOBYTE(LOWORD(UserData)),EJECT_NO_MESSAGE);

										if (!ResEject)
										{
											// восстановим пути - это избавит нас от левых данных в панели.
											if (AMode != PLUGIN_PANEL)
												CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(TmpADir, FALSE);

											if (CMode != PLUGIN_PANEL)
												SetCurDir(TmpCDir, FALSE);

											// ... и выведем месаг о...
											char MsgText[200];
											sprintf(MsgText,MSG(MChangeCouldNotEjectMedia),LOBYTE(LOWORD(UserData)));
											SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
											DoneEject=Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,MSG(MRetry),MSG(MCancel))!=0;
										}
										else
											DoneEject=TRUE;
									}

									// "отпустим" менюху выбора дисков
									ChDisk.Unlock();
									ChDisk.Show();
								}
							}
							else
							{
								int Code = ProcessDelDisk(LOBYTE(LOWORD(UserData)), HIWORD(UserData), &ChDisk);

								if (Code != DRIVE_DEL_FAIL)
								{
									// "BugZ#640 - отрисовка" - обновим экран.
									ScrBuf.Lock(); // отменяем всякую прорисовку
									FrameManager->ResizeAllFrame();
									FrameManager->PluginCommit(); // коммитим.
									ScrBuf.Unlock(); // разрешаем прорисовку
									// 19.01.2002 VVM  + Если диск был последним - в конце и останемся
									return (((DiskCount-SelPos)==1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos-1:SelPos;
								}
							}
						}

						/* DJ $ */
					}

					break;
				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDECIMAL:
				case KEY_SHIFTDEL:

					if (SelPos<DiskCount)
					{
						if ((UserData=(DWORD)(DWORD_PTR)ChDisk.GetUserData(NULL,0)) != 0 && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
						{
							// первая попытка удалить устройство
							int Code=ProcessRemoveHotplugDevice(LOBYTE(LOWORD(UserData)),EJECT_NOTIFY_AFTERREMOVE);

							if (Code == 0)
							{
								// запоминаем состояние панелей
								int CMode=GetMode();
								int AMode=CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
								char TmpCDir[NM], TmpADir[NM];
								GetCurDir(TmpCDir);
								CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(TmpADir);
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
									IfGoHome(LOBYTE(LOWORD(UserData)));
									// очередная попытка извлечения без вывода сообщения
									Code=ProcessRemoveHotplugDevice(LOBYTE(LOWORD(UserData)),EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);

									if (Code == 0)
									{
										// восстановим пути - это избавит нас от левых данных в панели.
										if (AMode != PLUGIN_PANEL)
											CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(TmpADir, FALSE);

										if (CMode != PLUGIN_PANEL)
											SetCurDir(TmpCDir, FALSE);

										// ... и выведем месаг о...
										char MsgText[200];
										sprintf(MsgText,MSG(MChangeCouldNotEjectHotPlugMedia),LOBYTE(LOWORD(UserData)));
										SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
										DoneEject=Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,MSG(MHRetry),MSG(MHCancel))!=0;
									}
									else
										DoneEject=TRUE;
								}

								// "отпустим" менюху выбора дисков
								ChDisk.Unlock();
								ChDisk.Show();
							}

							return(SelPos);
						}
					}

					break;
				case KEY_CTRL1:
				case KEY_RCTRL1:
					Opt.ChangeDriveMode^=DRIVE_SHOW_TYPE;
					return(SelPos);
				case KEY_CTRL2:
				case KEY_RCTRL2:
					Opt.ChangeDriveMode^=DRIVE_SHOW_NETNAME;
					return(SelPos);
				case KEY_CTRL3:
				case KEY_RCTRL3:
					Opt.ChangeDriveMode^=DRIVE_SHOW_LABEL;
					return(SelPos);
				case KEY_CTRL4:
				case KEY_RCTRL4:
					Opt.ChangeDriveMode^=DRIVE_SHOW_FILESYSTEM;
					return(SelPos);
				case KEY_CTRL5:
				case KEY_RCTRL5:

					if (Opt.ChangeDriveMode&DRIVE_SHOW_SIZE)
					{
						Opt.ChangeDriveMode^=DRIVE_SHOW_SIZE;
						Opt.ChangeDriveMode|=DRIVE_SHOW_SIZE_FLOAT;
					}
					else if (Opt.ChangeDriveMode&DRIVE_SHOW_SIZE_FLOAT)
					{
						Opt.ChangeDriveMode^=DRIVE_SHOW_SIZE_FLOAT;
					}
					else
						Opt.ChangeDriveMode^=DRIVE_SHOW_SIZE;

					return(SelPos);
				case KEY_CTRL6:
				case KEY_RCTRL6:
					Opt.ChangeDriveMode^=DRIVE_SHOW_REMOVABLE;
					return(SelPos);
				case KEY_CTRL7:
				case KEY_RCTRL7:
					Opt.ChangeDriveMode^=DRIVE_SHOW_PLUGINS;
					return(SelPos);
				case KEY_CTRL8:
				case KEY_RCTRL8:
					Opt.ChangeDriveMode^=DRIVE_SHOW_CDROM;
					return(SelPos);
				case KEY_CTRL9:
				case KEY_RCTRL9:
					Opt.ChangeDriveMode^=DRIVE_SHOW_REMOTE;
					return(SelPos);
					/* $ 27.03.2001 SVS
					  Shift-F1 на пункте плагина в меню выбора дисков тоже покажет хелп...
					*/
				case KEY_SHIFTF1:

					if (SelPos>DiskCount)
					{
						// Вызываем нужный топик, который передали в CommandsMenu()
						if ((UserData=(DWORD)(DWORD_PTR)ChDisk.GetUserData(NULL,0)) != 0)
							FarShowHelp(CtrlObject->Plugins.PluginsData[LOWORD(UserData)].ModuleName,
							            NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
					}

					break;
					/* SVS $ */
				case KEY_CTRLR:
					return(SelPos);
					/* $ 21.06.2001 tran
					   типа костыль...
					   самое простое и быстрое решение проблемы*/
				case KEY_F1:
				{
//            SaveScreen s;
					ChDisk.ProcessInput();
				}
				break;
				/* tran 21.06.2001 $ */
				default:
					ChDisk.ProcessInput();
					break;
			}

			/* $ 05.09.2000 SVS
			  Bug#12 -   При удалении сетевого диска по Del и отказе от меню
			         фар продолжает показывать удаленный диск. хотя не должен.
			         по ctrl-r переходит на ближайший.
			         Лучше будет, если он не даст выходить из меню если удален
			         текущий диск
			*/

			/* $ 06.09.2000 tran
			   правя баг, внесли пару новых:
			   1. strncpy не записывает 0 в конец строки
			   2. GetDriveType вызывается постоянно, что грузит комп.
			*/

			/* $ 07.09.2000 SVS
			   Еще одна поправочка (с подачи AT):
			       еще косяк, не дает выйти из меню, если у нас текущий путь - UNC
			       "\\server\share\"
			*/

			/* $ 30.04.2001 DJ
			   и еще одна поправочка: не дает выйти из меню, если оно вызвано
			   из quick view panel (в нем CurDir пустая)
			*/
			if (ChDisk.Done() && ChDisk.Modal::GetExitCode()<0 && *CurDir && strncmp(CurDir,"\\\\",2)!=0)
			{
				char RootDir[10];
				xstrncpy(RootDir,CurDir,3);
				RootDir[3]=0;

				if (FAR_GetDriveType(RootDir)==DRIVE_NO_ROOT_DIR)
					ChDisk.ClearDone();
			}

			/* DJ $ */
			/* SVS $ */
			/* tran $ */
			/* SVS $ */
		} // while (!Done)

		if (ChDisk.Modal::GetExitCode()<0)
			return(-1);

		{
			UserDataSize=ChDisk.Modal::GetExitCode()>DiskCount?2:3;
			UserData=(DWORD)(DWORD_PTR)ChDisk.GetUserData(NULL,0);
		}
	}

	if (Opt.CloseCDGate && UserData != 0 && IsDriveTypeCDROM(HIWORD(UserData)) && UserDataSize == 3)
	{
		sprintf(RootDir,"%c:",LOBYTE(LOWORD(UserData)));

		if (!IsDiskInDrive(RootDir))
		{
			if (!EjectVolume(LOBYTE(LOWORD(UserData)),EJECT_READY|EJECT_NO_MESSAGE))
			{
				SaveScreen SvScrn;
				Message(0,0,"",MSG(MChangeWaitingLoadDisk));
				EjectVolume(LOBYTE(LOWORD(UserData)),EJECT_LOAD_MEDIA|EJECT_NO_MESSAGE);
			}
		}
	}

	if (ProcessPluginEvent(FE_CLOSE,NULL))
		return(-1);

	ScrBuf.Flush();
	INPUT_RECORD rec;
	PeekInputRecord(&rec);

	if (UserDataSize==3)
	{
		while (1)
		{
			int NumDisk=LOBYTE(LOWORD(UserData))-'A';
			char MsgStr[NM],NewDir[NM];
			sprintf(NewDir,"%c:",LOBYTE(LOWORD(UserData)));
			FarChDir(NewDir);
			CtrlObject->CmdLine->GetCurDir(NewDir);

			if (toupper(*NewDir)==LOBYTE(LOWORD(UserData)))
				FarChDir(NewDir);

			if (getdisk()!=NumDisk)
			{
				char RootDir[NM];
				sprintf(RootDir,"%c:\\",LOBYTE(LOWORD(UserData)));
				FarChDir(RootDir);

				if (getdisk()==NumDisk)
					break;
			}
			else
				break;

			sprintf(MsgStr,MSG(MChangeDriveCannotReadDisk),LOBYTE(LOWORD(UserData)));

			if (Message(MSG_WARNING,2,MSG(MError),MsgStr,MSG(MRetry),MSG(MCancel))!=0)
				return(-1);
		}

		char NewCurDir[NM];
		FarGetCurDir(sizeof(NewCurDir),NewCurDir);
		// BugZ#208. Если пути совпадают, то ничего не делаем.
		//_tran(SysLog("PanelMode=%i (%i), CurDir=[%s], NewCurDir=[%s]",PanelMode,GetType(),
		//            CurDir,NewCurDir);)

		if (PanelMode == NORMAL_PANEL && GetType()==FILE_PANEL && !LocalStricmp(CurDir,NewCurDir) && IsVisible())
		{
			// А нужно ли делать здесь Update????
			Update(UPDATE_KEEP_SELECTION);
		}
		else
		{
			Focus=GetFocus();
			Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,FALSE);
			NewPanel->SetCurDir(NewCurDir,TRUE);
			NewPanel->Show();

			if (Focus || !CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
				NewPanel->SetFocus();

			if (!Focus && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}
	else if (UserDataSize==2)
	{
#if 1

		if (CtrlObject->Plugins.CallPlugin(LOWORD(UserData),OPEN_DISKMENU,(void*)(INT_PTR)HIWORD(UserData),NULL,this,true))
			if (!GetFocus() && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();

#else
		HANDLE hPlugin=CtrlObject->Plugins.OpenPlugin(LOWORD(UserData),OPEN_DISKMENU,HIWORD(UserData));

		if (hPlugin!=INVALID_HANDLE_VALUE)
		{
			Focus=GetFocus();
			Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
			NewPanel->SetPluginMode(hPlugin,"",Focus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!Focus && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}

#endif
	}

	return(-1);
}

/* $ 28.12.2001 DJ
   обработка Del в меню дисков
*/

int Panel::ProcessDelDisk(char Drive, int DriveType,VMenu *ChDiskMenu)
{
	char MsgText[200];
	int UpdateProfile=CONNECT_UPDATE_PROFILE;
	BOOL Processed=FALSE;
	char DiskLetter [4];
	DiskLetter[0] = Drive;
	DiskLetter[1] = ':';
	DiskLetter[2] = 0;

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
			char MsgText[200];
			sprintf(MsgText,MSG(MChangeSUBSTDisconnectDriveQuestion),Drive);

			if (Message(MSG_WARNING,2,MSG(MChangeSUBSTDisconnectDriveTitle),MsgText,MSG(MYes),MSG(MNo))!=0)
				return DRIVE_DEL_FAIL;
		}

		if (!DelSubstDrive(DiskLetter))
			return DRIVE_DEL_SUCCESS;
		else
		{
			int LastError=GetLastError();
			sprintf(MsgText,MSG(MChangeDriveCannotDelSubst),DiskLetter);

			if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,
				            "\x1",MSG(MChangeDriveOpenFiles),
				            MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
				{
					if (!DelSubstDrive(DiskLetter))
						return DRIVE_DEL_SUCCESS;
				}
				else
					return DRIVE_DEL_FAIL;

			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MsgText,MSG(MOk));
		}

		return DRIVE_DEL_FAIL; // блин. в прошлый раз забыл про это дело...
	}

	/* SVS $ */

	if (Processed)
	{
		if (WNetCancelConnection2(DiskLetter,UpdateProfile,FALSE)==NO_ERROR)
			return DRIVE_DEL_SUCCESS;
		else
		{
			int LastError=GetLastError();
			sprintf(MsgText,MSG(MChangeDriveCannotDisconnect),DiskLetter);

			if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,
				            "\x1",MSG(MChangeDriveOpenFiles),
				            MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
				{
					if (WNetCancelConnection2(DiskLetter,UpdateProfile,TRUE)==NO_ERROR)
						return DRIVE_DEL_SUCCESS;
				}
				else
					return DRIVE_DEL_FAIL;

			char RootDir[50];
			sprintf(RootDir,"%c:\\",*DiskLetter);

			if (FAR_GetDriveType(RootDir)==DRIVE_REMOTE)
				Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MsgText,MSG(MOk));
		}

		return DRIVE_DEL_FAIL;
	}

	return DRIVE_DEL_FAIL;
}
/* DJ $ */


void Panel::FastFindProcessName(Edit *FindEdit,const char *Src,char *LastName,char *Name)
{
	//_SVS(SysLog("Panel::FastFindProcessName ==> Src='%s',LastName='%s',Name='%s'",Src,LastName,Name));
	if (strlen(Src) <= NM*2) // сделаем разумное ограничение на размер...
	{
		char *Ptr=(char *)xf_malloc(strlen(Src)+strlen(FindEdit->GetStringAddr())+32);

		if (Ptr)
		{
			strcpy(Ptr,FindEdit->GetStringAddr());
			char *EndPtr=Ptr+strlen(Ptr);
			strcat(Ptr,Src);
			Unquote(EndPtr);
			EndPtr=Ptr+strlen(Ptr);
			DWORD Key;

			while (1)
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
					strcpy(LastName,Ptr);
					strcpy(Name,Ptr);
					FindEdit->Show();
					break;
				}

				*--EndPtr=0;
			}

			xf_free(Ptr);
		}
	}

	//_SVS(SysLog("Panel::FastFindProcessName <<== Src='%s',LastName='%s',Name='%s'",Src,LastName,Name));
}


__int64 Panel::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return _i64(0);
}

// корректировка букв
static DWORD _CorrectFastFindKbdLayout(INPUT_RECORD *rec,DWORD Key)
{
	if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && (Key&KEY_ALT))// && Key!=(KEY_ALT|0x3C))
	{
		// // _SVS(SysLog("_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
		if (rec->Event.KeyEvent.uChar.AsciiChar && (Key&KEY_MASKF) != rec->Event.KeyEvent.uChar.AsciiChar) //???
			Key=(Key&0xFFFFFF00)|rec->Event.KeyEvent.uChar.AsciiChar;   //???

		// // _SVS(SysLog("_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
	}

	return Key;
}

void Panel::FastFind(int FirstKey)
{
	//_SVS(CleverSysLog Clev("Panel::FastFind"));
	INPUT_RECORD rec;
	char LastName[NM],Name[NM];
	int Key,KeyToProcess=0;
	*LastName=0;
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
				//_SVS(SysLog("[%d] Panel::FastFind  FirstKey=%s  %s",__LINE__,_FARKEY_ToName(FirstKey),_INPUT_RECORD_Dump(FrameManager->GetLastInputRecord())));
				Key=FirstKey;
			}
			else
			{
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
					//_SVS(SysLog("[%d] Panel::FastFind  Key=%s",__LINE__,_FARKEY_ToName(Key)));
					// для вставки воспользуемся макродвижком...
					if (Key==KEY_CTRLV || Key==KEY_SHIFTINS || Key==KEY_SHIFTNUMPAD0)
					{
						char *ClipText=PasteFromClipboard();

						if (ClipText && *ClipText)
						{
							FastFindProcessName(&FindEdit,ClipText,LastName,Name);
							FastFindShow(FindX,FindY);
							xf_free(ClipText);
						}

						continue;
					}
					else if (Key == KEY_OP_XLAT)
					{
						char TempName[NM*2];
						FindEdit.Xlat();
						FindEdit.GetString(TempName,sizeof(TempName));
						FindEdit.SetString("");
						FastFindProcessName(&FindEdit,TempName,LastName,Name);
						FastFindShow(FindX,FindY);
						continue;
					}
					else if (Key == KEY_OP_DATE || Key == KEY_OP_PLAINTEXT) // MCODE_??????
					{
						char TempName[NM*2];
						FindEdit.ProcessKey(Key);
						FindEdit.GetString(TempName,sizeof(TempName));
						FindEdit.SetString("");
						FastFindProcessName(&FindEdit,TempName,LastName,Name);
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

			//_SVS(SysLog("[%d] Panel::FastFind  Key=%s",__LINE__,_FARKEY_ToName(Key)));

			if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255)
				Key=tolower(Key-KEY_ALT_BASE);

			if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255)
				Key=tolower(Key-KEY_ALTSHIFT_BASE);

			if (Key==KEY_MULTIPLY)
				Key='*';

			switch (Key)
			{
				case KEY_F1:
				{
					FindEdit.Hide();
					SaveScr.RestoreArea();
					{
						Help Hlp("FastFind");
					}
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				}
				case KEY_CTRLNUMENTER:
				case KEY_CTRLENTER:
					FindPartName(Name,TRUE,1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
					FindPartName(Name,TRUE,-1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_NONE:
				case KEY_IDLE:
					break;
				default:

					if ((Key<32 || Key>=256) && Key!=KEY_BS && Key!=KEY_CTRLY &&
					        Key!=KEY_CTRLBS && Key!=KEY_ALT && Key!=KEY_SHIFT &&
					        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
					        !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && !(Key==KEY_SHIFTINS||Key==KEY_SHIFTNUMPAD0))
					{
						KeyToProcess=Key;
						break;
					}

					if (FindEdit.ProcessKey(Key))
					{
						FindEdit.GetString(Name,sizeof(Name));
						//_SVS(SysLog("[%d] Panel::FastFind  FindEdit.GetString ==> Name=%s",__LINE__,Name));
						// уберем двойные '**'
						int LenName=(int)strlen(Name);

						if (LenName > 1 && Name[LenName-1] == '*' && Name[LenName-2] == '*')
						{
							Name[LenName-1]=0;
							FindEdit.SetString(Name);
						}

						/* $ 09.04.2001 SVS
						   проблемы с быстрым поиском.
						   Подробнее в 00573.ChangeDirCrash.txt
						*/
						if (*Name == '"')
						{
							memmove(Name,Name+1,sizeof(Name)-1);
							Name[strlen(Name)-1]=0;
							FindEdit.SetString(Name);
						}

						/* SVS $ */

						//_SVS(SysLog("[%d] Panel::FastFind  Call FindPartName(Name=%s)",__LINE__,Name));
						if (FindPartName(Name,FALSE,1,1))
							strcpy(LastName,Name);
						else
						{
							if (CtrlObject->Macro.IsExecuting())// && CtrlObject->Macro.GetLevelState() > 0) // если вставка макросом...
							{
								//CtrlObject->Macro.DropProcess(); // ... то дропнем макропроцесс
//                CtrlObject->Macro.PopState();
								;
							}

							FindEdit.SetString(LastName);
							strcpy(Name,LastName);
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
	Text(" ");
	GotoXY(FindX+20,FindY+1);
	Text(" ");
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
		FarChDir(CurDir);
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
		if (MouseEvent->dwMousePosition.X==ScrX)
		{
			if (Opt.ScreenSaver && (MouseEvent->dwButtonState & 3)==0)
			{
				EndDrag();
				ScreenSaver(TRUE);
				return(TRUE);
			}
		}
		else if ((MouseEvent->dwButtonState & 3)!=0 && MouseEvent->dwEventFlags==0)
		{
			EndDrag();

			if (MouseEvent->dwMousePosition.X==0)
				CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
			else
				ShellOptions(0,MouseEvent);

			return(TRUE);
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
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
			        !ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && DragSaveScr==NULL)
				{
					SrcDragPanel->GoToFile(DragName);
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

	if ((MouseEvent->dwButtonState & 3)==0)
		return(TRUE);

	if ((MouseEvent->dwButtonState & 1) && MouseEvent->dwEventFlags==0 &&
	        X2-X1<ScrX)
	{
		int FileAttr;
		MoveToMouse(MouseEvent);
		GetSelName(NULL,FileAttr);

		if (GetSelName(DragName,FileAttr) && !TestParentFolderName(DragName))
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
	char DragMsg[NM],SelName[NM];
	int SelCount,MsgX,Length;

	if ((SelCount=SrcDragPanel->GetSelCount())==0)
		return;

	if (SelCount==1)
	{
		char CvtName[NM+16];
		int FileAttr;
		SrcDragPanel->GetSelName(NULL,FileAttr);
		SrcDragPanel->GetSelName(SelName,FileAttr);
		strcpy(CvtName,PointToName(SelName));
		QuoteSpace(CvtName);
		strcpy(SelName,CvtName);
	}
	else
		sprintf(SelName,MSG(MDragFiles),SelCount);

	if (Move)
		sprintf(DragMsg,MSG(MDragMove),SelName);
	else
		sprintf(DragMsg,MSG(MDragCopy),SelName);

	if ((Length=(int)strlen(DragMsg))+X>ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			/* $ 01.10.2001 IS усекаем с конца, иначе теряется инф.нагрузка */
			Length=(int)strlen(TruncStrFromEnd(DragMsg,ScrX));
			/* IS $ */
		}
	}
	else
		MsgX=X;

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	delete DragSaveScr;
	DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(DragMsg);
}


int Panel::GetCurDir(char *CurDir)
{
	if (CurDir)
		strcpy(CurDir,Panel::CurDir); // TODO: ОПАСНО!!!

	return (int)strlen(Panel::CurDir);
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
BOOL Panel::SetCurDir(const char *CurDir,int ClosePlugin)
{
	_CHANGEDIR(CleverSysLog clv("Panel::SetCurDir"));
	_CHANGEDIR(SysLog("(CurDir=\"%s\", ClosePlugin=%d)",CurDir,ClosePlugin));
	_CHANGEDIR(SysLog("Panel::CurDir=\"%s\"",Panel::CurDir));
	_CHANGEDIR(SysLog("PanelMode!=PLUGIN_PANEL ==> %d",PanelMode!=PLUGIN_PANEL));

	if (LocalStricmp(Panel::CurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		xstrncpy(Panel::CurDir,CurDir,sizeof(Panel::CurDir)-1);

		if (PanelMode!=PLUGIN_PANEL)
			PrepareDiskPath(Panel::CurDir,sizeof(Panel::CurDir)-1);
	}

	_CHANGEDIR(SysLog("Panel::CurDir=\"%s\"",Panel::CurDir));
	return TRUE;
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void Panel::InitCurDir(char *CurDir)
{
	_CHANGEDIR(CleverSysLog clv("Panel::InitCurDir"));
	_CHANGEDIR(SysLog("(CurDir=\"%s\")",CurDir));
	_CHANGEDIR(SysLog("Panel::CurDir=\"%s\"",Panel::CurDir));
	_CHANGEDIR(SysLog("PanelMode!=PLUGIN_PANEL ==> %d",PanelMode!=PLUGIN_PANEL));

	if (LocalStricmp(Panel::CurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		xstrncpy(Panel::CurDir,CurDir,sizeof(Panel::CurDir)-1);

		if (PanelMode!=PLUGIN_PANEL)
			PrepareDiskPath(Panel::CurDir,sizeof(Panel::CurDir)-1);
	}

	_CHANGEDIR(SysLog("Panel::CurDir=\"%s\"",Panel::CurDir));
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
int  Panel::SetCurPath()
{
	if (GetMode()==PLUGIN_PANEL)
		return TRUE;

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=PLUGIN_PANEL)
	{
		if (isalpha(AnotherPanel->CurDir[0]) && AnotherPanel->CurDir[1]==':' &&
		        toupper(AnotherPanel->CurDir[0])!=toupper(CurDir[0]))
		{
			// сначала установим переменные окружения для пассивной панели
			// (без реальной смены пути, чтобы лишний раз пассивный каталог
			// не перечитывать)
			FarChDir(AnotherPanel->CurDir,FALSE);
		}
	}

	if (!FarChDir(CurDir)||(!(PathPrefix(CurDir)&&!strncmp(&CurDir[4],"pipe",4))&&GetFileAttributes(CurDir)==INVALID_FILE_ATTRIBUTES))
	{
		// здесь на выбор :-)
#if 1

		while (!FarChDir(CurDir))
		{
			char Root[1024];
			GetPathRoot(CurDir,Root);

			if (FAR_GetDriveType(Root) != DRIVE_REMOVABLE || IsDiskInDrive(Root))
			{
				int Result=CheckFolder(CurDir);

				if (Result == CHKFLD_NOTFOUND)
				{
					if (CheckShortcutFolder(CurDir,sizeof(CurDir)-1,FALSE,TRUE) && FarChDir(CurDir))
					{
						SetCurDir(CurDir,TRUE);
						return TRUE;
					}
				}
				else
					break;
			}

			if (FrameManager && FrameManager->ManagerStarted()) // сначала проверим - а запущен ли менеджер
			{
				SetCurDir(FarPath,TRUE);                         // если запущен - выставим путь который мы точно знаем что существует
				ChangeDisk();                                    // и вызовем меню выбора дисков
			}
			else                                               // оппа...
			{
				char *PtrCurDir=PointToFolderNameIfFolder(CurDir); // подымаемся вверх, для очередной порции ChDir

				if (PtrCurDir != CurDir)                           // есть ли ещё куда подниматся?
				{
					*PtrCurDir=0;

					if (FarChDir(CurDir))
					{
						SetCurDir(CurDir,TRUE);
						break;
					}
				}
				else                                             // здесь проблема - видимо диск недоступен
				{
					SetCurDir(FarPath,TRUE);                       // тогда просто сваливаем в каталог, откуда стартанул FAR.
					break;
				}
			}
		}

#else

		do
		{
			BOOL IsChangeDisk=FALSE;
			char Root[1024];
			GetPathRoot(CurDir,Root);

			if (FAR_GetDriveType(Root) == DRIVE_REMOVABLE && !IsDiskInDrive(Root))
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
/* IS $ */
/* SVS $ */
/* KM $ */


void Panel::Hide()
{
	ScreenObject::Hide();
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible())
	{
		if (AnotherPanel->GetFocus())
			if (AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen() ||
			        GetType()==FILE_PANEL && IsFullScreen())
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

	/* IS $ */
	/* $ 09.05.2001 OT */
//  SavePrevScreen();
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible() && !GetModalMode())
	{
		/* $ 09.05.2001 OT */
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
		/* $ 19.05.2001 DJ
		   переписано для показа диалогов
		*/
		int Viewers=FrameManager->GetFrameCountByType(MODALTYPE_VIEWER);
		int Editors=FrameManager->GetFrameCountByType(MODALTYPE_EDITOR);
		int Dialogs=FrameManager->GetFrameCountByType(MODALTYPE_DIALOG);

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			char ScreensText[100];
			sprintf(ScreensText,"[%d", Viewers);

			if (Editors > 0)
				sprintf(ScreensText+strlen(ScreensText), "+%d", Editors);

			if (Dialogs > 0)
				sprintf(ScreensText+strlen(ScreensText), ",%d", Dialogs);

			strcat(ScreensText, "]");
			GotoXY(Opt.ShowColumnTitles ? X1:X1+2,Y1);
			SetColor(COL_PANELSCREENSNUMBER);
			Text(ScreensText);
		}

		/* DJ $ */
	}
}


void Panel::SetTitle()
{
	_MANAGERLOG(CleverSysLog Clev("Panel::SetTitle()"));

	if (GetFocus())
	{
		char TitleDir[NM+16];
		*TitleDir='{';

		if (*CurDir)
			xstrncpy(TitleDir+1,CurDir,sizeof(TitleDir)-3);
		else
		{
			char CmdText[512];
			// $ 21.07.2000 IG - Bug 21 (заголовок после Ctrl-Q, Tab, F3, Esc был кривой)
			CtrlObject->CmdLine->GetCurDir(CmdText);
			xstrncpy(TitleDir+1,CmdText,sizeof(TitleDir)-3);
		}

		xstrncat(TitleDir,"}",sizeof(TitleDir)-3);
		strcpy(LastFarTitle,TitleDir);
		SetFarTitle(TitleDir);
	}
}

const char *Panel::GetTitle(char *lTitle,int LenTitle,int TruncSize)
{
	char Title[512];
	char TitleDir[512];
	*Title=0;

	if (PanelMode==PLUGIN_PANEL)
	{
		struct OpenPluginInfo PInfo;
		GetOpenPluginInfo(&PInfo);
		RemoveExternalSpaces(xstrncpy(TitleDir,NullToEmpty(PInfo.PanelTitle),sizeof(TitleDir)-1));
		//RemoveExternalSpaces(TitleDir);
		TruncStr(TitleDir,LenTitle-TruncSize);
	}
	else
	{
		if (ShowShortNames)
			ConvertNameToShort(CurDir,TitleDir,sizeof(TitleDir)-1);
		else
			xstrncpy(TitleDir,CurDir,sizeof(TitleDir)-1);

		TruncPathStr(TitleDir,LenTitle-TruncSize);
	}

	if (*TitleDir)
		sprintf(Title," %s ",TitleDir);

	xstrncpy(lTitle,Title,LenTitle);
	return lTitle;
}

int Panel::SetPluginCommand(int Command,void *Param)
{
	_ALGO(CleverSysLog clv("Panel::SetPluginCommand"));
	_ALGO(SysLog("(Command=%s, Param=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param,Param));
	int Result=FALSE;
	ProcessingPluginCommand++;
	FilePanels *FPanels=CtrlObject->Cp();
	Panel *AnotherPanel=FPanels->GetAnotherPanel(this);
	PluginCommand=Command;

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
		case FCTL_SETANOTHERVIEWMODE:
			Result=FPanels->ChangePanelViewMode(((Command==FCTL_SETVIEWMODE) ? this:AnotherPanel),(Param?*(int *)Param:0),
			                                    FPanels->IsTopFrame());
			break;
		case FCTL_SETSORTMODE:
		case FCTL_SETANOTHERSORTMODE:

			if (Param!=NULL)
			{
				int Mode=*(int *)Param;

				if ((Mode>SM_DEFAULT) && (Mode<=SM_NUMLINKS))
				{
					Panel *DestPanel=(Command==FCTL_SETSORTMODE) ? this:AnotherPanel;

					if (DestPanel!=NULL)
					{
						DestPanel->SetSortMode(--Mode); // Уменьшим на 1 из-за SM_DEFAULT
						Result=TRUE;
					}
				}
			}

			break;
		case FCTL_SETNUMERICSORT:
		case FCTL_SETANOTHERNUMERICSORT:
		{
			int NumericSortOrder = (Param && (*(int *)Param)) ? 1:0;
			Panel *DestPanel=(Command==FCTL_SETNUMERICSORT) ? this:AnotherPanel;

			if (DestPanel!=NULL)
			{
				DestPanel->SetNumericSort(NumericSortOrder);
				Result=TRUE;
			}
		}
		break;
		case FCTL_SETSORTORDER:
		case FCTL_SETANOTHERSORTORDER:
		{
			/* $ 22.01.2001 VVM
			   - Порядок сортировки задается аналогично StartSortOrder */
			int Order = (Param && (*(int *)Param)) ? -1:1;
			/* VVM $ */
			Panel *DestPanel=(Command==FCTL_SETSORTORDER) ? this:AnotherPanel;

			if (DestPanel!=NULL)
			{
				// $ 24.04.2001 VVVM Использовать функция ChangeSortOrder()
				DestPanel->ChangeSortOrder(Order);
				Result=TRUE;
			}
		}
		break;
		case FCTL_CLOSEPLUGIN:
			xstrncpy((char *)PluginParam,NullToEmpty((char *)Param),sizeof(PluginParam)-1);
			Result=TRUE;
			//if(Opt.CPAJHefuayor)
			//  CtrlObject->Plugins.ProcessCommandLine((char *)PluginParam);
			break;
		case FCTL_GETPANELINFO:
		case FCTL_GETANOTHERPANELINFO:
		case FCTL_GETPANELSHORTINFO:
		case FCTL_GETANOTHERPANELSHORTINFO:
		{
			if (Param == NULL || IsBadWritePtr(Param,sizeof(struct PanelInfo)))
				break;

			struct PanelInfo *Info=(struct PanelInfo *)Param;
			memset(Info,0,sizeof(*Info));
			Panel *DestPanel=(Command == FCTL_GETPANELINFO || Command == FCTL_GETPANELSHORTINFO) ? this:AnotherPanel;

			if (DestPanel==NULL)
				break;

			/* $ 19.03.2002 DJ
			   обеспечим наличие данных
			*/
			DestPanel->UpdateIfRequired();

			/* DJ $ */
			switch (DestPanel->GetType())
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

			Info->Plugin=DestPanel->GetMode()==PLUGIN_PANEL;
			int X1,Y1,X2,Y2;
			DestPanel->GetPosition(X1,Y1,X2,Y2);
			Info->PanelRect.left=X1;
			Info->PanelRect.top=Y1;
			Info->PanelRect.right=X2;
			Info->PanelRect.bottom=Y2;
			Info->Visible=DestPanel->IsVisible();
			Info->Focus=DestPanel->GetFocus();
			Info->ViewMode=DestPanel->GetViewMode();
			Info->SortMode=SM_UNSORTED-UNSORTED+DestPanel->GetSortMode();
			DestPanel->GetCurDir(Info->CurDir);
			/* $ 24.04.2001 SVS
			   Заполнение флагов PanelInfo.Flags
			*/
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

				for (int I=0; I < sizeof(PFLAGS)/sizeof(PFLAGS[0]); ++I)
					if (*(PFLAGS[I].Opt) != 0)
						Flags|=PFLAGS[I].Flags;

				Flags|=DestPanel->GetSortOrder()<0?PFLAGS_REVERSESORTORDER:0;
				Flags|=DestPanel->GetSortGroups()?PFLAGS_USESORTGROUPS:0;
				Flags|=DestPanel->GetSelectedFirstMode()?PFLAGS_SELECTEDFIRST:0;
				Flags|=DestPanel->GetNumericSort()?PFLAGS_NUMERICSORT:0;

				if (CtrlObject->Cp()->LeftPanel == DestPanel)
					Flags|=PFLAGS_PANELLEFT;

				Info->Flags=Flags;
			}

			/* SVS $ */
			if (DestPanel->GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)DestPanel;
				static int Reenter=0;

				if (!Reenter && Info->Plugin)
				{
					Reenter++;
					struct OpenPluginInfo PInfo;
					DestFilePanel->GetOpenPluginInfo(&PInfo);
					xstrncpy(Info->CurDir,PInfo.CurDir,sizeof(Info->CurDir)-1);

					/* $ 12.12.2001 DJ
					   обработаем флаги
					*/
					if (PInfo.Flags & OPIF_REALNAMES)
						Info->Flags |= PFLAGS_REALNAMES;

					if (!(PInfo.Flags & OPIF_USEHIGHLIGHTING))
						Info->Flags &= ~PFLAGS_HIGHLIGHT;

					/* DJ $ */
					Reenter--;
				}

				DestFilePanel->PluginGetPanelInfo(Info,(Command == FCTL_GETPANELINFO || Command == FCTL_GETANOTHERPANELINFO)?TRUE:FALSE);
			}

			if (!Info->Plugin) // $ 12.12.2001 DJ - на неплагиновой панели - всегда реальные имена
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}
		case FCTL_SETSELECTION:
		case FCTL_SETANOTHERSELECTION:
		{
			Panel *DestPanel=(Command==FCTL_SETSELECTION) ? this:AnotherPanel;

			if (DestPanel && DestPanel->GetType()==FILE_PANEL && !IsBadReadPtr(Param,sizeof(struct PanelInfo)))
			{
				((FileList *)DestPanel)->PluginSetSelection((struct PanelInfo *)Param);
				Result=TRUE;
			}

			break;
		}
		case FCTL_UPDATEPANEL:
			Update(Param==NULL ? 0:UPDATE_KEEP_SELECTION);
			Result=TRUE;
			break;
		case FCTL_UPDATEANOTHERPANEL:
			AnotherPanel->Update(Param==NULL ? 0:UPDATE_KEEP_SELECTION);

			if (AnotherPanel!=NULL && AnotherPanel->GetType()==QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;
		case FCTL_REDRAWPANEL:
		case FCTL_REDRAWANOTHERPANEL:
		{
			Panel *DestPanel=Command==FCTL_REDRAWANOTHERPANEL?AnotherPanel:this;

			if (DestPanel)
			{
				struct PanelRedrawInfo *Info=(struct PanelRedrawInfo *)Param;

				if (Info && !IsBadReadPtr(Info,sizeof(struct PanelRedrawInfo)))
				{
					DestPanel->CurFile=Info->CurrentItem;
					DestPanel->CurTopFile=Info->TopPanelItem;
				}

				// $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущий фрейм
				if (FPanels->IsTopFrame())
				{
					DestPanel->Redraw();
					Result=TRUE;
				}
			}

			break;
		}
		// $ 03.12.2001 DJ - скорректируем путь
		case FCTL_SETPANELDIR:
		case FCTL_SETANOTHERPANELDIR:
		{
			Panel *DestPanel=Command==FCTL_SETANOTHERPANELDIR?AnotherPanel:this;

			if (DestPanel && Param)
			{
				DestPanel->SetCurDir((char *)Param,TRUE);
				Result=TRUE;
			}

			break;
		}
	}

	ProcessingPluginCommand--;
	return Result;
}


int Panel::GetCurName(char *Name,char *ShortName)
{
	*Name=*ShortName=0;
	return(FALSE);
}

int Panel::GetCurBaseName(char *Name,char *ShortName)
{
	*Name=*ShortName=0;
	return(FALSE);
}

static int MessageRemoveConnection(char Letter, int &UpdateProfile)
{
	int Len1, Len2, Len3,Len4;
	BOOL IsPersistent;
	char MsgText[NM];
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
	static struct DialogData DCDlgData[]=
	{
		/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
		                                      Selected               Data
		*/
		/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 9, 0, 0, 0,                0,"",
		/* 1 */ DI_TEXT,      5, 2,  0, 2, 0, 0, DIF_SHOWAMPERSAND,0,"",
		/* 2 */ DI_TEXT,      5, 3,  0, 3, 0, 0, DIF_SHOWAMPERSAND,0,"",
		/* 3 */ DI_TEXT,      5, 4,  0, 4, 0, 0, DIF_SHOWAMPERSAND,0,"",
		/* 4 */ DI_TEXT,      0, 5,  0, 5, 0, 0, DIF_SEPARATOR,    0,"",
		/* 5 */ DI_CHECKBOX,  5, 6, 70, 6, 0, 0, 0,                0,"",
		/* 6 */ DI_TEXT,      0, 7,  0, 7, 0, 0, DIF_SEPARATOR,    0,"",
		/* 7 */ DI_BUTTON,    0, 8,  0, 8, 1, 0, DIF_CENTERGROUP,  1,"",
		/* 8 */ DI_BUTTON,    0, 8,  0, 8, 0, 0, DIF_CENTERGROUP,  0,""
	};
	MakeDialogItems(DCDlgData,DCDlg);
	Len1=(int)strlen(strcpy(DCDlg[0].Data,MSG(MChangeDriveDisconnectTitle)));
	sprintf(MsgText,MSG(MChangeDriveDisconnectQuestion),Letter);
	Len2=(int)strlen(strcpy(DCDlg[1].Data,MsgText));
	sprintf(MsgText,MSG(MChangeDriveDisconnectMapped),Letter);
	Len4=(int)strlen(strcpy(DCDlg[2].Data,MsgText));
	Len3=(int)strlen(strcpy(DCDlg[5].Data,MSG(MChangeDriveDisconnectReconnect)));
	Len1=Max(Len1,Max(Len2,Max(Len3,Len4)));
	strcpy(DCDlg[3].Data,TruncPathStr(DriveLocalToRemoteName(DRIVE_REMOTE,Letter,MsgText),Len1));
	strcpy(DCDlg[7].Data,MSG(MYes));
	strcpy(DCDlg[8].Data,MSG(MCancel));
	// проверяем - это было постоянное соедение или нет?
	// Если ветка в реестре HKCU\Network\БукваДиска есть - это
	//   есть постоянное подключение.
	{
		HKEY hKey;
		IsPersistent=TRUE;

		if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
			sprintf(MsgText,"Network\\%c",toupper(Letter));
		else
			sprintf(MsgText,"Network\\Persistent\\%c",toupper(Letter));

		if (RegOpenKeyEx(HKEY_CURRENT_USER,MsgText,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		{
			DCDlg[5].Flags|=DIF_DISABLE;
			DCDlg[5].Selected=0;
			IsPersistent=FALSE;
		}
		else
		{
			DCDlg[5].Selected=Opt.ChangeDriveDisconnetMode;
			RegCloseKey(hKey);
		}
	}
	// скорректируем размеры диалога - для дизайнУ
	DCDlg[0].X2=DCDlg[0].X1+Len1+3;
	int ExitCode=7;

	if (Opt.Confirm.RemoveConnection)
	{
		Dialog Dlg(DCDlg,sizeof(DCDlg)/sizeof(DCDlg[0]));
		Dlg.SetPosition(-1,-1,DCDlg[0].X2+4,11);
		Dlg.SetHelp("DisconnectDrive");
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
	        LocalStricmp(AnotherPanel->CurDir,CurDir)==0)
		return TRUE;

	return FALSE;
}

int Panel::ProcessShortcutFolder(int Key,BOOL ProcTreePanel)
{
	char *ShortcutFolder;
	char PluginModule[NM],PluginFile[NM],PluginData[MAXSIZE_SHORTCUTDATA];
	int SizeFolderNameShortcut=GetShortcutFolderSize(Key);
	ShortcutFolder=new char[SizeFolderNameShortcut+NM];

	if (ShortcutFolder)
	{
		if (GetShortcutFolder(Key,ShortcutFolder,SizeFolderNameShortcut+NM,PluginModule,PluginFile,PluginData))
		{
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

			if (ProcTreePanel)
			{
				if (AnotherPanel->GetType()==FILE_PANEL)
				{
					AnotherPanel->SetCurDir(ShortcutFolder,TRUE);
					AnotherPanel->Redraw();
				}
				else
				{
					SetCurDir(ShortcutFolder,TRUE);
					ProcessKey(KEY_ENTER);
				}
			}
			else
			{
				if (AnotherPanel->GetType()==FILE_PANEL && *PluginModule==0)
				{
					AnotherPanel->SetCurDir(ShortcutFolder,TRUE);
					AnotherPanel->Redraw();
				}
			}

			delete[] ShortcutFolder;
			return(TRUE);
		}

		delete[] ShortcutFolder;
	}

	return FALSE;
}

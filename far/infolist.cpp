﻿/*
infolist.cpp

Информационная панель
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

#include "infolist.hpp"

#include "imports.hpp"
#include "macroopcode.hpp"
#include "flink.hpp"
#include "farcolor.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "manager.hpp"
#include "cddrv.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "drivemix.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "colormix.hpp"
#include "vmenu2.hpp"
#include "lang.hpp"
#include "dizviewer.hpp"
#include "keybar.hpp"
#include "keyboard.hpp"
#include "datetime.hpp"
#include "cvtname.hpp"
#include "vmenu.hpp"
#include "global.hpp"

#include "platform.fs.hpp"

#include "common/enum_tokens.hpp"

#include "format.hpp"

static bool LastMode = false;
static bool LastDizWrapMode = false;
static bool LastDizWrapType = false;
static bool LastDizShowScrollbar = false;

enum InfoListSectionStateIndex
{
	// Порядок не менять! Только добавлять в конец!
	ILSS_DISKINFO,
	ILSS_MEMORYINFO,
	ILSS_DIRDESCRIPTION,
	ILSS_PLDESCRIPTION,
	ILSS_POWERSTATUS,

	ILSS_SIZE
};

struct InfoList::InfoListSectionState
{
	bool Show;   // раскрыть/свернуть?
	SHORT Y;     // Где?
};

info_panel_ptr InfoList::create(window_ptr Owner)
{
	return std::make_shared<InfoList>(private_tag(), Owner);
}

InfoList::InfoList(private_tag, window_ptr Owner):
	Panel(std::move(Owner)),
	OldWrapMode(),
	OldWrapType(),
	SectionState(ILSS_SIZE),
	PowerListener(update_power, [&]{ if (Global->Opt->InfoPanel.ShowPowerStatus && IsVisible() && SectionState[ILSS_POWERSTATUS].Show) { Redraw(); }})
{
	m_Type = panel_type::INFO_PANEL;
	if (Global->Opt->InfoPanel.strShowStatusInfo.empty())
	{
		std::for_each(RANGE(SectionState, i)
		{
			i.Show=true;
		});
	}
	else
	{
		size_t strShowStatusInfoLen = Global->Opt->InfoPanel.strShowStatusInfo.size();
		for_each_cnt(RANGE(SectionState, i, size_t index)
		{
			i.Show = index < strShowStatusInfoLen?Global->Opt->InfoPanel.strShowStatusInfo[index] == L'1':true;
		});
	}

	if (!LastMode)
	{
		LastMode = true;
		LastDizWrapMode = Global->Opt->ViOpt.ViewerIsWrap;
		LastDizWrapType = Global->Opt->ViOpt.ViewerWrap;
		LastDizShowScrollbar = Global->Opt->ViOpt.ShowScrollbar;
	}
}

InfoList::~InfoList()
{
	InfoList::CloseFile();
}

// перерисовка, только если мы текущее окно
void InfoList::Update(int Mode)
{
	if (!m_EnableUpdate)
		return;

	if (GetOwner().get() == Global->WindowManager->GetCurrentWindow().get())
		Redraw();
}

string InfoList::GetTitle() const
{
	return msg(lng::MInfoTitle);
}

void InfoList::DrawTitle(string &strTitle,int Id,int &CurY)
{
	SetColor(COL_PANELBOX);
	DrawSeparator(CurY);
	SetColor(COL_PANELTEXT);
	TruncStr(strTitle,m_X2-m_X1-3);
	GotoXY(m_X1+(m_X2-m_X1+1-(int)strTitle.size())/2,CurY);
	PrintText(strTitle);
	GotoXY(m_X1+1,CurY);
	PrintText(SectionState[Id].Show? L"[-]"sv : L"[+]"sv);
	SectionState[Id].Y=CurY;
	CurY++;
}

void InfoList::DisplayObject()
{
	if (m_Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	m_Flags.Set(FSCROBJ_ISREDRAWING);

	string strOutStr;
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	string strDriveRoot;
	string strVolumeName, strFileSystemName;
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
	string strDiskNumber;
	CloseFile();

	Box(m_X1,m_Y1,m_X2,m_Y2,colors::PaletteColorToFarColor(COL_PANELBOX),DOUBLE_BOX);
	SetScreen(m_X1+1,m_Y1+1,m_X2-1,m_Y2-1,L' ',colors::PaletteColorToFarColor(COL_PANELTEXT));
	SetColor(IsFocused()? COL_PANELSELECTEDTITLE : COL_PANELTITLE);

	const auto& strTitle = GetTitleForDisplay();
	if (!strTitle.empty())
	{
		GotoXY(m_X1+(m_X2-m_X1+1-(int)strTitle.size())/2,m_Y1);
		Text(strTitle);
	}

	SetColor(COL_PANELTEXT);

	int CurY=m_Y1+1;

	/* #1 - computer name/user name */
	{
		string strComputerName;
		os::GetComputerNameEx(static_cast<COMPUTER_NAME_FORMAT>(Global->Opt->InfoPanel.ComputerNameFormat.Get()), strComputerName) || os::GetComputerName(strComputerName);

		GotoXY(m_X1+2,CurY++);
		PrintText(lng::MInfoCompName);
		PrintInfo(strComputerName);

		os::netapi::ptr<SERVER_INFO_101> ServerInfo;
		if (NetServerGetInfo(nullptr, 101, reinterpret_cast<LPBYTE*>(&ptr_setter(ServerInfo))) == NERR_Success)
		{
			if(ServerInfo->sv101_comment && *ServerInfo->sv101_comment)
			{
				GotoXY(m_X1+2,CurY++);
				PrintText(lng::MInfoCompDescription);
				PrintInfo(ServerInfo->sv101_comment);
			}
		}

		string strUserName;
		os::GetUserNameEx(static_cast<EXTENDED_NAME_FORMAT>(Global->Opt->InfoPanel.UserNameFormat.Get()), strUserName) || os::GetUserName(strUserName);

		GotoXY(m_X1+2,CurY++);
		PrintText(lng::MInfoUserName);
		PrintInfo(strUserName);

		os::netapi::ptr<USER_INFO_1> UserInfo;
		if (NetUserGetInfo(nullptr, strUserName.c_str(), 1, reinterpret_cast<LPBYTE*>(&ptr_setter(UserInfo))) == NERR_Success)
		{
			if(UserInfo->usri1_comment && *UserInfo->usri1_comment)
			{
				GotoXY(m_X1+2,CurY++);
				PrintText(lng::MInfoUserDescription);
				PrintInfo(UserInfo->usri1_comment);
			}

			lng LabelId;
			switch (UserInfo->usri1_priv)
			{
			case USER_PRIV_GUEST:
				LabelId = lng::MInfoUserAccessLevelGuest;
				break;

			case USER_PRIV_USER:
				LabelId = lng::MInfoUserAccessLevelUser;
				break;

			case USER_PRIV_ADMIN:
				LabelId = lng::MInfoUserAccessLevelAdministrator;
				break;

			default:
				LabelId = lng::MInfoUserAccessLevelUnknown;
				break;
			}
			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoUserAccessLevel);
			PrintInfo(LabelId);
		}

	}

	string SectionTitle;

	/* #2 - disk info */
	if (SectionState[ILSS_DISKINFO].Show)
	{
		m_CurDir = AnotherPanel->GetCurDir();

		if (m_CurDir.empty())
			m_CurDir = os::fs::GetCurrentDirectory();

		/*
			Корректно отображать инфу при заходе в Juction каталог
			Рут-диск может быть другим
		*/
		if (os::fs::file_status(m_CurDir).check(FILE_ATTRIBUTE_REPARSE_POINT))
		{
			string strJuncName;

			if (GetReparsePointInfo(m_CurDir, strJuncName))
			{
				NormalizeSymlinkName(strJuncName);
				strDriveRoot = GetPathRoot(strJuncName); //"\??\D:\Junc\Src\"
			}
		}
		else
			strDriveRoot = GetPathRoot(m_CurDir);

		if (os::fs::GetVolumeInformation(strDriveRoot,&strVolumeName,
		                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
		                            &strFileSystemName))
		{
			lng IdxMsgID = lng::MInfoUnknown;
			int DriveType=FAR_GetDriveType(strDriveRoot, Global->Opt->InfoPanel.ShowCDInfo);

			switch (DriveType)
			{
				case DRIVE_REMOVABLE:
					IdxMsgID = lng::MInfoRemovable;
					break;
				case DRIVE_FIXED:
					IdxMsgID = lng::MInfoFixed;
					break;
				case DRIVE_REMOTE:
					IdxMsgID = lng::MInfoNetwork;
					break;
				case DRIVE_CDROM:
					IdxMsgID = lng::MInfoCDROM;
					break;
				case DRIVE_RAMDISK:
					IdxMsgID = lng::MInfoRAM;
					break;
				default:

					if (IsDriveTypeCDROM(DriveType))
						IdxMsgID = lng::MInfoCD_RW + (DriveType - DRIVE_CD_RW);

					break;
			}

			auto DiskTypeId = IdxMsgID;
			string strAssocPath;

			if (GetSubstName(DriveType,strDriveRoot,strAssocPath))
			{
				DiskTypeId = lng::MInfoSUBST;
				DriveType=DRIVE_SUBSTITUTE;
			}
			else if(DriveCanBeVirtual(DriveType) && GetVHDInfo(strDriveRoot,strAssocPath))
			{
				DiskTypeId = lng::MInfoVirtual;
				DriveType=DRIVE_VIRTUAL;
			}

			SectionTitle = concat(L' ', msg(DiskTypeId), L' ', msg(lng::MInfoDisk), L' ', strDriveRoot, L" ("sv, strFileSystemName, L") "sv);

			switch(DriveType)
			{
				case DRIVE_REMOTE:
					{
						auto DeviceName = strDriveRoot;
						DeleteEndSlash(DeviceName);
						os::WNetGetConnection(DeviceName, strAssocPath);
					}
					// TODO: check result
					[[fallthrough]];
				case DRIVE_SUBSTITUTE:
				case DRIVE_VIRTUAL:
				{
					SectionTitle += strAssocPath;
					SectionTitle += L' ';
				}
				break;
			}

			strDiskNumber = format(L"{0:04X}-{1:04X}", HIWORD(VolumeNumber), LOWORD(VolumeNumber));
		}
		else // Error!
			SectionTitle = strDriveRoot;
	}

	if (!SectionState[ILSS_DISKINFO].Show)
		SectionTitle = msg(lng::MInfoDiskTitle);
	DrawTitle(SectionTitle,ILSS_DISKINFO,CurY);

	const auto bytes_suffix = upper(msg(lng::MListBytes));
	const auto& size2str = [&bytes_suffix](ULONGLONG Size)
	{
		string str;
		if (Global->Opt->ShowBytes)
		{
			str = GroupDigits(Size); // + L' ';
		}
		else
		{
			str = FileSizeToStr(Size, 16, COLUMN_FLOATSIZE | COLUMN_SHOWUNIT);
			if (str.back() != bytes_suffix[0])
				str += bytes_suffix;
		}
		return str;
	};

	if (SectionState[ILSS_DISKINFO].Show)
	{
		/* #2.2 - disk info: size */
		unsigned long long TotalSize, UserFree;

		if (os::fs::get_disk_size(m_CurDir,&TotalSize, nullptr, &UserFree))
		{
			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoDiskTotal);
			PrintInfo(size2str(TotalSize));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoDiskFree);
			PrintInfo(size2str(UserFree));
		}

		/* #4 - disk info: label & SN */
		GotoXY(m_X1+2,CurY++);
		PrintText(lng::MInfoDiskLabel);
		PrintInfo(strVolumeName);

		GotoXY(m_X1+2,CurY++);
		PrintText(lng::MInfoDiskNumber);
		PrintInfo(strDiskNumber);
	}

	/* #3 - memory info */
	SectionTitle = msg(lng::MInfoMemory);
	DrawTitle(SectionTitle, ILSS_MEMORYINFO, CurY);

	if (SectionState[ILSS_MEMORYINFO].Show)
	{
		MEMORYSTATUSEX ms={sizeof(ms)};
		if (GlobalMemoryStatusEx(&ms))
		{
			if (!ms.dwMemoryLoad)
				ms.dwMemoryLoad=100-ToPercent(ms.ullAvailPhys+ms.ullAvailPageFile,ms.ullTotalPhys+ms.ullTotalPageFile);

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoMemoryLoad);
			PrintInfo(str(ms.dwMemoryLoad) + L'%');

			ULONGLONG TotalMemoryInKilobytes=0;
			if(imports.GetPhysicallyInstalledSystemMemory(&TotalMemoryInKilobytes))
			{
				GotoXY(m_X1+2,CurY++);
				PrintText(lng::MInfoMemoryInstalled);
				PrintInfo(size2str(TotalMemoryInKilobytes << 10));
			}

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoMemoryTotal);
			PrintInfo(size2str(ms.ullTotalPhys));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoMemoryFree);
			PrintInfo(size2str(ms.ullAvailPhys));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoVirtualTotal);
			PrintInfo(size2str(ms.ullTotalVirtual));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoVirtualFree);
			PrintInfo(size2str(ms.ullAvailVirtual));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPageFileTotal);
			PrintInfo(size2str(ms.ullTotalPageFile));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPageFileFree);
			PrintInfo(size2str(ms.ullAvailPageFile));
		}
	}

	/* #4 - power status */
	if (Global->Opt->InfoPanel.ShowPowerStatus)
	{
		SectionTitle = msg(lng::MInfoPowerStatus);
		DrawTitle(SectionTitle, ILSS_POWERSTATUS, CurY);

		if (SectionState[ILSS_POWERSTATUS].Show)
		{
			lng MsgID;
			SYSTEM_POWER_STATUS PowerStatus;
			GetSystemPowerStatus(&PowerStatus);

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPowerStatusAC);
			switch(PowerStatus.ACLineStatus)
			{
				case AC_LINE_OFFLINE:      MsgID = lng::MInfoPowerStatusACOffline; break;
				case AC_LINE_ONLINE:       MsgID = lng::MInfoPowerStatusACOnline; break;
				case AC_LINE_BACKUP_POWER: MsgID = lng::MInfoPowerStatusACBackUp; break;
				default:                   MsgID = lng::MInfoPowerStatusACUnknown; break;
			}
			PrintInfo(msg(MsgID));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPowerStatusBCLifePercent);
			if (PowerStatus.BatteryLifePercent > 100)
				strOutStr = msg(lng::MInfoPowerStatusBCLifePercentUnknown);
			else
				strOutStr = str(PowerStatus.BatteryLifePercent) + L'%';
			PrintInfo(strOutStr);

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPowerStatusBC);
			strOutStr.clear();
			// PowerStatus.BatteryFlag == 0: The value is zero if the battery is not being charged and the battery capacity is between low and high.
			if (!PowerStatus.BatteryFlag || PowerStatus.BatteryFlag == BATTERY_FLAG_UNKNOWN)
				strOutStr=msg(lng::MInfoPowerStatusBCUnknown);
			else if (PowerStatus.BatteryFlag & BATTERY_FLAG_NO_BATTERY)
				strOutStr=msg(lng::MInfoPowerStatusBCNoSysBat);
			else
			{
				if (PowerStatus.BatteryFlag & BATTERY_FLAG_HIGH)
					strOutStr = msg(lng::MInfoPowerStatusBCHigh);
				else if (PowerStatus.BatteryFlag & BATTERY_FLAG_LOW)
					strOutStr = msg(lng::MInfoPowerStatusBCLow);
				else if (PowerStatus.BatteryFlag & BATTERY_FLAG_CRITICAL)
					strOutStr = msg(lng::MInfoPowerStatusBCCritical);

				if (PowerStatus.BatteryFlag & BATTERY_FLAG_CHARGING)
				{
					if (!strOutStr.empty())
						strOutStr += L' ';
					strOutStr += msg(lng::MInfoPowerStatusBCCharging);
				}
			}
			PrintInfo(strOutStr);

			const auto& GetBatteryTime = [](size_t SecondsCount)
			{
				if (SecondsCount == BATTERY_LIFE_UNKNOWN)
					return string(msg(lng::MInfoPowerStatusUnknown));

				string Days, Time;
				const std::chrono::seconds Seconds(SecondsCount);
				ConvertDuration(Seconds, Days, Time);
				if (Days != L"0"sv)
				{
					const auto Hours = str(std::chrono::duration_cast<std::chrono::hours>(Seconds).count());
					Time = Hours + Time.substr(2);
				}

				// drop msec
				return Time.substr(0, Time.size() - 4);
			};

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPowerStatusBCTimeRem);
			PrintInfo(GetBatteryTime(PowerStatus.BatteryLifeTime));

			GotoXY(m_X1+2,CurY++);
			PrintText(lng::MInfoPowerStatusBCFullTimeRem);
			PrintInfo(GetBatteryTime(PowerStatus.BatteryFullLifeTime));
		}
	}

	if (AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL)
	{
		/* #5 - description */
		SectionTitle = msg(lng::MInfoDescription);
		DrawTitle(SectionTitle, ILSS_DIRDESCRIPTION, CurY);

		if (SectionState[ILSS_DIRDESCRIPTION].Show)
		{
			if (CurY < m_Y2 && ShowDirDescription(CurY))
			{
				DizView->SetPosition(m_X1+1,CurY,m_X2-1,m_Y2-1);
				CurY=m_Y2-1;
			}
			else
			{
				GotoXY(m_X1+2,CurY++);
				PrintText(lng::MInfoDizAbsent);
			}
		}
	}

	if (AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		/* #6 - Plugin Description */
		SectionTitle = msg(lng::MInfoPlugin);
		DrawTitle(SectionTitle, ILSS_PLDESCRIPTION, CurY);
		if (SectionState[ILSS_PLDESCRIPTION].Show)
		{
			if (ShowPluginDescription(CurY))
			{
				;
			}
		}
	}

	m_Flags.Clear(FSCROBJ_ISREDRAWING);
}

long long InfoList::VMProcess(int OpCode, void* vParam, long long iParam)
{
	if (DizView)
		return DizView->VMProcess(OpCode,vParam,iParam);

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return 1;
	}

	return 0;
}

void InfoList::SelectShowMode()
{
	menu_item ShowModeMenuItem[]
	{
		{ msg(lng::MMenuInfoShowModeDisk), LIF_SELECTED },
		{ msg(lng::MMenuInfoShowModeMemory), 0 },
		{ msg(lng::MMenuInfoShowModeDirDiz), 0 },
		{ msg(lng::MMenuInfoShowModePluginDiz), 0 },
		{ msg(lng::MMenuInfoShowModePower), 0 },
	};

	for_each_cnt(CONST_RANGE(SectionState, i, size_t index)
	{
		ShowModeMenuItem[index].SetCheck(i.Show ? L'+':L'-');
	});

	if (!Global->Opt->InfoPanel.ShowPowerStatus)
	{
		ShowModeMenuItem[ILSS_POWERSTATUS].SetDisable(TRUE);
		ShowModeMenuItem[ILSS_POWERSTATUS].SetCheck(L' ');
	}

	int ShowCode=-1;
	int ShowMode=-1;

	{
		// ?????
		// {BFC64A26-F433-4cf3-A1DE-8361CF762F68}
		//DEFINE_GUID(InfoListSelectShowModeId,0xbfc64a26, 0xf433, 0x4cf3, 0xa1, 0xde, 0x83, 0x61, 0xcf, 0x76, 0x2f, 0x68);
		// ?????

		const auto ShowModeMenu = VMenu2::create(msg(lng::MMenuInfoShowModeTitle), std::as_const(ShowModeMenuItem), 0);
		ShowModeMenu->SetHelp(L"InfoPanelShowMode"sv);
		ShowModeMenu->SetPosition(m_X1+4,-1,0,0);
		ShowModeMenu->SetMenuFlags(VMENU_WRAPMODE);

		ShowCode = ShowModeMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			int KeyProcessed = 1;
			switch (Key)
			{
				case KEY_MULTIPLY:
				case L'*':
					ShowMode=2;
					ShowModeMenu->Close();
					break;

				case KEY_ADD:
				case L'+':
					ShowMode=1;
					ShowModeMenu->Close();
					break;

				case KEY_SUBTRACT:
				case L'-':
					ShowMode=0;
					ShowModeMenu->Close();
					break;

				default:
					KeyProcessed = 0;
			}
			return KeyProcessed;
		});

		if (ShowCode<0)
			return;
	}

	switch (ShowMode)
	{
	case 0:
		SectionState[ShowCode].Show=false;
		break;
	case 1:
		SectionState[ShowCode].Show=true;
		break;
	default:
		SectionState[ShowCode].Show=!SectionState[ShowCode].Show;
		break;
	}
	Global->Opt->InfoPanel.strShowStatusInfo.clear();
	std::for_each(RANGE(SectionState, i)
	{
		Global->Opt->InfoPanel.strShowStatusInfo += i.Show? L"1"s : L"0"s;
	});

	Redraw();
}

bool InfoList::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (!IsVisible())
		return false;

	if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
	{
		ExecShortcutFolder(LocalKey-KEY_RCTRL0);
		return true;
	}

	switch (LocalKey)
	{
		case KEY_F1:
		{
			Help::create(L"InfoPanel"sv);
			return true;
		}
		case KEY_CTRLF12:
		case KEY_RCTRLF12:
			SelectShowMode();
			return true;
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (!strDizFileName.empty())
			{
				m_CurDir = Parent()->GetAnotherPanel(this)->GetCurDir();
				FarChDir(m_CurDir);
				FileViewer::create(strDizFileName, true);
			}

			Parent()->Redraw();
			return true;
		case KEY_F4:
			/* $ 30.04.2001 DJ
			не показываем редактор, если ничего не задано в именах файлов;
			не редактируем имена описаний со звездочками;
			убираем лишнюю перерисовку панелей
			*/
		{
			const auto AnotherPanel = Parent()->GetAnotherPanel(this);
			m_CurDir = AnotherPanel->GetCurDir();
			FarChDir(m_CurDir);

			if (!strDizFileName.empty())
			{
				FileEditor::create(strDizFileName,CP_DEFAULT,FFILEEDIT_ENABLEF6);
			}
			else
			{
				for (const auto& i: enum_tokens_with_quotes(Global->Opt->InfoPanel.strFolderInfoFiles.Get(), L",;"sv))
				{
					if (i.find_first_of(L"*?"sv) == string::npos)
					{
						FileEditor::create(i, CP_DEFAULT, FFILEEDIT_CANNEWFILE | FFILEEDIT_ENABLEF6);
						break;
					}
				}
			}

			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			//AnotherPanel->Redraw();
			Update(0);
			Parent()->Redraw();
			return true;
		}
		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			Redraw();
			return true;
		}
	}

	if (DizView && LocalKey >= 256)
	{
		int DVX1,DVX2,DVY1,DVY2;
		DizView->GetPosition(DVX1,DVY1,DVX2,DVY2);

		if (DVY1 < m_Y2)
		{
			const auto ret = DizView->ProcessKey(Key);

			if (LocalKey == KEY_F2 || LocalKey == KEY_SHIFTF2
			 || LocalKey == KEY_F4 || LocalKey == KEY_SHIFTF4
			 || LocalKey == KEY_F8 || LocalKey == KEY_SHIFTF8)
			{
				DynamicUpdateKeyBar();
				Parent()->GetKeybar().Redraw();
			}

			if (LocalKey == KEY_F7 || LocalKey == KEY_SHIFTF7)
			{
				long long Pos, Length;
				DWORD Flags;
				DizView->GetSelectedParam(Pos,Length,Flags);
				//ShellUpdatePanels(nullptr,FALSE);
				DizView->InRecursion++;
				Redraw();
				Parent()->GetAnotherPanel(this)->Redraw();
				DizView->SelectText(Pos,Length,Flags|1);
				DizView->InRecursion--;
			}

			return ret;
		}
	}

	return false;
}


bool InfoList::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsMouseInClientArea(MouseEvent))
		return false;

	if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		return false;

	bool NeedRedraw=false;
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	const auto ProcessDescription = AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL;
	const auto ProcessPluginDescription = AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL;
	if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && !(MouseEvent->dwEventFlags & MOUSE_MOVED))
	{
		if (MouseEvent->dwMousePosition.Y == SectionState[ILSS_DISKINFO].Y)
		{
			SectionState[ILSS_DISKINFO].Show=!SectionState[ILSS_DISKINFO].Show;
			NeedRedraw=true;
		}
		else if (MouseEvent->dwMousePosition.Y == SectionState[ILSS_MEMORYINFO].Y)
		{
			SectionState[ILSS_MEMORYINFO].Show=!SectionState[ILSS_MEMORYINFO].Show;
			NeedRedraw=true;
		}
		else if (ProcessDescription && MouseEvent->dwMousePosition.Y == SectionState[ILSS_DIRDESCRIPTION].Y)
		{
			SectionState[ILSS_DIRDESCRIPTION].Show=!SectionState[ILSS_DIRDESCRIPTION].Show;
			NeedRedraw=true;
		}
		else if (ProcessPluginDescription && MouseEvent->dwMousePosition.Y == SectionState[ILSS_PLDESCRIPTION].Y)
		{
			SectionState[ILSS_PLDESCRIPTION].Show=!SectionState[ILSS_PLDESCRIPTION].Show;
			NeedRedraw=true;
		}
		else if (MouseEvent->dwMousePosition.Y == SectionState[ILSS_POWERSTATUS].Y)
		{
			SectionState[ILSS_POWERSTATUS].Show=!SectionState[ILSS_POWERSTATUS].Show;
			NeedRedraw=true;
		}
	}

	int DVY1=-1;
	if (DizView)
	{
		int DVX1,DVX2,DVY2;
		DizView->GetPosition(DVX1,DVY1,DVX2,DVY2);
		if (DVY1 < m_Y2)
		{
			if (SectionState[ILSS_DIRDESCRIPTION].Show && MouseEvent->dwMousePosition.Y > SectionState[ILSS_DIRDESCRIPTION].Y)
			{
				if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
				        MouseEvent->dwMousePosition.X > DVX1+1 &&
				        MouseEvent->dwMousePosition.X < DVX2 - DizView->GetShowScrollbar() - 1 &&
				        MouseEvent->dwMousePosition.Y > DVY1+1 &&
				        MouseEvent->dwMousePosition.Y < DVY2-1
				   )
				{
					ProcessKey(Manager::Key(KEY_F3));
					return true;
				}

				if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
				{
					ProcessKey(Manager::Key(KEY_F4));
					return true;
				}
			}
		}
	}

	if (NeedRedraw)
		Redraw();

	Parent()->SetActivePanel(this);

	if (DizView)
	{
		if (DVY1 < m_Y2)
			return DizView->ProcessMouse(MouseEvent);
	}
	return true;
}


void InfoList::PrintText(string_view const Str) const
{
	if (WhereY()<=m_Y2-1)
	{
		Text(cut_right(Str, m_X2 - WhereX()));
	}
}


void InfoList::PrintText(lng MsgID) const
{
	PrintText(msg(MsgID));
}


void InfoList::PrintInfo(const string& str) const
{
	if (WhereY()>m_Y2-1)
		return;

	int MaxLength=m_X2-WhereX()-2;

	if (MaxLength<0)
		MaxLength=0;

	string strStr = str;
	TruncStr(strStr,MaxLength);
	int Length=(int)strStr.size();
	int NewX=m_X2-Length-1;

	if (NewX>m_X1 && NewX>WhereX())
	{
		GotoXY(NewX,WhereY());
		const auto SaveColor = GetColor();
		SetColor(COL_PANELINFOTEXT);
		Text(strStr + L' ');
		SetColor(SaveColor);
	}
}


void InfoList::PrintInfo(lng MsgID) const
{
	PrintInfo(msg(MsgID));
}


bool InfoList::ShowDirDescription(int YPos)
{
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	string strFullDizName(AnotherPanel->GetCurDir());
	
	if (!strFullDizName.empty())
		AddEndSlash(strFullDizName);

	const auto DirSize = strFullDizName.size();

	for (const auto& i: enum_tokens_with_quotes(Global->Opt->InfoPanel.strFolderInfoFiles.Get(), L",;"sv))
	{
		strFullDizName.resize(DirSize);
		append(strFullDizName, i);

		os::fs::find_data FindData;
		if (!os::fs::get_find_data(strFullDizName, FindData))
			continue;

		CutToSlash(strFullDizName, false);
		strFullDizName += FindData.FileName;

		if (OpenDizFile(strFullDizName, YPos))
			return true;
	}

	return false;
}


bool InfoList::ShowPluginDescription(int YPos)
{
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	static wchar_t VertcalLine[2]={BoxSymbols[BS_V2],0};

	OpenPanelInfo Info;
	AnotherPanel->GetOpenPanelInfo(&Info);

	int Y=YPos;
	for (size_t I=0; I<Info.InfoLinesNumber; I++, Y++)
	{
		if (Y >= m_Y2)
			break;

		const InfoPanelLine *InfoLine=&Info.InfoLines[I];
		GotoXY(m_X1,Y);
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		SetColor(COL_PANELTEXT);
		Text(string(m_X2 - m_X1 - 1, L' '));
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		GotoXY(m_X1+2,Y);

		if (InfoLine->Flags&IPLFLAGS_SEPARATOR)
		{
			string strTitle;

			if (InfoLine->Text && *InfoLine->Text)
				strTitle = concat(L' ', InfoLine->Text, L' ');

			DrawSeparator(Y);
			TruncStr(strTitle,m_X2-m_X1-3);
			GotoXY(m_X1+(m_X2-m_X1-(int)strTitle.size())/2,Y);
			SetColor(COL_PANELTEXT);
			PrintText(strTitle);
		}
		else
		{
			SetColor(COL_PANELTEXT);
			PrintText(NullToEmpty(InfoLine->Text));
			PrintInfo(NullToEmpty(InfoLine->Data));
		}
	}
	return true;
}

void InfoList::CloseFile()
{
	if (DizView)
	{
		if (DizView->InRecursion)
			return;

		LastDizWrapMode=DizView->GetWrapMode();
		LastDizWrapType=DizView->GetWrapType();
		LastDizShowScrollbar=DizView->GetShowScrollbar();
		DizView->SetWrapMode(OldWrapMode);
		DizView->SetWrapType(OldWrapType);
		DizView=nullptr;
	}

	strDizFileName.clear();
}

bool InfoList::OpenDizFile(const string& DizFile,int YPos)
{
	bool bOK=true;
	_tran(SysLog(L"InfoList::OpenDizFile([%s]",DizFile));

	if (!DizView)
	{
		DizView = std::make_unique<DizViewer>(GetOwner());

		_tran(SysLog(L"InfoList::OpenDizFile() create new Viewer = %p",DizView));
		DizView->SetRestoreScreenMode(false);
		DizView->SetPosition(m_X1+1,YPos,m_X2-1,m_Y2-1);
		DizView->SetStatusMode(0);
		DizView->EnableHideCursor(0);
		OldWrapMode = DizView->GetWrapMode();
		OldWrapType = DizView->GetWrapType();
		DizView->SetWrapMode(LastDizWrapMode);
		DizView->SetWrapType(LastDizWrapType);
		DizView->SetShowScrollbar(LastDizShowScrollbar);
	}
	else
	{
		//не будем менять внутренности если мы посреди операции со вьювером.
		bOK = !DizView->InRecursion;
	}

	if (bOK)
	{
		if (!DizView->OpenFile(DizFile,FALSE))
		{
			DizView=nullptr;
			return false;
		}

		strDizFileName = DizFile;
	}

	DizView->Show();

	auto strTitle = concat(L' ', PointToName(strDizFileName), L' ');
	int CurY=YPos-1;
	DrawTitle(strTitle,ILSS_DIRDESCRIPTION,CurY);
	return true;
}

bool InfoList::GetCurName(string &strName, string &strShortName) const
{
	strName = strDizFileName;
	strShortName = ConvertNameToShort(strName);
	return true;
}

void InfoList::UpdateKeyBar()
{
	Parent()->GetKeybar().SetLabels(lng::MInfoF1);
	DynamicUpdateKeyBar();
}

void InfoList::DynamicUpdateKeyBar() const
{
	auto& Keybar = Parent()->GetKeybar();

	if (DizView)
	{
		DizView->UpdateViewKeyBar(Keybar);
	}
	else
	{
		Keybar[KBL_MAIN][F2] = msg(lng::MF2);
		Keybar[KBL_SHIFT][F2].clear();
		Keybar[KBL_MAIN][F3].clear();
		Keybar[KBL_MAIN][F8].clear();
		Keybar[KBL_SHIFT][F8].clear();
		Keybar[KBL_ALT][F8] = msg(lng::MAltF8);  // стандартный для панели - "хистори"
	}

	Keybar.SetCustomLabels(KBA_INFO);
}

Viewer* InfoList::GetViewer()
{
	return DizView.get();
}

Viewer* InfoList::GetById(int ID)
{
	return DizView && ID==DizView->GetId()?GetViewer():nullptr;
}

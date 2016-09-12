/*
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

#include "headers.hpp"
#pragma hdrstop

#include "imports.hpp"
#include "infolist.hpp"
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
#include "language.hpp"
#include "dizviewer.hpp"
#include "locale.hpp"
#include "keybar.hpp"
#include "keyboard.hpp"

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
	Panel(Owner),
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
	return MSG(MInfoTitle);
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
	PrintText(SectionState[Id].Show?L"[-]":L"[+]");
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
	FormatString strDiskNumber;
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
		string strComputerName, strUserName;
		DWORD dwSize = 256; //MAX_COMPUTERNAME_LENGTH+1;
		wchar_t_ptr ComputerName(dwSize);
		if (Global->Opt->InfoPanel.ComputerNameFormat == ComputerNamePhysicalNetBIOS || !GetComputerNameEx(static_cast<COMPUTER_NAME_FORMAT>(Global->Opt->InfoPanel.ComputerNameFormat.Get()), ComputerName.get(), &dwSize))
		{
			dwSize = MAX_COMPUTERNAME_LENGTH+1;
			GetComputerName(ComputerName.get(), &dwSize);  // retrieves only the NetBIOS name of the local computer
		}
		strComputerName = ComputerName.get();

		GotoXY(m_X1+2,CurY++);
		PrintText(MInfoCompName);
		PrintInfo(strComputerName);

		os::memory::netapi::ptr<SERVER_INFO_101> ServerInfo;
		if (NetServerGetInfo(nullptr, 101, reinterpret_cast<LPBYTE*>(&ptr_setter(ServerInfo))) == NERR_Success)
		{
			if(ServerInfo->sv101_comment && *ServerInfo->sv101_comment)
			{
				GotoXY(m_X1+2,CurY++);
				PrintText(MInfoCompDescription);
				PrintInfo(ServerInfo->sv101_comment);
			}
		}


		dwSize = UNLEN+1;
		wchar_t_ptr UserName(dwSize);
		if (Global->Opt->InfoPanel.UserNameFormat == NameUnknown || !GetUserNameEx(static_cast<EXTENDED_NAME_FORMAT>(Global->Opt->InfoPanel.UserNameFormat.Get()), UserName.get(), &dwSize))
		{
			dwSize = UNLEN+1;
			GetUserName(UserName.get(), &dwSize);
		}
		strUserName = UserName.get();

		GotoXY(m_X1+2,CurY++);
		PrintText(MInfoUserName);
		PrintInfo(strUserName);

		dwSize = UNLEN+1;
		wchar_t UserNameBuffer[UNLEN+1];
		if (GetUserName(UserNameBuffer, &dwSize))
		{
			os::memory::netapi::ptr<USER_INFO_1> UserInfo;
			if (NetUserGetInfo(nullptr, strUserName.data(), 1, reinterpret_cast<LPBYTE*>(&ptr_setter(UserInfo))) == NERR_Success)
			{
				if(UserInfo->usri1_comment && *UserInfo->usri1_comment)
				{
					GotoXY(m_X1+2,CurY++);
					PrintText(MInfoUserDescription);
					PrintInfo(UserInfo->usri1_comment);
				}
				LNGID LabelId = MInfoUserAccessLevelUnknown;
				switch (UserInfo->usri1_priv)
				{
				case USER_PRIV_GUEST:
					LabelId = MInfoUserAccessLevelGuest;
					break;
				case USER_PRIV_USER:
					LabelId = MInfoUserAccessLevelUser;
						break;
				case USER_PRIV_ADMIN:
					LabelId = MInfoUserAccessLevelAdministrator;
						break;
				}
				GotoXY(m_X1+2,CurY++);
				PrintText(MInfoUserAccessLevel);
				PrintInfo(LabelId);
			}
		}

	}

	string SectionTitle;

	/* #2 - disk info */
	if (SectionState[ILSS_DISKINFO].Show)
	{
		m_CurDir = AnotherPanel->GetCurDir();

		if (m_CurDir.empty())
			m_CurDir = os::GetCurrentDirectory();

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

		if (os::GetVolumeInformation(strDriveRoot,&strVolumeName,
		                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
		                            &strFileSystemName))
		{
			LNGID IdxMsgID=MInfoUnknown;
			int DriveType=FAR_GetDriveType(strDriveRoot, Global->Opt->InfoPanel.ShowCDInfo);

			switch (DriveType)
			{
				case DRIVE_REMOVABLE:
					IdxMsgID=MInfoRemovable;
					break;
				case DRIVE_FIXED:
					IdxMsgID=MInfoFixed;
					break;
				case DRIVE_REMOTE:
					IdxMsgID=MInfoNetwork;
					break;
				case DRIVE_CDROM:
					IdxMsgID=MInfoCDROM;
					break;
				case DRIVE_RAMDISK:
					IdxMsgID=MInfoRAM;
					break;
				default:

					if (IsDriveTypeCDROM(DriveType))
						IdxMsgID=MInfoCD_RW+(DriveType-DRIVE_CD_RW);

					break;
			}

			LPCWSTR DiskType=MSG(IdxMsgID);
			string strAssocPath;

			if (GetSubstName(DriveType,strDriveRoot,strAssocPath))
			{
				DiskType = MSG(MInfoSUBST);
				DriveType=DRIVE_SUBSTITUTE;
			}
			else if(DriveCanBeVirtual(DriveType) && GetVHDInfo(strDriveRoot,strAssocPath))
			{
				DiskType = MSG(MInfoVirtual);
				DriveType=DRIVE_VIRTUAL;
			}

			SectionTitle = string(L" ") + DiskType + L" " + MSG(MInfoDisk) + L" " + strDriveRoot + L" (" + strFileSystemName + L") ";

			switch(DriveType)
			{
				case DRIVE_REMOTE:
				{
					os::WNetGetConnection(strDriveRoot, strAssocPath);
					// TODO: check result
				}
				break;

				case DRIVE_SUBSTITUTE:
				case DRIVE_VIRTUAL:
				{
					SectionTitle += strAssocPath;
					SectionTitle += L" ";
				}
				break;
			}

			strDiskNumber <<
				fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << HIWORD(VolumeNumber) << L'-' <<
				fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << LOWORD(VolumeNumber);
		}
		else // Error!
			SectionTitle = strDriveRoot;
	}

	if (!SectionState[ILSS_DISKINFO].Show)
		SectionTitle = MSG(MInfoDiskTitle);
	DrawTitle(SectionTitle,ILSS_DISKINFO,CurY);

	const auto bytes_suffix = Upper(MSG(MListBytes));
	auto size2str = [&bytes_suffix](ULONGLONG Size)
	{
		string str;
		if (Global->Opt->ShowBytes)
		{
			str = InsertCommas(Size) + L" ";
		}
		else
		{
			str = FileSizeToStr(Size, 16, COLUMN_FLOATSIZE | COLUMN_SHOWBYTESINDEX);
		}
		return str += bytes_suffix;
	};

	if (SectionState[ILSS_DISKINFO].Show)
	{
		/* #2.2 - disk info: size */
		unsigned long long TotalSize, UserFree;

		if (os::GetDiskSize(m_CurDir,&TotalSize, nullptr, &UserFree))
		{
			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoDiskTotal);
			PrintInfo(size2str(TotalSize));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoDiskFree);
			PrintInfo(size2str(UserFree));
		}

		/* #4 - disk info: label & SN */
		GotoXY(m_X1+2,CurY++);
		PrintText(MInfoDiskLabel);
		PrintInfo(strVolumeName);

		GotoXY(m_X1+2,CurY++);
		PrintText(MInfoDiskNumber);
		PrintInfo(strDiskNumber);
	}

	/* #3 - memory info */
	SectionTitle = MSG(MInfoMemory);
	DrawTitle(SectionTitle, ILSS_MEMORYINFO, CurY);

	if (SectionState[ILSS_MEMORYINFO].Show)
	{
		MEMORYSTATUSEX ms={sizeof(ms)};
		if (GlobalMemoryStatusEx(&ms))
		{
			if (!ms.dwMemoryLoad)
				ms.dwMemoryLoad=100-ToPercent(ms.ullAvailPhys+ms.ullAvailPageFile,ms.ullTotalPhys+ms.ullTotalPageFile);

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoMemoryLoad);
			PrintInfo(string_format(L"{0}%", ms.dwMemoryLoad));

			ULONGLONG TotalMemoryInKilobytes=0;
			if(Imports().GetPhysicallyInstalledSystemMemory(&TotalMemoryInKilobytes))
			{
				GotoXY(m_X1+2,CurY++);
				PrintText(MInfoMemoryInstalled);
				PrintInfo(size2str(TotalMemoryInKilobytes << 10));
			}

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoMemoryTotal);
			PrintInfo(size2str(ms.ullTotalPhys));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoMemoryFree);
			PrintInfo(size2str(ms.ullAvailPhys));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoVirtualTotal);
			PrintInfo(size2str(ms.ullTotalVirtual));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoVirtualFree);
			PrintInfo(size2str(ms.ullAvailVirtual));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPageFileTotal);
			PrintInfo(size2str(ms.ullTotalPageFile));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPageFileFree);
			PrintInfo(size2str(ms.ullAvailPageFile));
		}
	}

	/* #4 - power status */
	if (Global->Opt->InfoPanel.ShowPowerStatus)
	{
		SectionTitle = MSG(MInfoPowerStatus);
		DrawTitle(SectionTitle, ILSS_POWERSTATUS, CurY);

		if (SectionState[ILSS_POWERSTATUS].Show)
		{
			LNGID MsgID;
			SYSTEM_POWER_STATUS PowerStatus;
			GetSystemPowerStatus(&PowerStatus);

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPowerStatusAC);
			switch(PowerStatus.ACLineStatus)
			{
				case AC_LINE_OFFLINE:      MsgID=MInfoPowerStatusACOffline; break;
				case AC_LINE_ONLINE:       MsgID=MInfoPowerStatusACOnline; break;
				case AC_LINE_BACKUP_POWER: MsgID=MInfoPowerStatusACBackUp; break;
				default:                   MsgID=MInfoPowerStatusACUnknown; break;
			}
			PrintInfo(MSG(MsgID));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPowerStatusBCLifePercent);
			if (PowerStatus.BatteryLifePercent > 100)
				strOutStr = MSG(MInfoPowerStatusBCLifePercentUnknown);
			else
				strOutStr = std::to_wstring(PowerStatus.BatteryLifePercent) + L'%';
			PrintInfo(strOutStr);

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPowerStatusBC);
			strOutStr.clear();
			// PowerStatus.BatteryFlag == 0: The value is zero if the battery is not being charged and the battery capacity is between low and high.
			if (!PowerStatus.BatteryFlag || PowerStatus.BatteryFlag == BATTERY_FLAG_UNKNOWN)
				strOutStr=MSG(MInfoPowerStatusBCUnknown);
			else if (PowerStatus.BatteryFlag & BATTERY_FLAG_NO_BATTERY)
				strOutStr=MSG(MInfoPowerStatusBCNoSysBat);
			else
			{
				if (PowerStatus.BatteryFlag & BATTERY_FLAG_HIGH)
					strOutStr = MSG(MInfoPowerStatusBCHigh);
				else if (PowerStatus.BatteryFlag & BATTERY_FLAG_LOW)
					strOutStr = MSG(MInfoPowerStatusBCLow);
				else if (PowerStatus.BatteryFlag & BATTERY_FLAG_CRITICAL)
					strOutStr = MSG(MInfoPowerStatusBCCritical);

				if (PowerStatus.BatteryFlag & BATTERY_FLAG_CHARGING)
				{
					if (!strOutStr.empty())
						strOutStr += L" ";
					strOutStr += MSG(MInfoPowerStatusBCCharging);
				}
			}
			PrintInfo(strOutStr);

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPowerStatusBCTimeRem);
			if (PowerStatus.BatteryLifeTime != BATTERY_LIFE_UNKNOWN)
			{
				DWORD s = PowerStatus.BatteryLifeTime%60;
				DWORD m = (PowerStatus.BatteryLifeTime/60)%60;
				DWORD h = PowerStatus.BatteryLifeTime/3600;
				PrintInfo(FormatString() << fmt::MinWidth(2) << fmt::FillChar(L'0') << h << locale::GetTimeSeparator() << fmt::MinWidth(2) << fmt::FillChar(L'0') << m << locale::GetTimeSeparator() << fmt::MinWidth(2) << fmt::FillChar(L'0') << s);
			}
			else
				PrintInfo(MSG(MInfoPowerStatusBCTMUnknown));

			GotoXY(m_X1+2,CurY++);
			PrintText(MInfoPowerStatusBCFullTimeRem);
			if (PowerStatus.BatteryFullLifeTime != BATTERY_LIFE_UNKNOWN)
			{
				DWORD s = PowerStatus.BatteryLifeTime%60;
				DWORD m = (PowerStatus.BatteryLifeTime/60)%60;
				DWORD h = PowerStatus.BatteryLifeTime/3600;
				PrintInfo(FormatString() << fmt::MinWidth(2) << fmt::FillChar(L'0') << h << locale::GetTimeSeparator() << fmt::MinWidth(2) << fmt::FillChar(L'0') << m << locale::GetTimeSeparator() << fmt::MinWidth(2) << fmt::FillChar(L'0') << s);
			}
			else
				PrintInfo(MSG(MInfoPowerStatusBCFTMUnknown));
		}
	}

	if (AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL)
	{
		/* #5 - description */
		SectionTitle = MSG(MInfoDescription);
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
				PrintText(MInfoDizAbsent);
			}
		}
	}

	if (AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		/* #6 - Plugin Description */
		SectionTitle = MSG(MInfoPlugin);
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
	MenuDataEx ShowModeMenuItem[]=
	{
		MSG(MMenuInfoShowModeDisk),LIF_SELECTED,0,
		MSG(MMenuInfoShowModeMemory),0,0,
		MSG(MMenuInfoShowModeDirDiz),0,0,
		MSG(MMenuInfoShowModePluginDiz),0,0,
		MSG(MMenuInfoShowModePower),0,0,
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

		const auto ShowModeMenu = VMenu2::create(MSG(MMenuInfoShowModeTitle), ShowModeMenuItem, std::size(ShowModeMenuItem), 0);
		ShowModeMenu->SetHelp(L"InfoPanelShowMode");
		ShowModeMenu->SetPosition(m_X1+4,-1,0,0);
		ShowModeMenu->SetMenuFlags(VMENU_WRAPMODE);

		ShowCode=ShowModeMenu->Run([&](const Manager::Key& RawKey)
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

	if (ShowCode != -1)
	{
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
			Global->Opt->InfoPanel.strShowStatusInfo += i.Show? L"1" : L"0";
		});

		Redraw();
	}
}

int InfoList::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (!IsVisible())
		return FALSE;

	if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
	{
		ExecShortcutFolder(LocalKey-KEY_RCTRL0);
		return TRUE;
	}

	switch (LocalKey)
	{
		case KEY_F1:
		{
			Help::create(L"InfoPanel");
			return TRUE;
		}
		case KEY_CTRLF12:
		case KEY_RCTRLF12:
			SelectShowMode();
			return TRUE;
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (!strDizFileName.empty())
			{
				m_CurDir = Parent()->GetAnotherPanel(this)->GetCurDir();
				FarChDir(m_CurDir);
				FileViewer::create(strDizFileName, TRUE);//OT
			}

			Parent()->Redraw();
			return TRUE;
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
			else if (!Global->Opt->InfoPanel.strFolderInfoFiles.empty())
			{
				string strArgName;
				const wchar_t *p = Global->Opt->InfoPanel.strFolderInfoFiles.data();

				while ((p = GetCommaWord(p,strArgName)) != nullptr)
				{
					if (strArgName.find_first_of(L"*?") == string::npos)
					{
						FileEditor::create(strArgName, CP_DEFAULT, FFILEEDIT_CANNEWFILE | FFILEEDIT_ENABLEF6);
						break;
					}
				}
			}

			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			//AnotherPanel->Redraw();
			Update(0);
			Parent()->Redraw();
			return TRUE;
		}
		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			Redraw();
			return TRUE;
		}
	}

	if (DizView && LocalKey >= 256)
	{
		int DVX1,DVX2,DVY1,DVY2;
		DizView->GetPosition(DVX1,DVY1,DVX2,DVY2);

		if (DVY1 < m_Y2)
		{
			int ret = DizView->ProcessKey(Key);

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

	return FALSE;
}


int InfoList::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsMouseInClientArea(MouseEvent))
		return FALSE;

	if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		return FALSE;

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
					return TRUE;
				}

				if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
				{
					ProcessKey(Manager::Key(KEY_F4));
					return TRUE;
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
	return TRUE;
}


void InfoList::PrintText(const string& Str) const
{
	if (WhereY()<=m_Y2-1)
	{
		Global->FS << fmt::MaxWidth(m_X2-WhereX())<<Str;
	}
}


void InfoList::PrintText(LNGID MsgID) const
{
	PrintText(MSG(MsgID));
}


void InfoList::PrintInfo(const string& str) const
{
	if (WhereY()>m_Y2-1)
		return;

	FarColor SaveColor=GetColor();
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
		SetColor(COL_PANELINFOTEXT);
		Global->FS << strStr<<L" ";
		SetColor(SaveColor);
	}
}


void InfoList::PrintInfo(LNGID MsgID) const
{
	PrintInfo(MSG(MsgID));
}


bool InfoList::ShowDirDescription(int YPos)
{
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	string strDizDir(AnotherPanel->GetCurDir());

	if (!strDizDir.empty())
		AddEndSlash(strDizDir);

	string strArgName;
	const wchar_t *NamePtr = Global->Opt->InfoPanel.strFolderInfoFiles.data();

	while ((NamePtr=GetCommaWord(NamePtr,strArgName)) != nullptr)
	{
		string strFullDizName;
		strFullDizName = strDizDir;
		strFullDizName += strArgName;
		os::FAR_FIND_DATA FindData;

		if (!os::GetFindDataEx(strFullDizName, FindData))
			continue;

		CutToSlash(strFullDizName, false);
		strFullDizName += FindData.strFileName;

		if (OpenDizFile(strFullDizName,YPos))
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
		Global->FS << fmt::MinWidth(m_X2-m_X1-1)<<L"";
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		GotoXY(m_X1+2,Y);

		if (InfoLine->Flags&IPLFLAGS_SEPARATOR)
		{
			string strTitle;

			if (InfoLine->Text && *InfoLine->Text)
				strTitle.append(L" ").append(InfoLine->Text).append(L" ");

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

int InfoList::OpenDizFile(const string& DizFile,int YPos)
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
			return FALSE;
		}

		strDizFileName = DizFile;
	}

	DizView->Show();

	string strTitle;
	strTitle.append(L" ").append(PointToName(strDizFileName)).append(L" ");
	int CurY=YPos-1;
	DrawTitle(strTitle,ILSS_DIRDESCRIPTION,CurY);
	return TRUE;
}

int InfoList::GetCurName(string &strName, string &strShortName) const
{
	strName = strDizFileName;
	strShortName = ConvertNameToShort(strName);
	return TRUE;
}

void InfoList::UpdateKeyBar()
{
	Parent()->GetKeybar().SetLabels(MInfoF1);
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
		Keybar[KBL_MAIN][F2] = MSG(MF2);
		Keybar[KBL_SHIFT][F2].clear();
		Keybar[KBL_MAIN][F3].clear();
		Keybar[KBL_MAIN][F8].clear();
		Keybar[KBL_SHIFT][F8].clear();
		Keybar[KBL_ALT][F8] = MSG(MAltF8);  // стандартный для панели - "хистори"
	}

	Keybar.SetCustomLabels(KBA_INFO);
}

Viewer* InfoList::GetViewer(void)
{
	return DizView.get();
}

Viewer* InfoList::GetById(int ID)
{
	return DizView && ID==DizView->GetId()?GetViewer():nullptr;
}

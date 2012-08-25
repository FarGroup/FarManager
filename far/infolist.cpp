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
#include "colors.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
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
#include "palette.hpp"
#include "vmenu.hpp"
#include "datetime.hpp"
#include "window.hpp"

static bool LastMode = false;
static bool LastDizWrapMode = false;
static bool LastDizWrapType = false;
static bool LastDizShowScrollbar = false;

InfoList::InfoList():
	DizView(nullptr),
	PrevMacroMode(-1),
	OldWrapMode(nullptr),
	OldWrapType(nullptr)
{
	Type=INFO_PANEL;

	if (Opt.InfoPanel.strShowStatusInfo.GetLength() == 0)
	{
		for (size_t i=0; i < ARRAYSIZE(SectionState); ++i)
			SectionState[i].Show=true;
	}
	else
	{
		for (size_t i=0; i < ARRAYSIZE(SectionState); ++i)
			SectionState[i].Show=Opt.InfoPanel.strShowStatusInfo.At(i) == '0'?false:true;
	}

	if (!LastMode)
	{
		LastMode = true;
		LastDizWrapMode = Opt.ViOpt.ViewerIsWrap;
		LastDizWrapType = Opt.ViOpt.ViewerWrap;
		LastDizShowScrollbar = Opt.ViOpt.ShowScrollbar;
	}
}

InfoList::~InfoList()
{
	CloseFile();
	SetMacroMode(TRUE);
}

// перерисовка, только если мы текущий фрейм
void InfoList::Update(int Mode)
{
	if (!EnableUpdate)
		return;

	if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
		Redraw();
}

string &InfoList::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
	strTitle.Clear();
	strTitle.Append(L" ").Append(MSG(MInfoTitle)).Append(" ");
	TruncStr(strTitle,X2-X1-3);
	return strTitle;
}

void InfoList::DrawTitle(string &strTitle,int Id,int &CurY)
{
	SetColor(COL_PANELBOX);
	DrawSeparator(CurY);
	SetColor(COL_PANELTEXT);
	TruncStr(strTitle,X2-X1-3);
	GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,CurY);
	PrintText(strTitle);
	GotoXY(X1+1,CurY);
	PrintText(SectionState[Id].Show?L"[-]":L"[+]");
	SectionState[Id].Y=CurY;
	CurY++;
}

void InfoList::DisplayObject()
{
	if (Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	Flags.Set(FSCROBJ_ISREDRAWING);

	string strTitle;
	string strOutStr;
	Panel *AnotherPanel = CtrlObject->Cp()->GetAnotherPanel(this);
	string strDriveRoot;
	string strVolumeName, strFileSystemName;
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
	FormatString strDiskNumber;
	CloseFile();

	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_PANELBOX),DOUBLE_BOX);
	SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GetTitle(strTitle);

	if (!strTitle.IsEmpty())
	{
		GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1);
		Text(strTitle);
	}

	SetColor(COL_PANELTEXT);

	int CurY=Y1+1;

	/* #1 - computer name/user name */
	{
		string strComputerName, strUserName;
		DWORD dwSize = 256; //MAX_COMPUTERNAME_LENGTH+1;
		wchar_t *ComputerName = strComputerName.GetBuffer(dwSize);
		if (Opt.InfoPanel.ComputerNameFormat == ComputerNamePhysicalNetBIOS || !GetComputerNameEx(static_cast<COMPUTER_NAME_FORMAT>(Opt.InfoPanel.ComputerNameFormat.Get()), ComputerName, &dwSize))
		{
			dwSize = MAX_COMPUTERNAME_LENGTH+1;
			GetComputerName(ComputerName, &dwSize);  // retrieves only the NetBIOS name of the local computer
		}
		strComputerName.ReleaseBuffer();

		GotoXY(X1+2,CurY++);
		PrintText(MInfoCompName);
		PrintInfo(strComputerName);

		LPSERVER_INFO_101 ServerInfo = nullptr;
		if(NetServerGetInfo(nullptr, 101, reinterpret_cast<LPBYTE*>(&ServerInfo)) == NERR_Success)
		{
			if(ServerInfo->sv101_comment && *ServerInfo->sv101_comment)
			{
				GotoXY(X1+2,CurY++);
				PrintText(MInfoCompDescription);
				PrintInfo(ServerInfo->sv101_comment);
			}
			NetApiBufferFree(ServerInfo);
		}


		dwSize = UNLEN+1;
		wchar_t *UserName = strUserName.GetBuffer(dwSize);
		if (Opt.InfoPanel.UserNameFormat == NameUnknown || !GetUserNameEx(static_cast<EXTENDED_NAME_FORMAT>(Opt.InfoPanel.UserNameFormat.Get()), UserName, &dwSize))
		{
			dwSize = UNLEN+1;
			GetUserName(UserName, &dwSize);
		}
		strUserName.ReleaseBuffer();

		GotoXY(X1+2,CurY++);
		PrintText(MInfoUserName);
		PrintInfo(strUserName);

		dwSize = UNLEN+1;
		wchar_t UserNameBuffer[UNLEN+1];
		if (GetUserName(UserNameBuffer, &dwSize))
		{
			LPUSER_INFO_1 UserInfo = nullptr;
			if(NetUserGetInfo(nullptr, strUserName, 1, reinterpret_cast<LPBYTE*>(&UserInfo)) == NERR_Success)
			{
				if(UserInfo->usri1_comment && *UserInfo->usri1_comment)
				{
					GotoXY(X1+2,CurY++);
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
				GotoXY(X1+2,CurY++);
				PrintText(MInfoUserAccessLevel);
				PrintInfo(LabelId);

				NetApiBufferFree(UserInfo);
			}
		}

	}

	/* #2 - disk info */
	if (SectionState[ILSS_DISKINFO].Show)
	{
		AnotherPanel->GetCurDir(strCurDir);

		if (strCurDir.IsEmpty())
			apiGetCurrentDirectory(strCurDir);

		/*
			Корректно отображать инфу при заходе в Juction каталог
			Рут-диск может быть другим
		*/
		if ((apiGetFileAttributes(strCurDir)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
		{
			string strJuncName;

			if (GetReparsePointInfo(strCurDir, strJuncName))
			{
				NormalizeSymlinkName(strJuncName);
				GetPathRoot(strJuncName,strDriveRoot); //"\??\D:\Junc\Src\"
			}
		}
		else
			GetPathRoot(strCurDir, strDriveRoot);

		if (apiGetVolumeInformation(strDriveRoot,&strVolumeName,
		                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
		                            &strFileSystemName))
		{
			LNGID IdxMsgID=MInfoUnknown;
			int DriveType=FAR_GetDriveType(strDriveRoot,nullptr,Opt.InfoPanel.ShowCDInfo);

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
			else if(DriveCanBeVirtual(DriveType) && GetVHDName(strDriveRoot,strAssocPath))
			{
				DiskType = MSG(MInfoVirtual);
				DriveType=DRIVE_VIRTUAL;
			}

			strTitle=string(L" ")+DiskType+L" "+MSG(MInfoDisk)+L" "+strDriveRoot+L" ("+strFileSystemName+L") ";

			switch(DriveType)
			{
				case DRIVE_REMOTE:
				{
					apiWNetGetConnection(strDriveRoot, strAssocPath);
				}
				break;

				case DRIVE_SUBSTITUTE:
				case DRIVE_VIRTUAL:
				{
					strTitle += strAssocPath;
					strTitle += L" ";
				}
				break;
			}

			strDiskNumber <<
				fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << HIWORD(VolumeNumber) << L'-' <<
				fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << LOWORD(VolumeNumber);
		}
		else // Error!
			strTitle = strDriveRoot;
	}

	if (!SectionState[ILSS_DISKINFO].Show)
		strTitle=MSG(MInfoDiskTitle);
	DrawTitle(strTitle,ILSS_DISKINFO,CurY);

	if (SectionState[ILSS_DISKINFO].Show)
	{
		/* #2.2 - disk info: size */
		unsigned __int64 TotalSize, UserFree;

		if (apiGetDiskSize(strCurDir,&TotalSize, nullptr, &UserFree))
		{
			GotoXY(X1+2,CurY++);
			PrintText(MInfoDiskTotal);
			InsertCommas(TotalSize,strOutStr);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoDiskFree);
			InsertCommas(UserFree,strOutStr);
			PrintInfo(strOutStr);
		}

		/* #4 - disk info: label & SN */
		GotoXY(X1+2,CurY++);
		PrintText(MInfoDiskLabel);
		PrintInfo(strVolumeName);

	    GotoXY(X1+2,CurY++);
		PrintText(MInfoDiskNumber);
		PrintInfo(strDiskNumber);
	}

	/* #3 - memory info */
	strTitle = MSG(MInfoMemory);
	DrawTitle(strTitle,ILSS_MEMORYINFO,CurY);

	if (SectionState[ILSS_MEMORYINFO].Show)
	{
		MEMORYSTATUSEX ms={sizeof(ms)};
		if (GlobalMemoryStatusEx(&ms))
		{
			if (!ms.dwMemoryLoad)
				ms.dwMemoryLoad=100-ToPercent64(ms.ullAvailPhys+ms.ullAvailPageFile,ms.ullTotalPhys+ms.ullTotalPageFile);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoMemoryLoad);
			strOutStr.Format(L"%d%%",ms.dwMemoryLoad);
			PrintInfo(strOutStr);

			ULONGLONG TotalMemoryInKilobytes=0;
			if(ifn.GetPhysicallyInstalledSystemMemory(&TotalMemoryInKilobytes))
			{
				GotoXY(X1+2,CurY++);
				PrintText(MInfoMemoryInstalled);
				InsertCommas(TotalMemoryInKilobytes<<10,strOutStr);
				PrintInfo(strOutStr);
			}

			GotoXY(X1+2,CurY++);
			PrintText(MInfoMemoryTotal);
			InsertCommas(ms.ullTotalPhys,strOutStr);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoMemoryFree);
			InsertCommas(ms.ullAvailPhys,strOutStr);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoVirtualTotal);
			InsertCommas(ms.ullTotalVirtual,strOutStr);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoVirtualFree);
			InsertCommas(ms.ullAvailVirtual,strOutStr);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPageFileTotal);
			InsertCommas(ms.ullTotalPageFile,strOutStr);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPageFileFree);
			InsertCommas(ms.ullAvailPageFile,strOutStr);
			PrintInfo(strOutStr);
		}
	}

	/* #4 - power status */
	if (Opt.InfoPanel.ShowPowerStatus)
	{
		strTitle = MSG(MInfoPowerStatus);
		DrawTitle(strTitle,ILSS_POWERSTATUS,CurY);

		if (SectionState[ILSS_POWERSTATUS].Show)
		{
			LNGID MsgID;
			SYSTEM_POWER_STATUS PowerStatus;
			GetSystemPowerStatus(&PowerStatus);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPowerStatusAC);
			switch(PowerStatus.ACLineStatus)
			{
				case AC_LINE_OFFLINE:      MsgID=MInfoPowerStatusACOffline; break;
				case AC_LINE_ONLINE:       MsgID=MInfoPowerStatusACOnline; break;
				case AC_LINE_BACKUP_POWER: MsgID=MInfoPowerStatusACBackUp; break;
				default:                   MsgID=MInfoPowerStatusACUnknown; break;
			}
			PrintInfo(MSG(MsgID));

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPowerStatusBCLifePercent);
			if (PowerStatus.BatteryLifePercent > 100)
				strOutStr = MSG(MInfoPowerStatusBCLifePercentUnknown);
			else
				strOutStr.Format(L"%d%%",PowerStatus.BatteryLifePercent);
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPowerStatusBC);
			strOutStr=L"";
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
					if (!strOutStr.IsEmpty())
						strOutStr += L" ";
					strOutStr += MSG(MInfoPowerStatusBCCharging);
				}
			}
			PrintInfo(strOutStr);

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPowerStatusBCTimeRem);
			if (PowerStatus.BatteryLifeTime != BATTERY_LIFE_UNKNOWN)
			{
				DWORD s = PowerStatus.BatteryLifeTime%60;
				DWORD m = (PowerStatus.BatteryLifeTime/60)%60;
				DWORD h = PowerStatus.BatteryLifeTime/3600;
				PrintInfo(FormatString()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<h<<GetTimeSeparator()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<m<<GetTimeSeparator()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<s);
			}
			else
				PrintInfo(MSG(MInfoPowerStatusBCTMUnknown));

			GotoXY(X1+2,CurY++);
			PrintText(MInfoPowerStatusBCFullTimeRem);
			if (PowerStatus.BatteryFullLifeTime != BATTERY_LIFE_UNKNOWN)
			{
				DWORD s = PowerStatus.BatteryLifeTime%60;
				DWORD m = (PowerStatus.BatteryLifeTime/60)%60;
				DWORD h = PowerStatus.BatteryLifeTime/3600;
				PrintInfo(FormatString()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<h<<GetTimeSeparator()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<m<<GetTimeSeparator()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<s);
			}
			else
				PrintInfo(MSG(MInfoPowerStatusBCFTMUnknown));
		}
	}

	if (AnotherPanel->GetMode() == FILE_PANEL)
	{
		/* #5 - description */
		strTitle = MSG(MInfoDescription);
		DrawTitle(strTitle,ILSS_DIRDESCRIPTION,CurY);

		if (SectionState[ILSS_DIRDESCRIPTION].Show)
		{
			if (ShowDirDescription(CurY))
			{
				DizView->SetPosition(X1+1,CurY,X2-1,Y2-1);
				if (CurY < Y2)
					CurY=Y2-1;
			}
			else
			{
				GotoXY(X1+2,CurY++);
				PrintText(MInfoDizAbsent);
			}
		}
	}

	if (AnotherPanel->GetMode() == PLUGIN_PANEL)
	{
		/* #6 - Plugin Description */
		strTitle = MSG(MInfoPlugin);
		DrawTitle(strTitle,ILSS_PLDESCRIPTION,CurY);
		if (SectionState[ILSS_PLDESCRIPTION].Show)
		{
			if (ShowPluginDescription(CurY))
			{
				;
			}
		}
	}

	Flags.Clear(FSCROBJ_ISREDRAWING);
}

__int64 InfoList::VMProcess(int OpCode,void *vParam,__int64 iParam)
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

void InfoList::SelectShowMode(void)
{
	MenuDataEx ShowModeMenuItem[]=
	{
		MSG(MMenuInfoShowModeDisk),LIF_SELECTED,0,
		MSG(MMenuInfoShowModeMemory),0,0,
		MSG(MMenuInfoShowModeDirDiz),0,0,
		MSG(MMenuInfoShowModePluginDiz),0,0,
		MSG(MMenuInfoShowModePower),0,0,
	};

	for (size_t i=0; i<ARRAYSIZE(SectionState); i++)
		ShowModeMenuItem[i].SetCheck( SectionState[i].Show ? L'+':L'-');

	if (!Opt.InfoPanel.ShowPowerStatus)
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

		VMenu ShowModeMenu(MSG(MMenuInfoShowModeTitle),ShowModeMenuItem,ARRAYSIZE(ShowModeMenuItem),0);
		ShowModeMenu.SetHelp(L"InfoPanelShowMode");
		ShowModeMenu.SetPosition(X1+4,-1,0,0);
		ShowModeMenu.SetFlags(VMENU_WRAPMODE);
		bool MenuNeedRefresh=true;

		while (!ShowModeMenu.Done())
		{
			if (MenuNeedRefresh)
			{
				ShowModeMenu.Hide(); // спрячем
				ShowModeMenu.SetPosition(X1+4,-1,0,0);
				ShowModeMenu.Show();
				MenuNeedRefresh=false;
			}

			int Key=ShowModeMenu.ReadInput();
			int MenuPos=ShowModeMenu.GetSelectPos();

			switch (Key)
			{
				case KEY_MULTIPLY:
				case L'*':
					ShowMode=2;
					ShowModeMenu.SetExitCode(MenuPos);
					break;

				case KEY_ADD:
				case L'+':
					ShowMode=1;
					ShowModeMenu.SetExitCode(MenuPos);
					break;

				case KEY_SUBTRACT:
				case L'-':
					ShowMode=0;
					ShowModeMenu.SetExitCode(MenuPos);
					break;

				default:
					ShowModeMenu.ProcessInput();
					break;
			}
		}

		if ((ShowCode=ShowModeMenu.Modal::GetExitCode())<0)
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
		Opt.InfoPanel.strShowStatusInfo=L"";
		for (size_t i=0; i < ARRAYSIZE(SectionState); ++i)
			Opt.InfoPanel.strShowStatusInfo += SectionState[i].Show?L'1':L'0';

		Redraw();
	}
}

int InfoList::ProcessKey(int Key)
{
	if (!IsVisible())
		return FALSE;

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
	{
		ExecShortcutFolder(Key-KEY_RCTRL0);
		return TRUE;
	}

	switch (Key)
	{
		case KEY_F1:
		{
			Help Hlp(L"InfoPanel");
			return TRUE;
		}
		case KEY_CTRLF12:
		case KEY_RCTRLF12:
			SelectShowMode();
			return TRUE;
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (!strDizFileName.IsEmpty())
			{
				CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(strCurDir);
				FarChDir(strCurDir);
				new FileViewer(strDizFileName,TRUE);//OT
			}

			CtrlObject->Cp()->Redraw();
			return TRUE;
		case KEY_F4:
			/* $ 30.04.2001 DJ
			не показываем редактор, если ничего не задано в именах файлов;
			не редактируем имена описаний со звездочками;
			убираем лишнюю перерисовку панелей
			*/
		{
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
			AnotherPanel->GetCurDir(strCurDir);
			FarChDir(strCurDir);

			if (!strDizFileName.IsEmpty())
			{
				new FileEditor(strDizFileName,CP_DEFAULT,FFILEEDIT_ENABLEF6);
			}
			else if (!Opt.InfoPanel.strFolderInfoFiles.IsEmpty())
			{
				string strArgName;
				const wchar_t *p = Opt.InfoPanel.strFolderInfoFiles;

				while ((p = GetCommaWord(p,strArgName)) )
				{
					if (!wcspbrk(strArgName, L"*?"))
					{
						new FileEditor(strArgName,CP_DEFAULT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);
						break;
					}
				}
			}

			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			//AnotherPanel->Redraw();
			Update(0);
			CtrlObject->Cp()->Redraw();
			return TRUE;
		}
		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			Redraw();
			return TRUE;
		}
	}

	if (DizView && Key >= 256)
	{
		int DVX1,DVX2,DVY1,DVY2;
		DizView->GetPosition(DVX1,DVY1,DVX2,DVY2);

		if (DVY1 < Y2)
		{
			int ret = DizView->ProcessKey(Key);

			if (Key == KEY_F8 || Key == KEY_F2 || Key == KEY_SHIFTF2)
			{
				DynamicUpdateKeyBar();
				CtrlObject->MainKeyBar->Redraw();
			}

			if (Key == KEY_F7 || Key == KEY_SHIFTF7)
			{
				__int64 Pos, Length;
				DWORD Flags;
				DizView->GetSelectedParam(Pos,Length,Flags);
				//ShellUpdatePanels(nullptr,FALSE);
				DizView->InRecursion++;
				Redraw();
				CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
				DizView->SelectText(Pos,Length,Flags|1);
				DizView->InRecursion--;
			}

			return(ret);
		}
	}

	return FALSE;
}


int InfoList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int RetCode;

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return(RetCode);

	bool NeedRedraw=false;
	Panel *AnotherPanel = CtrlObject->Cp()->GetAnotherPanel(this);
	bool ProcessDescription = AnotherPanel->GetMode() == FILE_PANEL;
	bool ProcessPluginDescription = AnotherPanel->GetMode() == PLUGIN_PANEL;
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
		if (DVY1 < Y2)
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
					ProcessKey(KEY_F3);
					return TRUE;
				}

				if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
				{
					ProcessKey(KEY_F4);
					return TRUE;
				}
			}
		}
	}

	if (NeedRedraw)
		Redraw();

	SetFocus();

	if (DizView)
	{
		if (DVY1 < Y2)
			return(DizView->ProcessMouse(MouseEvent));
	}
	return TRUE;
}


void InfoList::PrintText(const wchar_t *Str)
{
	if (WhereY()<=Y2-1)
	{
		FS<<fmt::MaxWidth(X2-WhereX())<<Str;
	}
}


void InfoList::PrintText(LNGID MsgID)
{
	PrintText(MSG(MsgID));
}


void InfoList::PrintInfo(const wchar_t *str)
{
	if (WhereY()>Y2-1)
		return;

	FarColor SaveColor=GetColor();
	int MaxLength=X2-WhereX()-2;

	if (MaxLength<0)
		MaxLength=0;

	string strStr = str;
	TruncStr(strStr,MaxLength);
	int Length=(int)strStr.GetLength();
	int NewX=X2-Length-1;

	if (NewX>X1 && NewX>WhereX())
	{
		GotoXY(NewX,WhereY());
		SetColor(COL_PANELINFOTEXT);
		FS<<strStr<<L" ";
		SetColor(SaveColor);
	}
}


void InfoList::PrintInfo(LNGID MsgID)
{
	PrintInfo(MSG(MsgID));
}


bool InfoList::ShowDirDescription(int YPos)
{
	Panel *AnotherPanel = CtrlObject->Cp()->GetAnotherPanel(this);

	string strDizDir;
	AnotherPanel->GetCurDir(strDizDir);

	if (!strDizDir.IsEmpty())
		AddEndSlash(strDizDir);

	string strArgName;
	const wchar_t *NamePtr = Opt.InfoPanel.strFolderInfoFiles;

	while ((NamePtr=GetCommaWord(NamePtr,strArgName)))
	{
		string strFullDizName;
		strFullDizName = strDizDir;
		strFullDizName += strArgName;
		FAR_FIND_DATA_EX FindData;

		if (!apiGetFindDataEx(strFullDizName, FindData))
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
	Panel *AnotherPanel = CtrlObject->Cp()->GetAnotherPanel(this);

	static wchar_t VertcalLine[2]={BoxSymbols[BS_V2],0};

	OpenPanelInfo Info;
	AnotherPanel->GetOpenPanelInfo(&Info);

	int Y=YPos;
	for (size_t I=0; I<Info.InfoLinesNumber; I++, Y++)
	{
		if (Y >= Y2)
			break;

		const InfoPanelLine *InfoLine=&Info.InfoLines[I];
		GotoXY(X1,Y);
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		SetColor(COL_PANELTEXT);
		FS<<fmt::MinWidth(X2-X1-1)<<L"";
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		GotoXY(X1+2,Y);

		if (InfoLine->Separator)
		{
			string strTitle;

			if (InfoLine->Text && *InfoLine->Text)
				strTitle.Append(L" ").Append(InfoLine->Text).Append(L" ");

			DrawSeparator(Y);
			TruncStr(strTitle,X2-X1-3);
			GotoXY(X1+(X2-X1-(int)strTitle.GetLength())/2,Y);
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
		delete DizView;
		DizView=nullptr;
	}

	strDizFileName.Clear();
}

int InfoList::OpenDizFile(const wchar_t *DizFile,int YPos)
{
	bool bOK=true;
	_tran(SysLog(L"InfoList::OpenDizFile([%s]",DizFile));

	if (!DizView)
	{
		DizView=new DizViewer;

		if (!DizView)
			return FALSE;

		_tran(SysLog(L"InfoList::OpenDizFile() create new Viewer = %p",DizView));
		DizView->SetRestoreScreenMode(FALSE);
		DizView->SetPosition(X1+1,YPos,X2-1,Y2-1);
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
			delete DizView;
			DizView = nullptr;
			return FALSE;
		}

		strDizFileName = DizFile;
	}

	DizView->Show();
	string strTitle;
	strTitle.Append(L" ").Append(PointToName(strDizFileName)).Append(L" ");
	TruncStr(strTitle,X2-X1-3);
	GotoXY(X1+(X2-X1-(int)strTitle.GetLength())/2,YPos-1);
	SetColor(COL_PANELTEXT);
	PrintText(strTitle);
	return TRUE;
}

void InfoList::SetFocus()
{
	Panel::SetFocus();
	SetMacroMode(FALSE);
}

void InfoList::KillFocus()
{
	Panel::KillFocus();
	SetMacroMode(TRUE);
}

void InfoList::SetMacroMode(int Restore)
{
	if (!CtrlObject)
		return;

	if (PrevMacroMode == -1)
		PrevMacroMode = CtrlObject->Macro.GetMode();

	CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_INFOPANEL);
}


int InfoList::GetCurName(string &strName, string &strShortName)
{
	strName = strDizFileName;
	ConvertNameToShort(strName, strShortName);
	return (TRUE);
}

BOOL InfoList::UpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;
	KB->SetAllGroup(KBL_MAIN, MInfoF1, 12);
	KB->SetAllGroup(KBL_SHIFT, MInfoShiftF1, 12);
	KB->SetAllGroup(KBL_ALT, MInfoAltF1, 12);
	KB->SetAllGroup(KBL_CTRL, MInfoCtrlF1, 12);
	KB->SetAllGroup(KBL_CTRLSHIFT, MInfoCtrlShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALT, MInfoCtrlAltF1, 12);
	KB->SetAllGroup(KBL_ALTSHIFT, MInfoAltShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALTSHIFT, MInfoCtrlAltShiftF1, 12);
	DynamicUpdateKeyBar();
	return TRUE;
}

void InfoList::DynamicUpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;

	if (DizView)
	{
		KB->Change(MSG(MInfoF3), 3-1);

		if (DizView->GetCodePage() != GetOEMCP())
			KB->Change(MSG(MViewF8DOS), 7);
		else
			KB->Change(MSG(MInfoF8), 7);

		if (!DizView->GetWrapMode())
		{
			if (DizView->GetWrapType())
				KB->Change(MSG(MViewShiftF2), 2-1);
			else
				KB->Change(MSG(MViewF2), 2-1);
		}
		else
			KB->Change(MSG(MViewF2Unwrap), 2-1);

		if (DizView->GetWrapType())
			KB->Change(KBL_SHIFT, MSG(MViewF2), 2-1);
		else
			KB->Change(KBL_SHIFT, MSG(MViewShiftF2), 2-1);
	}
	else
	{
		KB->Change(MSG(MF2), 2-1);
		KB->Change(KBL_SHIFT, L"", 2-1);
		KB->Change(L"", 3-1);
		KB->Change(L"", 8-1);
		KB->Change(KBL_SHIFT, L"", 8-1);
		KB->Change(KBL_ALT, MSG(MAltF8), 8-1);  // стандартный для панели - "хистори"
	}

	KB->ReadRegGroup(L"Info",Opt.strLanguage);
	KB->SetAllRegGroup();
}

int InfoList::UpdateIfChanged(int UpdateMode)
{
	if (Opt.InfoPanel.ShowPowerStatus && SectionState[ILSS_POWERSTATUS].Show)
	{
		if (IsVisible() && Events.PowerChangeEvent.Signaled())
		{
			Redraw();
			return TRUE;
		}
	}

	return FALSE;
}

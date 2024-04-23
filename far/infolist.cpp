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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "infolist.hpp"

// Internal:
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
#include "interf.hpp"
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
#include "network.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/enum_tokens.hpp"
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

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
	int Y;     // Где?
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
	PowerListener([&]{ if (Global->Opt->InfoPanel.ShowPowerStatus && IsVisible() && SectionState[ILSS_POWERSTATUS].Show) { Redraw(); }})
{
	m_Type = panel_type::INFO_PANEL;
	if (Global->Opt->InfoPanel.strShowStatusInfo.empty())
	{
		for (auto& i: SectionState)
		{
			i.Show = true;
		}
	}
	else
	{
		const auto ShowStatusInfoLen = Global->Opt->InfoPanel.strShowStatusInfo.size();
		for (const auto& [i, index]: enumerate(SectionState))
		{
			i.Show = index < ShowStatusInfoLen?
				Global->Opt->InfoPanel.strShowStatusInfo[index] == L'1' :
				true;
		}
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

void InfoList::DrawTitle(string_view const Title, int Id, int &CurY)
{
	SetColor(COL_PANELBOX);
	DrawSeparator(CurY);
	SetColor(COL_PANELTEXT);

	auto strTitle = concat(L' ', Title, L' ');
	inplace::truncate_left(strTitle, std::max(0, m_Where.width() - 4));
	GotoXY(m_Where.left + (m_Where.width() - static_cast<int>(strTitle.size())) / 2, CurY);
	PrintText(strTitle);
	GotoXY(m_Where.left + 1, CurY);
	PrintText(SectionState[Id].Show? L"[-]"sv : L"[+]"sv);
	SectionState[Id].Y=CurY;
	CurY++;
}

void InfoList::DisplayObject()
{
	if (m_Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	m_Flags.Set(FSCROBJ_ISREDRAWING);

	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	string strVolumeName, strFileSystemName;
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
	string strDiskNumber;
	CloseFile();

	Box(m_Where, colors::PaletteColorToFarColor(COL_PANELBOX), DOUBLE_BOX);
	SetScreen({ m_Where.left + 1, m_Where.top + 1, m_Where.right - 1, m_Where.bottom - 1 }, L' ', colors::PaletteColorToFarColor(COL_PANELTEXT));
	SetColor(IsFocused()? COL_PANELSELECTEDTITLE : COL_PANELTITLE);

	const auto& strTitle = GetTitleForDisplay();
	if (!strTitle.empty())
	{
		GotoXY(m_Where.left + (m_Where.width() - static_cast<int>(strTitle.size())) / 2, m_Where.top);
		Text(strTitle);
	}

	SetColor(COL_PANELTEXT);

	int CurY= m_Where.top + 1;

	/* #1 - computer name/user name */
	{
		string strComputerName;
		if (!os::GetComputerNameEx(static_cast<COMPUTER_NAME_FORMAT>(Global->Opt->InfoPanel.ComputerNameFormat.Get()), strComputerName) && !os::GetComputerName(strComputerName))
		{
			// TODO: fallback?
		}

		GotoXY(m_Where.left + 2, CurY++);
		PrintText(lng::MInfoCompName);
		PrintInfo(strComputerName);

		os::netapi::ptr<SERVER_INFO_101> ServerInfo;
		if (NetServerGetInfo(nullptr, 101, std::bit_cast<BYTE**>(&ptr_setter(ServerInfo))) == NERR_Success)
		{
			if(ServerInfo->sv101_comment && *ServerInfo->sv101_comment)
			{
				GotoXY(m_Where.left + 2, CurY++);
				PrintText(lng::MInfoCompDescription);
				PrintInfo(ServerInfo->sv101_comment);
			}
		}

		string UserLogonName, UserExtendedName;
		const auto UserNameRead = os::GetUserName(UserLogonName);

		string_view DisplayName = UserLogonName;

		if (const auto NameFormat = static_cast<EXTENDED_NAME_FORMAT>(Global->Opt->InfoPanel.UserNameFormat.Get());
			NameFormat != NameUnknown && os::GetUserNameEx(NameFormat, UserExtendedName))
		{
			DisplayName = UserExtendedName;
		}

		GotoXY(m_Where.left + 2, CurY++);
		PrintText(lng::MInfoUserName);
		PrintInfo(DisplayName);

		os::netapi::ptr<USER_INFO_1> UserInfo;
		if (UserNameRead && NetUserGetInfo(nullptr, UserLogonName.c_str(), 1, std::bit_cast<BYTE**>(&ptr_setter(UserInfo))) == NERR_Success)
		{
			if(UserInfo->usri1_comment && *UserInfo->usri1_comment)
			{
				GotoXY(m_Where.left + 2, CurY++);
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
			GotoXY(m_Where.left + 2, CurY++);
			PrintText(lng::MInfoUserAccessLevel);
			PrintInfo(LabelId);
		}

		GotoXY(m_Where.left + 2, CurY++);
		PrintText(lng::MInfoUserAccessElevated);
		PrintInfo(os::security::is_admin()? lng::MYes : lng::MNo);
	}

	string SectionTitle;

	/* #2 - disk info */
	if (SectionState[ILSS_DISKINFO].Show)
	{
		m_CurDir = AnotherPanel->GetCurDir();

		if (m_CurDir.empty())
			m_CurDir = os::fs::get_current_directory();

		/*
			Корректно отображать инфу при заходе в Juction каталог
			Рут-диск может быть другим
		*/
		string strDriveRoot;
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
		{
			// GetPathRoot expands network drives, it's too early for that
			strDriveRoot = os::fs::drive::get_type(m_CurDir) == DRIVE_REMOTE?
				extract_root_directory(m_CurDir) :
				GetPathRoot(m_CurDir);
		}

		if (os::fs::GetVolumeInformation(strDriveRoot,&strVolumeName,
		                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
		                            &strFileSystemName))
		{
			auto DiskTypeId = lng::MInfoUnknown;
			const auto DriveType = os::fs::drive::get_type(strDriveRoot);
			string strAssocPath;
			bool UseAssocPath = false;

			switch (DriveType)
			{
				case DRIVE_REMOVABLE:
					DiskTypeId = lng::MInfoRemovable;
					break;

				case DRIVE_FIXED:
					DiskTypeId = lng::MInfoFixed;
					break;

				case DRIVE_REMOTE:
					{
						DiskTypeId = lng::MInfoNetwork;
						UseAssocPath = DriveLocalToRemoteName(false, strDriveRoot, strAssocPath);
					}
					break;

				case DRIVE_CDROM:
					if (Global->Opt->InfoPanel.ShowCDInfo)
					{
						static_assert(std::to_underlying(lng::MInfoHDDVDRAM) - std::to_underlying(lng::MInfoCDROM) == std::to_underlying(cd_type::hddvdram) - std::to_underlying(cd_type::cdrom));

						DiskTypeId = lng::MInfoCDROM + (std::to_underlying(get_cdrom_type(strDriveRoot)) - std::to_underlying(cd_type::cdrom));
					}
					else
					{
						DiskTypeId = lng::MInfoCDROM;
					}
					break;

				case DRIVE_RAMDISK:
					DiskTypeId = lng::MInfoRAM;
					break;

				default:
					break;
			}

			if (GetSubstName(DriveType,strDriveRoot,strAssocPath))
			{
				DiskTypeId = lng::MInfoSUBST;
				UseAssocPath = true;
			}
			else if(DriveCanBeVirtual(DriveType) && GetVHDInfo(strDriveRoot,strAssocPath))
			{
				DiskTypeId = lng::MInfoVirtual;
				UseAssocPath = true;
			}

			SectionTitle = concat(msg(DiskTypeId), L' ', msg(lng::MInfoDisk), L' ', strDriveRoot, L" ("sv, strFileSystemName, L')');

			if (UseAssocPath)
				append(SectionTitle, L' ', strAssocPath);

			strDiskNumber = far::format(
				L"{:04X}-{:04X}"sv,
				extract_integer<WORD, 1>(VolumeNumber),
				extract_integer<WORD, 0>(VolumeNumber)
			);
		}
		else // Error!
			SectionTitle = strDriveRoot;
	}
	else
		SectionTitle = msg(lng::MInfoDiskTitle);

	DrawTitle(SectionTitle,ILSS_DISKINFO,CurY);

	const auto bytes_suffix = upper(msg(lng::MListBytes));
	const auto size2str = [&bytes_suffix](uint64_t const Size)
	{
		string str;
		if (Global->Opt->ShowBytes)
		{
			str = GroupDigits(Size); // + L' ';
		}
		else
		{
			str = FileSizeToStr(Size, 0, COLFLAGS_FLOATSIZE | COLFLAGS_SHOW_MULTIPLIER);
			if (str.back() != bytes_suffix[0])
				str += bytes_suffix;
		}
		return str;
	};

	const auto PrintMetricText = [&](lng const Kind, lng const Metric)
	{
		PrintText(far::format(L"{}, {}"sv, msg(Kind), msg(Metric)));
	};

	const auto PrintMetric = [&](lng const Kind, unsigned long long const Total, unsigned long long const Available)
	{
		GotoXY(m_Where.left + 2, CurY++);
		PrintMetricText(Kind, lng::MInfoMetricTotal);
		PrintInfo(size2str(Total));
		GotoXY(m_Where.left + 2, CurY++);
		PrintMetricText(Kind, lng::MInfoMetricAvailable);
		PrintInfo(far::format(L"{}%, {}"sv, ToPercent(Available, Total), size2str(Available)));
	};

	if (SectionState[ILSS_DISKINFO].Show)
	{
		/* #2.2 - disk info: size */
		if (unsigned long long UserTotal, UserFree; os::fs::get_disk_size(m_CurDir, &UserTotal, {}, &UserFree))
			PrintMetric(lng::MInfoDiskSpace, UserTotal, UserFree);

		/* #4 - disk info: label & SN */
		GotoXY(m_Where.left + 2, CurY++);
		PrintText(lng::MInfoDiskLabel);
		PrintInfo(strVolumeName);

		GotoXY(m_Where.left + 2, CurY++);
		PrintText(lng::MInfoDiskNumber);
		PrintInfo(strDiskNumber);
	}

	/* #3 - memory info */
	DrawTitle(msg(lng::MInfoMemory), ILSS_MEMORYINFO, CurY);

	if (SectionState[ILSS_MEMORYINFO].Show)
	{
		MEMORYSTATUSEX ms{ sizeof(ms) };
		if (GlobalMemoryStatusEx(&ms))
		{
			PrintMetric(lng::MInfoMemoryCommittable, ms.ullTotalPageFile, ms.ullAvailPageFile);
			PrintMetric(lng::MInfoMemoryAddressable, ms.ullTotalVirtual, ms.ullAvailVirtual);
			PrintMetric(lng::MInfoMemoryPhysical, ms.ullTotalPhys, ms.ullAvailPhys);

			if (ULONGLONG TotalMemoryInKilobytes; imports.GetPhysicallyInstalledSystemMemory && imports.GetPhysicallyInstalledSystemMemory(&TotalMemoryInKilobytes))
			{
				GotoXY(m_Where.left + 2, CurY++);
				PrintMetricText(lng::MInfoMemoryPhysical, lng::MInfoMetricMemoryInstalled);
				PrintInfo(size2str(TotalMemoryInKilobytes << 10));
			}
		}
	}

	/* #4 - power status */
	if (Global->Opt->InfoPanel.ShowPowerStatus)
	{
		DrawTitle(msg(lng::MInfoPowerStatus), ILSS_POWERSTATUS, CurY);

		if (SectionState[ILSS_POWERSTATUS].Show)
		{
			lng MsgID;
			SYSTEM_POWER_STATUS PowerStatus;
			GetSystemPowerStatus(&PowerStatus);

			GotoXY(m_Where.left + 2, CurY++);
			PrintText(lng::MInfoPowerStatusAC);
			switch(PowerStatus.ACLineStatus)
			{
				case AC_LINE_OFFLINE:      MsgID = lng::MInfoPowerStatusACOffline; break;
				case AC_LINE_ONLINE:       MsgID = lng::MInfoPowerStatusACOnline; break;
				case AC_LINE_BACKUP_POWER: MsgID = lng::MInfoPowerStatusACBackUp; break;
				default:                   MsgID = lng::MInfoPowerStatusACUnknown; break;
			}
			PrintInfo(msg(MsgID));

			GotoXY(m_Where.left + 2, CurY++);

			PrintText(lng::MInfoPowerStatusBC);

			if (PowerStatus.BatteryFlag == BATTERY_FLAG_UNKNOWN)
			{
				PrintInfo(msg(lng::MInfoPowerStatusBCUnknown));
			}
			else
			{
				auto ChargeStatus = lng::MInfoPowerStatusBCUnknown;
				switch (PowerStatus.BatteryFlag & (BATTERY_FLAG_HIGH | BATTERY_FLAG_LOW | BATTERY_FLAG_CRITICAL | BATTERY_FLAG_NO_BATTERY))
				{
				case 0:                        ChargeStatus = lng::MInfoPowerStatusBCMedium;    break;
				case BATTERY_FLAG_HIGH:        ChargeStatus = lng::MInfoPowerStatusBCHigh;      break;
				case BATTERY_FLAG_LOW:         ChargeStatus = lng::MInfoPowerStatusBCLow;       break;
				case BATTERY_FLAG_CRITICAL:    ChargeStatus = lng::MInfoPowerStatusBCCritical;  break;
				case BATTERY_FLAG_NO_BATTERY:  ChargeStatus = lng::MInfoPowerStatusBCNoSysBat;  break;
				}

				auto strOutStr = far::format(L"{} ({})"sv,
					msg(ChargeStatus),
					PowerStatus.BatteryLifePercent > 100?
						msg(lng::MInfoPowerStatusBCLifePercentUnknown) :
						str(PowerStatus.BatteryLifePercent) + L'%'
				);

				if (PowerStatus.BatteryFlag & BATTERY_FLAG_CHARGING)
					append(strOutStr, L", "sv, msg(lng::MInfoPowerStatusBCCharging));

				PrintInfo(strOutStr);
			}

			const auto GetBatteryTime = [&](size_t SecondsCount)
			{
				if (SecondsCount == BATTERY_LIFE_UNKNOWN)
				{
					return PowerStatus.ACLineStatus == AC_LINE_ONLINE?
						L"-"s :
						msg(lng::MInfoPowerStatusUnknown);
				}

				return ConvertDurationToHMS(std::chrono::seconds{SecondsCount});
			};

			GotoXY(m_Where.left + 2, CurY++);
			PrintText(lng::MInfoPowerStatusBCTimeRem);
			PrintInfo(GetBatteryTime(PowerStatus.BatteryLifeTime));

			GotoXY(m_Where.left + 2, CurY++);
			PrintText(lng::MInfoPowerStatusBCFullTimeRem);
			PrintInfo(GetBatteryTime(PowerStatus.BatteryFullLifeTime));
		}
	}

	if (AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL)
	{
		/* #5 - description */
		DrawTitle(msg(lng::MInfoDescription), ILSS_DIRDESCRIPTION, CurY);

		if (SectionState[ILSS_DIRDESCRIPTION].Show)
		{
			if (CurY < m_Where.bottom && ShowDirDescription(CurY))
			{
				DizView->SetPosition({ m_Where.left + 1, CurY, m_Where.right - 1, m_Where.bottom - 1 });
				CurY = m_Where.bottom - 1;
			}
			else
			{
				GotoXY(m_Where.left + 2, CurY++);
				PrintText(lng::MInfoDizAbsent);
			}
		}
	}

	if (AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		/* #6 - Plugin Description */
		DrawTitle(msg(lng::MInfoPlugin), ILSS_PLDESCRIPTION, CurY);
		if (SectionState[ILSS_PLDESCRIPTION].Show)
		{
			if (ShowPluginDescription(CurY))
			{
				// ;
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

	for (const auto& [i, index]: enumerate(SectionState))
	{
		ShowModeMenuItem[index].SetCustomCheck(i.Show? L'+' : L'-');
	}

	if (!Global->Opt->InfoPanel.ShowPowerStatus)
	{
		ShowModeMenuItem[ILSS_POWERSTATUS].SetDisable(true);
		ShowModeMenuItem[ILSS_POWERSTATUS].SetCustomCheck(L' ');
	}

	int ShowCode=-1;
	int ShowMode=-1;

	{
		const auto ShowModeMenu = VMenu2::create(msg(lng::MMenuInfoShowModeTitle), std::as_const(ShowModeMenuItem));
		ShowModeMenu->SetHelp(L"InfoPanelShowMode"sv);
		ShowModeMenu->SetPosition({ m_Where.left + 4, -1, 0, 0 });
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

	for (const auto& i: SectionState)
	{
		Global->Opt->InfoPanel.strShowStatusInfo += i.Show? L"1"s : L"0"s;
	}

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
			help::show(L"InfoPanel"sv);
			return true;

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
					if (i.empty())
						continue;

					if (i.find_first_of(L"*?"sv) != string::npos)
						continue;

					FileEditor::create(i, CP_DEFAULT, FFILEEDIT_CANNEWFILE | FFILEEDIT_ENABLEF6);
					break;
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
		if (DizView->GetPosition().top < m_Where.bottom)
		{
			const auto ret = DizView->ProcessKey(Key);

			if (any_of(LocalKey, KEY_F2, KEY_SHIFTF2, KEY_F4, KEY_SHIFTF4, KEY_F8, KEY_SHIFTF8))
			{
				DynamicUpdateKeyBar();
				Parent()->GetKeybar().Redraw();
			}

			if (any_of(LocalKey, KEY_F7, KEY_SHIFTF7))
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

	const std::pair<InfoListSectionStateIndex, bool> Sections[]
	{
		{ ILSS_DISKINFO,        true },
		{ ILSS_MEMORYINFO,      true },
		{ ILSS_DIRDESCRIPTION,  AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL },
		{ ILSS_PLDESCRIPTION,   AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL },
		{ ILSS_POWERSTATUS,     true },
	};

	if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && IsMouseButtonEvent(MouseEvent->dwEventFlags))
	{
		for (const auto& [Section, Enabled]: Sections)
		{
			if (!Enabled || MouseEvent->dwMousePosition.Y != SectionState[Section].Y)
				continue;

			SectionState[Section].Show = !SectionState[Section].Show;
			NeedRedraw = true;
		}
	}

	int DVTop=-1;
	if (DizView)
	{
		const auto DV = DizView->GetPosition();
		DVTop = DV.top;
		if (DV.top < m_Where.bottom)
		{
			if (SectionState[ILSS_DIRDESCRIPTION].Show && MouseEvent->dwMousePosition.Y > SectionState[ILSS_DIRDESCRIPTION].Y)
			{
				if (
					MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED &&
					MouseEvent->dwMousePosition.X > DV.left + 1 &&
					MouseEvent->dwMousePosition.X < DV.right - DizView->GetShowScrollbar() - 1 &&
					MouseEvent->dwMousePosition.Y > DV.top + 1 &&
					MouseEvent->dwMousePosition.Y < DV.bottom - 1
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
		if (DVTop < m_Where.bottom)
			return DizView->ProcessMouse(MouseEvent);
	}
	return true;
}


void InfoList::PrintText(string_view const Str) const
{
	if (WhereY() <= m_Where.bottom - 1)
	{
		Text(cut_right(Str, m_Where.right - WhereX()));
	}
}


void InfoList::PrintText(lng MsgID) const
{
	PrintText(msg(MsgID));
}


void InfoList::PrintInfo(string_view const Str) const
{
	if (WhereY() > m_Where.bottom - 1)
		return;

	int MaxLength = m_Where.right - WhereX() - 2;

	if (MaxLength<0)
		MaxLength=0;

	const auto strStr = truncate_left(Str, MaxLength);
	const auto Length=static_cast<int>(strStr.size());
	const auto NewX = m_Where.right - Length - 1;

	if (NewX > m_Where.left && NewX > WhereX())
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
		if (i.empty())
			continue;

		strFullDizName.resize(DirSize);
		append(strFullDizName, i);

		os::fs::find_data FindData;
		if (!os::fs::get_find_data(strFullDizName, FindData))
			continue;

		CutToSlash(strFullDizName);
		strFullDizName += FindData.FileName;

		if (OpenDizFile(strFullDizName, YPos))
			return true;
	}

	return false;
}


bool InfoList::ShowPluginDescription(int YPos) const
{
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	static wchar_t VertcalLine[]{ BoxSymbols[BS_V2], 0 };

	OpenPanelInfo Info;
	AnotherPanel->GetOpenPanelInfo(&Info);

	int Y=YPos;
	for (const auto& InfoLine: std::span(Info.InfoLines, Info.InfoLinesNumber))
	{
		if (Y >= m_Where.bottom)
			break;

		GotoXY(m_Where.left, Y);
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		SetColor(COL_PANELTEXT);
		Text(string(std::max(0, m_Where.width() - 2), L' '));
		SetColor(COL_PANELBOX);
		Text(VertcalLine);
		GotoXY(m_Where.left + 2, Y);

		if (InfoLine.Flags & IPLFLAGS_SEPARATOR)
		{
			string strTitle;

			if (InfoLine.Text && *InfoLine.Text)
				strTitle = concat(L' ', InfoLine.Text, L' ');

			DrawSeparator(Y);
			inplace::truncate_left(strTitle, std::max(0, m_Where.width() - 4));
			GotoXY(m_Where.left + (m_Where.width() - 1 - static_cast<int>(strTitle.size())) / 2, Y);
			SetColor(COL_PANELTEXT);
			PrintText(strTitle);
		}
		else
		{
			SetColor(COL_PANELTEXT);
			PrintText(NullToEmpty(InfoLine.Text));
			PrintInfo(NullToEmpty(InfoLine.Data));
		}

		++Y;
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

bool InfoList::OpenDizFile(string_view const DizFile, int const YPos)
{
	bool bOK=true;

	if (!DizView)
	{
		DizView = std::make_unique<DizViewer>(GetOwner());

		DizView->SetRestoreScreenMode(false);
		DizView->SetPosition({ m_Where.left + 1, YPos, m_Where.right - 1, m_Where.bottom - 1 });
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
		if (!DizView->OpenFile(DizFile, false))
		{
			DizView=nullptr;
			return false;
		}

		strDizFileName = DizFile;
	}

	DizView->Show();

	int CurY=YPos-1;
	DrawTitle(PointToName(strDizFileName), ILSS_DIRDESCRIPTION, CurY);
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

InfoList::power_listener::power_listener(std::function<void()> EventHandler):
	listener(update_power, std::move(EventHandler))
{
	message_manager::instance().enable_power_notifications();
}

InfoList::power_listener::~power_listener()
{
	message_manager::instance().disable_power_notifications();
}

Viewer* InfoList::GetViewer()
{
	return DizView.get();
}

Viewer* InfoList::GetById(int ID)
{
	return DizView && ID==DizView->GetId()?GetViewer():nullptr;
}

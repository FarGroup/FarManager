#include <cstdlib>
#include <windows.h>
#include <lm.h>
#include "NetCommon.hpp"
#include "Network.hpp"
#include "NetCfg.hpp"
#include "NetFavorites.hpp"
#include "NetClass.hpp"
#include "NetLng.hpp"
#include "guid.hpp"
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include <SimpleString.hpp>
#include <string>

NetResourceList* CommonRootResources;
BOOL SavedCommonRootResources = FALSE;

// -- NetResourceList --------------------------------------------------------
#ifdef NETWORK_LOGGING
FILE* NetBrowser::LogFile{};
int NetBrowser::LogFileRef = 0;

void NetBrowser::OpenLogFile(const wchar_t *lpFileName)
{
	if (!LogFileRef)
		LogFile = _wfopen(lpFileName, L"a+t");

	if (LogFile)
		fwprintf(LogFile, L"Opening plugin\n");

	LogFileRef++;
}

void NetBrowser::CloseLogfile()
{
	LogFileRef--;

	if (!LogFileRef && LogFile)fclose(LogFile),LogFile = {};
}

void NetBrowser::LogData(const wchar_t * Data)
{
	if (LogFile)
	{
		fwprintf(LogFile,L"%s\n", Data);
		wchar_t buffer[MAX_PATH];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, {}, GetLastError(), 0, buffer, ARRAYSIZE(buffer), {});
		fwprintf(LogFile,L"GetLastError returns: %s\n", buffer);
	}
}
#endif

NetResourceList::NetResourceList()
	: ResList(),
	ResCount(0)
{
}

NetResourceList::~NetResourceList()
{
	Clear();
}

NetResourceList& NetResourceList::operator=(NetResourceList& other)
{
	Clear();

	for (unsigned I = 0; I < other.Count(); I++)
		Push(other[I]);

	return *this;
}

void NetResourceList::Clear()
{
	for (unsigned i = 0; i < ResCount; i++)
		DeleteNetResource(ResList[i]);

	free(ResList);
	ResList = {};
	ResCount = 0;
}

void NetResourceList::DeleteNetResource(NETRESOURCE& Res)
{
	free(Res.lpRemoteName);
	free(Res.lpLocalName);
	free(Res.lpComment);
	free(Res.lpProvider);
}

wchar_t* NetResourceList::CopyText(const wchar_t* Text)
{
	return Text? _wcsdup(Text) : nullptr;
}

void NetResourceList::CopyNetResource(NETRESOURCE& Dest, const NETRESOURCE& Src)
{
	free(Dest.lpRemoteName);
	free(Dest.lpLocalName);
	free(Dest.lpComment);
	free(Dest.lpProvider);
	memcpy(&Dest, &Src, sizeof(NETRESOURCE));
	Dest.lpRemoteName = CopyText(Src.lpRemoteName);
	Dest.lpLocalName = CopyText(Src.lpLocalName);
	Dest.lpComment = CopyText(Src.lpComment);
	Dest.lpProvider = CopyText(Src.lpProvider);
}

void NetResourceList::Push(NETRESOURCE& Res)
{
	ResList = (NETRESOURCE*)realloc(ResList, (ResCount + 1) * sizeof(*ResList));
	ZeroMemory(&ResList [ResCount], sizeof(*ResList));
	CopyNetResource(ResList[ResCount], Res);
	ResCount++;
}

NETRESOURCE* NetResourceList::Top()
{
	if (ResCount == 0)
		return {};

	return &ResList[ResCount - 1];
}

void NetResourceList::Pop()
{
	if (ResCount > 0)
	{
		DeleteNetResource(ResList[ResCount - 1]);
		ResList = (NETRESOURCE*)realloc(ResList, (ResCount - 1) * sizeof(*ResList));
		ResCount--;
	}
}

BOOL NetResourceList::Enumerate(
	DWORD dwScope,
	DWORD dwType,
	DWORD dwUsage,
	LPNETRESOURCE lpNetResource)
{
	Clear();

	if (EnumFavorites(lpNetResource, this))
		return TRUE;

	if (!Opt.ScanNetwork && lpNetResource && (lpNetResource->dwDisplayType == RESOURCEDISPLAYTYPE_DOMAIN
		|| lpNetResource->dwDisplayType == RESOURCEDISPLAYTYPE_NETWORK))
		return TRUE;

	HANDLE hEnum;

	if (WNetOpenEnum(dwScope, dwType, dwUsage, lpNetResource, &hEnum) != NO_ERROR)
		return FALSE;

	BOOL EnumFailed = FALSE;

	for (;;)
	{
		NETRESOURCE nr[1024];
		DWORD NetSize = sizeof(nr), NetCount = ARRAYSIZE(nr);
		DWORD EnumCode = WNetEnumResource(hEnum, &NetCount, nr, &NetSize);

		if (EnumCode != NO_ERROR)
		{
			if (EnumCode != ERROR_NO_MORE_ITEMS)
			{
				Clear();
				EnumFailed = TRUE;
			}

			break;
		}

		if (NetCount > 0)
		{
			ResList = (NETRESOURCE*)realloc(ResList, (ResCount + NetCount) * sizeof(*ResList));
			ZeroMemory(&ResList [ResCount], sizeof(*ResList)*NetCount);

			for (unsigned I = 0; I < NetCount; I++)
				CopyNetResource(ResList[ResCount + I], nr[I]);

			ResCount += NetCount;
		}
	}

	WNetCloseEnum(hEnum);
	return !EnumFailed;
}

// -- NetBrowser -------------------------------------------------------------

NetBrowser::NetBrowser()
{
	{
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		settings.Get(0, StrPanelMode, m_PanelMode,ARRAYSIZE(m_PanelMode), L"3");
	}

	if (SavedCommonRootResources)
	{
		RootResources = *CommonRootResources;
		PCurResource = RootResources.Top();
	}
	else
	{
		NetResourceList::CopyNetResource(CurResource, CommonCurResource);

		if (PCommonCurResource)
			PCurResource = &CurResource;
		else
			PCurResource = {};
	}

	NetListRemoteName[0] = L'\0';
	CmdLinePath[0] = L'\0';
#ifdef NETWORK_LOGGING
	OpenLogFile(L"c:\\network.log");
#endif
}


NetBrowser::~NetBrowser()
#ifdef NETWORK_LOGGING
{
	CloseLogfile();
}
#else
= default;
#endif


#ifdef NETWORK_LOGGING

void NetBrowser::LogNetResource(NETRESOURCE &Res)
{
	if (LogFile)
	{
		fwprintf(LogFile, L"dwScope = %u\ndwType = %u\ndwDisplayType = %u\ndwUsage = %u\n", Res.dwScope, Res.dwType, Res.dwDisplayType, Res.dwUsage);
		fwprintf(LogFile, L"lpLocalName = %s\nlpRemoteName = %s\nlpComment = %s\nlpProvider = %s\n\n", Res.lpLocalName, Res.lpRemoteName, Res.lpComment, Res.lpProvider);
	}
}

#endif

BOOL NetBrowser::EnumerateNetList()
{
	if (PCurResource && PCurResource->lpRemoteName)
		lstrcpy(NetListRemoteName, PCurResource->lpRemoteName);
	else
		NetListRemoteName[0] = L'\0';

	if (!NetList.Enumerate(RESOURCE_GLOBALNET,RESOURCETYPE_ANY, 0, PCurResource))
	{
		if (!PCurResource)
		{
			const wchar_t* MsgItems[] = {GetMsg(MError), GetMsg(MNetCannotBrowse), GetMsg(MOk)};
			PsInfo.Message(&MainGuid, nullptr, FMSG_WARNING | FMSG_ERRORTYPE, {}, MsgItems,ARRAYSIZE(MsgItems), 1);
			return (FALSE);
		}
		else
		{
			// try again with connection
			AddConnection(PCurResource);

			if (!NetList.Enumerate(RESOURCE_GLOBALNET,RESOURCETYPE_ANY, 0, PCurResource))
				NetList.Clear();
		}
	}

	if (!CheckFavoriteItem(PCurResource) && Opt.HiddenShares)
	{
		PanelInfo PInfo = {sizeof(PanelInfo)};
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

		if (!Opt.HiddenSharesAsHidden || (PInfo.Flags & PFLAGS_SHOWHIDDEN))
		{
			// Check whether we need to get the hidden shares.

			if (PCurResource)
			{
				// If the parent of the current folder is not a server
				if (PCurResource->dwDisplayType != RESOURCEDISPLAYTYPE_SERVER)
				{
					return TRUE;
				}
			}
			if (NetList.Count() > 0)
			{
				// If there are elements, check the first element
				if ((NetList[NetList.Count() - 1].dwDisplayType) != RESOURCEDISPLAYTYPE_SHARE)
				{
					return TRUE;
				}
			}

			GetHiddenShares();
		}
	}

	/*
	if (NetCount==0 && CurResource && AddConnection(CurResource))
	if (WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,CurResource,&hEnum)==NO_ERROR)
	{
	GetNetList(hEnum,NetRes,NetCount);
	WNetCloseEnum(hEnum);
	}
	*/
	return TRUE;
}

BOOL NetBrowser::GotoFavorite(wchar_t* lpPath)
{
#ifdef NEWTWORK_LOGGING
	LogData(L"Entered NetBrowser::GotoFavorite")
#endif
	NETRESOURCE nr = {0};

	if (GetFavoriteResource(lpPath, &nr))
	{
#ifdef NETWORK_LOGGING
		LogData(L"GetFavoriteResource SUCCEEDED");
		LogNetResource(nr);
#endif
		NetResourceList::CopyNetResource(CurResource, nr);
		NetResourceList::DeleteNetResource(nr);
		PCurResource = &CurResource;
		PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 0, {});
		PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
		return TRUE;
	}

	return FALSE;
}

int NetBrowser::GetFindData(PluginPanelItem** pPanelItem, size_t* pItemsNumber, OPERATION_MODES OpMode)
{
#ifdef NETWORK_LOGGING
	LogData(L"Entering NetBrowser::GetFindData");
#endif

	if (OpMode & OPM_FIND)
		return FALSE;

	if (ReenterGetFindData)
		return TRUE;

	ReenterGetFindData++;

	if (ChangeDirSuccess)
	{
		if (CmdLinePath[0])
		{
			// prevent recursion
			wchar_t TmpCmdLinePath[MAX_PATH];
			lstrcpy(TmpCmdLinePath, CmdLinePath);
			CmdLinePath[0] = 0;
			ReenterGetFindData--;

			if (!GotoFavorite(TmpCmdLinePath))
				GotoComputer(TmpCmdLinePath);

			ReenterGetFindData++;
		}

		*pPanelItem = {};
		*pItemsNumber = 0;
		TSaveScreen SS;

		// get the list of connections, so that we can show mapped drive letters
		if (!ConnectedList.Enumerate(RESOURCE_CONNECTED,RESOURCETYPE_DISK, 0, {}))
		{
			const wchar_t* MsgItems[] = {GetMsg(MError), GetMsg(MNetCannotBrowse), GetMsg(MOk)};
			PsInfo.Message(&MainGuid, nullptr, FMSG_WARNING | FMSG_ERRORTYPE, {}, MsgItems,ARRAYSIZE(MsgItems), 1);
			ReenterGetFindData--;
			return FALSE;
		}

		if (!EnumerateNetList())
		{
			ReenterGetFindData--;
			return FALSE;
		}
	}

	ChangeDirSuccess = TRUE;
	PluginPanelItem* NewPanelItem = (PluginPanelItem*)malloc(sizeof(PluginPanelItem) * NetList.Count());
	*pPanelItem = NewPanelItem;

	if (!NewPanelItem)
	{
		ReenterGetFindData--;
		return (FALSE);
	}

	int CurItemPos = 0;

	for (unsigned I = 0; I < NetList.Count(); I++)
	{
		if (NetList[I].dwType == RESOURCETYPE_PRINT && !Opt.ShowPrinters)
			continue;

		wchar_t RemoteName[MAX_PATH], LocalName[MAX_PATH], Comment[300];
		GetRemoteName(&NetList[I], RemoteName);

		if (!NetList[I].lpComment)
			*Comment = 0;
		else
			lstrcpy(Comment, NetList[I].lpComment);

		memset(&NewPanelItem[CurItemPos], 0, sizeof(PluginPanelItem));
		GetLocalName(NetList[I].lpRemoteName, LocalName);
		LPTSTR* CustomColumnData = (LPTSTR*)malloc(sizeof(LPTSTR) * 2);
		CustomColumnData[0] = _wcsdup(LocalName);
		CustomColumnData[1] = _wcsdup(Comment);
		NewPanelItem[CurItemPos].CustomColumnData = CustomColumnData;
		NewPanelItem[CurItemPos].CustomColumnNumber = 2;
		NewPanelItem[CurItemPos].FileName = _wcsdup(RemoteName);
		NewPanelItem[CurItemPos].Description = _wcsdup(Comment);
		DWORD attr = FILE_ATTRIBUTE_DIRECTORY;

		if (NetList[I].dwType == RESOURCETYPE_PRINT)
			attr = FILE_ATTRIBUTE_VIRTUAL;

		if (Opt.HiddenSharesAsHidden && RemoteName[lstrlen(RemoteName) - 1] == L'$')
			attr |= FILE_ATTRIBUTE_HIDDEN;

		NewPanelItem[CurItemPos].FileAttributes = attr;
		CurItemPos++;
	}

	*pItemsNumber = CurItemPos;
	ReenterGetFindData--;
	return (TRUE);
}

void NetBrowser::FreeFindData(PluginPanelItem* PanelItem, size_t ItemsNumber)
{
	for (size_t I = 0; I < ItemsNumber; I++)
	{
		free(const_cast<wchar_t*>(PanelItem[I].CustomColumnData[0]));
		free(const_cast<wchar_t*>(PanelItem[I].CustomColumnData[1]));
		free(const_cast<wchar_t**>(PanelItem[I].CustomColumnData));
		free(const_cast<wchar_t*>(PanelItem[I].FileName));
		free(const_cast<wchar_t*>(PanelItem[I].Description));
	}

	free(PanelItem);
}


int NetBrowser::ProcessEvent(intptr_t Event, void* /*Param*/)
{
	if (Event == FE_CLOSE)
	{
		{
			PanelInfo PInfo = {sizeof(PanelInfo)};
			PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);
			wchar_t Mode[2] = {static_cast<wchar_t>(PInfo.ViewMode + 0x30), 0};
			PluginSettings settings(MainGuid, PsInfo.SettingsControl);
			settings.Set(0, StrPanelMode, Mode);
		}

		if (!PCurResource || IsMSNetResource(*PCurResource))
		{
			NetResourceList::CopyNetResource(CommonCurResource, CurResource);
			PCommonCurResource = PCurResource? &CommonCurResource : nullptr;
			SavedCommonRootResources = false;
		}
		else
		{
			*CommonRootResources = RootResources;
			SavedCommonRootResources = true;
		}
	}

	return FALSE;
}


int NetBrowser::DeleteFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, OPERATION_MODES /*OpMode*/)
{
	if (ItemsNumber == 0)
		return (TRUE);
	if (CheckFavoriteItem(PCurResource))
	{
		//Deleting from favorites
		RemoveItems();
	}
	else
	{
		// unmap disks if exists
		for (size_t I = 0; I < ItemsNumber; I++)
			if (PanelItem[I].CustomColumnNumber == 2 && PanelItem[I].CustomColumnData)
			{
				if (*PanelItem[I].CustomColumnData[0])
					if (!CancelConnection(PanelItem[I].FileName))
						break;
			}
	}

	return (TRUE);
}

BOOL NetBrowser::CancelConnection(const wchar_t* RemoteName)
{
	wchar_t LocalName[MAX_PATH];
	wchar_t szFullName[MAX_PATH];
	szFullName[0] = 0;

	if (Opt.FullPathShares)
		lstrcpy(szFullName, RemoteName);
	else if (PCurResource && PCurResource->lpRemoteName)
		FSF.sprintf(szFullName, L"%s\\%s", PCurResource->lpRemoteName, RemoteName);
	else
		return FALSE;

	if (!GetDriveToDisconnect(szFullName, LocalName))
		return FALSE;

	int UpdateProfile = 0;

	if (!ConfirmCancelConnection(LocalName, szFullName, UpdateProfile))
		return FALSE;

	DWORD status = WNetCancelConnection2(LocalName, UpdateProfile,FALSE);

	// if we're on the drive we're disconnecting, set the directory to
	// a different drive and try again
	if (status != NO_ERROR && HandsOffDisconnectDrive(LocalName))
		status = WNetCancelConnection2(LocalName, UpdateProfile,FALSE);

	if (status != NO_ERROR)
	{
		int Failed = FALSE;
		wchar_t MsgText[200];
		FSF.sprintf(MsgText, GetMsg(MNetCannotDisconnect), LocalName);
		int LastError = GetLastError();

		if (LastError == ERROR_OPEN_FILES || LastError == ERROR_DEVICE_IN_USE)
		{
			const wchar_t* MsgItems[] = {
				GetMsg(MError), MsgText, L"\x1", GetMsg(MOpenFiles), GetMsg(MAskDisconnect), GetMsg(MOk),
				GetMsg(MCancel)
			};

			if (PsInfo.Message(
				&MainGuid,
				nullptr,
				FMSG_WARNING | FMSG_ERRORTYPE,
				{},
				MsgItems,
				ARRAYSIZE(MsgItems),
				2) == 0)

				// всегда рвать соединение
				if (WNetCancelConnection2(LocalName, UpdateProfile,TRUE) != NO_ERROR)
					Failed = TRUE;
		}
		else
			Failed = TRUE;

		if (Failed)
		{
			const wchar_t* MsgItems[] = {GetMsg(MError), MsgText, GetMsg(MOk)};
			PsInfo.Message(&MainGuid, nullptr, FMSG_WARNING | FMSG_ERRORTYPE, {}, MsgItems,ARRAYSIZE(MsgItems), 1);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL NetBrowser::GetDriveToDisconnect(const wchar_t* RemoteName, wchar_t* LocalName)
{
	wchar_t LocalNames[MAX_PATH][10];
	DWORD LocalNameCount = 0;
	DWORD i;

	for (i = 0; i < ConnectedList.Count(); i++)
	{
		NETRESOURCE& connRes = ConnectedList[i];

		if (connRes.lpRemoteName && connRes.lpLocalName &&
			*connRes.lpLocalName && lstrcmpi(connRes.lpRemoteName, RemoteName) == 0)
		{
			if (connRes.dwScope == RESOURCE_CONNECTED ||
				connRes.dwScope == RESOURCE_REMEMBERED)
			{
				lstrcpy(LocalNames[LocalNameCount++], connRes.lpLocalName);

				if (LocalNameCount == 10)
					break;
			}
		}
	}

	if (!LocalNameCount)
		return FALSE; // hmmm... strange

	if (LocalNameCount == 1)
		lstrcpy(LocalName, LocalNames[0]);
	else
	{
		wchar_t MsgText[512];
		FSF.sprintf(MsgText, GetMsg(MMultipleDisconnect), RemoteName);

		for (i = 0; i < LocalNameCount; i++)
		{
			lstrcat(MsgText, LocalNames[i]);
			lstrcat(MsgText, L"\n");
		}

		int index = (int)PsInfo.Message(
			&MainGuid,
			nullptr,
			FMSG_ALLINONE,
			{},
			(const wchar_t**)MsgText,
			3 + LocalNameCount,
			LocalNameCount);

		if (index < 0)
			return FALSE;

		lstrcpy(LocalName, LocalNames[index]);
	}

	return TRUE;
}

BOOL NetBrowser::ConfirmCancelConnection(wchar_t* LocalName, wchar_t* RemoteName, int& UpdateProfile)
{
	wchar_t MsgText[MAX_PATH];
	BOOL IsPersistent = TRUE;
	// Check if this was a permanent connection or not.
	{
		HKEY hKey{};
		FSF.sprintf(MsgText, L"Network\\%c", FSF.LUpper(LocalName[0]));

		if (RegOpenKeyEx(HKEY_CURRENT_USER, MsgText, 0,KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		{
			IsPersistent = FALSE;
		}

		if (hKey)
			RegCloseKey(hKey);
	}
	FSF.sprintf(MsgText, GetMsg(MConfirmDisconnectQuestion), LocalName);
	/*
	wchar_t tmp[MAX_PATH];
	DialogItems[3].Data = tmp;
	{
	  size_t rc = lstrlen(DialogItems[0].Data);
	  if(Len1 < rc) Len1 = rc;
	  rc = lstrlen(DialogItems[5].Data);
	  if(Len1 < rc) Len1 = rc;
	}
	lstrcpy((wchar_t*)DialogItems[3].Data, FSF.TruncPathStr(RemoteName, (int)Len1));
	*/
	PluginDialogBuilder Builder(PsInfo, MainGuid, DisconnectDialogGuid, MConfirmDisconnectTitle, L"DisconnectDrive");
	Builder.AddText(MsgText);
	Builder.AddText(RemoteName);
	Builder.AddSeparator();
	Builder.AddCheckbox(MConfirmDisconnectReconnect, &Opt.DisconnectMode)->Flags |= IsPersistent? 0 : DIF_DISABLE;
	Builder.AddOKCancel(MYes, MCancel);

	if (!NeedConfirmCancelConnection() || Builder.ShowDialog())
	{
		UpdateProfile = Opt.DisconnectMode? 0 : CONNECT_UPDATE_PROFILE;

		if (IsPersistent)
		{
			PluginSettings settings(MainGuid, PsInfo.SettingsControl);
			settings.Set(0, StrDisconnectMode, Opt.DisconnectMode);
		}

		return TRUE;
	}

	return FALSE;
}


BOOL NetBrowser::NeedConfirmCancelConnection()
{
	return GetSetting(FSSF_CONFIRMATIONS, L"RemoveConnection")? true : false;
}


BOOL NetBrowser::HandsOffDisconnectDrive(const wchar_t* LocalName)
{
	wchar_t DirBuf[MAX_PATH];
	GetCurrentDirectory(ARRAYSIZE(DirBuf) - 1, DirBuf);

	if (FSF.LUpper(DirBuf[0]) != FSF.LUpper(LocalName[0]))
		return FALSE;

	// change to the root of the drive where network.dll resides
	if (!GetModuleFileName({}, DirBuf, ARRAYSIZE(DirBuf) - 1))
		return FALSE;

	DirBuf[3] = L'\0'; // truncate to "X:\\"
	return SetCurrentDirectory(DirBuf);
}

void NetBrowser::GetOpenPanelInfo(OpenPanelInfo* Info)
{
#ifdef NETWORK_LOGGING__

	if (PCurResource)
		LogData(L"Entering NetBrowser::GetOpenPluginInfo. Info->Flags will contain OPIF_ADDDOTS");
	else
		LogData(L"Entering NetBrowser::GetOpenPluginInfo. Info->Flags will NOT contain OPIF_ADDDOTS");

#endif
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_RAWSELECTION | OPIF_SHOWPRESERVECASE | OPIF_SHORTCUT;
	Info->HostFile = {};

	if (!PCurResource)
	{
		Info->CurDir = L"";

		if (!Opt.RootDoublePoint)
			Info->Flags &= ~OPIF_ADDDOTS;
	}
	else
	{
		static wchar_t CurDir[MAX_PATH];

		if (!PCurResource->lpRemoteName)
		{
			if (CheckFavoriteItem(PCurResource))
				lstrcpy(CurDir, GetMsg(MFavorites));
			else
				lstrcpy(CurDir, PCurResource->lpProvider);
		}
		else
		{
			lstrcpy(CurDir, PCurResource->lpRemoteName);
		}

		Info->CurDir = CurDir;
	}

	Info->Format = const_cast<wchar_t*>(GetMsg(MNetwork));
	static wchar_t Title[MAX_PATH];
	FSF.sprintf(Title, L" %s: %s ", GetMsg(MNetwork), Info->CurDir);
	Info->PanelTitle = Title;
	Info->InfoLines = {};
	Info->InfoLinesNumber = 0;
	Info->DescrFiles = {};
	Info->DescrFilesNumber = 0;
	static PanelMode PanelModesArray[10];
	static const wchar_t* ColumnTitles[3];
	ColumnTitles[0] = GetMsg(MColumnName);
	ColumnTitles[1] = GetMsg(MColumnDisk);
	ColumnTitles[2] = GetMsg(MColumnComment);
	PanelModesArray[3].ColumnTypes = L"N,C0,C1";
	PanelModesArray[3].ColumnWidths = L"0,2,0";
	PanelModesArray[3].ColumnTitles = ColumnTitles;
	PanelModesArray[4].ColumnTypes = L"N,C0";
	PanelModesArray[4].ColumnWidths = L"0,2";
	PanelModesArray[4].ColumnTitles = ColumnTitles;
	PanelModesArray[5].ColumnTypes = L"N,C0,C1";
	PanelModesArray[5].ColumnWidths = L"0,2,0";
	PanelModesArray[5].ColumnTitles = ColumnTitles;
	Info->PanelModesArray = PanelModesArray;
	Info->PanelModesNumber = ARRAYSIZE(PanelModesArray);
	Info->StartPanelMode = m_PanelMode[0];

	if (PCurResource && PCurResource->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
	{
		Info->Flags |= OPIF_REALNAMES;
		static WORD FKeys[] =
		{
			VK_F3, 0, 0,
			VK_F4, 0, MF4,
			VK_F5, 0, MF5,
			VK_F6, 0, MF6,
			VK_F7, 0, 0,
			VK_F8, 0, MF8,
			VK_F5,LEFT_CTRL_PRESSED, 0,
			VK_F6,LEFT_CTRL_PRESSED, 0,
			VK_F3,LEFT_ALT_PRESSED, 0,
			VK_F4,LEFT_ALT_PRESSED, 0,
			VK_F5,LEFT_ALT_PRESSED, 0,
			VK_F1,SHIFT_PRESSED, 0,
			VK_F2,SHIFT_PRESSED, 0,
			VK_F3,SHIFT_PRESSED, 0,
			VK_F4,SHIFT_PRESSED, 0,
			VK_F5,SHIFT_PRESSED, MSHIFTF5,
			VK_F6,SHIFT_PRESSED, MSHIFTF6,
			VK_F7,SHIFT_PRESSED, 0,
			VK_F8,SHIFT_PRESSED, 0,
		};
		static KeyBarLabel kbl[ARRAYSIZE(FKeys) / 3];
		static KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

		for (size_t j = 0, i = 0; i < ARRAYSIZE(FKeys); i += 3, ++j)
		{
			kbl[j].Key.VirtualKeyCode = FKeys[i];
			kbl[j].Key.ControlKeyState = FKeys[i + 1];

			if (FKeys[i + 2])
			{
				kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i + 2]);
			}
			else
			{
				kbl[j].Text = kbl[j].LongText = L"";
			}
		}

		Info->KeyBar = &kbt;
	}
	else
	{
		static WORD FKeys[] =
		{
			VK_F3, 0, 0,
			VK_F4, 0, (WORD)-1,
			VK_F5, 0, 0,
			VK_F6, 0, 0,
			VK_F7, 0, 0,
			VK_F8, 0, (WORD)-1,
			VK_F5,LEFT_CTRL_PRESSED, 0,
			VK_F6,LEFT_CTRL_PRESSED, 0,
			VK_F3,LEFT_ALT_PRESSED, 0,
			VK_F4,LEFT_ALT_PRESSED, 0,
			VK_F5,LEFT_ALT_PRESSED, 0,
			VK_F6,LEFT_ALT_PRESSED, 0,
			VK_F1,SHIFT_PRESSED, 0,
			VK_F2,SHIFT_PRESSED, 0,
			VK_F3,SHIFT_PRESSED, 0,
			VK_F4,SHIFT_PRESSED, 0,
			VK_F5,SHIFT_PRESSED, 0,
			VK_F6,SHIFT_PRESSED, 0,
			VK_F7,SHIFT_PRESSED, 0,
			VK_F8,SHIFT_PRESSED, 0,
		};
		static KeyBarLabel kbl[ARRAYSIZE(FKeys) / 3];
		static KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

		for (size_t j = 0, i = 0; i < ARRAYSIZE(FKeys); i += 3, ++j)
		{
			kbl[j].Key.VirtualKeyCode = FKeys[i];
			kbl[j].Key.ControlKeyState = FKeys[i + 1];

			switch (FKeys[i + 2])
			{
			case 0:
				kbl[j].Text = kbl[j].LongText = L"";
				break;

			case static_cast<WORD>(-1):
				switch (FKeys[i])
				{
				case VK_F4:
					kbl[j].Text = kbl[j].LongText = (PCurResource && PCurResource->dwDisplayType ==
						                                RESOURCEDISPLAYTYPE_DOMAIN)?
						                                GetMsg(MF4) :
						                                L"";
					break;

				case VK_F8:
					kbl[j].Text = kbl[j].LongText = CheckFavoriteItem(PCurResource)? GetMsg(MF8Fav) : L"";
					break;

				default:
					break;
				}
				break;

			default:
				kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i + 2]);
				break;
			}
		}
		Info->KeyBar = &kbt;
	}
}


int NetBrowser::SetDirectory(const wchar_t* Dir, OPERATION_MODES OpMode)
{
	if (OpMode & OPM_FIND)
		return TRUE;

	ChangeDirSuccess = TRUE;

	if (OpenFromFilePanel)
		PCurResource = {};

	BOOL TmpOpenFromFilePanel = OpenFromFilePanel;
	OpenFromFilePanel = FALSE;

	if (!Dir || lstrcmp(Dir, L"\\") == 0)
	{
		PCurResource = {};
		RootResources.Clear();
		return (TRUE);
	}

	if (lstrcmp(Dir, L"..") == 0)
	{
		if (!PCurResource)
			return FALSE;

		if (IsMSNetResource(*PCurResource))
		{
			NETRESOURCE nrParent{};

			if (!GetResourceParent(*PCurResource, &nrParent))
				PCurResource = {};
			else
			{
				CurResource = nrParent;
				PCurResource = &CurResource;
			}
		}
		else
		{
			RootResources.Pop();
			PCurResource = RootResources.Top();
		}

		return TRUE;
	}
	else
	{
		ChangeDirSuccess = TRUE;

		if (ChangeToDirectory(Dir, OpMode, false))
			return ChangeDirSuccess;

		if (GetLastError() == ERROR_CANCELLED)
			return FALSE;

		wchar_t AnsiDir[MAX_PATH];
		lstrcpy(AnsiDir, Dir);

		if (AnsiDir[0] == L'/')
			AnsiDir[0] = L'\\';

		if (AnsiDir[1] == L'/')
			AnsiDir[1] = L'\\';

		// if still haven't found and the name starts with \\, try to jump to a
		// computer in a different domain
		if (wcsncmp(AnsiDir, L"\\\\", 2) == 0)
		{
			if (!TmpOpenFromFilePanel && wcschr(AnsiDir + 2, L'\\'))
			{
				if (!IsReadable(AnsiDir))
				{
#ifdef NETWORK_LOGGING
					wchar_t szErrBuff[MAX_PATH*2];
					_snwprintf(szErrBuff, ARRAYSIZE(szErrBuff), L"GetLastError = %d at line %d, file %S", GetLastError(), __LINE__, __FILE__);
					LogData(szErrBuff);
#endif
					PsInfo.Message(
						&MainGuid,
						nullptr,
						FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
						{},
						reinterpret_cast<const wchar_t* const*>(GetMsg(MError)),
						0,
						0);
					return FALSE;
				}

				PsInfo.PanelControl(this, FCTL_CLOSEPANEL, 0, const_cast<wchar_t*>(Dir));
				return TRUE;
			}

			ChangeDirSuccess = GotoComputer(AnsiDir);
			return ChangeDirSuccess;
		}
	}

	return (FALSE);
}


BOOL NetBrowser::ChangeToDirectory(const wchar_t* Dir, OPERATION_MODES opmodes, bool IsExplicit)
{
	bool IsFind = (opmodes & OPM_FIND) != 0;
	bool IsPgDn = (opmodes & OPM_PGDN) != 0;

	// if we already have the resource list for the current directory,
	// do not scan it again
	if (!PCurResource || !PCurResource->lpRemoteName ||
		lstrcmp(PCurResource->lpRemoteName, NetListRemoteName) != 0)
		EnumerateNetList();

	wchar_t AnsiDir[MAX_PATH];
	lstrcpy(AnsiDir, Dir);

	if (AnsiDir[0] == L'/')
		AnsiDir[0] = L'\\';

	if (AnsiDir[1] == L'/')
		AnsiDir[1] = L'\\';

	for (unsigned I = 0; I < NetList.Count(); I++)
	{
		wchar_t RemoteName[MAX_PATH];
		GetRemoteName(&NetList[I], RemoteName);

		if (FSF.LStricmp(AnsiDir, RemoteName) == 0)
		{
			if (CheckFavoriteItem(&NetList[I]))
			{
				NetResourceList::CopyNetResource(CurResource, NetList[I]);
				PCurResource = &CurResource;
				//RootResources.Push (CurResource);
				return TRUE;
			}

			if ((NetList[I].dwUsage & RESOURCEUSAGE_CONTAINER) == 0 &&
				(NetList[I].dwType & RESOURCETYPE_DISK) &&
				NetList[I].lpRemoteName)
			{
				if (IsFind)
					return (FALSE);

				wchar_t NewDir[MAX_PATH], LocalName[MAX_PATH];
				GetLocalName(NetList[I].lpRemoteName, LocalName);

				if (IsPgDn && *LocalName)
					if (IsReadable(LocalName))
						lstrcpy(NewDir, LocalName);
					else
					{
						PsInfo.Message(
							&MainGuid,
							nullptr,
							FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
							{},
							reinterpret_cast<const wchar_t* const*>(GetMsg(MError)),
							0,
							0);
						return TRUE;
					}
				else
				{
					BOOL ConnectError = FALSE;
					lstrcpy(NewDir, NetList[I].lpRemoteName);

					if (IsExplicit)
					{
						if (!AddConnectionExplicit(&NetList[I]) || !IsReadable(NewDir))
							ConnectError = TRUE;
					}
					else
					{
						if (!IsReadable(NewDir))
							if (!AddConnection(&NetList[I]) || !IsReadable(NewDir))
								ConnectError = TRUE;
					}

					if (ConnectError)
					{
						DWORD res = GetLastError();

						if (!IsExplicit)
							if (res == ERROR_INVALID_PASSWORD || res == ERROR_LOGON_FAILURE || res ==
								ERROR_ACCESS_DENIED || res == ERROR_INVALID_HANDLE)
								ConnectError = !((AddConnectionFromFavorites(&NetList[I]) ||
									AddConnectionExplicit(&NetList[I])) && IsReadable(NewDir));

						if (ConnectError)
						{
							ChangeDirSuccess = FALSE;

							if (GetLastError() != ERROR_CANCELLED)
								PsInfo.Message(
									&MainGuid,
									nullptr,
									FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
									{},
									reinterpret_cast<const wchar_t*const*>(GetMsg(MError)),
									0,
									0);

							return TRUE;
						}
					}
				}

				PsInfo.PanelControl(this, FCTL_CLOSEPANEL, 0, NewDir);
				return (TRUE);
			}

			if (IsExplicit? !AddConnectionExplicit(&NetList[I]) : !IsResourceReadable(NetList[I]))
			{
				int res = GetLastError();

				if (res == ERROR_INVALID_PASSWORD || res == ERROR_LOGON_FAILURE || res == ERROR_ACCESS_DENIED || res ==
					ERROR_LOGON_TYPE_NOT_GRANTED)
					ChangeDirSuccess = IsExplicit?
						                   FALSE :
						                   (AddConnectionFromFavorites(&NetList[I]) || AddConnectionExplicit(
							                   &NetList[I]));
				else
					ChangeDirSuccess = FALSE;

				if (!ChangeDirSuccess)
				{
					if (GetLastError() != ERROR_CANCELLED)
						PsInfo.Message(
							&MainGuid,
							nullptr,
							FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
							{},
							reinterpret_cast<const wchar_t*const*>(GetMsg(MError)),
							0,
							0);

					return FALSE;
				}
			}

			NetResourceList::CopyNetResource(CurResource, NetList[I]);
			PCurResource = &CurResource;

			if (!IsMSNetResource(CurResource))
			{
#ifdef NETWORK_LOGGING
				LogData(L"Resource is not MSN");
				LogNetResource(CurResource);
#endif
				RootResources.Push(CurResource);
			}

#ifdef NETWORK_LOGGING
			else
			{
				LogData(L"Resource is MSN");
				LogNetResource(CurResource);
			}

#endif
			return (TRUE);
		}
	}

	return FALSE;
}


BOOL NetBrowser::IsMSNetResource(const NETRESOURCE& Res)
{
	if (!Res.lpProvider)
		return TRUE;

	return wcsstr(Res.lpProvider, L"Microsoft") ||
		CheckFavoriteItem(const_cast<LPNETRESOURCE>(&Res));
}


BOOL NetBrowser::IsResourceReadable(NETRESOURCE& Res)
{
	if (CheckFavoriteItem(&Res))
		return TRUE;

	if (!Opt.ScanNetwork && (Res.dwDisplayType == RESOURCEDISPLAYTYPE_DOMAIN || Res.dwDisplayType ==
		RESOURCEDISPLAYTYPE_NETWORK))
		return TRUE;

	HANDLE hEnum = INVALID_HANDLE_VALUE;
	DWORD result = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, &Res, &hEnum);

	if (result != NO_ERROR)
	{
		if (!AddConnection(&Res))
			return FALSE;

		result = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, &Res, &hEnum);

		if (result != NO_ERROR)
			return FALSE;
	}

	if (hEnum != INVALID_HANDLE_VALUE)
		WNetCloseEnum(hEnum);

	return TRUE;
}

/*DELETING
BOOL NetBrowser::GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent)
{
  if(!FNetDfsGetInfo)
    return FALSE;
  //we should allocate memory for Wide chars
  int nSize = MultiByteToWideChar(CP_ACP, 0, SrcRes.lpRemoteName, -1, {}, 0);
  if(!nSize)
    return FALSE;
  WCHAR *szRes = new WCHAR[nSize++];

  if(!szRes)
    return FALSE;

    int Res = FALSE;
    if(MultiByteToWideChar(CP_ACP, 0, SrcRes.lpRemoteName, -1, szRes, nSize*sizeof(WCHAR)))
    {
    LPDFS_INFO_3 lpData;
    if(ERROR_SUCCESS == FNetDfsGetInfo(szRes, {}, {}, 3, (LPBYTE *) &lpData))
    {
      DWORD dwBuffSize = 32*sizeof(NETRESOURCE);
      NETRESOURCE *resResult = (NETRESOURCE *)malloc(dwBuffSize);
      CHAR *pszSys{};
      for(DWORD i = 0; i < lpData->NumberOfStorages; i++)
      {
        nSize = WideCharToMultiByte(CP_ACP, 0, lpData->Storage[i].ServerName,
          -1, {}, 0, {}, {});
        if(!nSize)
          break;
        nSize += 3;
        CHAR *szServ =new CHAR[nSize];
        szServ[0] = L'\\';
        szServ[1] = L'\\';

        WideCharToMultiByte(CP_ACP, 0, lpData->Storage[i].ServerName,
          -1, &szServ[2], nSize - 2, {}, {});
        NETRESOURCE inRes = {0};
        inRes.dwScope = RESOURCE_CONNECTED;
        inRes.dwType = RESOURCETYPE_ANY;
        inRes.lpRemoteName = szServ;
        DWORD dwRes;
        while((dwRes = WNetGetResourceInformation(&inRes, resResult,
          &dwBuffSize, &pszSys)) == ERROR_MORE_DATA)
        {
          resResult = (NETRESOURCE *)realloc(resResult, dwBuffSize);
        }
        if(dwRes == ERROR_SUCCESS)
        {
          if(IsResourceReadable(*resResult))
          {
            NetResourceList::CopyNetResource(Parent, *resResult);
            Res = TRUE;
          }
        }

        delete(szServ);
        if(Res)
          break;
      }
      free(resResult);
    }
  }

  delete (szRes);
  return Res;
}
*/


BOOL NetBrowser::GetResourceInfo(wchar_t* SrcName, LPNETRESOURCE DstNetResource)
{
#ifdef NETWORK_LOGGING

	if (LogFile)
		fwprintf(LogFile, L"GetResourceInfo %s\n", SrcName);

#endif
	NETRESOURCE nrOut[32]; // provide buffer space
	NETRESOURCE* lpnrOut = &nrOut[0];
	DWORD cbBuffer = sizeof(nrOut);
	LPTSTR pszSystem{}; // pointer to variable-length strings
	NETRESOURCE nr{};
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	nr.dwScope = RESOURCE_GLOBALNET;
	nr.dwType = RESOURCETYPE_ANY;
	nr.dwUsage = RESOURCEUSAGE_ALL;
	nr.lpRemoteName = SrcName;
	DWORD dwError = WNetGetResourceInformation(&nr, lpnrOut, &cbBuffer, &pszSystem);

	// If the call fails because the buffer is too small,
	//   call the LocalAlloc function to allocate a larger buffer.
	if (dwError == ERROR_MORE_DATA)
	{
		lpnrOut = static_cast<NETRESOURCE*>(LocalAlloc(LMEM_FIXED, cbBuffer));
		if (lpnrOut)
			dwError = WNetGetResourceInformation(&nr, lpnrOut, &cbBuffer, &pszSystem);
	}

	if (dwError == NO_ERROR)
	{
		if (DstNetResource)
			NetResourceList::CopyNetResource(*DstNetResource, *lpnrOut);

#ifdef NETWORK_LOGGING

		if (LogFile)
			fwprintf(LogFile, L"Result:\n");

		LogNetResource(*DstNetResource);
#endif

		if (lpnrOut != &nrOut[0])
			LocalFree(lpnrOut);

		return TRUE;
	}

#ifdef NETWORK_LOGGING
	else
	{
		if (LogFile)
			fwprintf(LogFile, L"error %u\n", GetLastError());
	}

#endif
	return FALSE;
}


BOOL NetBrowser::GetResourceParent(NETRESOURCE& SrcRes, LPNETRESOURCE DstNetResource)
{
	if (CheckFavoriteItem(&SrcRes) ||
		Opt.FavoritesFlags & FAVORITES_UPBROWSE_TO_FAVORITES)
	{
		if (GetFavoritesParent(SrcRes, DstNetResource))
			return TRUE;
	}
	if (!Opt.ScanNetwork)
		return FALSE;

#ifdef NETWORK_LOGGING

	if (LogFile)
		fwprintf(LogFile, L"GetResourceParent( for:\n");

	LogNetResource(SrcRes);
#endif
	TSaveScreen ss;
	BOOL Ret = FALSE;
	NETRESOURCE nrOut[32]; // provide buffer space
	NETRESOURCE* lpnrOut = &nrOut[0];
	DWORD cbBuffer = sizeof(nrOut);
	LPTSTR pszSystem{}; // pointer to variable-length strings
	NETRESOURCE nrSrc = SrcRes;
	nrSrc.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	nrSrc.dwScope = RESOURCE_GLOBALNET;
	nrSrc.dwUsage = 0;
	nrSrc.dwType = RESOURCETYPE_ANY;
	DWORD dwError = WNetGetResourceInformation(&nrSrc, lpnrOut, &cbBuffer, &pszSystem);

	// If the call fails because the buffer is too small,
	//   call the LocalAlloc function to allocate a larger buffer.
	if (dwError == ERROR_MORE_DATA)
	{
		lpnrOut = static_cast<LPNETRESOURCE>(LocalAlloc(LMEM_FIXED, cbBuffer));
		if (lpnrOut)
			dwError = WNetGetResourceInformation(&nrSrc, lpnrOut, &cbBuffer, &pszSystem);
	}

	if (dwError == NO_ERROR)
	{
#ifdef NETWORK_LOGGING

		if (LogFile)
			fwprintf(LogFile, L"WNetGetResourceInformation() returned:\n");

		LogNetResource(*lpnrOut);
#endif
		nrSrc.lpProvider = lpnrOut->lpProvider;

		if (WNetGetResourceParent(&nrSrc, lpnrOut, &cbBuffer) == NO_ERROR)
		{
			if (DstNetResource)
				NetResourceList::CopyNetResource(*DstNetResource, *lpnrOut);

#ifdef NETWORK_LOGGING

			if (LogFile)
				fwprintf(LogFile, L"Result:\n");

			LogNetResource(*DstNetResource);
#endif
			Ret = TRUE;
		}

		if (lpnrOut != &nrOut[0])
			LocalFree(lpnrOut);
	}

	return Ret;
}

BOOL NetBrowser::EditFavorites()
{
	if (!PCurResource)
		return TRUE;

	// First we should determine the type of Favorite Item under cursor
	string Path;

	PanelInfo PInfo = {sizeof(PanelInfo)};
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

	size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELDIRECTORY, 0, nullptr);
	FarPanelDirectory* dir = static_cast<FarPanelDirectory*>(malloc(Size));
	dir->StructSize = sizeof(FarPanelDirectory);
	PsInfo.PanelControl(this, FCTL_GETPANELDIRECTORY, Size, dir);
	Path = dir->Name;
	free(dir);
	Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
	PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

	if (PPI)
	{
		FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
		PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);
		Path += PPI->FileName;
		free(PPI);
	}

	NETRESOURCE nr = {0};

	if (GetFavoriteResource(Path, &nr))
	{
		Path = nr.lpRemoteName;
		std::wstring message{L"Info\n"};

		switch (nr.dwDisplayType)
		{
		case RESOURCEDISPLAYTYPE_DOMAIN:
			message.append(L"This is a domain");
			break;
		case RESOURCEDISPLAYTYPE_SERVER:
			message.append(L"This is a SERVER");
			break;
		default:
			message.append(Path.CPtr());
		}
		PsInfo.Message(
			&MainGuid,
			nullptr,
			FMSG_ALLINONE,
			L"Data",
			reinterpret_cast<const wchar_t* const *>(message.c_str()),
			0,
			1);

		NetResourceList::DeleteNetResource(nr);
		return TRUE;
	}

	return FALSE;
}


int NetBrowser::ProcessKey(const INPUT_RECORD* Rec)
{
	if ((Rec->EventType & (~0x8000)) != KEY_EVENT || !Rec->Event.KeyEvent.bKeyDown)
		return FALSE;

	int Key = Rec->Event.KeyEvent.wVirtualKeyCode;
	int Shift = Rec->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED;
	int Ctrl = Rec->Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED);
	int Alt = Rec->Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED);

	if (!Ctrl && !Alt && (Key == VK_F5 || Key == VK_F6))
	{
		if (PCurResource && PCurResource->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
		{
			PanelInfo PInfo = {sizeof(PanelInfo)};
			PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

			for (size_t I = 0; I < PInfo.SelectedItemsNumber; ++I)
			{
				const wchar_t* pRemoteName{};
				size_t Size = PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, I, {});
				PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

				if (PPI)
				{
					FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
					PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, I, &gpi);
					pRemoteName = PPI->FileName;
					if (!Opt.FullPathShares)
					{
						for (unsigned J = 0; J < NetList.Count(); ++J)
						{
							wchar_t RemoteName[MAX_PATH];
							GetRemoteName(&NetList[J], RemoteName);

							if (FSF.LStricmp(PPI->FileName, RemoteName) == 0)
							{
								pRemoteName = NetList[J].lpRemoteName;
								break;
							}
						}
					}
				}

				if (!PPI || !MapNetworkDrive(pRemoteName, (Key == VK_F6), (Shift == 0)))
				{
					free(PPI);
					break;
				}

				free(PPI);
			}

			PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 0, {});
			PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
		}

		return (TRUE);
	}
	else if (Key == L'F' && Ctrl)
	{
		FileNames2Clipboard(TRUE);
		return TRUE;
	}
	else if (Key == VK_INSERT && Alt && Shift)
	{
		FileNames2Clipboard(FALSE);
		return TRUE;
	}
	else if (Key == VK_INSERT && Alt && Ctrl)
	{
		FileNames2Clipboard(FALSE);
		return TRUE;
	}
	else if (Key == VK_F4 && !Alt && !Ctrl && !Shift)
	{
		PanelInfo PInfo = {sizeof(PanelInfo)};
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);
		size_t Size = PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, 0, {});
		PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
			PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, 0, &gpi);
		}

		if (PPI && lstrcmp(PPI->FileName, L".."))
			if (ChangeToDirectory(PPI->FileName, OPM_NONE, true))
				if (FSF.PointToName(PPI->FileName) -
					PPI->FileName <= 2)
				{
					PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 1, {});
					PanelRedrawInfo ri = {sizeof(PanelRedrawInfo)};
					ri.CurrentItem = ri.TopPanelItem = 0;
					PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, &ri);
				}

		free(PPI);
		return TRUE;
	}
	else if (Key == VK_F4 && Shift)
	{
		EditFavorites();
		return TRUE;
	}
		// disable processing of F3 - avoid unnecessary slowdown
	else if ((Key == VK_F3 || Key == VK_CLEAR) && !Ctrl && !Shift)
	{
		return TRUE;
	}
	else if ((Key == VK_PRIOR || Key == 0xDC) && Ctrl && !PCurResource && !Opt.RootDoublePoint)
	{
		return TRUE;
	}

	return FALSE;
}


BOOL NetBrowser::MapNetworkDrive(const wchar_t* RemoteName, BOOL AskDrive, BOOL Permanent)
{
	wchar_t AnsiRemoteName[MAX_PATH];
	lstrcpy(AnsiRemoteName, RemoteName);
	DWORD DriveMask = GetLogicalDrives();
	wchar_t NewLocalName[10];
	*NewLocalName = 0;

	if (!AskDrive)
		GetFreeLetter(DriveMask, NewLocalName);
	else
	{
		if (!AskMapDrive(NewLocalName, Permanent))
			return FALSE;
	}

	if (*NewLocalName)
	{
		NETRESOURCE newnr;
		// wchar_t LocalName[10];
		newnr.dwType = RESOURCETYPE_DISK;
		newnr.lpLocalName = NewLocalName;
		newnr.lpRemoteName = AnsiRemoteName;
		newnr.lpProvider = {};

		for (;;)
		{
			if (IsReadable(AnsiRemoteName))
			{
				if (AddConnection(&newnr, Permanent))
					break;
			}
			else if ((AddConnectionFromFavorites(&newnr, Permanent) || AddConnectionExplicit(&newnr, Permanent)) &&
				IsReadable(newnr.lpLocalName))
				break;
			else if (ERROR_CANCELLED == GetLastError())
				break;

			if (GetLastError() == ERROR_DEVICE_ALREADY_REMEMBERED)
			{
				if (!AskDrive)
				{
					GetFreeLetter(DriveMask, NewLocalName);

					if (*NewLocalName == 0)
					{
						const wchar_t* MsgItems[] = {GetMsg(MError), GetMsg(MNoFreeLetters), GetMsg(MOk)};
						PsInfo.Message(
							&MainGuid,
							nullptr,
							FMSG_WARNING | FMSG_ERRORTYPE,
							{},
							MsgItems,
							ARRAYSIZE(MsgItems),
							1);
						return FALSE;
					}
				}
				else
				{
					const wchar_t* MsgItems[] = {GetMsg(MError), GetMsg(MAlreadyRemembered), GetMsg(MOk)};
					PsInfo.Message(
						&MainGuid,
						nullptr,
						FMSG_WARNING | FMSG_ERRORTYPE,
						{},
						MsgItems,
						ARRAYSIZE(MsgItems),
						1);
					return FALSE;
				}
			}
			else
			{
				wchar_t MsgText[300];
				FSF.sprintf(MsgText, GetMsg(MNetCannotConnect), RemoteName, NewLocalName);
				const wchar_t* MsgItems[] = {GetMsg(MError), MsgText, GetMsg(MOk)};
				PsInfo.Message(&MainGuid, nullptr, FMSG_WARNING | FMSG_ERRORTYPE, {}, MsgItems,ARRAYSIZE(MsgItems), 1);
				return FALSE;
			}
		}
	}
	else
	{
		const wchar_t* MsgItems[] = {GetMsg(MError), GetMsg(MNoFreeLetters), GetMsg(MOk)};
		PsInfo.Message(&MainGuid, nullptr, FMSG_WARNING | FMSG_ERRORTYPE, {}, MsgItems,ARRAYSIZE(MsgItems), 1);
		return FALSE;
	}

	return TRUE;
}


BOOL NetBrowser::AskMapDrive(wchar_t* NewLocalName, BOOL& Permanent)
{
	int ExitCode = 0;

	for (;;)
	{
		FarMenuItem MenuItems['Z' - 'A' + 1];
		int MenuItemsNumber = 0;
		memset(MenuItems, 0, sizeof(MenuItems));
		wchar_t umname[ARRAYSIZE(MenuItems)][4];
		memset(umname, 0, sizeof(umname));

		for (size_t n = 0; n < ARRAYSIZE(MenuItems); n++)
			MenuItems[n].Text = umname[n];

		DWORD DriveMask = GetLogicalDrives();

		for (int I = 0; I <= 'Z' - 'A'; I++)
			if ((DriveMask & (1 << I)) == 0)
				FSF.sprintf(const_cast<wchar_t*>(MenuItems[MenuItemsNumber++].Text), L"&%c:", L'A' + I);

		MenuItems[ExitCode].Flags = MIF_SELECTED;

		if (!MenuItemsNumber)
			return FALSE;

		const wchar_t* MenuTitle,* MenuBottom;

		if (Permanent)
		{
			MenuTitle = GetMsg(MPermanentTo);
			MenuBottom = GetMsg(MToggleTemporary);
		}
		else
		{
			MenuTitle = GetMsg(MTemporaryTo);
			MenuBottom = GetMsg(MTogglePermanent);
		}

		FarKey BreakKeys[] = {{VK_F6, 0}, {0, 0}};
		intptr_t BreakCode;
		ExitCode = (int)PsInfo.Menu(
			&MainGuid,
			nullptr,
			-1,
			-1,
			0,
			0,
			MenuTitle,
			MenuBottom,
			StrHelpNetBrowse,
			BreakKeys,
			&BreakCode,
			MenuItems,
			MenuItemsNumber);

		if (ExitCode < 0)
			return FALSE;

		if (BreakCode == -1)
		{
			lstrcpy(NewLocalName, MenuItems[ExitCode].Text + 1);
			break;
		}

		Permanent = !Permanent;
	}

	return TRUE;
}


void NetBrowser::GetFreeLetter(DWORD& DriveMask, wchar_t* DiskName)
{
	*DiskName = 0;

	for (wchar_t I = 2; I <= 'Z' - 'A'; I++)
		if ((DriveMask & (1 << I)) == 0)
		{
			DriveMask |= 1 << I;
			DiskName[0] = L'A' + I;
			DiskName[1] = L':';
			DiskName[2] = 0;
			break;
		}
}


int NetBrowser::AddConnection(NETRESOURCE* nr, int Remember)
{
	NETRESOURCE connectnr = *nr;
	DWORD lastErrDebug = WNetAddConnection2(&connectnr, {}, {}, (Remember? CONNECT_UPDATE_PROFILE : 0));

	if (lastErrDebug == NO_ERROR)
	{
		lastErrDebug = GetLastError();
		return (TRUE);
	}

	return (FALSE);
}

int NetBrowser::AddConnectionExplicit(NETRESOURCE* nr, int Remember)
{
	wchar_t Name[256], Password[256];
	NETRESOURCE connectnr = *nr;
	/*static*/
	BOOL bSelected = FALSE;
	NameAndPassInfo passInfo = {connectnr.lpRemoteName, Name, Password, &bSelected};

	if (!GetNameAndPassword(&passInfo))
	{
		SetLastError(ERROR_CANCELLED);
		return (FALSE);
	}

	if (AddConnectionWithLogon(&connectnr, Name, Password, Remember))
	{
		if (bSelected)
		{
			FAVORITEITEM Item{};
			Item.lpRemoteName = connectnr.lpRemoteName;
			Item.lpUserName = Name;
			Item.lpPassword = Password;
			WriteFavoriteItem(&Item);
		}

		return TRUE;
	}

	return FALSE;
}

int NetBrowser::AddConnectionWithLogon(NETRESOURCE* nr, wchar_t* Name, wchar_t* Password, int Remember)
{
	for (;;)
	{
		if (NO_ERROR == WNetAddConnection2(nr, Password, *Name? Name : nullptr, (Remember? CONNECT_UPDATE_PROFILE : 0)))
		{
			return TRUE;
		}
		else
		{
			if (ERROR_SESSION_CREDENTIAL_CONFLICT == GetLastError())
			{
				//Trying to cancel existing connections
				DisconnectFromServer(nr);

				if (NO_ERROR == WNetAddConnection2(
					nr,
					Password,
					*Name? Name : nullptr,
					(Remember? CONNECT_UPDATE_PROFILE : 0)))
				{
					return TRUE;
				}
			}
		}

		if (ERROR_SUCCESS != GetLastError() && Name? (!wcsstr(Name, L"\\") && !wcsstr(Name, L"@")) : FALSE)
		{
			//If the specified user name does not look like "ComputerName\UserName" nor "User@Domain"
			//and the plug-in failed to log on to the remote machine, the specified user name can be
			//interpreted as user name of the remote computer, so let's transform it to look like
			//"Computer\User"
			wchar_t szServer[MAX_PATH];
			wchar_t szNameCopy[MAX_PATH];
			//make copy of Name
			lstrcpy(szNameCopy, Name);
			wchar_t* p = nr->lpRemoteName;
			int n = (int)(FSF.PointToName(p) - p);

			if (n <= 2)
				lstrcpyn(szServer, p + n, ARRAYSIZE(szServer));
			else
			{
				while (*++p == L'\\')
					n--;

				if (n > MAX_PATH)
					n = MAX_PATH;

				lstrcpyn(szServer, p, n - 1);
			}

			FSF.sprintf(Name, L"%s\\%s", szServer, szNameCopy);
			//Try again to log on with the transformed user name.
			continue;
		}

		return FALSE;
	}
}

int NetBrowser::AddConnectionFromFavorites(NETRESOURCE* nr, int Remember)
{
	//Try to search login info in registry
	if (nr)
	{
		wchar_t Name[MAX_PATH];
		wchar_t Pass[MAX_PATH];
		Name[0] = Pass[0] = 0;
		FAVORITEITEM Item =
		{
			nr->lpRemoteName,
			Name,
			Pass
		};

		if (ReadFavoriteItem(&Item))
		{
			return AddConnectionWithLogon(nr, Name, Pass, Remember);
		}
	}

	return FALSE;
}

void NetBrowser::DisconnectFromServer(NETRESOURCE* nr)
{
	//First we should know a name of the server
	int n = (int)(FSF.PointToName(nr->lpRemoteName) - nr->lpRemoteName);

	if (n <= 2)
		n = lstrlen(nr->lpRemoteName) + 1;

	wchar_t* szServer = (wchar_t*)malloc((n + 1) * sizeof(wchar_t));

	if (szServer)
	{
		wchar_t* szBuff = (wchar_t*)malloc((n + 1) * sizeof(wchar_t));

		if (szBuff)
		{
			lstrcpyn(szServer, nr->lpRemoteName, n);
			NETRESOURCE* lpBuff{};
			HANDLE hEnum;

			if (NO_ERROR == WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, {}, &hEnum))
			{
				DWORD cCount = (DWORD)-1;
				DWORD nBuffSize = 0;

				//Let's determine buffer's size we need to store all the connections
				if (ERROR_MORE_DATA == WNetEnumResource(hEnum, &cCount, {}, &nBuffSize))
				{
					lpBuff = (NETRESOURCE*)malloc(nBuffSize);

					if (lpBuff)
					{
						cCount = (DWORD)-1;

						if (NO_ERROR != WNetEnumResource(hEnum, &cCount, lpBuff, &nBuffSize))
						{
							free(lpBuff);
							lpBuff = {};
						}
					}
				}

				WNetCloseEnum(hEnum);

				if (lpBuff)
				{
					for (DWORD i = 0; i < cCount; i++)
					{
						lstrcpyn(szBuff, lpBuff[i].lpRemoteName, n);

						if (0 == lstrcmpi(szServer, szBuff))
							WNetCancelConnection2(lpBuff[i].lpRemoteName, 0, TRUE);
					}

					free(lpBuff);
				}
			}

			free(szBuff);
		}

		//Trying harder to disconnect from the server
		WNetCancelConnection2(szServer, 0, TRUE);
		free(szServer);
	}

	//Let's check the current dir and if it's remote try to change it to %TMP%
	wchar_t lpszPath[MAX_PATH];

	if (GetCurrentDirectory(MAX_PATH, lpszPath))
	{
		if (lpszPath[0] == L'\\')
		{
			ExpandEnvironmentStrings(L"%TMP%", lpszPath, MAX_PATH);
			SetCurrentDirectory(lpszPath);
		}
	}
}


void NetBrowser::GetLocalName(wchar_t* RemoteName, wchar_t* LocalName)
{
	*LocalName = 0;

	if (RemoteName && *RemoteName)
		for (int I = ConnectedList.Count() - 1; I >= 0; I--)
			if (ConnectedList[I].lpRemoteName && ConnectedList[I].lpLocalName &&
				*ConnectedList[I].lpLocalName &&
				lstrcmpi(ConnectedList[I].lpRemoteName, RemoteName) == 0)
			{
				if (ConnectedList[I].dwScope == RESOURCE_CONNECTED ||
					ConnectedList[I].dwScope == RESOURCE_REMEMBERED)
					lstrcpy(LocalName, ConnectedList[I].lpLocalName);

				break;
			}
}


int NetBrowser::GetNameAndPassword(NameAndPassInfo* passInfo)
{
	static wchar_t LastName[256], LastPassword[256];
	PluginDialogBuilder Builder(
		PsInfo,
		MainGuid,
		UserPassDialogGuid,
		passInfo->Title? passInfo->Title : L"",
		StrHelpNetBrowse);
	Builder.AddText(MNetUserName);
	Builder.AddEditField(LastName, ARRAYSIZE(LastName), 60, L"NetworkUser", true);
	Builder.AddText(MNetUserPassword);
	Builder.AddPasswordField(LastPassword, ARRAYSIZE(LastPassword), 60);
	Builder.AddSeparator();
	int disabled = 0;

	if (passInfo->pRemember)
		Builder.AddCheckbox(MRememberPass, passInfo->pRemember);
	else
		Builder.AddCheckbox(MRememberPass, &disabled)->Flags |= DIF_DISABLE;

	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog())
	{
		lstrcpy(passInfo->Name, LastName);
		lstrcpy(passInfo->Password, LastPassword);
		return TRUE;
	}

	return FALSE;
}


void NetBrowser::FileNames2Clipboard(BOOL ToCommandLine)
{
	PanelInfo PInfo = {sizeof(PanelInfo)};
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

	wchar_t* CopyData = nullptr;
	long DataSize = 0;

	if (ToCommandLine)
	{
		if (PInfo.ItemsNumber > 0)
		{
			wchar_t CurFile[MAX_PATH];
			size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
			PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

			if (PPI)
			{
				FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
				PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);
				lstrcpy(CurFile, PPI->FileName);
				free(PPI);
			}

			if (!lstrcmp(CurFile, L".."))
			{
				if (!PCurResource)
					lstrcpy(CurFile, L".\\");
				else
					lstrcpy(CurFile, PCurResource->lpRemoteName);
			}

			FSF.QuoteSpaceOnly(CurFile);

			lstrcat(CurFile, L" ");
			PsInfo.PanelControl(this, FCTL_INSERTCMDLINE, 0, CurFile);
		}

		return;
	}

	for (size_t I = 0; I < PInfo.SelectedItemsNumber; ++I)
	{
		if (DataSize > 0)
		{
			lstrcat(CopyData + DataSize, L"\r\n");
			DataSize += 2;
		}

		size_t Size = PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, I, {});
		PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
			PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, I, &gpi);

			wchar_t CurFile[MAX_PATH];
			lstrcpy(CurFile, PPI->FileName);
			if (!lstrcmp(CurFile, L".."))
			{
				if (!PCurResource)
					lstrcpy(CurFile, L".\\");
				else
					lstrcpy(CurFile, PCurResource->lpRemoteName);
			}
			FSF.QuoteSpaceOnly(CurFile);
			int Length = lstrlen(CurFile);

			wchar_t* NewPtr = (wchar_t*)realloc(CopyData, (DataSize + Length + 3) * sizeof(wchar_t));
			if (!NewPtr)
			{
				if (CopyData)
				{
					free(CopyData);
					CopyData = nullptr;
				}
			}
			else
			{
				CopyData = NewPtr;
				CopyData[DataSize] = 0;
				lstrcpy(CopyData + DataSize, CurFile);
				DataSize += Length;
			}
			free(PPI);

			if (!CopyData)
				break;
		}
	}

	if (CopyData)
	{
		FSF.CopyToClipboard(FCT_STREAM, CopyData);
		free(CopyData);
	}
}


void NetBrowser::ManualConnect()
{
	PanelInfo PInfo = {sizeof(PanelInfo)};
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

	if (PInfo.ItemsNumber)
	{
		size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
		PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
			PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);
			ChangeToDirectory(PPI->FileName, OPM_NONE, true);
			free(PPI);
		}
	}
}


void NetBrowser::GetRemoteName(NETRESOURCE* NetRes, wchar_t* RemoteName)
{
	if (CheckFavoriteItem(NetRes))
	{
		if (!NetRes->lpRemoteName)
		{
			lstrcpy(RemoteName, GetMsg(MFavorites));
		}
		else
		{
			free(NetRes->lpComment);
			NetRes->lpComment = NetResourceList::CopyText(GetMsg(MFavoritesFolder));
			lstrcpy(RemoteName, FSF.PointToName(NetRes->lpRemoteName));
		}
	}
	else if (NetRes->lpProvider && (!NetRes->lpRemoteName ||
		NetRes->dwDisplayType == RESOURCEDISPLAYTYPE_NETWORK))
		lstrcpy(RemoteName, NetRes->lpProvider);
	else if (!NetRes->lpRemoteName)
		*RemoteName = 0;
	else if (Opt.FullPathShares)
		lstrcpy(RemoteName, NetRes->lpRemoteName);
	else
		lstrcpy(RemoteName, FSF.PointToName(NetRes->lpRemoteName));
}


BOOL NetBrowser::IsReadable(const wchar_t* Remote)
{
	wchar_t Mask[MAX_PATH];

	if (*Remote == L'\\' && *(Remote + 1) == L'\\')
		FSF.sprintf(Mask, L"\\\\?\\UNC%s\\*", Remote + 1);
	else
		FSF.sprintf(Mask, L"%s\\*", Remote);

	HANDLE FindHandle;
	WIN32_FIND_DATA FindData;
	FindHandle = FindFirstFile(Mask, &FindData);
	DWORD err = GetLastError();
	FindClose(FindHandle);
	SetLastError(err);

	if (err == ERROR_FILE_NOT_FOUND)
	{
		SetLastError(0);
		return TRUE;
	}

	return (FindHandle != INVALID_HANDLE_VALUE);
}

void NetBrowser::SetOpenFromCommandLine(wchar_t* ShareName)
{
	//lstrcpy (CmdLinePath, ShareName);
#ifdef NETWORK_LOGGING
	LogData(L"SetOpenFromCommandLine ShareName is");
	LogData(ShareName);
#endif
	lstrcpy(CmdLinePath, ShareName);
	/*if(!GotoFavorite(ShareName))
	  GotoComputer(ShareName);*/
}

BOOL NetBrowser::SetOpenFromFilePanel(wchar_t* ShareName)
{
	NETRESOURCE nr{};

	if (!GetResourceInfo(ShareName, &nr))
		return FALSE;

	if (!IsMSNetResource(nr))
		return FALSE;

	OpenFromFilePanel = TRUE;
	return TRUE;
}

int NetBrowser::GotoComputer(const wchar_t* Dir)
{
#ifdef NETWORK_LOGGING
	LogData(L"Entering GotoComputer");
#endif
	// if there are backslashes in the name, truncate them
	wchar_t ComputerName[MAX_PATH];
	lstrcpy(ComputerName, Dir);
	BOOL IsShare = FALSE;
	wchar_t* p = wcschr(ComputerName + 2, L'\\'); // skip past leading backslashes

	if (p)
	{
		IsShare = TRUE;
		*p = L'\0';
	}
	else
	{
		p = wcschr(ComputerName + 2, L'/');

		if (p)
		{
			IsShare = TRUE;
			*p = L'\0';
		}
	}

	CharUpper(ComputerName);
	NETRESOURCE res{};

	if (!GetResourceInfo(ComputerName, &res))
		return FALSE;

	/*
	if (!IsMSNetResource (res))
	  return FALSE;
	*/
	if (!IsResourceReadable(res))
	{
		int err = GetLastError();

		if (err == ERROR_INVALID_PASSWORD || err == ERROR_LOGON_FAILURE || err == ERROR_ACCESS_DENIED || err ==
			ERROR_INVALID_HANDLE || err == ERROR_LOGON_TYPE_NOT_GRANTED)
			if (!((AddConnectionFromFavorites(&res) || AddConnectionExplicit(&res)) && IsResourceReadable(res)))
			{
				if (GetLastError() != ERROR_CANCELLED)
					PsInfo.Message(
						&MainGuid,
						nullptr,
						FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
						{},
						reinterpret_cast<const wchar_t*const*>(GetMsg(MError)),
						0,
						0);

				return FALSE;
			}
	}

	CurResource = res;
	PCurResource = &CurResource;
	/*int result = */
	PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 0, {});

	if (IsShare)
	{
		wchar_t ShareName[MAX_PATH];
		lstrcpy(ShareName, Dir);

		// replace forward slashes with backslashes
		for (p = ShareName; *p; p++)
			if (*p == L'/')
				*p = L'\\';

		SetCursorToShare(ShareName);
	}
	else
		PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});

	return TRUE;
}

void NetBrowser::GotoLocalNetwork()
{
	TSaveScreen SS;
	wchar_t ComputerName[MAX_PATH];
	lstrcpy(ComputerName, L"\\\\");
	DWORD ComputerNameLength = MAX_PATH - 3;

	if (!GetComputerName(ComputerName + 2, &ComputerNameLength))
		return;

	NETRESOURCE res{};

	if (!GetResourceInfo(ComputerName, &res) || !IsMSNetResource(res))
		return;

	NETRESOURCE parent{};

	if (!GetResourceParent(res, &parent))
		return;

	NetResourceList::CopyNetResource(CurResource, parent);
	PCurResource = &CurResource;
}

void NetBrowser::SetCursorToShare(wchar_t* Share)
{
	PanelInfo PInfo = {sizeof(PanelInfo)};
	// this returns the items in sorted order, so we can position correctly
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

	if (PInfo.ItemsNumber)
	{
		// prevent recursion
		for (size_t i = 0; i < PInfo.ItemsNumber; i++)
		{
			wchar_t szAnsiName[MAX_PATH];
			size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, i, {});
			PluginPanelItem* PPI = (PluginPanelItem*)malloc(Size);

			if (PPI)
			{
				FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
				PsInfo.PanelControl(this, FCTL_GETPANELITEM, i, &gpi);
				lstrcpy(szAnsiName, PPI->FileName);
				free(PPI);
			}

			if (!FSF.LStricmp(szAnsiName, Opt.FullPathShares? Share : FSF.PointToName(Share)))
			{
				PanelRedrawInfo info = {sizeof(PanelRedrawInfo)};
				info.CurrentItem = i;
				info.TopPanelItem = 0;
				PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, &info);
				break;
			}
		}
	}
}

void NetBrowser::RemoveItems()
{
	if (!CheckFavoriteItem(PCurResource))
		return;

	// We are in Favorites folder, so we can remove items from this folder
	PanelInfo PInfo = {sizeof(PanelInfo)};
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

	wchar_t szConfirmation[MAX_PATH * 2];

	if (PInfo.SelectedItemsNumber == 1)
	{
		size_t Size = PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, 0, nullptr);
		auto* PPI = (PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
			PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, 0, &gpi);
			FSF.sprintf(szConfirmation, GetMsg(MRemoveFavItem), PPI->FileName);
			free(PPI);
		}
	}
	else // PInfo.SelectedItemsNumber > 1
		FSF.sprintf(szConfirmation, GetMsg(MRemoveFavItems), PInfo.SelectedItemsNumber);

	const wchar_t* Msg[4];
	Msg[0] = GetMsg(MRemoveFavCaption);
	Msg[1] = szConfirmation;
	Msg[2] = GetMsg(MOk);
	Msg[3] = GetMsg(MCancel);

	if (0 != PsInfo.Message(&MainGuid, nullptr, FMSG_WARNING, L"RemoveItemFav", Msg, std::size(Msg), 2))
	{
		return; // User canceled deletion
	}

	for (size_t i = 0; i < PInfo.SelectedItemsNumber; i++)
	{
		size_t Size = PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, i, {});
		auto* PPI = (PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, PPI};
			PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, i, &gpi);
			RemoveFromFavorites(PPI->FileName);
			free(PPI);
		}
	}
}

void NetBrowser::GetHiddenShares()
{
	wchar_t lpwsNetPath[MAX_PATH];
	PSHARE_INFO_1 BufPtr, p;
	NET_API_STATUS res;

	if (!PCurResource)
		return;

	LPTSTR lpszServer = PCurResource->lpRemoteName;
	wchar_t szResPath[MAX_PATH];
	LPTSTR pszSystem;
	NETRESOURCE pri;
	NETRESOURCE nr[256];
	DWORD er = 0, tr = 0, resume = 0, rrsiz;
	lstrcpyn(lpwsNetPath, lpszServer,ARRAYSIZE(lpwsNetPath));

	do
	{
		res = NetShareEnum((LPWSTR)lpwsNetPath, 1, (LPBYTE*)&BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);

		if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
		{
			p = BufPtr;

			for (DWORD J = 0; J < er; J++)
			{
				memset((void*)&pri, 0, sizeof(pri));
				pri.dwScope = RESOURCE_GLOBALNET;
				pri.dwType = RESOURCETYPE_DISK;
				pri.lpLocalName = {};
				lstrcpy(szResPath, lpszServer);
				lstrcat(szResPath, L"\\");
				{
					size_t pos = lstrlen(szResPath);
					lstrcpyn(&szResPath[pos], p->shi1_netname, (int)(ARRAYSIZE(szResPath) - pos));
				}

				if (szResPath[lstrlen(szResPath) - 1] == L'$' &&
					lstrcmp(&szResPath[lstrlen(szResPath) - 4], L"IPC$"))
				{
					pri.lpRemoteName = szResPath;
					pri.dwUsage = RESOURCEUSAGE_CONTAINER;
					pri.lpProvider = {};
					rrsiz = sizeof(nr);
					// we need to provide buffer space for WNetGetResourceInformation
					int rc = WNetGetResourceInformation(&pri, (void*)&nr[0], &rrsiz, &pszSystem);

					if (rc != NO_ERROR)
					{
						p++;
						continue;
						//break; //?????
					}

					switch (p->shi1_type)
					{
					case STYPE_DISKTREE:
					case STYPE_SPECIAL:
						nr[0].dwType = RESOURCETYPE_DISK;
						break;
					case STYPE_PRINTQ:
						nr[0].dwType = RESOURCETYPE_PRINT;
						break;
					}

					NetList.Push(nr[0]);
				}

				p++;
			}

			NetApiBufferFree(BufPtr);
		}

		if (res == ERROR_SUCCESS)
			break;
	}
	while (res == ERROR_MORE_DATA);
}

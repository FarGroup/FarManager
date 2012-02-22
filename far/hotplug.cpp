/*
hotplug.cpp

Отключение Hotplug-устройств
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

#include "hotplug.hpp"
#include "language.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "imports.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "window.hpp"

struct DeviceInfo
{
	DEVINST hDevInst; // device instance
	DWORD dwDriveMask; // mask of associated drives
};

static int  GetHotplugDevicesInfo(DeviceInfo **pInfo);
static void FreeHotplugDevicesInfo(DeviceInfo *pInfo);
static bool GetDeviceProperty(DEVINST hDevInst, DWORD Property, string &strValue, bool bSearchChild);
static int __RemoveHotplugDevice(DEVINST hDevInst);
static int RemoveHotplugDevice(DEVINST hDevInst,DWORD DevMasks,DWORD Flags);
static DeviceInfo *EnumHotPlugDevice(LPARAM lParam);


DeviceInfo *EnumHotPlugDevice(LPARAM lParam)
{
	VMenu *HotPlugList=(VMenu *)lParam;
	DeviceInfo *pInfo=nullptr;
	int nCount = GetHotplugDevicesInfo(&pInfo);

	if (nCount)
	{
		MenuItemEx ListItem;

		for (int I = 0; I < nCount; I++)
		{
			string strFriendlyName;
			string strDescription;
			DEVINST hDevInst=pInfo[I].hDevInst;
			ListItem.Clear();

			if (GetDeviceProperty(hDevInst, SPDRP_DEVICEDESC, strDescription, true))
			{
				if (!strDescription.IsEmpty())
				{
					RemoveExternalSpaces(strDescription);
					ListItem.strName = strDescription;
				}
			}

			if (GetDeviceProperty(hDevInst, SPDRP_FRIENDLYNAME, strFriendlyName, true))
			{
				RemoveExternalSpaces(strFriendlyName);

				if (!strDescription.IsEmpty())
				{
					if (!strFriendlyName.IsEmpty() && StrCmpI(strDescription,strFriendlyName))
					{
						ListItem.strName += L" \"" + strFriendlyName + L"\"";
					}
				}
				else if (!strFriendlyName.IsEmpty())
				{
					ListItem.strName = strFriendlyName;
				}
			}

			if (!ListItem.strName.IsEmpty())
				HotPlugList->SetUserData(&I,sizeof(I),HotPlugList->AddItem(&ListItem));
		}
	}

	return pInfo;
}

static void RefreshHotplugMenu(DeviceInfo*& pInfo,VMenu& HotPlugList)
{
	if (pInfo) FreeHotplugDevicesInfo(pInfo);

	pInfo=nullptr;
	HotPlugList.Hide();
	HotPlugList.DeleteItems();
	HotPlugList.SetPosition(-1,-1,0,0);
	pInfo=EnumHotPlugDevice((LPARAM)&HotPlugList);
	HotPlugList.Show();
}

void ShowHotplugDevice()
{
	Events.DeviceArivalEvent.Reset();
	Events.DeviceRemoveEvent.Reset();

	DeviceInfo *pInfo=nullptr;
	VMenu HotPlugList(MSG(MHotPlugListTitle),nullptr,0,ScrY-4);
	HotPlugList.SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	HotPlugList.SetPosition(-1,-1,0,0);
	pInfo=EnumHotPlugDevice((LPARAM)&HotPlugList);
	HotPlugList.AssignHighlights(TRUE);
	HotPlugList.SetBottomTitle(MSG(MHotPlugListBottom));

	HotPlugList.Show();

	while (!HotPlugList.Done())
	{
		int Key=Events.DeviceArivalEvent.Signaled() || Events.DeviceRemoveEvent.Signaled()?KEY_CTRLR:HotPlugList.ReadInput();
		switch (Key)
		{
			case KEY_F1:
			{
				Help Hlp(L"HotPlugList");
				break;
			}
			case KEY_CTRLR:
			{
				RefreshHotplugMenu(pInfo,HotPlugList);
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				if (HotPlugList.GetItemCount() > 0)
				{
					int bResult;
					int I=*static_cast<int*>(HotPlugList.GetUserData(nullptr,0));

					if ((bResult=RemoveHotplugDevice(pInfo[I].hDevInst,pInfo[I].dwDriveMask,EJECT_NOTIFY_AFTERREMOVE)) == 1)
					{
						HotPlugList.Hide();

						if (pInfo)
							FreeHotplugDevicesInfo(pInfo);

						pInfo=nullptr;
						RefreshHotplugMenu(pInfo,HotPlugList);
					}
					else if (bResult != -1)
					{
						SetLastError(ERROR_DRIVE_LOCKED); // ...ю "The disk is in use or locked by another process."
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
						        MSG(MChangeCouldNotEjectHotPlugMedia2),HotPlugList.GetItemPtr(I)->strName,MSG(MOk));
					}
				}

				break;
			}
			default:
				HotPlugList.ProcessInput();
				break;
		}
	}

	if (pInfo)
		FreeHotplugDevicesInfo(pInfo);
}


int ProcessRemoveHotplugDevice(wchar_t Drive, DWORD Flags)
{
	int bResult = -1;
	DeviceInfo *pInfo;
	DWORD dwDriveMask = (1 << (Upper(Drive)-L'A'));
	DWORD SavedLastError=ERROR_SUCCESS;

	int nCount = GetHotplugDevicesInfo(&pInfo);

	if (nCount)
	{
		for (int i = 0; i < nCount; i++)
		{
			if (pInfo[i].dwDriveMask & dwDriveMask)
			{
				bResult=RemoveHotplugDevice(pInfo[i].hDevInst,pInfo[i].dwDriveMask,Flags);
				// ??? break; ???
			}
		}

		FreeHotplugDevicesInfo(pInfo);
		pInfo=nullptr;
	}

	SetLastError(SavedLastError);
	return bResult;
}



/**+
A device is considered a HotPlug device if the following are TRUE:
- does NOT have problem CM_PROB_DEVICE_NOT_THERE
- does NOT have problem CM_PROB_HELD_FOR_EJECT
- does NOT have problem CM_PROB_DISABLED
- has Capability CM_DEVCAP_REMOVABLE
- does NOT have Capability CM_DEVCAP_SURPRISEREMOVALOK
- does NOT have Capability CM_DEVCAP_DOCKDEVICE

Returns:
TRUE if this is a HotPlug device
FALSE if this is not a HotPlug device.
-**/

bool CheckChild(DEVINST hDevInst)
{
	bool Result=false;
	DEVINST hDevChild;

	if (CM_Get_Child(&hDevChild,hDevInst,0)==CR_SUCCESS)
	{
		wchar_t szDeviceID[MAX_DEVICE_ID_LEN];
		if (CM_Get_Device_ID(hDevChild, szDeviceID, ARRAYSIZE(szDeviceID), 0) == CR_SUCCESS)
		{
			HDEVINFO Info = SetupDiGetClassDevs(&GUID_DEVINTERFACE_VOLUME, szDeviceID, nullptr, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
			if(Info != INVALID_HANDLE_VALUE)
			{
				SP_DEVINFO_DATA DeviceInfoData = {sizeof(DeviceInfoData)};
				if(SetupDiOpenDeviceInfo(Info, szDeviceID, nullptr, 0, &DeviceInfoData))
				{
					DWORD Capabilities = 0;
					if(SetupDiGetDeviceRegistryProperty(Info, &DeviceInfoData, SPDRP_CAPABILITIES, nullptr, reinterpret_cast<PBYTE>(&Capabilities), sizeof(Capabilities), nullptr))
					{
						Result=!(Capabilities&CM_DEVCAP_SURPRISEREMOVALOK)&&(Capabilities&CM_DEVCAP_UNIQUEID);
					}
				}
				SetupDiDestroyDeviceInfoList(Info);
			}
		}
	}

	return Result;
}

bool IsHotPlugDevice(DEVINST hDevInst)
{
	bool Result = false;
	wchar_t szDeviceID[MAX_DEVICE_ID_LEN];
	if (CM_Get_Device_ID(hDevInst, szDeviceID, ARRAYSIZE(szDeviceID), 0) == CR_SUCCESS)
	{
		HDEVINFO Info = SetupDiGetClassDevs(&GUID_DEVINTERFACE_VOLUME, szDeviceID, nullptr, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
		if(Info != INVALID_HANDLE_VALUE)
		{
			SP_DEVINFO_DATA DeviceInfoData = {sizeof(DeviceInfoData)};
			if(SetupDiOpenDeviceInfo(Info, szDeviceID, nullptr, 0, &DeviceInfoData))
			{
				DWORD Capabilities = 0;
				if(SetupDiGetDeviceRegistryProperty(Info, &DeviceInfoData, SPDRP_CAPABILITIES, nullptr, reinterpret_cast<PBYTE>(&Capabilities), sizeof(Capabilities), nullptr))
				{
					DWORD Status = 0, Problem = 0;
					if (CM_Get_DevNode_Status(&Status, &Problem, hDevInst, 0) == CR_SUCCESS)
					{
						if ((Problem != CM_PROB_DEVICE_NOT_THERE) &&
								(Problem != CM_PROB_HELD_FOR_EJECT) && //возможно, надо проверять на наличие проблем вообще
								(Problem != CM_PROB_DISABLED) &&
								(Capabilities & CM_DEVCAP_REMOVABLE) &&
								(!(Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) || CheckChild(hDevInst)) &&
								!(Capabilities & CM_DEVCAP_DOCKDEVICE))
							Result = true;
					}
				}
			}
			SetupDiDestroyDeviceInfoList(Info);
		}
	}
	return Result;
}


DWORD DriveMaskFromVolumeName(const wchar_t *lpwszVolumeName)
{
	string strCurrentVolumeName;
	string MountPoint(L"\\\\?\\A:\\");

	for (wchar_t Letter = L'A'; Letter <= L'Z'; Letter++)
	{
		MountPoint.Replace(4, Letter);
		if(apiGetVolumeNameForVolumeMountPoint(MountPoint,strCurrentVolumeName) && strCurrentVolumeName.IsSubStrAt(0,lpwszVolumeName))
			return (1 << (Letter-L'A'));
	}

	return 0;
}

DWORD GetDriveMaskFromMountPoints(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	wchar_t szDeviceID[MAX_DEVICE_ID_LEN];

	if (CM_Get_Device_ID(hDevInst, szDeviceID, ARRAYSIZE(szDeviceID), 0) == CR_SUCCESS)
	{
		{
			HDEVINFO Info = SetupDiGetClassDevs(&GUID_DEVINTERFACE_VOLUME, szDeviceID, nullptr, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
			if(Info != INVALID_HANDLE_VALUE)
			{
				SP_DEVICE_INTERFACE_DATA sdid = {sizeof(sdid)};
				DWORD MemberIndex = 0;
				while(SetupDiEnumDeviceInterfaces(Info, nullptr, &GUID_DEVINTERFACE_VOLUME, MemberIndex, &sdid))
				{
					MemberIndex++;
					DWORD RequiredSize = 0;
					SetupDiGetDeviceInterfaceDetail(Info, &sdid, nullptr, 0, &RequiredSize, nullptr);
					if(RequiredSize)
					{
						PSP_DEVICE_INTERFACE_DETAIL_DATA DData = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(xf_malloc(RequiredSize));
						if(DData)
						{
							DData->cbSize = sizeof(*DData);
							if(SetupDiGetDeviceInterfaceDetail(Info, &sdid, DData, RequiredSize, nullptr, nullptr))
							{
								string strMountPoint(DData->DevicePath);
								AddEndSlash(strMountPoint);
								string strVolumeName;
								if (apiGetVolumeNameForVolumeMountPoint(strMountPoint,strVolumeName))
								{
										dwMask |= DriveMaskFromVolumeName(strVolumeName);
								}
							}
							xf_free(DData);
						}
					}
				}
				SetupDiDestroyDeviceInfoList(Info);
			}
		}
	}
	return dwMask;
}

DWORD GetRelationDrivesMask(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	DEVINST hRelationDevInst;
	wchar_t szDeviceID [MAX_DEVICE_ID_LEN];

	if (CM_Get_Device_ID(hDevInst, szDeviceID, sizeof(szDeviceID)/sizeof(wchar_t), 0) == CR_SUCCESS)
	{
		DWORD dwSize = 0;

		if (CM_Get_Device_ID_List_Size(&dwSize, szDeviceID, CM_GETIDLIST_FILTER_REMOVALRELATIONS) == CR_SUCCESS)
		{
			if (dwSize)
			{
				wchar_t *lpDeviceIdList = (wchar_t*)xf_malloc(dwSize*sizeof(wchar_t));

				if (CM_Get_Device_ID_List(szDeviceID, lpDeviceIdList, dwSize, CM_GETIDLIST_FILTER_REMOVALRELATIONS) == CR_SUCCESS)
				{
					wchar_t *p = lpDeviceIdList;

					while (*p)
					{
						if (CM_Locate_DevNode(&hRelationDevInst, p, 0) == CR_SUCCESS)
							dwMask |= GetDriveMaskFromMountPoints(hRelationDevInst);

						p += wcslen(p)+1;
					}
				}

				xf_free(lpDeviceIdList);
			}
		}
	}

	return dwMask;
}

DWORD GetDriveMaskForDeviceInternal(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	DEVINST hDevChild;

	do
	{
		if (!IsHotPlugDevice(hDevInst))
		{
			dwMask |= GetDriveMaskFromMountPoints(hDevInst);
			dwMask |= GetRelationDrivesMask(hDevInst);

			if (CM_Get_Child(&hDevChild, hDevInst, 0) == CR_SUCCESS)
				dwMask |= GetDriveMaskForDeviceInternal(hDevChild);
		}
	}
	while (CM_Get_Sibling(&hDevInst, hDevInst, 0) == CR_SUCCESS);

	return dwMask;
}


DWORD GetDriveMaskForDevice(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	DEVINST hDevChild;
	dwMask |= GetDriveMaskFromMountPoints(hDevInst);
	dwMask |= GetRelationDrivesMask(hDevInst);

	if (CM_Get_Child(&hDevChild, hDevInst, 0) == CR_SUCCESS)
		dwMask |= GetDriveMaskForDeviceInternal(hDevChild);

	return dwMask;
}

int GetHotplugDriveDeviceInfoInternal(DEVINST hDevInst, DeviceInfo **pInfo, int nCount)
{
	DEVINST hDevChild;

	do
	{
		if (IsHotPlugDevice(hDevInst))
		{
			nCount++;
			*pInfo = (DeviceInfo*)xf_realloc(*pInfo, nCount*sizeof(DeviceInfo));
			DeviceInfo *pItem = &(*pInfo)[nCount-1];
			pItem->dwDriveMask = GetDriveMaskForDevice(hDevInst);
			pItem->hDevInst = hDevInst;
		}

		if (CM_Get_Child(&hDevChild, hDevInst, 0) == CR_SUCCESS)
			nCount = GetHotplugDriveDeviceInfoInternal(hDevChild, pInfo, nCount);
	}
	while (CM_Get_Sibling(&hDevInst, hDevInst, 0) == CR_SUCCESS);

	return nCount;
}

int GetHotplugDevicesInfo(DeviceInfo **pInfo)
{
	if (pInfo)
	{
		*pInfo = nullptr;

		DEVNODE hDevRoot;

		if (CM_Locate_DevNodeW(&hDevRoot, nullptr, CM_LOCATE_DEVNODE_NORMAL) == CR_SUCCESS)
		{
			DEVINST hDevChild;

			if (CM_Get_Child(&hDevChild, hDevRoot, 0) == CR_SUCCESS)
				return GetHotplugDriveDeviceInfoInternal(hDevChild, pInfo, 0);
		}
	}

	return 0;
}

void FreeHotplugDevicesInfo(DeviceInfo *pInfo)
{
	xf_free(pInfo);
}

bool GetDeviceProperty(DEVINST hDevInst, DWORD Property, string& strValue, bool bSearchChild)
{
	bool Result = false;
	wchar_t szDeviceID[MAX_DEVICE_ID_LEN];
	if (CM_Get_Device_ID(hDevInst, szDeviceID, ARRAYSIZE(szDeviceID), 0) == CR_SUCCESS)
	{
		HDEVINFO Info = SetupDiGetClassDevs(&GUID_DEVINTERFACE_VOLUME, szDeviceID, nullptr, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
		if(Info != INVALID_HANDLE_VALUE)
		{
			SP_DEVINFO_DATA DeviceInfoData = {sizeof(DeviceInfoData)};
			if(SetupDiOpenDeviceInfo(Info, szDeviceID, nullptr, 0, &DeviceInfoData))
			{
				DWORD RequiredSize = 0;
				SetupDiGetDeviceRegistryProperty(Info, &DeviceInfoData, Property, nullptr, nullptr, 0, &RequiredSize);
				if(RequiredSize)
				{
					PBYTE Buffer = reinterpret_cast<PBYTE>(strValue.GetBuffer(RequiredSize));
					if(Buffer)
					{
						if(SetupDiGetDeviceRegistryProperty(Info, &DeviceInfoData, Property, nullptr, Buffer, RequiredSize, nullptr))
						{
							Result = true;
						}
					}
					strValue.ReleaseBuffer();
				}
				else
				{
					if(bSearchChild)
					{
						DEVINST hDevChild;
						if (CM_Get_Child(&hDevChild, hDevInst, 0) == CR_SUCCESS)
						{
							Result = GetDeviceProperty(hDevChild, Property, strValue, bSearchChild);
						}
					}
				}
			}
			SetupDiDestroyDeviceInfoList(Info);
		}
	}
	return Result;
}


int __RemoveHotplugDevice(DEVINST hDevInst)
{
	PNP_VETO_TYPE pvtVeto = PNP_VetoTypeUnknown;
	CONFIGRET crResult;
	wchar_t wszDescription[MAX_PATH]={}; //BUGBUG
	crResult = CM_Request_Device_Eject(hDevInst, &pvtVeto, (wchar_t*)&wszDescription, MAX_PATH, 0);

	if ((crResult != CR_SUCCESS) || (pvtVeto != PNP_VetoTypeUnknown))   //M$ баг, если есть szDecsription, то даже при ошибке возвращается CR_SUCCESS
	{
		SetLastError((pvtVeto != PNP_VetoTypeUnknown)?ERROR_DRIVE_LOCKED:ERROR_UNABLE_TO_UNLOAD_MEDIA); // ...о "The disk is in use or locked by another process."
		return 0;
	}

	SetLastError(ERROR_SUCCESS);
	return 1;
}

int RemoveHotplugDevice(DEVINST hDevInst,DWORD dwDriveMask,DWORD Flags)
{
	int bResult = -1; // сразу выставим -1, иначе, на обычном HDD операция Shift-Del ругается, что мол деайс залочен
	string strFriendlyName;
	string strDescription;
	GetDeviceProperty(hDevInst, SPDRP_FRIENDLYNAME, strFriendlyName, true);
	RemoveExternalSpaces(strFriendlyName);
	GetDeviceProperty(hDevInst, SPDRP_DEVICEDESC, strDescription, true);
	RemoveExternalSpaces(strDescription);
	int DoneEject=0;

	if (!(Flags&EJECT_NO_MESSAGE) && Opt.Confirm.RemoveHotPlug)
	{
		wchar_t Disks[256], *pDisk=Disks;
		*pDisk=0;

		for (int Drive='A'; Drive <= 'Z'; ++Drive)
		{
			if (dwDriveMask & (1 << (Drive-'A')))
			{
				*pDisk++=(wchar_t)Drive;
				*pDisk++=L':';
				*pDisk++=L',';
			}
		}

		*pDisk=0;

		if (pDisk != Disks)
			*--pDisk=0;

		if (StrCmpI(strDescription,strFriendlyName) && !strFriendlyName.IsEmpty())
		{
			if (*Disks)
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strFriendlyName,LangString(MHotPlugDisks) << Disks,MSG(MHRemove),MSG(MHCancel));
			else
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strFriendlyName,MSG(MHRemove),MSG(MHCancel));
		}
		else
		{
			if (*Disks)
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,LangString(MHotPlugDisks) << Disks,MSG(MHRemove),MSG(MHCancel));
			else
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,MSG(MHRemove),MSG(MHCancel));
		}
	}

	if (!DoneEject)
		bResult = __RemoveHotplugDevice(hDevInst);
	else
		bResult = -1;

	if (bResult == 1 && (Flags&EJECT_NOTIFY_AFTERREMOVE))
	{
		Message(0,1,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugNotify1),strDescription,strFriendlyName,MSG(MChangeHotPlugNotify2),MSG(MOk));
	}

	return bResult;
}

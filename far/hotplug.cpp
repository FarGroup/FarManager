/*
hotplug.cpp

ќтключение Hotplug-устройств
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

#if defined(__GNUC__)  || (defined(_MSC_VER) && _MSC_VER <= 1200)

#define __NTDDK_H
#if defined(__GNUC__)
#include <ddk/cfgmgr32.h>
#else
#include <cfgmgr32.h>
#endif
#ifdef __cplusplus
  #define MY_EXTERN_C extern "C"
#else
  #define MY_EXTERN_C extern
#endif
#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  MY_EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define VolumeClassGuid             GUID_DEVINTERFACE_VOLUME
MY_DEFINE_GUID(GUID_DEVINTERFACE_VOLUME, 0x53f5630dL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#define CM_DRP_FRIENDLYNAME                (0x0000000D)
#define CM_DRP_DEVICEDESC                  (0x00000001)
#define CM_DRP_CAPABILITIES                (0x00000010)
#define CM_DEVCAP_REMOVABLE         (0x00000004)
#define CM_DEVCAP_SURPRISEREMOVALOK (0x00000080)
#define CM_DEVCAP_DOCKDEVICE        (0x00000008)

#else
#include <cfgmgr32.h>
#endif

#include <setupapi.h>
#pragma hdrstop

#include "lang.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "BlockExtKey.hpp"

struct DeviceInfo {
  DEVINST hDevInst; // device instance
  DWORD dwDriveMask; // mask of associated drives
};



static int  GetHotplugDevicesInfo (DeviceInfo **pInfo);
static void FreeHotplugDevicesInfo (DeviceInfo *pInfo);
static bool GetDeviceProperty (DEVINST hDevInst, int nProperty, string &strValue, bool bSearchChild);
static int __RemoveHotplugDevice (DEVINST hDevInst);
static int RemoveHotplugDevice(DEVINST hDevInst,DWORD DevMasks,DWORD Flags);
static DeviceInfo *EnumHotPlugDevice(LPARAM lParam);


HMODULE g_hSetupAPI = NULL;

typedef BOOL (__stdcall *GETVOLUMENAMEFORVOLUMEMOUNTPOINT) (
    const wchar_t *lpszVolumeMountPoint,
    wchar_t *lpszVolumeName,
    DWORD cchBufferLength
    );

typedef DWORD (__stdcall *CMGETDEVNODEREGISTRYPROPERTY) (
    DEVINST dnDevInst,
    ULONG ulProperty,
    PULONG pulRegDataType,
    PVOID Buffer,
    PULONG pulLength,
    ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVNODESTATUS) (
    OUT PULONG pulStatus,
    OUT PULONG pulProblemNumber,
    IN DEVINST dnDevInst,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEID) (
    IN DEVINST dnDevInst,
    OUT wchar_t *Buffer,
    IN ULONG BufferLen,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEIDLISTSIZE) (
    OUT PULONG pulLen,
    IN const wchar_t *pszFilter,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEIDLIST) (
    IN const wchar_t *pszFilter,
    OUT wchar_t *Buffer,
    IN ULONG BufferLen,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEINTERFACELISTSIZE) (
    IN PULONG pulLen,
    IN LPGUID InterfaceClassGuid,
    IN DEVINSTID_W pDeviceID,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEINTERFACELIST) (
    IN LPGUID InterfaceClassGuid,
    IN DEVINSTID_W pDeviceID,
    OUT wchar_t *Buffer,
    IN ULONG BufferLen,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMLOCATEDEVNODE) (
    OUT PDEVINST pdnDevInst,
    IN DEVINSTID_W pDeviceID,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETCHILD) (
    OUT PDEVINST pdnDevInst,
    IN DEVINST DevInst,
    IN ULONG ulFlags
    );


typedef CONFIGRET (__stdcall *CMGETSIBLING) (
    OUT PDEVINST pdnDevInst,
    IN DEVINST DevInst,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMREQUESTDEVICEEJECT) (
    IN DEVINST dnDevInst,
    OUT PPNP_VETO_TYPE pVetoType,
    OUT wchar_t *pszVetoName,
    IN ULONG ulNameLength,
    IN ULONG ulFlags
    );


CMGETDEVNODEREGISTRYPROPERTY pfnGetDevNodeRegistryProperty;
CMGETDEVNODESTATUS pfnGetDevNodeStatus;
CMGETDEVICEID pfnGetDeviceID;
CMGETDEVICEIDLISTSIZE pfnGetDeviceIDListSize;
CMGETDEVICEIDLIST pfnGetDeviceIDList;
CMGETDEVICEINTERFACELISTSIZE pfnGetDeviceInterfaceListSize;
CMGETDEVICEINTERFACELIST pfnGetDeviceInterfaceList;
CMLOCATEDEVNODE pfnLocateDevNode;
CMGETCHILD pfnGetChild;
CMGETSIBLING pfnGetSibling;
CMREQUESTDEVICEEJECT pfnRequestDeviceEject;
GETVOLUMENAMEFORVOLUMEMOUNTPOINT pfnGetVolumeNameForVolumeMountPoint;

DeviceInfo *EnumHotPlugDevice(LPARAM lParam)
{
  VMenu *HotPlugList=(VMenu *)lParam;

  DeviceInfo *pInfo=NULL;
  int nCount = GetHotplugDevicesInfo (&pInfo);
  if ( nCount )
  {
    struct MenuItemEx ListItem;
    string strFriendlyName;
    string strDescription;

    for (int I = 0; I < nCount; I++)
    {
      DEVINST hDevInst=pInfo[I].hDevInst;

      GetDeviceProperty (hDevInst,CM_DRP_FRIENDLYNAME,strFriendlyName,true);
      RemoveExternalSpaces(strFriendlyName);
      GetDeviceProperty (hDevInst,CM_DRP_DEVICEDESC,strDescription,true);
      RemoveExternalSpaces(strDescription);

      ListItem.Clear ();

      if ( !strDescription.IsEmpty() )
        ListItem.strName = strDescription;

      if ( !strFriendlyName.IsEmpty() && StrCmpI(strDescription, strFriendlyName) )
      {
        if ( !strDescription.IsEmpty() )
          ListItem.strName += L" \"";

        ListItem.strName += strFriendlyName;

        if ( !strDescription.IsEmpty() )
          ListItem.strName += L"\"";
      }

      if(StrCmpI(strDescription,strFriendlyName) && !strFriendlyName.IsEmpty ())
      {
        //TruncStr(szDescription,sizeof(ListItem.Name)-1);
        ListItem.strName = strDescription + L" \"" + strFriendlyName + L"\"";
      }
      else
      {
        if(!strDescription.IsEmpty ())
          ListItem.strName = strDescription;
      }

      RemoveExternalSpaces(ListItem.strName);
      if(!ListItem.strName.IsEmpty ())
        HotPlugList->SetUserData((void*)(INT_PTR)I,sizeof(I),HotPlugList->AddItem(&ListItem));

    }
  }

  return pInfo;
}

void ShowHotplugDevice ()
{
  if( !g_hSetupAPI )
  {
    SetLastError(ERROR_INVALID_FUNCTION);
    return;
  }

  DeviceInfo *pInfo=NULL;
  int I;

  VMenu HotPlugList(UMSG(MHotPlugListTitle),NULL,0,ScrY-4);
  HotPlugList.SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
  HotPlugList.SetPosition(-1,-1,0,0);

  pInfo=EnumHotPlugDevice((LPARAM)&HotPlugList);

  HotPlugList.AssignHighlights(TRUE);
  HotPlugList.SetBottomTitle(UMSG(MHotPlugListBottom));
  HotPlugList.Show();

  while (!HotPlugList.Done())
  {
    int Key=HotPlugList.ReadInput();
    switch(Key)
    {
      case KEY_F1:
      {
        BlockExtKey blockExtKey;
        {
          Help Hlp (L"HotPlugList");
        }
        break;
      }

      case KEY_CTRLR:
      {

        if(pInfo)
          FreeHotplugDevicesInfo (pInfo);
        pInfo=NULL;

        HotPlugList.Hide();
        HotPlugList.DeleteItems();
        HotPlugList.SetPosition(-1,-1,0,0);
        pInfo=EnumHotPlugDevice((LPARAM)&HotPlugList);
        HotPlugList.Show();

        break;
      }

      case KEY_NUMDEL:
      case KEY_DEL:
      {
        if(HotPlugList.GetItemCount() > 0)
        {
          BlockExtKey blockExtKey;

          I=(int)(INT_PTR)HotPlugList.GetUserData(NULL,0);
          if(RemoveHotplugDevice(pInfo[I].hDevInst,pInfo[I].dwDriveMask,EJECT_NOTIFY_AFTERREMOVE) == 1)
          {
            HotPlugList.Hide();
            if(pInfo)
              FreeHotplugDevicesInfo (pInfo);
            ShowHotplugDevice();
            return;
          }
        }
        break;
      }

      default:
        HotPlugList.ProcessInput();
        break;
    }
  }

  if(pInfo)
    FreeHotplugDevicesInfo (pInfo);
}


int ProcessRemoveHotplugDevice (wchar_t Drive, DWORD Flags)
{
  int bResult = -1;
  DeviceInfo *pInfo;
  DWORD dwDriveMask = (1 << (Upper(Drive)-L'A'));

  DWORD SavedLastError=ERROR_SUCCESS;

  if ( !g_hSetupAPI )
  {
    SetLastError(ERROR_INVALID_FUNCTION);
    return -1; //???
  }

  int nCount = GetHotplugDevicesInfo (&pInfo);

  if ( nCount )
  {
    for (int i = 0; i < nCount; i++)
    {
      if ( pInfo[i].dwDriveMask & dwDriveMask )
      {
        bResult=RemoveHotplugDevice (pInfo[i].hDevInst,pInfo[i].dwDriveMask,Flags);
        // ??? break; ???
      }
    }

    FreeHotplugDevicesInfo (pInfo);
  }

  SetLastError(SavedLastError);

  return bResult;
}

bool CheckInitSetupAPI ()
{
  return g_hSetupAPI != NULL;
}

bool InitializeSetupAPI ()
{
  bool bResult = false;

  if ( !g_hSetupAPI )
  {
    g_hSetupAPI = LoadLibraryW (L"setupapi.dll");

    if ( g_hSetupAPI )
    {
      pfnGetVolumeNameForVolumeMountPoint = (GETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress (
          GetModuleHandleW (L"KERNEL32.DLL"),
          "GetVolumeNameForVolumeMountPointW"
          );

      pfnGetDevNodeRegistryProperty = (CMGETDEVNODEREGISTRYPROPERTY)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_DevNode_Registry_PropertyW"
          );

      pfnGetDevNodeStatus = (CMGETDEVNODESTATUS)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_DevNode_Status"
          );

      pfnGetDeviceID = (CMGETDEVICEID)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_IDW"
          );

      pfnGetDeviceIDListSize = (CMGETDEVICEIDLISTSIZE)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_ID_List_SizeW"
          );

      pfnGetDeviceIDList = (CMGETDEVICEIDLIST)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_ID_ListW"
          );

      pfnGetDeviceInterfaceListSize = (CMGETDEVICEINTERFACELISTSIZE)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_Interface_List_SizeW"
          );

      pfnGetDeviceInterfaceList = (CMGETDEVICEINTERFACELIST)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_Interface_ListW"
          );

      pfnLocateDevNode = (CMLOCATEDEVNODE)GetProcAddress (
          g_hSetupAPI,
          "CM_Locate_DevNodeW"
          );

      pfnGetChild = (CMGETCHILD)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Child"
          );

      pfnGetSibling  = (CMGETCHILD)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Sibling"
          );

      pfnRequestDeviceEject = (CMREQUESTDEVICEEJECT)GetProcAddress (
          g_hSetupAPI,
          "CM_Request_Device_EjectW"
          );

      if ( pfnGetVolumeNameForVolumeMountPoint &&
         pfnGetDevNodeRegistryProperty &&
         pfnGetDevNodeStatus &&
         pfnGetDeviceID &&
         pfnGetDeviceIDListSize &&
         pfnGetDeviceIDList &&
         pfnGetDeviceInterfaceListSize &&
         pfnGetDeviceInterfaceList &&
         pfnLocateDevNode &&
         pfnGetChild &&
         pfnGetSibling &&
         pfnRequestDeviceEject )
        bResult = true;
      else
      {
        FreeLibrary (g_hSetupAPI);
        g_hSetupAPI = NULL;
      }
    }
  }

  return bResult;
}

void FinalizeSetupAPI ()
{
  FreeLibrary (g_hSetupAPI);
  g_hSetupAPI = NULL;
}

/**+
A device is considered a HotPlug device if the following are TRUE:
- Does NOT have problem CM_PROB_DEVICE_NOT_THERE
- has Capability CM_DEVCAP_REMOVABLE
- does NOT have Capability CM_DEVCAP_SURPRISEREMOVALOK
- does NOT have Capability CM_DEVCAP_DOCKDEVICE

Returns:
TRUE if this is a HotPlug device
FALSE if this is not a HotPlug device.
-**/


BOOL IsHotPlugDevice (DEVINST hDevInst)
{
  DWORD Capabilities;
  DWORD Len;
  DWORD Status,Problem;

  Capabilities = 0;
  Status = 0;
  Problem = 0;

  Len = sizeof(Capabilities);

  if ( pfnGetDevNodeRegistryProperty (
      hDevInst,
      CM_DRP_CAPABILITIES,
      NULL,
      (PVOID)&Capabilities,
      &Len,
      0
      ) == CR_SUCCESS )
  {
    if ( pfnGetDevNodeStatus (
        &Status,
        &Problem,
        hDevInst,
        0
        ) == CR_SUCCESS )
    {
      if ( (Problem != CM_PROB_DEVICE_NOT_THERE) &&
           (Problem != CM_PROB_HELD_FOR_EJECT) && //возможно, надо провер€ть на наличие проблем вообще
         (Capabilities & CM_DEVCAP_REMOVABLE) &&
         !(Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) &&
         !(Capabilities & CM_DEVCAP_DOCKDEVICE) )
         return TRUE;
    }
  }

  return FALSE;
}


DWORD DriveMaskFromVolumeName (const wchar_t *lpwszVolumeName)
{
  wchar_t wszCurrentVolumeName[MAX_PATH];
  wchar_t wszMountPoint[4];

  wszMountPoint[1] = L':';
  wszMountPoint[2] = L'\\';
  wszMountPoint[3] = 0;

  for (wchar_t Letter = L'A'; Letter <= L'Z'; Letter++)
  {
    wszMountPoint[0] = Letter;

    pfnGetVolumeNameForVolumeMountPoint (wszMountPoint, wszCurrentVolumeName, MAX_PATH);

    if ( !StrCmpI (wszCurrentVolumeName, lpwszVolumeName) )
      return (1 << (Letter-L'A'));
  }

  return 0;
}

DWORD GetDriveMaskFromMountPoints (DEVINST hDevInst)
{
  DWORD dwMask = 0;
  wchar_t szDeviceID [MAX_DEVICE_ID_LEN];

  if ( pfnGetDeviceID (
      hDevInst,
      szDeviceID,
      sizeof(szDeviceID)/sizeof (wchar_t),
      0
      ) == CR_SUCCESS )
  {
    DWORD dwSize = 0;

    if ( pfnGetDeviceInterfaceListSize (
        &dwSize,
        (LPGUID)&VolumeClassGuid,
        (DEVINSTID_W)&szDeviceID,
        0
        ) == CR_SUCCESS )
    {
      if ( dwSize > 1 )
      {
        wchar_t *lpwszDeviceInterfaceList = (wchar_t*)xf_malloc (dwSize*sizeof (wchar_t));

        if ( pfnGetDeviceInterfaceList (
            (LPGUID)&VolumeClassGuid,
            (DEVINSTID_W)&szDeviceID,
            lpwszDeviceInterfaceList,
            dwSize,
            0
            ) == CR_SUCCESS )
        {
          wchar_t *p = lpwszDeviceInterfaceList;

          while ( *p )
          {
            wchar_t *lpwszMountPoint = (wchar_t*)xf_malloc ((wcslen (p)+1+1)*sizeof (wchar_t)); //for trailing slash

            wcscpy (lpwszMountPoint, p);

            if ( !wcschr (p+4, L'\\') )
              wcscat (lpwszMountPoint, L"\\");

            wchar_t wszVolumeName[MAX_PATH];

            if ( pfnGetVolumeNameForVolumeMountPoint (
                lpwszMountPoint,
                (wchar_t*)&wszVolumeName,
                MAX_PATH
                ) )
              dwMask |= DriveMaskFromVolumeName (wszVolumeName);

            xf_free (lpwszMountPoint);

            p += wcslen (p)+1;
          }
        }

        xf_free (lpwszDeviceInterfaceList);
      }
    }
  }

  return dwMask;
}

DWORD GetRelationDrivesMask (DEVINST hDevInst)
{
  DWORD dwMask = 0;
  DEVINST hRelationDevInst;
  wchar_t szDeviceID [MAX_DEVICE_ID_LEN];

  if ( pfnGetDeviceID (
      hDevInst,
      szDeviceID,
      sizeof (szDeviceID)/sizeof (wchar_t),
      0
      ) == CR_SUCCESS )
  {
    DWORD dwSize = 0;

    if ( pfnGetDeviceIDListSize (
        &dwSize,
        szDeviceID,
        CM_GETIDLIST_FILTER_REMOVALRELATIONS
        ) == CR_SUCCESS )
    {
      if ( dwSize )
      {
        wchar_t *lpDeviceIdList = (wchar_t*)xf_malloc (dwSize*sizeof (wchar_t));

        if ( pfnGetDeviceIDList (
            szDeviceID,
            lpDeviceIdList,
            dwSize,
            CM_GETIDLIST_FILTER_REMOVALRELATIONS
            ) == CR_SUCCESS )
        {
          wchar_t *p = lpDeviceIdList;

          while ( *p )
          {
            if ( pfnLocateDevNode (
                &hRelationDevInst,
                p,
                0
                ) == CR_SUCCESS )
              dwMask = GetDriveMaskFromMountPoints (hRelationDevInst);

            p += wcslen (p)+1;
          }
        }

        xf_free (lpDeviceIdList);
      }
    }
  }

  return dwMask;
}

DWORD GetDriveMaskForDeviceInternal (DEVINST hDevInst)
{
  DWORD dwMask = 0;
  DEVINST hDevChild;

  do {

    if ( !IsHotPlugDevice(hDevInst) )
    {
      dwMask |= GetDriveMaskFromMountPoints (hDevInst);
      dwMask |= GetRelationDrivesMask (hDevInst);

      if ( pfnGetChild (
          &hDevChild,
          hDevInst,
          0
          ) == CR_SUCCESS )
        dwMask |= GetDriveMaskForDeviceInternal (hDevChild);
    }

  } while ( pfnGetSibling (&hDevInst, hDevInst, 0) == CR_SUCCESS );

  return dwMask;
}


DWORD GetDriveMaskForDevice (DEVINST hDevInst)
{
  DWORD dwMask = 0;
  DEVINST hDevChild;

  dwMask |= GetDriveMaskFromMountPoints (hDevInst);
  dwMask |= GetRelationDrivesMask (hDevInst);

  if ( pfnGetChild (
      &hDevChild,
      hDevInst,
      0
      ) == CR_SUCCESS )
    dwMask |= GetDriveMaskForDeviceInternal (hDevChild);

  return dwMask;
}

int GetHotplugDriveDeviceInfoInternal (DEVINST hDevInst, DeviceInfo **pInfo, int nCount)
{
  DEVINST hDevChild;

  do {

    if ( IsHotPlugDevice(hDevInst) )
    {
        nCount++;
        *pInfo = (DeviceInfo*)xf_realloc (*pInfo, nCount*sizeof (DeviceInfo));

        DeviceInfo *pItem = &(*pInfo)[nCount-1];

        pItem->dwDriveMask = GetDriveMaskForDevice (hDevInst);
        pItem->hDevInst = hDevInst;
    }

      if ( pfnGetChild (
          &hDevChild,
          hDevInst,
          0
          ) == CR_SUCCESS )
        nCount = GetHotplugDriveDeviceInfoInternal (hDevChild, pInfo, nCount);

  } while ( pfnGetSibling (&hDevInst, hDevInst, 0) == CR_SUCCESS );

  return nCount;
}

int GetHotplugDevicesInfo (DeviceInfo **pInfo)
{
  if ( pInfo )
  {
    *pInfo = NULL;

    if ( g_hSetupAPI )
    {
      DEVNODE hDevRoot;

      if ( pfnLocateDevNode (
          &hDevRoot,
          NULL,
          CM_LOCATE_DEVNODE_NORMAL
          ) == CR_SUCCESS )
      {
        DEVINST hDevChild;

        if ( pfnGetChild (
            &hDevChild,
            hDevRoot,
            0
            ) == CR_SUCCESS )
          return GetHotplugDriveDeviceInfoInternal (hDevChild, pInfo, 0);
      }
    }
  }

  return 0;
}

void FreeHotplugDevicesInfo (DeviceInfo *pInfo)
{
  xf_free (pInfo);
}

bool GetDeviceProperty (
    DEVINST hDevInst,
    int nProperty,
    string &strValue,
    bool bSearchChild
    )
{
  bool bResult = false;

  if ( g_hSetupAPI )
  {
    DWORD dwSize = 0;
    CONFIGRET crResult;

    crResult = pfnGetDevNodeRegistryProperty (
        hDevInst,
        nProperty,//CM_DRP_FRIENDLYNAME,
        NULL,
        NULL,
        &dwSize,
        0
        );

    if ( dwSize == 0 )
    {
      if ( bSearchChild )
      {
        DEVINST hDevChild;

        if ( pfnGetChild (&hDevChild, hDevInst, 0) == CR_SUCCESS )
          bResult = GetDeviceProperty (hDevChild, nProperty, strValue, bSearchChild);
      }
    }
    else
    {
      wchar_t *lpwszBuffer = strValue.GetBuffer (dwSize+1);

      crResult = pfnGetDevNodeRegistryProperty (
          hDevInst,
          nProperty,//CM_DRP_FRIENDLYNAME,
          NULL,
          lpwszBuffer,
          &dwSize,
          0
          );

      strValue.ReleaseBuffer ();

      bResult = (crResult == ERROR_SUCCESS);
    }
  }

  return bResult;
}


int __RemoveHotplugDevice (DEVINST hDevInst)
{
  PNP_VETO_TYPE pvtVeto = PNP_VetoTypeUnknown;
  CONFIGRET crResult;

  wchar_t wszDescription[MAX_PATH]; //BUGBUG
  memset (wszDescription, 0, MAX_PATH*sizeof (wchar_t));

  crResult = pfnRequestDeviceEject (
      hDevInst,
      &pvtVeto,
      (wchar_t*)&wszDescription,
      MAX_PATH,
      0
      );

  if ( (crResult != CR_SUCCESS) || (pvtVeto != PNP_VetoTypeUnknown) ) //M$ баг, если есть szDecsription, то даже при ошибке возвращаетс€ CR_SUCCESS
  {
    SetLastError((pvtVeto != PNP_VetoTypeUnknown)?ERROR_DRIVE_LOCKED:ERROR_UNABLE_TO_UNLOAD_MEDIA); // ...о "The disk is in use or locked by another process."
    return 0;
  }

  SetLastError(ERROR_SUCCESS);
  return 1;
}

int RemoveHotplugDevice(DEVINST hDevInst,DWORD dwDriveMask,DWORD Flags)
{
  int bResult = -1; // сразу выставим -1, иначе, на обычном HDD операци€ Shift-Del ругаетс€, что мол деайс залочен

  string strFriendlyName;
  string strDescription;

  GetDeviceProperty (hDevInst,CM_DRP_FRIENDLYNAME,strFriendlyName,true);
  RemoveExternalSpaces(strFriendlyName);
  GetDeviceProperty (hDevInst,CM_DRP_DEVICEDESC,strDescription,true);
  RemoveExternalSpaces(strDescription);

  int DoneEject=0;
  if(!(Flags&EJECT_NO_MESSAGE) && Opt.Confirm.RemoveHotPlug)
  {
    string strDiskMsg;
    wchar_t Disks[256], *pDisk=Disks;
    *pDisk=0;

    for(int Drive='A'; Drive <= 'Z'; ++Drive)
    {
      if(dwDriveMask & (1 << (Drive-'A')))
      {
        *pDisk++=(wchar_t)Drive;
        *pDisk++=L':';
        *pDisk++=L',';
      }
    }

    *pDisk=0;
    if(pDisk != Disks)
      *--pDisk=0;

    if(*Disks)
      strDiskMsg.Format(UMSG(MHotPlugDisks),Disks);

    if(StrCmpI(strDescription,strFriendlyName) && !strFriendlyName.IsEmpty ())
    {
      if(!strDiskMsg.IsEmpty ())
        DoneEject=Message(MSG_WARNING,2,UMSG(MChangeHotPlugDisconnectDriveTitle),UMSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strFriendlyName,strDiskMsg,UMSG(MHRemove),UMSG(MHCancel));
      else
        DoneEject=Message(MSG_WARNING,2,UMSG(MChangeHotPlugDisconnectDriveTitle),UMSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strFriendlyName,UMSG(MHRemove),UMSG(MHCancel));
    }
    else
    {
      if(!strDiskMsg.IsEmpty ())
        DoneEject=Message(MSG_WARNING,2,UMSG(MChangeHotPlugDisconnectDriveTitle),UMSG(MChangeHotPlugDisconnectDriveQuestion),strFriendlyName,strDiskMsg,UMSG(MHRemove),UMSG(MHCancel));
      else
        DoneEject=Message(MSG_WARNING,2,UMSG(MChangeHotPlugDisconnectDriveTitle),UMSG(MChangeHotPlugDisconnectDriveQuestion),strFriendlyName,UMSG(MHRemove),UMSG(MHCancel));
    }
  }

  if(!DoneEject)
    bResult = __RemoveHotplugDevice (hDevInst);
  else
    bResult = -1;
  if(bResult == 1 && (Flags&EJECT_NOTIFY_AFTERREMOVE))
  {
    Message(0,1,UMSG(MChangeHotPlugDisconnectDriveTitle),UMSG(MChangeHotPlugNotify1),strDescription,strFriendlyName,UMSG(MChangeHotPlugNotify2),UMSG(MOk));
  }

  return bResult;
}

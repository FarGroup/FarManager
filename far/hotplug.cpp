/*
hotplug.cpp

Отключение Hotplug-устройств

*/

/* Revision: 1.00 22.05.2006 $ */

#pragma once

#include "headers.hpp"

#if defined(__BORLANDC__)
// -----------------------------------------------------------------
#if (__BORLANDC__ <= 0x0520)
// недостающие для BCC 5.02 данные

#ifndef _BASETSD_H_
#include <basetsd.h>
#endif

#ifndef _NTDEF_
//#include <ntdef.h>

//
// Neutral ANSI/UNICODE types and macros
//
#ifdef  UNICODE                     // r_winnt

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPWSTR LPTCH, PTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR PCTSTR, LPCTSTR;
typedef LPUWSTR PUTSTR, LPUTSTR;
typedef LPCUWSTR PCUTSTR, LPCUTSTR;
typedef LPWSTR LP;
#define __TEXT(quote) L##quote      // r_winnt

#else   /* UNICODE */               // r_winnt

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
#define __TEXT(quote) quote         // r_winnt

#endif /* UNICODE */                // r_winnt

#endif /* _NTDEF_ */

const GUID GUID_DEVINTERFACE_VOLUME = { 0x53f5630dL, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } };
#define VolumeClassGuid                 GUID_DEVINTERFACE_VOLUME

#define DIGCF_DEVICEINTERFACE            0x00000010
#define ERROR_INVALID_DRIVE_OBJECT       4321L

#endif  /* __BORLANDC__ <= 0x0520 */
// -----------------------------------------------------------------
#endif  /* __BORLANDC__ */

#include <cfgmgr32.h>
#include <setupapi.h>
#pragma hdrstop

#include "lang.hpp"
#include "fn.hpp"
#include "plugin.hpp"

struct DeviceInfo {
  DEVINST hDevInst; // device instance
  DWORD dwDriveMask; // mask of associated drives
};


static bool InitializeSetupAPI ();
static void FinalizeSetupAPI ();

static int  GetHotplugDevicesInfo (DeviceInfo **pInfo);
static void FreeHotplugDevicesInfo (DeviceInfo *pInfo);
static bool GetDeviceProperty (DEVINST hDevInst, int nProperty, char *lpBuffer, DWORD dwMaxSize, bool bSearchChild);
static int RemoveHotplugDevice (DEVINST hDevInst, char Drive, DWORD Flags);


HMODULE g_hSetupAPI = NULL;

typedef BOOL (__stdcall *GETVOLUMENAMEFORVOLUMEMOUNTPOINT) (
    LPCTSTR lpszVolumeMountPoint,
    LPTSTR lpszVolumeName,
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
    OUT PTCHAR Buffer,
    IN ULONG BufferLen,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEIDLISTSIZE) (
    OUT PULONG pulLen,
    IN PCTSTR pszFilter,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEIDLIST) (
    IN PCTSTR pszFilter,
    OUT PTCHAR Buffer,
    IN ULONG BufferLen,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEINTERFACELISTSIZE) (
    IN PULONG pulLen,
    IN LPGUID InterfaceClassGuid,
    IN DEVINSTID pDeviceID,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMGETDEVICEINTERFACELIST) (
    IN LPGUID InterfaceClassGuid,
    IN DEVINSTID pDeviceID,
    OUT PTCHAR Buffer,
    IN ULONG BufferLen,
    IN ULONG ulFlags
    );

typedef CONFIGRET (__stdcall *CMLOCATEDEVNODE) (
    OUT PDEVINST pdnDevInst,
    IN DEVINSTID pDeviceID,
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
    OUT LPTSTR pszVetoName,
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


int ProcessRemoveHotplugDevice (char Drive, DWORD Flags)
{
  int bResult = 0;
  DeviceInfo *pInfo;
  DWORD dwDriveMask = (1 << (toupper(Drive)-'A'));

  DWORD SavedLastError=ERROR_SUCCESS;

  if(!InitializeSetupAPI())
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
        char szFriendlyName[MAX_PATH];
        char szDescription[MAX_PATH];

        memset (szFriendlyName, 0, sizeof (szFriendlyName));
        memset (szDescription, 0, sizeof (szDescription));

        GetDeviceProperty (pInfo[i].hDevInst,CM_DRP_FRIENDLYNAME,szFriendlyName,MAX_PATH,true);

        GetDeviceProperty (pInfo[i].hDevInst,CM_DRP_DEVICEDESC,szDescription,MAX_PATH,true);

        int DoneEject=0;
        if(!(Flags&EJECT_NO_MESSAGE) && Opt.Confirm.RemoveHotPlug)
          DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),szDescription,szFriendlyName,MSG(MHRemove),MSG(MHCancel));

        if(!DoneEject)
        {
          bResult = RemoveHotplugDevice (pInfo[i].hDevInst,Drive,Flags);
          SavedLastError=GetLastError();
        }
        else
          bResult = -1;

        if(bResult == 1 && Opt.Confirm.RemoveHotPlug) // ??? Opt.Confirm.RemoveHotPlug ???
           Message(0,1,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugNotify1),szDescription,szFriendlyName,MSG(MChangeHotPlugNotify2),MSG(MOk));

        // ??? break; ???
      }
    }

    FreeHotplugDevicesInfo (pInfo);
  }

  FinalizeSetupAPI();

  SetLastError(SavedLastError);

  return bResult;
}

bool InitializeSetupAPI ()
{
  bool bResult = false;

  if ( !g_hSetupAPI )
  {
    g_hSetupAPI = LoadLibrary ("setupapi.dll");

    if ( g_hSetupAPI )
    {
      pfnGetVolumeNameForVolumeMountPoint = (GETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress (
          GetModuleHandle ("kernel32.dll"),
          "GetVolumeNameForVolumeMountPointA"
          );

      pfnGetDevNodeRegistryProperty = (CMGETDEVNODEREGISTRYPROPERTY)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_DevNode_Registry_PropertyA"
          );

      pfnGetDevNodeStatus = (CMGETDEVNODESTATUS)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_DevNode_Status"
          );

      pfnGetDeviceID = (CMGETDEVICEID)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_IDA"
          );

      pfnGetDeviceIDListSize = (CMGETDEVICEIDLISTSIZE)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_ID_List_SizeA"
          );

      pfnGetDeviceIDList = (CMGETDEVICEIDLIST)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_ID_ListA"
          );

      pfnGetDeviceInterfaceListSize = (CMGETDEVICEINTERFACELISTSIZE)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_Interface_List_SizeA"
          );

      pfnGetDeviceInterfaceList = (CMGETDEVICEINTERFACELIST)GetProcAddress (
          g_hSetupAPI,
          "CM_Get_Device_Interface_ListA"
          );

      pfnLocateDevNode = (CMLOCATEDEVNODE)GetProcAddress (
          g_hSetupAPI,
          "CM_Locate_DevNodeA"
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
          "CM_Request_Device_EjectA"
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

  Len=sizeof(Capabilities);

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
         (Capabilities & CM_DEVCAP_REMOVABLE) &&
         !(Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) &&
         !(Capabilities & CM_DEVCAP_DOCKDEVICE) )
         return TRUE;
    }
  }

  return FALSE;
}


DWORD DriveMaskFromVolumeName (const char *lpVolumeName)
{
  DWORD dwMask = 0;

  char szCurrentVolumeName[MAX_PATH];
  char szMountPoint[4];

  szMountPoint[1] = ':';
  szMountPoint[2] = '\\';
  szMountPoint[3] = 0;

  for (char Letter = 'A'; Letter <= 'Z'; Letter++)
  {
    szMountPoint[0] = Letter;

    pfnGetVolumeNameForVolumeMountPoint (szMountPoint, szCurrentVolumeName, MAX_PATH);

    if ( !strcmpi (szCurrentVolumeName, lpVolumeName) )
      return (1 << (Letter-'A'));
  }

  return 0;
}

DWORD GetDriveMaskFromMountPoints (DEVINST hDevInst)
{
  DWORD dwMask = 0;
  char szDeviceID [MAX_DEVICE_ID_LEN];

  if ( pfnGetDeviceID (
      hDevInst,
      szDeviceID,
      sizeof(szDeviceID),
      0
      ) == CR_SUCCESS )
  {
    DWORD dwSize = 0;

    if ( pfnGetDeviceInterfaceListSize (
        &dwSize,
        (LPGUID)&VolumeClassGuid,
        (DEVINSTID_A)&szDeviceID,
        0
        ) == CR_SUCCESS )
    {
      if ( dwSize > 1 )
      {
        char *lpDeviceInterfaceList = (char*)malloc (dwSize);

        if ( pfnGetDeviceInterfaceList (
            (LPGUID)&VolumeClassGuid,
            (DEVINSTID_A)&szDeviceID,
            lpDeviceInterfaceList,
            dwSize,
            0
            ) == CR_SUCCESS )
        {
          char *p = lpDeviceInterfaceList;

          while ( *p )
          {
            char *lpMountPoint = (char*)malloc (strlen (p)+1+1); //for trailing slash

            strcpy (lpMountPoint, p);

            if ( !strchr (p+4, '\\') )
              strcat (lpMountPoint, "\\");

            char szVolumeName[MAX_PATH];

            if ( pfnGetVolumeNameForVolumeMountPoint (
                lpMountPoint,
                (char*)&szVolumeName,
                MAX_PATH
                ) )
              dwMask |= DriveMaskFromVolumeName (szVolumeName);

            free (lpMountPoint);

            p += strlen (p);
          }
        }

        free (lpDeviceInterfaceList);
      }
    }
  }

  return dwMask;
}

DWORD GetRelationDrivesMask (DEVINST hDevInst)
{
  DWORD dwMask = 0;
  DEVINST hRelationDevInst;
  char szDeviceID [MAX_DEVICE_ID_LEN];

  if ( pfnGetDeviceID (
      hDevInst,
      szDeviceID,
      sizeof (szDeviceID),
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
        char *lpDeviceIdList = (char*)malloc (dwSize);

        if ( pfnGetDeviceIDList (
            szDeviceID,
            lpDeviceIdList,
            dwSize,
            CM_GETIDLIST_FILTER_REMOVALRELATIONS
            ) == CR_SUCCESS )
        {
          char *p = lpDeviceIdList;

          while ( *p )
          {
            if ( pfnLocateDevNode (
                &hRelationDevInst,
                p,
                0
                ) == CR_SUCCESS )
              dwMask = GetDriveMaskFromMountPoints (hRelationDevInst);

            p += strlen (p);
          }
        }

        free (lpDeviceIdList);
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
        *pInfo = (DeviceInfo*)realloc (*pInfo, nCount*sizeof (DeviceInfo));

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
  free (pInfo);
}

bool GetDeviceProperty (
    DEVINST hDevInst,
    int nProperty,
    char *lpBuffer,
    DWORD dwMaxSize,
    bool bSearchChild
    )
{
  bool bResult = false;

  if ( g_hSetupAPI )
  {
    DWORD dwSize = dwMaxSize;
    CONFIGRET crResult;

    crResult = pfnGetDevNodeRegistryProperty (
        hDevInst,
        nProperty,//CM_DRP_FRIENDLYNAME,
        NULL,
        lpBuffer,
        &dwSize,
        0
        );

    if ( (crResult != CR_SUCCESS) || !*lpBuffer )
    {
      if ( bSearchChild )
      {
        DEVINST hDevChild;

        if ( pfnGetChild (&hDevChild, hDevInst, 0) == CR_SUCCESS )
          bResult = GetDeviceProperty (hDevChild, nProperty, lpBuffer, dwMaxSize, bSearchChild);
      }
    }
    else
    {
      CharToOem (lpBuffer, lpBuffer);
      bResult = true;
    }
  }

  return bResult;
}


int RemoveHotplugDevice (DEVINST hDevInst, char Drive, DWORD Flags)
{
  PNP_VETO_TYPE pvtVeto = PNP_VetoTypeUnknown;
  CONFIGRET crResult;

  char szDescription[MAX_PATH];
  memset (szDescription, 0, MAX_PATH);

  crResult = pfnRequestDeviceEject (
      hDevInst,
      &pvtVeto,
      (char*)&szDescription,
      MAX_PATH,
      0
      );

  if ( (crResult != CR_SUCCESS) || (pvtVeto != PNP_VetoTypeUnknown) ) //M$ баг, если есть szDecsription, то даже при ошибке возвращается CR_SUCCESS
  {
    //if(!(Flags&EJECT_NO_MESSAGE))
    {
    //   char MsgText[200];
    //   DWORD ErrCode=GetLastError();
    //   sprintf(MsgText,MSG(MChangeCouldNotEjectHotPlugMedia),Drive);
       SetLastError((pvtVeto != PNP_VetoTypeUnknown)?ERROR_DRIVE_LOCKED:ERROR_UNABLE_TO_UNLOAD_MEDIA); // ...о "The disk is in use or locked by another process."
    //   if(Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,MSG(MRetry),MSG(MCancel)) != 0)
    //     return -1;
    }
    return 0;
  }

  SetLastError(ERROR_SUCCESS);
  return 1;
}

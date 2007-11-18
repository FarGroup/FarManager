/*
usbeject.cpp

Detect & Eject USB носителей
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

#if defined(__BORLANDC__)
#if (__BORLANDC__ <= 0x0520)
#ifndef _BASETSD_H_
#include <basetsd.h>
#endif
#endif
#endif

//#define USED_USB_DETECT_AND_EJECT

#if defined(USED_USB_DETECT_AND_EJECT)

#if defined(__BORLANDC__)
#if (__BORLANDC__ <= 0x0520)
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

#endif
#endif
#endif

#include <setupapi.h> // Это находится в SDK
#include <cfgmgr32.h> // Это находится в SDK. Правда ему требуется еще cfg.h, которого в моем sdk

#endif
                      // почему-то не оказалось, зато нашлось в DDK, поэтому я его скопировал оттуда.

#include "fn.hpp"


#if defined(USED_USB_DETECT_AND_EJECT)

#ifndef __DDKDEFS_FOR_USB_REMOVE_H__
#define __DDKDEFS_FOR_USB_REMOVE_H__

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Define the access check value for any access
//
//
// The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
// ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
// constants *MUST* always be in sync.
//
//
// FILE_SPECIAL_ACCESS is checked by the NT I/O system the same as FILE_ANY_ACCESS.
// The file systems, however, may add additional access checks for I/O and FS controls
// that use this value.
//


#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

#define MOUNTDEVCONTROLTYPE  ((ULONG) 'M')

//
// The following IOCTL is supported by mounted devices.
//

#define IOCTL_MOUNTDEV_QUERY_DEVICE_NAME    CTL_CODE(MOUNTDEVCONTROLTYPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Output structure for IOCTL_MOUNTDEV_QUERY_DEVICE_NAME.
//

typedef struct _MOUNTDEV_NAME {
    USHORT  NameLength;
    WCHAR   Name[1];
} MOUNTDEV_NAME, *PMOUNTDEV_NAME;

#endif



typedef BOOL (WINAPI *tSetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);
typedef CONFIGRET (WINAPI
  *tCM_Get_DevNode_Status)(
    OUT PULONG  pulStatus,
    OUT PULONG  pulProblemNumber,
    IN DEVINST  dnDevInst,
    IN ULONG  ulFlags
    );
typedef CONFIGRET (WINAPI
  *tCM_Get_DevNode_Registry_PropertyA)(
    IN DEVINST  dnDevInst,
    IN ULONG  ulProperty,
    OUT PULONG  pulRegDataType, OPTIONAL
    OUT PVOID  Buffer, OPTIONAL
    IN OUT PULONG  pulLength,
    IN ULONG  ulFlags
    );
typedef CONFIGRET (WINAPI
  *tCM_Get_Parent)(
    OUT PDEVINST  pdnDevInst,
    IN DEVINST  dnDevInst,
    IN ULONG  ulFlags
    );
typedef CONFIGRET (WINAPI
  *tCM_Get_Device_Interface_List_SizeA)(
    IN PULONG  pulLen,
    IN LPGUID  InterfaceClassGuid,
    IN DEVINSTID  pDeviceID,  OPTIONAL
    IN ULONG  ulFlags
    );
typedef CONFIGRET (WINAPI
  *tCM_Get_Device_IDA)(
    IN DEVINST  dnDevInst,
    OUT PTCHAR  Buffer,
    IN ULONG  BufferLen,
    IN ULONG  ulFlags
    );
typedef CONFIGRET (WINAPI
  *tCM_Get_Device_Interface_ListA)(
    IN LPGUID  InterfaceClassGuid,
    IN DEVINSTID  pDeviceID,  OPTIONAL
    OUT PTCHAR  Buffer,
    IN ULONG  BufferLen,
    IN ULONG  ulFlags
    );
typedef CONFIGRET (WINAPI
  *tCM_Request_Device_EjectA)(
    IN DEVINST  dnDevInst,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPSTR  pszVetoName,
    IN ULONG  ulNameLength,
    IN ULONG  ulFlags
    );
typedef BOOL (WINAPI
  *tSetupDiEnumDeviceInfo)(
    IN HDEVINFO  DeviceInfoSet,
    IN DWORD  MemberIndex,
    OUT PSP_DEVINFO_DATA  DeviceInfoData
    );
typedef HDEVINFO (WINAPI *tSetupDiGetClassDevsA)(
  const GUID* ClassGuid,
  PCTSTR Enumerator,
  HWND hwndParent,
  DWORD Flags
);

#endif

#if 0

static tSetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList = NULL;
static tCM_Get_DevNode_Status pCM_Get_DevNode_Status = NULL;
static tCM_Get_DevNode_Registry_PropertyA pCM_Get_DevNode_Registry_PropertyA = NULL;
static tCM_Get_Parent pCM_Get_Parent = NULL;
static tCM_Get_Device_Interface_List_SizeA pCM_Get_Device_Interface_List_SizeA = NULL;
static tCM_Get_Device_IDA pCM_Get_Device_IDA = NULL;
static tCM_Get_Device_Interface_ListA pCM_Get_Device_Interface_ListA = NULL;
static tCM_Request_Device_EjectA pCM_Request_Device_EjectA = NULL;
static tSetupDiEnumDeviceInfo pSetupDiEnumDeviceInfo = NULL;
static tSetupDiGetClassDevsA pSetupDiGetClassDevsA = NULL;

static BOOL Init_SETUPAPI()
{
  static HMODULE hSetupApi={0};

  if(hSetupApi)
    return TRUE;
  hSetupApi = LoadLibrary("setupapi.dll");
  if(hSetupApi)
  {
    pSetupDiDestroyDeviceInfoList = (tSetupDiDestroyDeviceInfoList)GetProcAddress(hSetupApi, "SetupDiDestroyDeviceInfoList");
    pCM_Get_DevNode_Status = (tCM_Get_DevNode_Status)GetProcAddress(hSetupApi, "CM_Get_DevNode_Status");
    pCM_Get_DevNode_Registry_PropertyA = (tCM_Get_DevNode_Registry_PropertyA)GetProcAddress(hSetupApi, "CM_Get_DevNode_Registry_PropertyA");
    pCM_Get_Parent = (tCM_Get_Parent)GetProcAddress(hSetupApi, "CM_Get_Parent");
    pCM_Get_Device_Interface_List_SizeA = (tCM_Get_Device_Interface_List_SizeA)GetProcAddress(hSetupApi, "CM_Get_Device_Interface_List_SizeA");
    pCM_Get_Device_IDA = (tCM_Get_Device_IDA)GetProcAddress(hSetupApi, "CM_Get_Device_IDA");
    pCM_Get_Device_Interface_ListA = (tCM_Get_Device_Interface_ListA)GetProcAddress(hSetupApi, "CM_Get_Device_Interface_ListA");
    pCM_Request_Device_EjectA = (tCM_Request_Device_EjectA)GetProcAddress(hSetupApi, "CM_Request_Device_EjectA");
    pSetupDiEnumDeviceInfo = (tSetupDiEnumDeviceInfo)GetProcAddress(hSetupApi, "SetupDiEnumDeviceInfo");
    pSetupDiGetClassDevsA = (tSetupDiGetClassDevsA)GetProcAddress(hSetupApi, "SetupDiGetClassDevsA");
    return TRUE;
  }
  return FALSE;
}


// Вместо того, чтобы импортировать функцию GetVolumeNameForVolumeMountPoint,
// можно использовать нижеследующую функцию.

static BOOL GetVolumeName(const char* szDevice, char* szVolumeName, int cbVolumeName)
{
  HANDLE hDevice;

  hDevice = FAR_CreateFile(szDevice,FILE_READ_ATTRIBUTES|SYNCHRONIZE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if(hDevice == INVALID_HANDLE_VALUE)
    return FALSE;
  else
  {
    WCHAR buff[MAX_PATH]={0};
    DWORD dwBytes;
    BOOL res;
    res = DeviceIoControl(hDevice,IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,NULL, 0,&buff,sizeof(buff),&dwBytes,NULL);
    CloseHandle(hDevice);
    if(!res)
      return FALSE;
    if(WideCharToMultiByte(CP_ACP, 0, ((PMOUNTDEV_NAME)buff)->Name, ((PMOUNTDEV_NAME)buff)->NameLength,
                                       szVolumeName, cbVolumeName, NULL, NULL))
      return TRUE;
  }
  return FALSE;
}

#define USBSTOR "USBSTOR"

#endif

BOOL IsDriveUsb(
                char DriveName,                 // Имя диска
                void *pDevInst  // Здесь может быть NULL, если не NULL
                                                //  то сюда вернется
                                                // идентификатор устройства, который нужно
                                                // передать функции CM_Request_Device_Eject
                                                // для того, чтобы отключить указанный диск
                                                // в случае если, DriveName - USB
                )
{

#if defined(USED_USB_DETECT_AND_EJECT)

#if defined(__BORLANDC__) && (__BORLANDC__ <= 0x0520)
const GUID GUID_DEVINTERFACE_VOLUME = { 0x53f5630dL, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } };
#define VolumeClassGuid             GUID_DEVINTERFACE_VOLUME

#define DIGCF_DEVICEINTERFACE   0x00000010
#define ERROR_INVALID_DRIVE_OBJECT       4321L

#endif


  HDEVINFO hDevs;
  char dosDev[7];
  char dosDevVolume[MAX_PATH];
  SP_DEVINFO_DATA DeviceInfo={sizeof(SP_DEVINFO_DATA)};
  DWORD dwIndex;
  BOOL res = {0};
  ULONG cbInterfaceList={0};
  PCHAR szInterfaceList={0};

  if(!Init_SETUPAPI())
    return FALSE;


  if(pDevInst)
    *(PDEVINST)pDevInst = 0;


  dosDev[0] = '\\';
  dosDev[1] = '\\';
  dosDev[2] = '?';
  dosDev[3] = '\\';
  dosDev[4] = DriveName;
  dosDev[5] = ':';
  dosDev[6] = 0;

  if(!GetVolumeName(dosDev, dosDevVolume, sizeof(dosDevVolume)/sizeof(dosDevVolume[0])))
    return FALSE;


  hDevs = pSetupDiGetClassDevsA(&VolumeClassGuid,NULL,NULL,DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);

  if(hDevs == INVALID_HANDLE_VALUE)
    return FALSE;


  for(dwIndex = 0; pSetupDiEnumDeviceInfo(hDevs, dwIndex, &DeviceInfo); dwIndex++)
  {
    CHAR szDevId[MAX_DEVICE_ID_LEN];
    ULONG ulNewInterfaceList;
    CHAR szDeviceVolume[MAX_PATH];

    if(CR_SUCCESS != pCM_Get_Device_IDA(DeviceInfo.DevInst, szDevId, sizeof(szDevId)/sizeof(szDevId[0]), 0))
      break;
    ulNewInterfaceList = cbInterfaceList;
    if(CR_SUCCESS != pCM_Get_Device_Interface_List_SizeA(&ulNewInterfaceList, (LPGUID)&VolumeClassGuid, szDevId, 0))
      continue;
    if(ulNewInterfaceList > cbInterfaceList)
    {
      cbInterfaceList = ulNewInterfaceList;
      //szInterfaceList = xf_malloc(cbInterfaceList*sizeof(szInterfaceList[0]));
      szInterfaceList = (char *)xf_realloc(szInterfaceList,cbInterfaceList*sizeof(szInterfaceList[0]));
    }
    if(CR_SUCCESS != pCM_Get_Device_Interface_ListA((LPGUID)&VolumeClassGuid, szDevId, szInterfaceList, cbInterfaceList, 0))
        continue;
    if(!GetVolumeName(szInterfaceList, szDeviceVolume, sizeof(szDeviceVolume)/sizeof(szDeviceVolume[0])))
        continue;
    if(!lstrcmpi(szDeviceVolume, dosDevVolume))
    {
      // Нашли наш том, теперь нужно узнать, является ли он USB-шным.
      // Для этого нужно посмотреть на его отца и, если у папочки идентификатор
      // начинается с букв USBSTOR, то это - USB диск.
      DEVINST devDrive;
      if(CR_SUCCESS == pCM_Get_Parent(&devDrive, DeviceInfo.DevInst, 0))
      {
        char szDevDrive[MAX_DEVICE_ID_LEN];
        char *p;
        if(CR_SUCCESS != pCM_Get_Device_IDA(devDrive, szDevDrive, sizeof(szDevDrive)/sizeof(szDevDrive[0]), 0))
          break;
        p = strchr(szDevDrive, '\\');
        if(p)
        {
          *p = 0;
          res = !lstrcmpi(szDevDrive, USBSTOR);
          if(res && pDevInst)
          {
            // Теперь нам нужно получить устройство, которое следует отключить.
            DEVINST devToRemove;
            if(CR_SUCCESS == pCM_Get_Parent(&devToRemove, devDrive, 0))
            {
              // Мы можем отключить только устройство обладающее следующими свойствами
              // 1. У него нет проблемы CM_PROB_DEVICE_NOT_THERE
              // 2. Его можно удалить.
              // 3. Устройство не приемлет внезапного удаления
              // 4. Это не докуюемое устройство. (NOT dockable)
              DWORD Capabilities;
              ULONG Problem;
              ULONG Status;
              ULONG Len;

              Len = sizeof(Capabilities);

              if(CR_SUCCESS != pCM_Get_DevNode_Registry_PropertyA(
                  devToRemove,
                  CM_DRP_CAPABILITIES,
                  NULL,
                  (LPVOID)&Capabilities,
                  &Len,
                  0))
                  Capabilities = 0;

              if(CR_SUCCESS != pCM_Get_DevNode_Status(&Status, &Problem, devToRemove, 0))
                  Problem = 0;

              if((Problem != CM_PROB_DEVICE_NOT_THERE)
                  && (Capabilities & CM_DEVCAP_REMOVABLE)
                  && !(Capabilities & CM_DEVCAP_SURPRISEREMOVALOK)
                  && !(Capabilities & CM_DEVCAP_DOCKDEVICE))
                  *(PDEVINST)pDevInst = devToRemove;
            }
          }
        }
      }
      break; // Мы нашли то, что нужно.
    }
  }

  pSetupDiDestroyDeviceInfoList(hDevs);

  if(szInterfaceList)
		xf_free(szInterfaceList);

  return res;
#else
  return FALSE;
#endif
}

// TODO: здесь предусмотреть вывод месага, если соответствующий Flags выставлен.
BOOL RemoveUSBDrive(char Letter,DWORD Flags)
{
#if defined(USED_USB_DETECT_AND_EJECT)
  DEVINST devToRemove;

  if(!Init_SETUPAPI())
  {
    SetLastError(ERROR_INVALID_FUNCTION);
    return FALSE;
  }

  if(!IsDriveUsb(Letter, &devToRemove))
  {
    //It is not usb device
    SetLastError(ERROR_INVALID_DRIVE_OBJECT);
    return FALSE;
  }
  else if(!devToRemove)
  {
    // Failed to get devinst
    SetLastError(ERROR_UNRECOGNIZED_MEDIA);
    return FALSE;
  }
  else
  {
    _PNP_VETO_TYPE ulVeto;
    char szVeto[MAX_PATH];
    if(CR_SUCCESS != pCM_Request_Device_EjectA(devToRemove, &ulVeto, szVeto, sizeof(szVeto)/sizeof(szVeto[0]), 0))
    {
      // Failed to remove device
      SetLastError(ERROR_UNABLE_TO_UNLOAD_MEDIA);
      return FALSE;
    }
    else
    {
      if(ulVeto == 0)
      {
        // Removed device
        SetLastError(ERROR_SUCCESS);
        return TRUE;
      }
      else
      {
        // Failed to remove device - busi
        SetLastError(ERROR_DRIVE_LOCKED);
      }
    }
  }
#endif
  return FALSE;
}

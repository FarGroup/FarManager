/*
eject.cpp

Eject съемных носителей
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

#include "fn.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "imports.hpp"
#include "cddrv.hpp"

#if 0
static BOOL DismountVolume(HANDLE hVolume)
{
  DWORD dwBytesReturned;
  return DeviceIoControl(hVolume,FSCTL_DISMOUNT_VOLUME,NULL, 0,NULL, 0,&dwBytesReturned,NULL);
}
#endif

/* Функция by Vadim Yegorov <zg@matrica.apollo.lv>
   Доработанная! Умеет "вставлять" диск :-)
*/
BOOL EjectVolume(wchar_t Letter,DWORD Flags)
{
  HANDLE DiskHandle;
  BOOL Retry=TRUE;
  BOOL fAutoEject=FALSE;
  DWORD temp;
  BOOL ReadOnly=FALSE;
  UINT uDriveType;
  DWORD dwAccessFlags;
  BOOL fRemoveSafely = FALSE;
  BOOL foundError=FALSE;
  wchar_t szRootName[8]=L"\\\\.\\ :\\";

  szRootName[4]=Letter;

  // OpenVolume
  uDriveType = FAR_GetDriveType(szRootName+4);
  szRootName[6]=0;
  switch(uDriveType)
  {
    case DRIVE_REMOVABLE:
      dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
      break;
    default:
      if(IsDriveTypeCDROM(uDriveType))
      {
        dwAccessFlags = GENERIC_READ;
        break;
      }
      return FALSE;
  }

  DiskHandle=apiCreateFile(szRootName,dwAccessFlags,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                        0);
  if((DiskHandle==INVALID_HANDLE_VALUE) && (GetLastError()==ERROR_ACCESS_DENIED))
  {
    DiskHandle=apiCreateFile(szRootName,GENERIC_READ,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                          0);
    ReadOnly=FALSE;
  }

  if(DiskHandle!=INVALID_HANDLE_VALUE)
  {
    while(Retry)
    {
      if(DeviceIoControl(DiskHandle,FSCTL_LOCK_VOLUME,NULL,0,NULL,0,&temp,NULL))
      {
        foundError=FALSE;
        if(!ReadOnly)
          FlushFileBuffers(DiskHandle);

#if 0
// TODO: ЭТОТ КУСОК НУЖНО РАСКОММЕНТИТЬ ВМЕСТЕ С ПОДЪЕМОМ ПРОЕКТА ПО USB
/*
  ЭТО чудо нужно для того, чтобы, скажем, имея картридер на 3 карточки,
  дисмоунтить только 1 карточку, а не отключать все устройство!
*/
        if(!(Flags&EJECT_LOAD_MEDIA))
        {
          if(DismountVolume(DiskHandle))
            fRemoveSafely = TRUE;
          else
            foundError=TRUE;
        }
#endif
        if(!foundError)
        {
          PREVENT_MEDIA_REMOVAL PreventMediaRemoval;
          PreventMediaRemoval.PreventMediaRemoval=FALSE;

          if(DeviceIoControl(DiskHandle,IOCTL_STORAGE_MEDIA_REMOVAL,&PreventMediaRemoval,sizeof(PreventMediaRemoval),NULL,0,&temp,NULL))
          {
            #if 1
            // чистой воды шаманство...
            if(Flags&EJECT_READY)
            {
              fAutoEject=DeviceIoControl(DiskHandle,
                           IOCTL_STORAGE_CHECK_VERIFY,
                           NULL,0,0,0,&temp,NULL);
              // ...если ошибка = "нет доступа", то это похоже на то,
              // что диск вставлен
              // Способ экспериментальный, потому афишировать не имеет смысла.
              if(!fAutoEject && GetLastError() == 5)
                fAutoEject=TRUE;
              Retry=FALSE;
            }
            else
            #endif
              fAutoEject=DeviceIoControl(DiskHandle,
                                         (Flags&EJECT_LOAD_MEDIA)?IOCTL_STORAGE_LOAD_MEDIA:IOCTL_STORAGE_EJECT_MEDIA,
                                         NULL,0,NULL,0,&temp,NULL
                                        );
          }
          Retry=FALSE;
        }
      }
      else
        foundError=TRUE;

      if(foundError)
      {
        if(!(Flags&EJECT_NO_MESSAGE))
        {
          string strMsgText;
          strMsgText.Format (MSG(MChangeCouldNotEjectMedia),Letter);
          if(Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),strMsgText,MSG(MRetry),MSG(MCancel)))
            Retry=FALSE;
        }
        else
          Retry=FALSE;
      }
      else if(!(Flags&EJECT_LOAD_MEDIA) && fRemoveSafely)
      {
        //printf("Media in Drive %c can be safely removed.\n",cDriveLetter);
        //if(Flags&EJECT_NOTIFY_AFTERREMOVE)
          ;
      }
    } // END: while(Retry)


    DeviceIoControl(DiskHandle,FSCTL_UNLOCK_VOLUME,NULL,0,NULL,0,&temp,NULL);
    CloseHandle(DiskHandle);
  }
  return fAutoEject||fRemoveSafely; //???
}

BOOL IsEjectableMedia(wchar_t Letter,UINT DriveType,BOOL ForceCDROM)
{
  BOOL IsEjectable=FALSE;

  if (ForceCDROM && IsDriveTypeCDROM(DriveType))
  {
    IsEjectable = TRUE;
  }
  else
  {
    wchar_t win_name[]=L"\\\\.\\?:";
    win_name[4]=Letter;

    HANDLE h=apiCreateFile(win_name, 0, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0);

    if (h==INVALID_HANDLE_VALUE)
     return FALSE;

    DISK_GEOMETRY disk_g={0};
    DWORD b_ret=0;
    int ret=DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY,
                               NULL,                          // lpInBuffer
                               0,                             // nInBufferSize
                               &disk_g,                       // output buffer
                               sizeof(disk_g),                // size of output buffer
                               &b_ret,                        // number of bytes returned
                               0                              // OVERLAPPED structure
                           );
    if(ret)
      IsEjectable=disk_g.MediaType == RemovableMedia;
    CloseHandle(h);
  }

  return IsEjectable;
}

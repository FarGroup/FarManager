/*
eject.cpp

Eject съемных носителей

*/

/* Revision: 1.13 26.01.2003 $ */

/*
Modify:
  26.01.2003 IS
    ! FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
  27.11.2002 SVS
    ! BugZ#700 - CD tray insertion: <Ins> began to work as <Del>
      Изменения для масдая по Sergei Antonov <project@quake.ru>
  22.11.2002 SVS
    ! Некорректная компиляция куска кода под масдаем для Eject`а
  22.05.2002 SVS
    ! Откатимся обратно (про "вместо имени устройства") - есть сигнал про
      нерабочее состояние механизмУ.
  08.05.2002 VVM
    - Используем вместо имени устройства его ID
  27.02.2002 SVS
    ! переезд #include "mmsystem.h" в headers.hpp
  21.02.2002 SVS
    - некомпиляция под VC.
  21.02.2002 SVS
    ! Юзание mci-команд для масдая (хотя, WC, падла, не юзает msiSend...)
  13.02.2002 SVS
    ! Уборка варнингов
    - Упс. Потаенная бага... - проверка результата FAR_CreateFile
  06.05.2001 DJ
    ! перетрях #include
  27.04.2001 SVS
    ! Т.к. нет уверенного способа (пока нет) получить состояние устройства,
      то убираем "шаманство" с IOCTL_STORAGE_CHECK_VERIFY
  22.04.2001 SVS
    ! Изменена функция EjectVolume (по мотивам функции by
      Vadim Yegorov <zg@matrica.apollo.lv>)
      Умеет под NT/2000 "вставлять" диск :-)
  28.03.2001 SVS
    - Кхе. Забыли вернуть значение из EjectVolume95 :-(
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля
*/
#define __USE_MCI    1

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "global.hpp"

/* $ 14.12.2000 SVS
   Добавлен код для выполнения Eject съемных носителей для
   Win9x & WinNT/2K
*/

/*
   Program to programmatically eject removable media from a drive on
   Windows 95.
   http://support.microsoft.com/support/kb/articles/q168/1/80.asp
*/
//-----------------------------------------------------------------------
// DeviceIoControl infrastructure

#if !defined (VWIN32_DIOC_DOS_IOCTL)
#define VWIN32_DIOC_DOS_IOCTL      1

typedef struct _DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;
#endif

// Intel x86 processor status flags
#define CARRY_FLAG             0x0001

//-----------------------------------------------------------------------
// DOS IOCTL function support

#if defined(__BORLANDC__)
#pragma option -a1
#else
#pragma pack(1)
#endif

// Parameters for locking/unlocking removable media
typedef struct _PARAMBLOCK {
   BYTE bOperation;
   BYTE bNumLocks;
} PARAMBLOCK, *PPARAMBLOCK;

#if defined(__BORLANDC__)
#pragma option -a.
#else
#pragma pack()
#endif

/*-----------------------------------------------------------------------
UnlockMedia (hVWin32, bDrive)

Purpose:
   Unlocks removable media from the specified drive so that it can be
   ejected.

Parameters:
   hVWin32
      A handle to VWIN32. Used to issue request to unlock the media.

   bDrive
      The logical drive number to unlock. 0 = default, 1 = A, 2 = B,
      etc.

Return Value:
   If successful, returns TRUE; if unsuccessful, returns FALSE.
-----------------------------------------------------------------------*/
static BOOL UnlockMedia (HANDLE hVWin32, BYTE bDrive)
{
   DIOC_REGISTERS regs = {0};
   PARAMBLOCK     unlockParams = {0};
   int   i;
   BOOL  fResult;
   DWORD cb;

   // First, check the lock status. This way, you'll know the number of
   // pending locks you must unlock.

   unlockParams.bOperation = 2;   // return lock/unlock status

   regs.reg_EAX = 0x440D;
   regs.reg_EBX = bDrive;
   regs.reg_ECX = MAKEWORD(0x48, 0x08);
   regs.reg_EDX = (DWORD)&unlockParams;

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the unlock succeeded.
   if (fResult)
   {
      /*
         DeviceIoControl succeeded. Now see if the unlock succeeded. It
         succeeded if the carry flag is not set, or if the carry flag is
         set but EAX is 0x01 or 0xB0.

         It failed if the carry flag is set and EAX is not 0x01 or 0xB0.

         If the carry flag is clear, then unlock succeeded. However, you
         don't need to set fResult because it is already TRUE when you get
         in here.

      */
      if (regs.reg_Flags & CARRY_FLAG)
         fResult = (regs.reg_EAX == 0xB0) || (regs.reg_EAX == 0x01);
   }

   if (!fResult)
      return (FALSE);

   // Now, let's unlock the media for every time it has been locked;
   // this will totally unlock the media.

   for (i = 0; i < unlockParams.bNumLocks; ++i)
   {
      unlockParams.bOperation = 1;   // unlock the media

      regs.reg_EAX = 0x440D;
      regs.reg_EBX = bDrive;
      regs.reg_ECX = MAKEWORD(0x48, 0x08);
      regs.reg_EDX = (DWORD)&unlockParams;

      fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                                 &regs, sizeof(regs), &regs, sizeof(regs),
                                 &cb, 0);

      // See if DeviceIoControl and the lock succeeded
      fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);
      if (!fResult)
         break;
   }
   return fResult;
}

/*-----------------------------------------------------------------------
EjectMedia (hVWin32, bDrive)

Purpose:
   Ejects removable media from the specified drive.

Parameters:
   hVWin32
      A handle to VWIN32. Used to issue request to unlock the media.

   bDrive
      The logical drive number to unlock. 0 = default, 1 = A, 2 = B,
      etc.

Return Value:
   If successful, returns TRUE; if unsuccessful, returns FALSE.
-----------------------------------------------------------------------*/
static BOOL EjectMedia (HANDLE hVWin32, BYTE bDrive)
{
   DIOC_REGISTERS regs = {0};
   BOOL  fResult;
   DWORD cb;

   regs.reg_EAX = 0x440D;
   regs.reg_EBX = bDrive;
   regs.reg_ECX = MAKEWORD(0x49, 0x08);

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the lock succeeded
   fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);

   return fResult;
}

/*-----------------------------------------------------------------------
LockLogicalVolume (hVWin32, bDriveNum, bLockLevel, wPermissions)

Purpose:
   Takes a logical volume lock on a logical volume.

Parameters:
   hVWin32
      An open handle to VWIN32.

   bDriveNum
      The logical drive number to lock. 0 = default, 1 = A:, 2 = B:,
      3 = C:, etc.

   bLockLevel
      Can be 0, 1, 2, or 3. Level 0 is an exclusive lock that can only
      be taken when there are no open files on the specified drive.
      Levels 1 through 3 form a hierarchy where 1 must be taken before
      2, which must be taken before 3.

   wPermissions
      Specifies how the lock will affect file operations when lock levels
      1 through 3 are taken. Also specifies whether a formatting lock
      should be taken after a level 0 lock.

      Zero is a valid permission.

Return Value:
   If successful, returns TRUE.  If unsuccessful, returns FALSE.
-----------------------------------------------------------------------*/
static BOOL LockLogicalVolume (HANDLE hVWin32,
                               BYTE   bDriveNum,
                               BYTE   bLockLevel,
                               WORD   wPermissions)
{
   BOOL           fResult;
   DIOC_REGISTERS regs = {0};
   BYTE           bDeviceCat;  // can be either 0x48 or 0x08
   DWORD          cb;

   /*
      Try first with device category 0x48 for FAT32 volumes. If it
      doesn't work, try again with device category 0x08. If that
      doesn't work, then the lock failed.
   */

   bDeviceCat = 0x48;

ATTEMPT_AGAIN:
   // Set up the parameters for the call.
   regs.reg_EAX = 0x440D;
   regs.reg_EBX = MAKEWORD(bDriveNum, bLockLevel);
   regs.reg_ECX = MAKEWORD(0x4A, bDeviceCat);
   regs.reg_EDX = wPermissions;

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the lock succeeded
   fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);

   // If DeviceIoControl or the lock failed, and device category 0x08
   // hasn't been tried, retry the operation with device category 0x08.
   if (!fResult && (bDeviceCat != 0x08))
   {
      bDeviceCat = 0x08;
      goto ATTEMPT_AGAIN;
   }

   return fResult;
}

/*-----------------------------------------------------------------------
UnlockLogicalVolume (hVWin32, bDriveNum)

Purpose:
   Unlocks a logical volume that was locked with LockLogicalVolume().

Parameters:
   hVWin32
      An open handle to VWIN32.

   bDriveNum
      The logical drive number to unlock. 0 = default, 1 = A:, 2 = B:,
      3 = C:, etc.

Return Value:
   If successful, returns TRUE. If unsuccessful, returns FALSE.

Comments:
   Must be called the same number of times as LockLogicalVolume() to
   completely unlock a volume.

   Only the lock owner can unlock a volume.
-----------------------------------------------------------------------*/
static BOOL UnlockLogicalVolume (HANDLE hVWin32, BYTE bDriveNum)
{
   BOOL           fResult;
   DIOC_REGISTERS regs = {0};
   BYTE           bDeviceCat;  // can be either 0x48 or 0x08
   DWORD          cb;

   /* Try first with device category 0x48 for FAT32 volumes. If it
      doesn't work, try again with device category 0x08. If that
      doesn't work, then the unlock failed.
   */

   bDeviceCat = 0x48;

ATTEMPT_AGAIN:
   // Set up the parameters for the call.
   regs.reg_EAX = 0x440D;
   regs.reg_EBX = bDriveNum;
   regs.reg_ECX = MAKEWORD(0x6A, bDeviceCat);

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the unlock succeeded
   fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);

   // If DeviceIoControl or the unlock failed, and device category 0x08
   // hasn't been tried, retry the operation with device category 0x08.
   if (!fResult && (bDeviceCat != 0x08))
   {
      bDeviceCat = 0x08;
      goto ATTEMPT_AGAIN;
   }
   return fResult;
}

/*-----------------------------------------------------------------------
This program ejects media from the specified drive, if the media is
removable and the device supports software-controlled media removal.
This code works on Windows 95 only.
-----------------------------------------------------------------------*/
BOOL EjectVolume95 (char Letter,DWORD Flags)
{
   HANDLE hVWin32;
   BYTE   bDrive;
   BOOL   fDriveLocked;
   char MsgText[200];

   // convert command line arg 1 from a drive letter to a DOS drive
   // number
   bDrive = (toupper (Letter) - 'A') + 1;

   // OpenVWin32
   /* Opens a handle to VWIN32 that can be used to issue low-level disk I/O
     commands. */
   hVWin32 = FAR_CreateFile ("\\\\.\\vwin32", 0, 0, NULL, 0,
                      FILE_FLAG_DELETE_ON_CLOSE, NULL);

   if(hVWin32 == INVALID_HANDLE_VALUE)
     return FALSE;

   BOOL Ret=FALSE;

   if (!(Flags&EJECT_LOAD_MEDIA))
   {
      // Make sure no other applications are using the drive.
      fDriveLocked = LockLogicalVolume (hVWin32, bDrive, 0, 0);
      if (!fDriveLocked)
      {
         if(!(Flags&EJECT_NO_MESSAGE))
         {
           sprintf(MsgText,MSG(MChangeVolumeInUse),Letter);
           Message(MSG_WARNING,1,MSG(MError),MsgText,MSG(MChangeVolumeInUse2),MSG(MOk));
         }
         goto CLEANUP_AND_EXIT_APP;
      }

      // Make sure there is no software lock keeping the media in the drive.
      if (!UnlockMedia (hVWin32, bDrive))
      {
         if(!(Flags&EJECT_NO_MESSAGE))
         {
           sprintf(MsgText,MSG(MChangeCouldNotUnlockMedia),Letter);
           Message(MSG_WARNING,1,MSG(MError),MsgText,MSG(MOk));
         }
         goto CLEANUP_AND_EXIT_APP;
      }

      // Eject the media
      if ((Ret=EjectMedia (hVWin32, bDrive)) == 0)
      {
         if(!(Flags&EJECT_NO_MESSAGE))
         {
           sprintf(MsgText,MSG(MChangeCouldNotEjectMedia),Letter);
           Message(MSG_WARNING,1,MSG(MError),MsgText,MSG(MOk));
         }
      }

CLEANUP_AND_EXIT_APP:
      if (fDriveLocked)
         UnlockLogicalVolume (hVWin32, bDrive);
   }
   else
   {
      // возврашаемые значения не проверяю, по моему так лучше
      UnlockMedia (hVWin32, bDrive);
      char cmd[100];
      sprintf(cmd, "open %c: type cdaudio alias ejcd shareable", toupper (Letter));

      typedef MCIERROR (WINAPI *PMCISENDSTRING)(LPCSTR lpstrCommand, LPSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);
      static PMCISENDSTRING pmciSendString=NULL;
      if(!pmciSendString)
        pmciSendString=(PMCISENDSTRING)GetProcAddress(LoadLibrary("WINMM.DLL"),"mciSendStringA");

      if(!pmciSendString)
        return FALSE;

      pmciSendString(cmd, 0, 0, 0);
      pmciSendString("set ejcd door closed", 0, 0, 0);
      pmciSendString("close ejcd", 0, 0, 0);
      Ret = TRUE;
   }
   if (hVWin32 != INVALID_HANDLE_VALUE)
      CloseHandle (hVWin32);

   return Ret;
}

/* Функция by Vadim Yegorov <zg@matrica.apollo.lv>
   Доработанная! Умеет под NT/2000 "вставлять" диск :-)
*/
BOOL EjectVolume(char Letter,DWORD Flags)
{
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
  {
    return EjectVolume95(Letter,Flags);
  }

  HANDLE DiskHandle;
  BOOL Retry=TRUE;
  BOOL fAutoEject=FALSE;
  DWORD temp;
  BOOL ReadOnly=FALSE;
  UINT uDriveType;
  DWORD dwAccessFlags;
  char szRootName[8]="\\\\.\\ :\\";

  szRootName[4]=Letter;

  uDriveType = GetDriveType(szRootName+4);
  szRootName[6]=0;
  switch(uDriveType)
  {
    case DRIVE_REMOVABLE:
      dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
      break;
    case DRIVE_CDROM:
      dwAccessFlags = GENERIC_READ;
      break;
    default:
      return FALSE;
  }

  DiskHandle=FAR_CreateFile(szRootName,dwAccessFlags,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                        0,0);
  if((DiskHandle==INVALID_HANDLE_VALUE) && (GetLastError()==ERROR_ACCESS_DENIED))
  {
    DiskHandle=FAR_CreateFile(szRootName,GENERIC_READ,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                          0,0);
    ReadOnly=FALSE;
  }
  if(DiskHandle!=INVALID_HANDLE_VALUE)
  {
    while(Retry)
      if(DeviceIoControl(DiskHandle,FSCTL_LOCK_VOLUME,NULL,0,NULL,0,&temp,NULL))
      {
        if(!ReadOnly)
          FlushFileBuffers(DiskHandle);
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
              NULL,0,NULL,0,&temp,NULL);
        }
        Retry=FALSE;
      }
      else
      {
        if(!(Flags&EJECT_NO_MESSAGE))
        {
          char MsgText[200];
          sprintf(MsgText,MSG(MChangeCouldNotEjectMedia),Letter);
          if(Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,MSG(MRetry),MSG(MCancel)))
            Retry=FALSE;
        }
        else
          Retry=FALSE;
      }
    DeviceIoControl(DiskHandle,FSCTL_UNLOCK_VOLUME,NULL,0,NULL,0,&temp,NULL);
    CloseHandle(DiskHandle);
  }
  return fAutoEject;
}

/*
eject.cpp

Eject съемных носителей

*/

/* Revision: 1.01 28.03.2001 $ */

/*
Modify:
  28.03.2001 SVS
    - Кхе. Забыли вернуть значение из EjectVolume95 :-(
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"

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
   HANDLE hVWin32      = INVALID_HANDLE_VALUE;
   BYTE   bDrive;
   BOOL   fDriveLocked = FALSE;
   char MsgText[200];
   BOOL Ret=FALSE;

   // convert command line arg 1 from a drive letter to a DOS drive
   // number
   bDrive = (toupper (Letter) - 'A') + 1;

   // OpenVWin32
   /* Opens a handle to VWIN32 that can be used to issue low-level disk I/O
     commands. */
   hVWin32 = CreateFile ("\\\\.\\vwin32", 0, 0, NULL, 0,
                      FILE_FLAG_DELETE_ON_CLOSE, NULL);

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

   if (hVWin32 != INVALID_HANDLE_VALUE)
      CloseHandle (hVWin32);

   return Ret;
}


/*
  HOWTO: Ejecting Removable Media in Windows NT/Windows 2000
  http://support.microsoft.com/support/kb/articles/Q165/7/21.ASP
*/

#define LOCK_TIMEOUT        10000       // 10 Seconds
#define LOCK_RETRIES        20
static BOOL LockVolume(HANDLE hVolume)
{
    DWORD dwBytesReturned;
    DWORD dwSleepAmount;
    int nTryCount;

    dwSleepAmount = LOCK_TIMEOUT / LOCK_RETRIES;

    // Do this in a loop until a timeout period has expired
    for (nTryCount = 0; nTryCount < LOCK_RETRIES; nTryCount++) {
        if (DeviceIoControl(hVolume,
                            FSCTL_LOCK_VOLUME,
                            NULL, 0,
                            NULL, 0,
                            &dwBytesReturned,
                            NULL))
            return TRUE;

        Sleep(dwSleepAmount);
    }

    return FALSE;
}


BOOL EjectVolume(char Letter,DWORD Flags)
{
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
  {
    return EjectVolume95(Letter,Flags);
  }

  HANDLE hVolume;

  BOOL fRemoveSafely = FALSE;
  BOOL fAutoEject = FALSE;
  DWORD dwBytesReturned;
  char szRootName[5];
  UINT uDriveType;
  char szVolumeName[8];
  DWORD dwAccessFlags;

  // Open the volume.

  sprintf(szRootName, "%c:\\", Letter);

  uDriveType = GetDriveType(szRootName);
  switch(uDriveType) {
  case DRIVE_REMOVABLE:
      dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
      break;
  case DRIVE_CDROM:
      dwAccessFlags = GENERIC_READ;
      break;
  default:
      return FALSE;
  }

  sprintf(szVolumeName, "\\\\.\\%c:", Letter);

  hVolume = CreateFile(szVolumeName,
                       dwAccessFlags,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL );
  if (hVolume == INVALID_HANDLE_VALUE)
  {
    if(!(Flags&EJECT_NO_MESSAGE))
    {
      char MsgText[200];
      sprintf(MsgText,MSG(MChangeCouldNotEjectMedia),Letter);
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MsgText,MSG(MOk));
    }
    return FALSE;
  }

  // Lock and dismount the volume.
  if (LockVolume(hVolume) &&
      DeviceIoControl( hVolume,  // DismountVolume
                            FSCTL_DISMOUNT_VOLUME,
                            NULL, 0,
                            NULL, 0,
                            &dwBytesReturned,
                            NULL)
     )
  {
      fRemoveSafely = TRUE;

      PREVENT_MEDIA_REMOVAL PMRBuffer;
      PMRBuffer.PreventMediaRemoval = FALSE;

      // Set prevent removal to false and eject the volume.
      if (DeviceIoControl( hVolume,  // PreventRemovalOfVolume
             IOCTL_STORAGE_MEDIA_REMOVAL,
             &PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL),
             NULL, 0,
             &dwBytesReturned,
             NULL)
          &&
          DeviceIoControl( hVolume,                     // AutoEjectVolume
             IOCTL_STORAGE_EJECT_MEDIA,
             NULL, 0,
             NULL, 0,
             &dwBytesReturned,
             NULL)
         )
        fAutoEject = TRUE;
  }

  // Close the volume so other processes can use the drive.
  if (!CloseHandle(hVolume))
      return FALSE;
/*
  if (fAutoEject)
      printf("Media in Drive %c has been ejected safely.\n",
             Letter);
  else {
      if (fRemoveSafely)
          printf("Media in Drive %c can be safely removed.\n",
          Letter);
  }
*/
  return TRUE;
}
/* SVS $ */

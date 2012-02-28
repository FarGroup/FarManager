/*
eject.cpp

Eject съемных носителей
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

#include "eject.hpp"
#include "language.hpp"
#include "imports.hpp"
#include "cddrv.hpp"
#include "message.hpp"
#include "plugin.hpp"
#include "stddlg.hpp"

#if 0
static BOOL DismountVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;
	return DeviceIoControl(hVolume,FSCTL_DISMOUNT_VOLUME,nullptr, 0,nullptr, 0,&dwBytesReturned,nullptr);
}
#endif

/* Функция by Vadim Yegorov <zg@matrica.apollo.lv>
   Доработанная! Умеет "вставлять" диск :-)
*/
BOOL EjectVolume(wchar_t Letter,UINT64 Flags)
{
	BOOL Retry=TRUE;
	BOOL fAutoEject=FALSE;
	DWORD temp;
	BOOL ReadOnly=FALSE;
	UINT uDriveType;
	DWORD dwAccessFlags;
	BOOL fRemoveSafely = FALSE;
	BOOL foundError=FALSE;
	string RootName=L"\\\\.\\ :\\";
	RootName.Replace(4, Letter);
	// OpenVolume
	uDriveType = FAR_GetDriveType(RootName+4);
	RootName.SetLength(6);
	switch (uDriveType)
	{
		case DRIVE_REMOVABLE:
			dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
			break;
		default:

			if (IsDriveTypeCDROM(uDriveType))
			{
				dwAccessFlags = GENERIC_READ;
				break;
			}

			return FALSE;
	}

	File Disk;
	bool Opened = Disk.Open(RootName, dwAccessFlags, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
	if(!Opened && GetLastError()==ERROR_ACCESS_DENIED)
	{
		Opened = Disk.Open(RootName,GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
		ReadOnly=FALSE;
	}

	if (Opened)
	{
		while (Retry)
		{
			if (Disk.IoControl(FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &temp))
			{
				foundError=FALSE;
				if (!ReadOnly)
				{
					Disk.FlushBuffers();
				}

#if 0

// TODO: ЭТОТ КУСОК НУЖНО РАСКОММЕНТИТЬ ВМЕСТЕ С ПОДЪЕМОМ ПРОЕКТА ПО USB
				/*
				  ЭТО чудо нужно для того, чтобы, скажем, имея картридер на 3 карточки,
				  дисмоунтить только 1 карточку, а не отключать все устройство!
				*/
				if (!(Flags&EJECT_LOAD_MEDIA))
				{
					if (DismountVolume(DiskHandle))
						fRemoveSafely = TRUE;
					else
						foundError=TRUE;
				}

#endif

				if (!foundError)
				{
					PREVENT_MEDIA_REMOVAL PreventMediaRemoval={FALSE};

					if (Disk.IoControl(IOCTL_STORAGE_MEDIA_REMOVAL, &PreventMediaRemoval, sizeof(PreventMediaRemoval), nullptr, 0, &temp))
					{
#if 1

						// чистой воды шаманство...
						if (Flags&EJECT_READY)
						{
							fAutoEject=Disk.IoControl(IOCTL_STORAGE_CHECK_VERIFY, nullptr, 0, 0, 0, &temp);

							// ...если ошибка = "нет доступа", то это похоже на то,
							// что диск вставлен
							// Способ экспериментальный, потому афишировать не имеет смысла.
							if (!fAutoEject && GetLastError() == ERROR_ACCESS_DENIED)
								fAutoEject=TRUE;

							Retry=FALSE;
						}
						else
#endif
							fAutoEject=Disk.IoControl((Flags&EJECT_LOAD_MEDIA)?IOCTL_STORAGE_LOAD_MEDIA:IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &temp);
					}

					Retry=FALSE;
				}
			}
			else
				foundError=TRUE;

			if (foundError)
			{
				if (!(Flags&EJECT_NO_MESSAGE))
				{
					//if (Message(MSG_WARNING|MSG_ERRORTYPE, 2, MSG(MError), LangString(MChangeCouldNotEjectMedia) << Letter, MSG(M Retry), MSG(MCancel)))
					if(OperationFailed(RootName, MError, LangString(MChangeCouldNotEjectMedia) << Letter, false))
						Retry=FALSE;
				}
				else
					Retry=FALSE;
			}
			else if (!(Flags&EJECT_LOAD_MEDIA) && fRemoveSafely)
			{
				//printf("Media in Drive %c can be safely removed.\n",cDriveLetter);
				//if(Flags&EJECT_NOTIFY_AFTERREMOVE)
				;
			}
		} // END: while(Retry)

		Disk.IoControl(FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &temp);
		Disk.Close();
	}

	return fAutoEject||fRemoveSafely; //???
}

bool IsEjectableMedia(wchar_t Letter,UINT DriveType,BOOL ForceCDROM)
{
	bool Result = false;

	if (ForceCDROM && IsDriveTypeCDROM(DriveType))
	{
		Result = true;
	}
	else
	{
		string name(L"\\\\.\\?:");
		name.Replace(4, Letter);
		File file;
		if(file.Open(name, 0, FILE_SHARE_WRITE, 0, OPEN_EXISTING))
		{
			DISK_GEOMETRY dg={};
			DWORD Bytes=0;
			if(file.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &dg, sizeof(dg), &Bytes))
			{
				Result = dg.MediaType == RemovableMedia;
			}
			file.Close();
		}
	}
	return Result;
}

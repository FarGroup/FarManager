/*
eject.cpp

Eject съёмных носителей
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

#include "eject.hpp"

#include "lang.hpp"
#include "cddrv.hpp"
#include "stddlg.hpp"
#include "exception.hpp"
#include "plugin.hpp"

#include "platform.fs.hpp"

#include "format.hpp"

#if 0
static bool DismountVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;
	return DeviceIoControl(hVolume,FSCTL_DISMOUNT_VOLUME,nullptr, 0,nullptr, 0,&dwBytesReturned,nullptr) != FALSE;
}
#endif

/* Функция by Vadim Yegorov <zg@matrica.apollo.lv>
   Доработанная! Умеет "вставлять" диск :-)
*/
bool EjectVolume(wchar_t Letter, unsigned long long Flags)
{
	auto fAutoEject = false;
	auto fRemoveSafely = false;

	auto RootName = L"\\\\.\\ :\\"s;
	RootName[4] = Letter;
	// OpenVolume
	const auto DriveType = FAR_GetDriveType(string_view(RootName).substr(4));
	RootName.pop_back();

	bool ReadOnly;
	DWORD dwAccessFlags;

	switch (DriveType)
	{
	case DRIVE_REMOVABLE:
		dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
		ReadOnly = false;
		break;

	default:
		if (IsDriveTypeCDROM(DriveType))
		{
			dwAccessFlags = GENERIC_READ;
			ReadOnly = true;
			break;
		}

		return false;
	}

	os::fs::file Disk;
	auto Opened = Disk.Open(RootName, dwAccessFlags, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
	if(!Opened && GetLastError()==ERROR_ACCESS_DENIED)
	{
		Opened = Disk.Open(RootName,GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
		ReadOnly = true;
	}

	if (Opened)
	{
		DWORD temp;
		auto Retry = true;
		auto foundError = false;

		while (Retry)
		{
			if (Disk.IoControl(FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &temp))
			{
				foundError = false;
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
						fRemoveSafely = true;
					else
						foundError = true;
				}

#endif

				if (!foundError)
				{
					PREVENT_MEDIA_REMOVAL PreventMediaRemoval{};
					if (Disk.IoControl(IOCTL_STORAGE_MEDIA_REMOVAL, &PreventMediaRemoval, sizeof(PreventMediaRemoval), nullptr, 0, &temp))
					{
#if 1
						// чистой воды шаманство...
						if (Flags & EJECT_READY)
						{
							fAutoEject = Disk.IoControl(IOCTL_STORAGE_CHECK_VERIFY, nullptr, 0, nullptr, 0, &temp);

							// ...если ошибка = "нет доступа", то это похоже на то,
							// что диск вставлен
							// Способ экспериментальный, потому афишировать не имеет смысла.
							if (!fAutoEject && GetLastError() == ERROR_ACCESS_DENIED)
								fAutoEject = true;

							Retry = false;
						}
						else
#endif
							fAutoEject = Disk.IoControl((Flags&EJECT_LOAD_MEDIA)? IOCTL_STORAGE_LOAD_MEDIA : IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &temp);
					}

					Retry = false;
				}
			}
			else
				foundError = true;

			if (foundError)
			{
				if (!(Flags & EJECT_NO_MESSAGE))
				{
					const auto ErrorState = error_state::fetch();

					if(OperationFailed(ErrorState, RootName, lng::MError, format(msg(lng::MChangeCouldNotEjectMedia), Letter), false) != operation::retry)
						Retry = false;
				}
				else
					Retry = false;
			}
			else if (!(Flags&EJECT_LOAD_MEDIA) && fRemoveSafely)
			{
				//printf("Media in Drive %c can be safely removed.\n",cDriveLetter);
				//if(Flags&EJECT_NOTIFY_AFTERREMOVE)
				;
			}
		} // END: while(Retry)

		Disk.IoControl(FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &temp);
	}

	return fAutoEject || fRemoveSafely; //???
}

bool IsEjectableMedia(wchar_t Letter)
{
	const string Disk = { L'\\', L'\\', L'.', L'\\', Letter, L':' };
	const os::fs::file File(Disk, 0, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
	if (!File)
		return false;

	DISK_GEOMETRY dg;
	DWORD Bytes;
	return File.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &dg, sizeof(dg), &Bytes) && dg.MediaType == RemovableMedia;
}

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

// Self:
#include "eject.hpp"

// Internal:
#include "cddrv.hpp"
#include "exception.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/* Функция by Vadim Yegorov <zg@matrica.apollo.lv>
*/
void EjectVolume(wchar_t Letter)
{
	const auto RootName = concat(L"\\\\.\\"sv, Letter, L':');

	bool ReadOnly;
	DWORD dwAccessFlags;

	const auto DriveType = FAR_GetDriveType(string_view(RootName).substr(4));
	if (DriveType == DRIVE_REMOVABLE)
	{
		dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
		ReadOnly = false;
	}
	else if (IsDriveTypeCDROM(DriveType))
	{
		dwAccessFlags = GENERIC_READ;
		ReadOnly = true;
	}
	else
	{
		throw MAKE_FAR_EXCEPTION(L"Unknown drive type"sv);
	}

	os::fs::file Disk;
	if (!Disk.Open(RootName, dwAccessFlags, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING) && GetLastError() == ERROR_ACCESS_DENIED)
	{
		if (!Disk.Open(RootName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
			throw MAKE_FAR_EXCEPTION(L"Cannot open disk"sv);

		ReadOnly = true;
	}

	if (!Disk.IoControl(FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0))
		throw MAKE_FAR_EXCEPTION(L"FSCTL_LOCK_VOLUME"sv);

	SCOPE_EXIT{ (void)Disk.IoControl(FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0); };

	if (!ReadOnly)
	{
		Disk.FlushBuffers();
	}
#if 0
	// TODO: ЭТОТ КУСОК НУЖНО РАСКОММЕНТИТЬ ВМЕСТЕ С ПОДЪЕМОМ ПРОЕКТА ПО USB
	/*
	  Это чудо нужно для того, чтобы, скажем, имея картридер на 3 карточки,
	  дисмоунтить только 1 карточку, а не отключать все устройство!
	*/
	if (Disk.IoControl(FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0))
		throw MAKE_FAR_EXCEPTION(L"DismountVolume"sv);
#endif

	PREVENT_MEDIA_REMOVAL PreventMediaRemoval{};
	if (!Disk.IoControl(IOCTL_STORAGE_MEDIA_REMOVAL, &PreventMediaRemoval, sizeof(PreventMediaRemoval), nullptr, 0))
		throw MAKE_FAR_EXCEPTION(L"IOCTL_STORAGE_MEDIA_REMOVAL"sv);

	if (!Disk.IoControl(IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0))
		throw MAKE_FAR_EXCEPTION(L"IOCTL_STORAGE_EJECT_MEDIA"sv);
}

void LoadVolume(wchar_t const Letter)
{
	const auto RootName = concat(L"\\\\.\\"sv, Letter, L':');

	os::fs::file Disk;
	if (!Disk.Open(RootName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
		throw MAKE_FAR_EXCEPTION(L"Cannot open disk"sv);

	if (!Disk.IoControl(IOCTL_STORAGE_LOAD_MEDIA, nullptr, 0, nullptr, 0))
		throw MAKE_FAR_EXCEPTION(L"IOCTL_STORAGE_LOAD_MEDIA"sv);
}

bool IsEjectableMedia(wchar_t Letter)
{
	const string Disk = { L'\\', L'\\', L'.', L'\\', Letter, L':' };
	const os::fs::file File(Disk, 0, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
	if (!File)
		return false;

	DISK_GEOMETRY dg;
	return File.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &dg, sizeof(dg)) && dg.MediaType == RemovableMedia;
}

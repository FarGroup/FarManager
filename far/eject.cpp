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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "eject.hpp"

// Internal:
#include "exception.hpp"
#include "pathmix.hpp"
#include "flink.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

/* Функция by Vadim Yegorov <zg@matrica.apollo.lv>
*/
void EjectVolume(string_view const Path)
{
	// Ejecting VHD ISO is a bad idea.
	// Currently OS ejects the medium but doesn't remove the device, which might be confusing
	if (auto IsVhd = false; detach_vhd(Path, IsVhd) || IsVhd)
		return;

	const auto DeviceName = extract_root_device(Path);

	bool ReadOnly;
	DWORD WriteFlag;

	const auto DriveType = os::fs::drive::get_type(DeviceName);
	if (DriveType == DRIVE_REMOVABLE)
	{
		WriteFlag = GENERIC_WRITE;
		ReadOnly = false;
	}
	else if (DriveType == DRIVE_CDROM)
	{
		WriteFlag = 0;
		ReadOnly = true;
	}
	else
	{
		throw far_exception(L"Unknown drive type"sv);
	}

	os::fs::file File;

	const auto OpenForWrite = [&](string_view const Name, const auto&... Args)
	{
		if (File.Open(Name, WriteFlag | GENERIC_READ, Args...))
			return true;

		if (WriteFlag && GetLastError() == ERROR_ACCESS_DENIED && File.Open(Name, GENERIC_READ, Args...))
			return false;

		throw far_exception(L"Cannot open the disk"sv);
	};

	if (!OpenForWrite(DeviceName, os::fs::file_share_all, nullptr, OPEN_EXISTING))
		ReadOnly = true;

	if (!File.IoControl(FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0))
		throw far_exception(L"FSCTL_LOCK_VOLUME"sv);

	SCOPE_EXIT
	{
		if (!File.IoControl(FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0))
		{
			LOGERROR(L"IoControl(FSCTL_UNLOCK_VOLUME, {}): {}"sv, File.GetName(), os::last_error());
		}
	};

	if (!ReadOnly)
	{
		if (!File.FlushBuffers())
		{
			LOGWARNING(L"FlushBuffers({}): {}"sv, File.GetName(), os::last_error());
		}
	}

	if constexpr ((false))
	{
		// TODO: ЭТОТ КУСОК НУЖНО РАСКОММЕНТИТЬ ВМЕСТЕ С ПОДЪЕМОМ ПРОЕКТА ПО USB
		/*
		  Это чудо нужно для того, чтобы, скажем, имея картридер на 3 карточки,
		  дисмоунтить только 1 карточку, а не отключать все устройство!
		*/
		if (File.IoControl(FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0))
			throw far_exception(L"DismountVolume"sv);
	}

	PREVENT_MEDIA_REMOVAL PreventMediaRemoval{};
	if (!File.IoControl(IOCTL_STORAGE_MEDIA_REMOVAL, &PreventMediaRemoval, sizeof(PreventMediaRemoval), nullptr, 0))
		throw far_exception(L"IOCTL_STORAGE_MEDIA_REMOVAL"sv);

	if (!File.IoControl(IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0))
		throw far_exception(L"IOCTL_STORAGE_EJECT_MEDIA"sv);
}

void LoadVolume(string_view const Path)
{
	const os::fs::file File(extract_root_device(Path), GENERIC_READ, os::fs::file_share_all, nullptr, OPEN_EXISTING);
	if (!File)
		throw far_exception(L"Cannot open the disk"sv);

	if (!File.IoControl(IOCTL_STORAGE_LOAD_MEDIA, nullptr, 0, nullptr, 0))
		throw far_exception(L"IOCTL_STORAGE_LOAD_MEDIA"sv);
}

bool IsEjectableMedia(string_view const Path)
{
	const os::fs::file File(extract_root_device(Path), 0, os::fs::file_share_all, nullptr, OPEN_EXISTING);
	if (!File)
		return false;

	DISK_GEOMETRY DiskGeometry{};
	return File.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &DiskGeometry, sizeof(DiskGeometry)) && DiskGeometry.MediaType == RemovableMedia;
}

/*
flink.cpp

Куча разных функций по обработке Link`ов - Hard&Sym
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
#include "flink.hpp"

// Internal:
#include "imports.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "dirmix.hpp"
#include "treelist.hpp"
#include "elevation.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "stddlg.hpp"
#include "string_utils.hpp"

// Platform:
#include "platform.fs.hpp"
#include "platform.security.hpp"

// Common:
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

bool CreateVolumeMountPoint(string_view const TargetVolume, const string& Object)
{
	string VolumeName;
	return os::fs::GetVolumeNameForVolumeMountPoint(TargetVolume, VolumeName) && SetVolumeMountPoint(Object.c_str(), VolumeName.c_str());
}

static bool FillREPARSE_DATA_BUFFER(REPARSE_DATA_BUFFER& rdb, string_view const PrintName, string_view const SubstituteName)
{
	struct name_data
	{
		size_t Offset, Length;
	};

	const auto fill_header = [&](size_t const DataLength)
	{
		if (DataLength + REPARSE_DATA_BUFFER_HEADER_SIZE > MAXIMUM_REPARSE_DATA_BUFFER_SIZE)
			return false;

		rdb.ReparseDataLength = static_cast<USHORT>(DataLength);
		rdb.Reserved = 0;
		return true;
	};

	const auto fill = [&](auto& Buffer, name_data const Substitute, name_data const Print)
	{
		Buffer.SubstituteNameOffset = static_cast<USHORT>(Substitute.Offset);
		Buffer.SubstituteNameLength = static_cast<USHORT>(Substitute.Length);
		Buffer.PrintNameOffset = static_cast<USHORT>(Print.Offset);
		Buffer.PrintNameLength = static_cast<USHORT>(Print.Length);

		return std::pair
		{
			copy_string(SubstituteName, Buffer.PathBuffer + Substitute.Offset / sizeof(wchar_t)),
			copy_string(PrintName, Buffer.PathBuffer + Print.Offset / sizeof(wchar_t))
		};
	};

	switch (rdb.ReparseTag)
	{
	// IO_REPARSE_TAG_MOUNT_POINT and IO_REPARSE_TAG_SYMLINK buffers are filled differently:
	// different order of print and substitute names and additional zero bytes in IO_REPARSE_TAG_MOUNT_POINT.
	// No particular reason to do that, but Windows for some reason does and we just mimic its approach to make
	// reparse points created by Far indistinguishable from created by, say, mklink.

	case IO_REPARSE_TAG_MOUNT_POINT:
		{
			name_data const Substitute{ 0, SubstituteName.size() * sizeof(wchar_t) };
			name_data const Print{ Substitute.Length + 1 * sizeof(wchar_t), PrintName.size() * sizeof(wchar_t) };

			const size_t ReparseDataLength = offsetof(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) - REPARSE_DATA_BUFFER_HEADER_SIZE + Print.Offset + Print.Length + 1 * sizeof(wchar_t);

			if (!fill_header(ReparseDataLength))
				return false;

			const auto [End1, End2] = fill(rdb.MountPointReparseBuffer, Substitute, Print);
			*End1 = *End2 = {};

			return true;
		}

	case IO_REPARSE_TAG_SYMLINK:
		{
			name_data const Print { 0, PrintName.size() * sizeof(wchar_t) };
			name_data const Substitute { Print.Length, SubstituteName.size() * sizeof(wchar_t) };

			const size_t ReparseDataLength = offsetof(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) - REPARSE_DATA_BUFFER_HEADER_SIZE + Substitute.Offset + Substitute.Length;

			if (!fill_header(ReparseDataLength))
				return false;

			fill(rdb.SymbolicLinkReparseBuffer, Substitute, Print);

			return true;
		}

	default:
		return false;
	}
}

static auto GetDesiredAccessForReparsePointChange()
{
	static const auto DesiredAccess = IsWindowsXPOrGreater()? FILE_WRITE_ATTRIBUTES : GENERIC_WRITE;
	return DesiredAccess;
}

static bool SetREPARSE_DATA_BUFFER(const string_view Object, REPARSE_DATA_BUFFER& rdb)
{
	SCOPED_ACTION(os::security::privilege){ SE_CREATE_SYMBOLIC_LINK_NAME };

	const auto Attributes = os::fs::get_file_attributes(Object);
	if (Attributes == INVALID_FILE_ATTRIBUTES)
		return false;

	if(Attributes&FILE_ATTRIBUTE_READONLY)
	{
		(void)os::fs::set_file_attributes(Object, Attributes&~FILE_ATTRIBUTE_READONLY); //BUGBUG
	}

	SCOPE_EXIT
	{
		if (Attributes&FILE_ATTRIBUTE_READONLY)
		(void)os::fs::set_file_attributes(Object, Attributes); //BUGBUG
	};

	if (Attributes & FILE_ATTRIBUTE_REPARSE_POINT)
		DeleteReparsePoint(Object);

	const auto SetBuffer = [&](bool ForceElevation)
	{
		const os::fs::file fObject(Object, GetDesiredAccessForReparsePointChange(), 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr, ForceElevation);
		return fObject && fObject.IoControl(FSCTL_SET_REPARSE_POINT, &rdb, rdb.ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE, nullptr, 0);
	};

	if (SetBuffer(false))
		return true;

	if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		// Open() succeeded, but IoControl() failed. We can't handle this automatically :(
		return SetBuffer(true);
	}

	return false;
}

static bool PrepareAndSetREPARSE_DATA_BUFFER(REPARSE_DATA_BUFFER& rdb, string_view const Object, string_view const Target)
{
	switch (rdb.ReparseTag)
	{
	case IO_REPARSE_TAG_MOUNT_POINT:
		{
			const auto PrintName = ConvertNameToFull(Target);
			const auto SubstituteName = KernelPath(NTPath(PrintName));
			if (!FillREPARSE_DATA_BUFFER(rdb, PrintName, SubstituteName))
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}
		}
		break;

	case IO_REPARSE_TAG_SYMLINK:
		{
			const auto PrintName = Target;
			auto SubstituteName = Target;
			string SubstituteNameBuffer;

			if (IsAbsolutePath(Target))
			{
				SubstituteNameBuffer = KernelPath(NTPath(SubstituteName));
				SubstituteName = SubstituteNameBuffer;
				rdb.SymbolicLinkReparseBuffer.Flags = 0;
			}
			else
			{
				rdb.SymbolicLinkReparseBuffer.Flags = SYMLINK_FLAG_RELATIVE;
			}

			if (!FillREPARSE_DATA_BUFFER(rdb, PrintName, SubstituteName))
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}
		}
		break;

	default:
		return false;
	}

	return SetREPARSE_DATA_BUFFER(Object, rdb);
}

bool CreateReparsePoint(string_view const Target, string_view const Object, ReparsePointTypes Type)
{
	switch (Type)
	{
	case RP_EXACTCOPY:
		return DuplicateReparsePoint(Target, Object);

	case RP_SYMLINK:
	case RP_SYMLINKFILE:
	case RP_SYMLINKDIR:
		{
			if(Type == RP_SYMLINK)
				Type = os::fs::is_directory(Target)? RP_SYMLINKDIR : RP_SYMLINKFILE;

			os::fs::file_status const ObjectStatus(Object);
			if (imports.CreateSymbolicLinkW && !os::fs::exists(ObjectStatus))
				return os::fs::CreateSymbolicLink(Object, Target, Type == RP_SYMLINKDIR? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);

			const auto ObjectCreated = Type==RP_SYMLINKDIR?
				os::fs::is_directory(ObjectStatus) || os::fs::create_directory(Object) :
				os::fs::is_file(ObjectStatus) || os::fs::file(Object, 0, 0, nullptr, CREATE_NEW);

			if (!ObjectCreated)
				return false;

			const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			rdb->ReparseTag = IO_REPARSE_TAG_SYMLINK;
			return PrepareAndSetREPARSE_DATA_BUFFER(*rdb, Object, Target);
		}

	case RP_JUNCTION:
	case RP_VOLMOUNT:
		{
			const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			rdb->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
			return PrepareAndSetREPARSE_DATA_BUFFER(*rdb, Object, Target);
		}

	default:
		return false;
	}
}

static bool GetREPARSE_DATA_BUFFER(string_view const Object, REPARSE_DATA_BUFFER& rdb)
{
	const auto FileAttr = os::fs::get_file_attributes(Object);
	if (FileAttr == INVALID_FILE_ATTRIBUTES || !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
		return false;

	const os::fs::file fObject(Object, 0, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!fObject)
		return false;

	return fObject.IoControl(FSCTL_GET_REPARSE_POINT, nullptr, 0, &rdb, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
}

bool DeleteReparsePoint(string_view const Object)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!GetREPARSE_DATA_BUFFER(Object, *rdb))
		return false;

	const os::fs::file fObject(Object, GetDesiredAccessForReparsePointChange(), 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!fObject)
		return false;

	REPARSE_GUID_DATA_BUFFER rgdb{rdb->ReparseTag};
	return fObject.IoControl(FSCTL_DELETE_REPARSE_POINT, &rgdb, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, nullptr, 0);
}

bool GetReparsePointInfo(string_view const Object, string& DestBuffer, LPDWORD ReparseTag)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!GetREPARSE_DATA_BUFFER(Object, *rdb))
		return false;

	if (ReparseTag)
		*ReparseTag=rdb->ReparseTag;

	const auto Extract = [&](const auto& Buffer)
	{
		const wchar_t* PathBuffer;
		auto NameLength = Buffer.PrintNameLength / sizeof(wchar_t);

		if (NameLength)
		{
			PathBuffer = &Buffer.PathBuffer[Buffer.PrintNameOffset / sizeof(wchar_t)];
		}
		else
		{
			NameLength = Buffer.SubstituteNameLength / sizeof(wchar_t);
			PathBuffer = &Buffer.PathBuffer[Buffer.SubstituteNameOffset / sizeof(wchar_t)];
		}

		if (!NameLength)
			return false;

		DestBuffer.assign(PathBuffer, NameLength);
		return true;
	};

	switch (rdb->ReparseTag)
	{
	case IO_REPARSE_TAG_SYMLINK:
		return Extract(rdb->SymbolicLinkReparseBuffer);

	case IO_REPARSE_TAG_MOUNT_POINT:
		return Extract(rdb->MountPointReparseBuffer);

	default:
		return false;
	}
}

std::optional<size_t> GetNumberOfLinks(string_view const Name)
{
	const os::fs::file File(Name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!File)
		return {};

	BY_HANDLE_FILE_INFORMATION bhfi;
	if (!File.GetInformation(bhfi))
		return {};

	return bhfi.nNumberOfLinks;
}

bool MkHardLink(string_view const ExistingName, string_view const NewName, bool const Silent)
{
	for (;;)
	{
		if (os::fs::create_hard_link(NewName, ExistingName, nullptr))
			return true;

		if (Silent)
			return false;

		const auto ErrorState = error_state::fetch();

		if (OperationFailed(ErrorState, NewName, lng::MError, msg(lng::MCopyCannotCreateLink), false) != operation::retry)
			break;
	}

	return false;
}

bool EnumStreams(string_view const FileName, unsigned long long& StreamsSize, size_t& StreamsCount)
{
	bool Result=false;

	unsigned long long Size = 0;
	size_t Count = 0;

	for (const auto& i: os::fs::enum_streams(FileName))
	{
		++Count;
		Size += i.StreamSize.QuadPart;
	}

	if (Count)
	{
		StreamsCount = Count;
		StreamsSize = Size;
		Result = true;
	}

	return Result;
}

bool DelSubstDrive(string_view const DeviceName)
{
	string strTargetPath;
	return os::fs::QueryDosDevice(DeviceName, strTargetPath) &&
		DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE, null_terminated(DeviceName).c_str(), strTargetPath.c_str()) != FALSE;
}

bool GetSubstName(int DriveType, string_view const Path, string &strTargetPath)
{
	/*
	+ Обработка в зависимости от Global->Opt->SubstNameRule
	битовая маска:
	0 - если установлен, то опрашивать сменные диски
	1 - если установлен, то опрашивать все остальные
	*/
	const auto DriveRemovable = DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_CDROM;

	const auto
		CheckRemovable = Global->Opt->SubstNameRule & 0b01,
		CheckOther = Global->Opt->SubstNameRule & 0b10;

	if ((DriveRemovable && !CheckRemovable) || (!DriveRemovable && !CheckOther))
		return false;

	const auto Type = ParsePath(Path);
	if (Type != root_type::drive_letter)
		return false;

	const auto Drive = Path.substr(0, 2);

	string Device;
	if (!os::fs::QueryDosDevice(Drive, Device))
		return false;

	if (starts_with_icase(Device, L"\\??\\UNC\\"sv))
	{
		strTargetPath = concat(L"\\\\"sv, string_view(Device).substr(8));
		return true;
	}

	if (starts_with(Device, L"\\??\\"sv))
	{
		strTargetPath.assign(Device, 4, string::npos); // gcc 7.3-8.1 bug: npos required. TODO: Remove after we move to 8.2 or later
		return true;
	}

	return false;
}

bool GetVHDInfo(string_view const RootDirectory, string &strVolumePath, VIRTUAL_STORAGE_TYPE* StorageType)
{
	const os::fs::file Root(RootDirectory, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
	if (!Root)
		return false;

	block_ptr<STORAGE_DEPENDENCY_INFO> StorageDependencyInfo;

	const auto InitStorage = [&](size_t Size)
	{
		StorageDependencyInfo.reset(Size);
		StorageDependencyInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
	};

	DWORD Size = 4096;

	for (;;)
	{
		InitStorage(Size);
		if (Root.GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES, Size, StorageDependencyInfo.data(), &Size))
			break;
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			return false;
	}

	if (!StorageDependencyInfo->NumberEntries)
		return false;

	if(StorageType)
		*StorageType = StorageDependencyInfo->Version2Entries[0].VirtualStorageType;

	// trick: ConvertNameToReal also converts \\?\{UUID} to drive letter, if possible.
	strVolumePath = ConvertNameToReal(concat(StorageDependencyInfo->Version2Entries[0].HostVolumeName, StorageDependencyInfo->Version2Entries[0].DependentVolumeRelativePath));

	return true;
}

bool detach_vhd(string_view const RootDirectory, bool& IsVhd)
{
	IsVhd = false;

	string VhdFileName;
	VIRTUAL_STORAGE_TYPE VirtualStorageType;
	if (!GetVHDInfo(RootDirectory, VhdFileName, &VirtualStorageType))
		return false;

	IsVhd = true;
	return os::fs::detach_virtual_disk(VhdFileName, VirtualStorageType);
}

string GetPathRoot(string_view const Path)
{
	return extract_root_directory(ConvertNameToReal(Path));
}

bool ModifyReparsePoint(string_view const Object, string_view const Target)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	return GetREPARSE_DATA_BUFFER(Object, *rdb) && PrepareAndSetREPARSE_DATA_BUFFER(*rdb, Object, Target);
}

bool DuplicateReparsePoint(string_view const Src, string_view const Dst)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	return GetREPARSE_DATA_BUFFER(Src, *rdb) && SetREPARSE_DATA_BUFFER(Dst, *rdb);
}

void NormalizeSymlinkName(string &strLinkName)
{
	if (!starts_with(strLinkName, L"\\??\\"sv))
		return;

	if (ParsePath(strLinkName) != root_type::win32nt_drive_letter)
		return;

	strLinkName.erase(0, 4);
}

// Кусок для создания SymLink для каталогов.
bool MkSymLink(string_view const Target, string_view const LinkName, ReparsePointTypes LinkType, bool Silent, bool HoldTarget)
{
	string strFullTarget;
	// выделим имя
	string strSelOnlyName(Target);
	DeleteEndSlash(strSelOnlyName);
	const auto SlashPos = FindLastSlash(strSelOnlyName);

	const auto symlink = LinkType == RP_SYMLINK || LinkType == RP_SYMLINKFILE || LinkType == RP_SYMLINKDIR;

	if (Target[1] == L':' && (!Target[2] || (IsSlash(Target[2]) && !Target[3]))) // C: или C:/
	{
		// if(Flags&FCOPY_VOLMOUNT)
		{
			strFullTarget = Target;
			AddEndSlash(strFullTarget);
		}
		/*
			Вот здесь - ну очень умное поведение!
			Т.е. если в качестве SelName передали "C:", то в этом куске происходит
			коррекция типа линка - с symlink`а на volmount
		*/
		LinkType = RP_VOLMOUNT;
	}
	else
		strFullTarget = ConvertNameToFull(Target);

	auto strFullLink = ConvertNameToFull(LinkName);

	if (IsSlash(strFullLink.back()))
	{
		if (LinkType != RP_VOLMOUNT)
		{
			const auto SelName = SlashPos != string::npos?
				string_view(strSelOnlyName).substr(SlashPos + 1) :
				string_view(strSelOnlyName);
			append(strFullLink, SelName);
		}
		else
		{
			append(strFullLink, L"Disk_"sv, Target.front());
		}
	}

	if (LinkType == RP_VOLMOUNT)
	{
		AddEndSlash(strFullTarget);
		AddEndSlash(strFullLink);
	}

	if (symlink)
	{
		// в этом случае создается путь, но не сам каталог
		string_view Path = strFullLink;

		if (CutToSlash(Path))
		{
			if (!os::fs::exists(Path))
				CreatePath(Path);
		}
	}
	else
	{
		bool CreateDir = true;

		if (LinkType == RP_EXACTCOPY)
		{
			// в этом случае создается или каталог, или пустой файл
			if (os::fs::is_file(strFullTarget))
				CreateDir = false;
		}

		if (CreateDir)
		{
			if (os::fs::create_directory(strFullLink))
				TreeList::AddTreeName(strFullLink);
			else
				CreatePath(strFullLink);
		}
		else
		{
			string_view Path = strFullLink;

			if (CutToSlash(Path))
			{
				if (!os::fs::exists(Path))
					CreatePath(Path);
				os::fs::file(strFullLink, 0, 0, nullptr, CREATE_NEW, os::fs::get_file_attributes(strFullTarget));
			}
		}

		if (!os::fs::exists(strFullLink))
		{
			if (!Silent)
			{
				const auto ErrorState = error_state::fetch();

				Message(MSG_WARNING, ErrorState,
					msg(lng::MError),
					{
						msg(lng::MCopyCannotCreateLink),
						strFullLink
					},
					{ lng::MOk });
			}

			return false;
		}
	}

	if (LinkType == RP_VOLMOUNT)
	{
		if (CreateVolumeMountPoint(strFullTarget, strFullLink))
			return true;

		if (!Silent)
		{
			const auto ErrorState = error_state::fetch();

			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					format(msg(lng::MCopyMountVolFailed), Target),
					format(msg(lng::MCopyMountVolFailed2), strFullLink)
				},
				{ lng::MOk });
		}

		return false;
	}
	else
	{
		if (CreateReparsePoint(HoldTarget && symlink? Target : strFullTarget, strFullLink, LinkType))
			return true;

		if (!Silent)
		{
			const auto ErrorState = error_state::fetch();

			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					msg(lng::MCopyCannotCreateLink),
					strFullLink
				},
				{ lng::MOk });
		}

		return false;
	}
}

static string_view reparse_tag_to_string(DWORD ReparseTag)
{
	switch (ReparseTag)
	{
	case IO_REPARSE_TAG_MOUNT_POINT:                 return msg(lng::MListJunction);
	case IO_REPARSE_TAG_SYMLINK:                     return msg(lng::MListSymlink);
	case IO_REPARSE_TAG_HSM:                         return L"HSM"sv;
	case IO_REPARSE_TAG_HSM2:                        return L"HSM2"sv;
	case IO_REPARSE_TAG_SIS:                         return L"SIS"sv;
	case IO_REPARSE_TAG_WIM:                         return L"WIM"sv;
	case IO_REPARSE_TAG_CSV:                         return L"CSV"sv;
	case IO_REPARSE_TAG_DFS:                         return L"DFS"sv;
	case IO_REPARSE_TAG_DFSR:                        return L"DFSR"sv;
	case IO_REPARSE_TAG_DEDUP:                       return L"DEDUP"sv;
	case IO_REPARSE_TAG_NFS:                         return L"NFS"sv;
	case IO_REPARSE_TAG_FILE_PLACEHOLDER:            return L"FILE PLACEHOLDER"sv;
	case IO_REPARSE_TAG_WOF:                         return L"WOF"sv;
	case IO_REPARSE_TAG_WCI:                         return L"WCI"sv;
	case IO_REPARSE_TAG_WCI_1:                       return L"WCI 1"sv;
	case IO_REPARSE_TAG_GLOBAL_REPARSE:              return L"GLOBAL_REPARSE"sv;
	case IO_REPARSE_TAG_CLOUD:                       return L"CLOUD"sv;
	case IO_REPARSE_TAG_CLOUD_1:                     return L"CLOUD 1"sv;
	case IO_REPARSE_TAG_CLOUD_2:                     return L"CLOUD 2"sv;
	case IO_REPARSE_TAG_CLOUD_3:                     return L"CLOUD 3"sv;
	case IO_REPARSE_TAG_CLOUD_4:                     return L"CLOUD 4"sv;
	case IO_REPARSE_TAG_CLOUD_5:                     return L"CLOUD 5"sv;
	case IO_REPARSE_TAG_CLOUD_6:                     return L"CLOUD 6"sv;
	case IO_REPARSE_TAG_CLOUD_7:                     return L"CLOUD 7"sv;
	case IO_REPARSE_TAG_CLOUD_8:                     return L"CLOUD 8"sv;
	case IO_REPARSE_TAG_CLOUD_9:                     return L"CLOUD 9"sv;
	case IO_REPARSE_TAG_CLOUD_A:                     return L"CLOUD A"sv;
	case IO_REPARSE_TAG_CLOUD_B:                     return L"CLOUD B"sv;
	case IO_REPARSE_TAG_CLOUD_C:                     return L"CLOUD C"sv;
	case IO_REPARSE_TAG_CLOUD_D:                     return L"CLOUD D"sv;
	case IO_REPARSE_TAG_CLOUD_E:                     return L"CLOUD E"sv;
	case IO_REPARSE_TAG_CLOUD_F:                     return L"CLOUD F"sv;
	case IO_REPARSE_TAG_APPEXECLINK:                 return L"APPEXECLINK"sv;
	case IO_REPARSE_TAG_PROJFS:                      return L"PROJFS"sv;
	case IO_REPARSE_TAG_STORAGE_SYNC:                return L"STORAGE SYNC"sv;
	case IO_REPARSE_TAG_WCI_TOMBSTONE:               return L"WCI TOMBSTONE"sv;
	case IO_REPARSE_TAG_UNHANDLED:                   return L"UNHANDLED"sv;
	case IO_REPARSE_TAG_ONEDRIVE:                    return L"ONEDRIVE"sv;
	case IO_REPARSE_TAG_PROJFS_TOMBSTONE:            return L"PROJFS TOMBSTONE"sv;
	case IO_REPARSE_TAG_AF_UNIX:                     return L"AF UNIX"sv;
	case IO_REPARSE_TAG_LX_SYMLINK:                  return L"LX SYMLINK"sv;
	case IO_REPARSE_TAG_LX_FIFO:                     return L"LX FIFO"sv;
	case IO_REPARSE_TAG_LX_CHR:                      return L"LX CHR"sv;
	case IO_REPARSE_TAG_LX_BLK:                      return L"LX BLK"sv;
	case IO_REPARSE_TAG_DRIVE_EXTENDER:              return L"DRIVE EXTENDER"sv;
	case IO_REPARSE_TAG_FILTER_MANAGER:              return L"FILTER MANAGER"sv;
	case IO_REPARSE_TAG_IIS_CACHE:                   return L"IIS CACHE"sv;
	case IO_REPARSE_TAG_APPXSTRM:                    return L"APPXSTRM"sv;
	case IO_REPARSE_TAG_DFM:                         return L"DFM"sv;
	default:                                         return {};
	}
}

bool reparse_tag_to_string(DWORD ReparseTag, string& Str)
{
	Str = reparse_tag_to_string(ReparseTag);

	if (!Str.empty())
		return true;

	Str = format(FSTR(L":{0:0>8X}"), ReparseTag);
	return false;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("flink.fill.reparse.buffer")
{
	const auto BufferSize = 100;
	block_ptr<REPARSE_DATA_BUFFER> const Buffer(BufferSize);

	{
		char const ExpectedData[]
		{
			// ReparseTag
			0x03, 0x00, 0x00, 0xa0,
			// ReparseDataLength
			0x3c, 0x00,
			// Reserved
			0x00, 0x00,
			// SubstituteNameOffset
			0x00, 0x00,
			// SubstituteNameLength
			0x1c, 0x00,
			// PrintNameOffset
			0x1e, 0x00,
			// PrintNameLength
			0x14, 0x00,
			// PathBuffer
			0x5c, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x5c, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x5c, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x77, 0x00, 0x73, 0x00, 0x00, 0x00,
			0x63, 0x00, 0x3a, 0x00, 0x5c, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x77, 0x00, 0x73, 0x00, 0x00, 0x00,

		};

		static_assert(BufferSize >= std::size(ExpectedData));

		Buffer->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
		FillREPARSE_DATA_BUFFER(*Buffer, L"c:\\windows"sv, L"\\??\\c:\\windows"sv);

		REQUIRE(Buffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT);
		REQUIRE(Buffer->ReparseDataLength == 60);
		REQUIRE(Buffer->Reserved == 0);
		REQUIRE(Buffer->MountPointReparseBuffer.SubstituteNameOffset == 0);
		REQUIRE(Buffer->MountPointReparseBuffer.SubstituteNameLength == 28);
		REQUIRE(Buffer->MountPointReparseBuffer.PrintNameOffset == 30);
		REQUIRE(Buffer->MountPointReparseBuffer.PrintNameLength == 20);

		REQUIRE(std::equal(ALL_CONST_RANGE(ExpectedData), static_cast<char const*>(static_cast<void const*>(Buffer.data()))));
	}

	{
		char const ExpectedData[]
		{
			// ReparseTag
			0x0c, 0x00, 0x00, 0xa0,
			// ReparseDataLength
			0x3c, 0x00,
			// Reserved
			0x00, 0x00,
			// SubstituteNameOffset
			0x14, 0x00,
			// SubstituteNameLength
			0x1c, 0x00,
			// PrintNameOffset
			0x00, 0x00,
			// PrintNameLength
			0x14, 0x00,
			// Flags
			0x00, 0x00, 0x00, 0x00,
			// PathBuffer
			0x63, 0x00, 0x3a, 0x00, 0x5c, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x77, 0x00, 0x73, 0x00,
			0x5c, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x5c, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x5c, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x77, 0x00, 0x73, 0x00,
		};

		static_assert(BufferSize >= std::size(ExpectedData));

		Buffer->ReparseTag = IO_REPARSE_TAG_SYMLINK;
		Buffer->SymbolicLinkReparseBuffer.Flags = 0;
		FillREPARSE_DATA_BUFFER(*Buffer, L"c:\\windows"sv, L"\\??\\c:\\windows"sv);

		REQUIRE(Buffer->ReparseTag == IO_REPARSE_TAG_SYMLINK);
		REQUIRE(Buffer->ReparseDataLength == 60);
		REQUIRE(Buffer->Reserved == 0);
		REQUIRE(Buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset == 20);
		REQUIRE(Buffer->SymbolicLinkReparseBuffer.SubstituteNameLength == 28);
		REQUIRE(Buffer->SymbolicLinkReparseBuffer.PrintNameOffset == 0);
		REQUIRE(Buffer->SymbolicLinkReparseBuffer.PrintNameLength == 20);
		REQUIRE(Buffer->SymbolicLinkReparseBuffer.Flags == 0);

		REQUIRE(std::equal(ALL_CONST_RANGE(ExpectedData), static_cast<char const*>(static_cast<void const*>(Buffer.data()))));
	}
}
#endif

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
#include "reparse_tags.hpp"
#include "stddlg.hpp"
#include "string_utils.hpp"
#include "log.hpp"
#include "exception.hpp"
#include "encoding.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"
#include "platform.security.hpp"

// Common:
#include "common/enum_substrings.hpp"
#include "common/scope_exit.hpp"

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
	static const auto DesiredAccess = IsWindowsXPOrGreater()? FILE_WRITE_ATTRIBUTES : FILE_WRITE_DATA;
	return DesiredAccess;
}

static bool SetREPARSE_DATA_BUFFER(const string_view Object, REPARSE_DATA_BUFFER& rdb)
{
	SCOPED_ACTION(os::security::privilege){ SE_CREATE_SYMBOLIC_LINK_NAME };

	const auto Attributes = os::fs::get_file_attributes(Object);
	if (Attributes == INVALID_FILE_ATTRIBUTES)
		return false;

	if (Attributes & FILE_ATTRIBUTE_READONLY && !os::fs::set_file_attributes(Object, Attributes & ~FILE_ATTRIBUTE_READONLY)) //BUGBUG
	{
		LOGWARNING(L"set_file_attributes({}): {}"sv, Object, os::last_error());
	}

	SCOPE_EXIT
	{
		if (Attributes & FILE_ATTRIBUTE_READONLY && !os::fs::set_file_attributes(Object, Attributes)) //BUGBUG
		{
			LOGWARNING(L"set_file_attributes({}): {}"sv, Object, os::last_error());
		}

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
			const auto SubstituteName = kernel_path(nt_path(PrintName));
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
				SubstituteNameBuffer = kernel_path(nt_path(SubstituteName));
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

bool CreateReparsePoint(string_view const Target, string_view const Object, ReparsePointTypes Type)
{
	assert(any_of(Type, RP_EXACTCOPY, RP_JUNCTION, RP_VOLMOUNT, RP_SYMLINK, RP_SYMLINKFILE, RP_SYMLINKDIR));

	if (Type == RP_SYMLINK)
		Type = os::fs::is_directory(Target)? RP_SYMLINKDIR : RP_SYMLINKFILE;

	os::fs::file_status const ObjectStatus(Object);

	if (any_of(Type, RP_SYMLINKDIR, RP_SYMLINKFILE))
	{
		if (imports.CreateSymbolicLinkW && !os::fs::exists(ObjectStatus))
			return os::fs::CreateSymbolicLink(Object, Target, Type == RP_SYMLINKDIR? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
	}

	const auto NeedDirectory = any_of(Type, RP_SYMLINKDIR, RP_JUNCTION, RP_VOLMOUNT) || (Type == RP_EXACTCOPY && os::fs::is_directory(Target));

	bool ObjectCreated{};
	const auto ensure_object = [&]
	{
		if (os::fs::exists(ObjectStatus))
			return true;

		ObjectCreated = NeedDirectory?
			os::fs::create_directory(Object) :
			!!os::fs::file(Object, 0, 0, nullptr, CREATE_NEW);

		return ObjectCreated;
	};

	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

	if (Type == RP_EXACTCOPY)
	{
		if (!ensure_object())
			return false;

		if (GetREPARSE_DATA_BUFFER(Target, *rdb) && SetREPARSE_DATA_BUFFER(Object, *rdb))
			return true;
	}
	else if (any_of(Type, RP_JUNCTION, RP_VOLMOUNT))
	{
		if (!ensure_object())
			return false;

		rdb->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
		if (PrepareAndSetREPARSE_DATA_BUFFER(*rdb, Object, Target))
			return true;
	}
	else if (any_of(Type, RP_SYMLINKDIR, RP_SYMLINKFILE))
	{
		if (!ensure_object())
			return false;

		rdb->ReparseTag = IO_REPARSE_TAG_SYMLINK;
		if (PrepareAndSetREPARSE_DATA_BUFFER(*rdb, Object, Target))
			return true;
	}
	else
		return false;

	if (ObjectCreated)
	{
		if (NeedDirectory)
		{
			if (!os::fs::remove_directory(Object))
				LOGWARNING(L"remove_directory({}): {}"sv, Object, os::last_error());
		}
		else
		{
			if (!os::fs::delete_file(Object))
				LOGWARNING(L"delete_file({}): {}"sv, Object, os::last_error());
		}
	}

	return false;
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
		if (const auto NameLength = Buffer.PrintNameLength / sizeof(wchar_t))
		{
			DestBuffer.assign(Buffer.PathBuffer + Buffer.PrintNameOffset / sizeof(wchar_t), NameLength);
			return true;
		}

		if (const auto NameLength = Buffer.SubstituteNameLength / sizeof(wchar_t))
		{
			DestBuffer.assign(Buffer.PathBuffer + Buffer.SubstituteNameOffset / sizeof(wchar_t), NameLength);
			return true;
		}

		return false;
	};

	switch (rdb->ReparseTag)
	{
	case IO_REPARSE_TAG_SYMLINK:
		if (rdb->ReparseDataLength < sizeof(rdb->SymbolicLinkReparseBuffer))
			return false;
		return Extract(rdb->SymbolicLinkReparseBuffer);

	case IO_REPARSE_TAG_MOUNT_POINT:
		if (rdb->ReparseDataLength < sizeof(rdb->MountPointReparseBuffer))
			return false;
		return Extract(rdb->MountPointReparseBuffer);

	case IO_REPARSE_TAG_NFS:
		{
			constexpr auto NFS_SPECFILE_LNK = 0x014B4E4C;

			struct NFS_REPARSE_DATA_BUFFER
			{
				ULONG64 Type;
				WCHAR   DataBuffer[1];
			};

			if (rdb->ReparseDataLength < sizeof(NFS_REPARSE_DATA_BUFFER))
				return false;

			const auto& NfsReparseBuffer = view_as<NFS_REPARSE_DATA_BUFFER>(rdb->GenericReparseBuffer.DataBuffer);
			if (NfsReparseBuffer.Type != NFS_SPECFILE_LNK)
				return false;

			DestBuffer.assign(NfsReparseBuffer.DataBuffer, (rdb->ReparseDataLength - sizeof(NfsReparseBuffer.Type)) / sizeof(wchar_t));
			return true;
		}

	case IO_REPARSE_TAG_APPEXECLINK:
		{
			// The current protocol version is 3. It is known that in all 3 versions the third string in the list is the target filename.
			// Hopefully it stays like that in the future, but if no, the worse thing that could happen is a wrong string.
			constexpr size_t FilenameIndex = 2;

			struct APPEXECLINK_REPARSE_DATA_BUFFER
			{
				ULONG Version;
				WCHAR StringList[1];
			};

			if (rdb->ReparseDataLength < sizeof(APPEXECLINK_REPARSE_DATA_BUFFER))
				return false;

			const auto& AppExecLinkReparseBuffer = view_as<APPEXECLINK_REPARSE_DATA_BUFFER>(rdb->GenericReparseBuffer.DataBuffer);

			size_t Index = 0;
			const auto StringSize = (rdb->ReparseDataLength - sizeof(AppExecLinkReparseBuffer.Version)) / sizeof(*AppExecLinkReparseBuffer.StringList);
			string_view const StringList{ AppExecLinkReparseBuffer.StringList, StringSize };

			for (const auto& i: enum_substrings(StringList))
			{
				if (Index < FilenameIndex)
				{
					++Index;
					continue;
				}

				DestBuffer = i;
				return true;
			}
			return false;
		}

	case IO_REPARSE_TAG_LX_SYMLINK:
		{
			struct LX_SYMLINK_REPARSE_DATA_BUFFER
			{
				DWORD FileType;
				char  PathBuffer[1];
			};

			if (rdb->ReparseDataLength < sizeof(LX_SYMLINK_REPARSE_DATA_BUFFER))
				return false;

			const auto& LxSymlinkReparseBuffer = view_as<LX_SYMLINK_REPARSE_DATA_BUFFER>(rdb->GenericReparseBuffer.DataBuffer);
			DestBuffer = encoding::utf8::get_chars({ LxSymlinkReparseBuffer.PathBuffer, rdb->ReparseDataLength - sizeof(LxSymlinkReparseBuffer.FileType) });
			return true;
		}

	case IO_REPARSE_TAG_WCI:
	case IO_REPARSE_TAG_WCI_1:
	case IO_REPARSE_TAG_WCI_LINK:
	case IO_REPARSE_TAG_WCI_LINK_1:
		{
			struct WCI_REPARSE_DATA_BUFFER
			{
				ULONG Version;
				ULONG Reserved;
				GUID LookupGuid;
				USHORT WciNameLength;
				WCHAR WciName[1];
			};

			if (rdb->ReparseDataLength < sizeof(WCI_REPARSE_DATA_BUFFER))
				return false;

			const auto& WciReparseBuffer = view_as<WCI_REPARSE_DATA_BUFFER>(rdb->GenericReparseBuffer.DataBuffer);
			DestBuffer.assign(WciReparseBuffer.WciName, WciReparseBuffer.WciNameLength / sizeof(wchar_t));
			return true;
		}

	default:
		return false;
	}
}

std::optional<size_t> GetNumberOfLinks(string_view const Name)
{
	const os::fs::file File(Name, 0, os::fs::file_share_all, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!File)
		return {};

	BY_HANDLE_FILE_INFORMATION bhfi;
	if (!File.GetInformation(bhfi))
		return {};

	return bhfi.nNumberOfLinks;
}

bool MkHardLink(string_view const ExistingName, string_view const NewName, std::optional<error_state_ex>& ErrorState, bool const Silent)
{
	for (;;)
	{
		if (os::fs::create_hard_link(NewName, ExistingName, nullptr))
			return true;

		if (Silent)
			return false;

		ErrorState = os::last_error();

		if (OperationFailed(*ErrorState, NewName, lng::MError, msg(lng::MCopyCannotCreateLink), false) != operation::retry)
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

	if (Device.starts_with(L"\\??\\"sv))
	{
		strTargetPath.assign(Device, 4);
		return true;
	}

	return false;
}

bool GetVHDInfo(string_view const RootDirectory, string &strVolumePath, VIRTUAL_STORAGE_TYPE* StorageType)
{
	const os::fs::file Root(RootDirectory, FILE_READ_ATTRIBUTES, os::fs::file_share_all, nullptr, OPEN_EXISTING);
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

WARNING_PUSH()
WARNING_DISABLE_GCC("-Warray-bounds")

	if(StorageType)
		*StorageType = StorageDependencyInfo->Version2Entries[0].VirtualStorageType;

WARNING_POP()

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

void NormalizeSymlinkName(string &strLinkName)
{
	if (!strLinkName.starts_with(L"\\??\\"sv))
		return;

	if (ParsePath(strLinkName) != root_type::win32nt_drive_letter)
		return;

	strLinkName.erase(0, 4);
}

// Кусок для создания SymLink для каталогов.
bool MkSymLink(string_view const Target, string_view const LinkName, ReparsePointTypes LinkType, std::optional<error_state_ex>& ErrorState, bool Silent, bool HoldTarget)
{
	assert(any_of(LinkType, RP_EXACTCOPY, RP_JUNCTION, RP_VOLMOUNT, RP_SYMLINK, RP_SYMLINKFILE, RP_SYMLINKDIR));

	string strFullTarget;

	if (auto RootOnly = false; LinkType == RP_JUNCTION && ParsePath(Target, {}, &RootOnly) == root_type::drive_letter && RootOnly)
	{
		strFullTarget = Target;
		AddEndSlash(strFullTarget);

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

	if (path::is_separator(strFullLink.back()))
	{
		if (LinkType == RP_VOLMOUNT)
		{
			append(strFullLink, L"Disk_"sv, Target.front());
		}
		else
		{
			append(strFullLink, PointToFolderNameIfFolder(Target));
		}
	}

	if (string_view Path = strFullLink; CutToParent(Path) && !os::fs::exists(Path))
	{
		if (!CreatePath(Path))
			return false;
	}

	if (LinkType == RP_VOLMOUNT)
	{
		AddEndSlash(strFullLink);
		AddEndSlash(strFullTarget);

		if (CreateVolumeMountPoint(strFullTarget, strFullLink))
			return true;

		if (!Silent)
		{
			ErrorState = os::last_error();

			Message(MSG_WARNING, *ErrorState,
				msg(lng::MError),
				{
					far::vformat(msg(lng::MCopyMountVolFailed), Target),
					far::vformat(msg(lng::MCopyMountVolFailed2), strFullLink)
				},
				{ lng::MOk });
		}

		return false;
	}
	else
	{
		if (CreateReparsePoint(HoldTarget && LinkType != RP_JUNCTION? Target : strFullTarget, strFullLink, LinkType))
			return true;

		if (!Silent)
		{
			ErrorState = os::last_error();

			Message(MSG_WARNING, *ErrorState,
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
	default: return {};

#define TAG_STR(name) case IO_REPARSE_TAG_##name: return WIDE_SV(#name);
	// MS tags:
	TAG_STR(MOUNT_POINT)
	TAG_STR(HSM)
	TAG_STR(DRIVE_EXTENDER)
	TAG_STR(HSM2)
	TAG_STR(SIS)
	TAG_STR(WIM)
	TAG_STR(CSV)
	TAG_STR(DFS)
	TAG_STR(FILTER_MANAGER)
	TAG_STR(SYMLINK)
	TAG_STR(IIS_CACHE)
	TAG_STR(DFSR)
	TAG_STR(DEDUP)
	TAG_STR(APPXSTRM)
	TAG_STR(NFS)
	TAG_STR(FILE_PLACEHOLDER)
	TAG_STR(DFM)
	TAG_STR(WOF)
	TAG_STR(WCI)
	TAG_STR(WCI_1)
	TAG_STR(GLOBAL_REPARSE)
	TAG_STR(CLOUD)
	TAG_STR(CLOUD_1)
	TAG_STR(CLOUD_2)
	TAG_STR(CLOUD_3)
	TAG_STR(CLOUD_4)
	TAG_STR(CLOUD_5)
	TAG_STR(CLOUD_6)
	TAG_STR(CLOUD_7)
	TAG_STR(CLOUD_8)
	TAG_STR(CLOUD_9)
	TAG_STR(CLOUD_A)
	TAG_STR(CLOUD_B)
	TAG_STR(CLOUD_C)
	TAG_STR(CLOUD_D)
	TAG_STR(CLOUD_E)
	TAG_STR(CLOUD_F)
	TAG_STR(APPEXECLINK)
	TAG_STR(PROJFS)
	TAG_STR(LX_SYMLINK)
	TAG_STR(STORAGE_SYNC)
	TAG_STR(WCI_TOMBSTONE)
	TAG_STR(UNHANDLED)
	TAG_STR(ONEDRIVE)
	TAG_STR(PROJFS_TOMBSTONE)
	TAG_STR(AF_UNIX)
	TAG_STR(LX_FIFO)
	TAG_STR(LX_CHR)
	TAG_STR(LX_BLK)
	TAG_STR(WCI_LINK)
	TAG_STR(WCI_LINK_1)
	TAG_STR(DATALESS_CIM)

	// Non-MS tags:
	TAG_STR(IFSTEST_CONGRUENT)
	TAG_STR(MOONWALK_HSM)
	TAG_STR(TSINGHUA_UNIVERSITY_RESEARCH)
	TAG_STR(ARKIVIO)
	TAG_STR(SOLUTIONSOFT)
	TAG_STR(COMMVAULT)
	TAG_STR(OVERTONE)
	TAG_STR(SYMANTEC_HSM2)
	TAG_STR(ENIGMA_HSM)
	TAG_STR(SYMANTEC_HSM)
	TAG_STR(INTERCOPE_HSM)
	TAG_STR(KOM_NETWORKS_HSM)
	TAG_STR(MEMORY_TECH_HSM)
	TAG_STR(BRIDGEHEAD_HSM)
	TAG_STR(OSR_SAMPLE)
	TAG_STR(GLOBAL360_HSM)
	TAG_STR(ALTIRIS_HSM)
	TAG_STR(HERMES_HSM)
	TAG_STR(POINTSOFT_HSM)
	TAG_STR(GRAU_DATASTORAGE_HSM)
	TAG_STR(COMMVAULT_HSM)
	TAG_STR(DATASTOR_SIS)
	TAG_STR(EDSI_HSM)
	TAG_STR(HP_HSM)
	TAG_STR(SER_HSM)
	TAG_STR(DOUBLE_TAKE_HSM)
	TAG_STR(WISDATA_HSM)
	TAG_STR(MIMOSA_HSM)
	TAG_STR(HSAG_HSM)
	TAG_STR(ADA_HSM)
	TAG_STR(AUTN_HSM)
	TAG_STR(NEXSAN_HSM)
	TAG_STR(DOUBLE_TAKE_SIS)
	TAG_STR(SONY_HSM)
	TAG_STR(ELTAN_HSM)
	TAG_STR(UTIXO_HSM)
	TAG_STR(QUEST_HSM)
	TAG_STR(DATAGLOBAL_HSM)
	TAG_STR(QI_TECH_HSM)
	TAG_STR(DATAFIRST_HSM)
	TAG_STR(C2CSYSTEMS_HSM)
	TAG_STR(WATERFORD)
	TAG_STR(RIVERBED_HSM)
	TAG_STR(CARINGO_HSM)
	TAG_STR(MAXISCALE_HSM)
	TAG_STR(CITRIX_PM)
	TAG_STR(OPENAFS_DFS)
	TAG_STR(ZLTI_HSM)
	TAG_STR(EMC_HSM)
	TAG_STR(VMWARE_PM)
	TAG_STR(ARCO_BACKUP)
	TAG_STR(CARROLL_HSM)
	TAG_STR(COMTRADE_HSM)
	TAG_STR(EASEVAULT_HSM)
	TAG_STR(HDS_HSM)
	TAG_STR(MAGINATICS_RDR)
	TAG_STR(GOOGLE_HSM)
	TAG_STR(QUADDRA_HSM)
	TAG_STR(HP_BACKUP)
	TAG_STR(DROPBOX_HSM)
	TAG_STR(ADOBE_HSM)
	TAG_STR(HP_DATA_PROTECT)
	TAG_STR(ACTIVISION_HSM)
	TAG_STR(HDS_HCP_HSM)
	TAG_STR(AURISTOR_FS)
	TAG_STR(ITSTATION)
	TAG_STR(SPHARSOFT)
	TAG_STR(ALERTBOOT)
	TAG_STR(MTALOS)
	TAG_STR(CTERA_HSM)
	TAG_STR(NIPPON_HSM)
	TAG_STR(REDSTOR_HSM)
	TAG_STR(NEUSHIELD)
	TAG_STR(DOR_HSM)
	TAG_STR(SHX_BACKUP)
	TAG_STR(NVIDIA_UNIONFS)
	TAG_STR(HUBSTOR_HSM)
	TAG_STR(IMANAGE_HSM)
	TAG_STR(EASEFILTER_HSM)
	TAG_STR(ACRONIS_HSM_0)
	TAG_STR(ACRONIS_HSM_1)
	TAG_STR(ACRONIS_HSM_2)
	TAG_STR(ACRONIS_HSM_3)
	TAG_STR(ACRONIS_HSM_4)
	TAG_STR(ACRONIS_HSM_5)
	TAG_STR(ACRONIS_HSM_6)
	TAG_STR(ACRONIS_HSM_7)
	TAG_STR(ACRONIS_HSM_8)
	TAG_STR(ACRONIS_HSM_9)
	TAG_STR(ACRONIS_HSM_A)
	TAG_STR(ACRONIS_HSM_B)
	TAG_STR(ACRONIS_HSM_C)
	TAG_STR(ACRONIS_HSM_D)
	TAG_STR(ACRONIS_HSM_E)
	TAG_STR(ACRONIS_HSM_F)
#undef TAG_STR
	}
}

bool reparse_tag_to_string(DWORD ReparseTag, string& Str, bool const ShowUnknown)
{
	Str = reparse_tag_to_string(ReparseTag);

	if (!Str.empty())
		return true;

	if (ShowUnknown)
		Str = far::format(L":{:0>8X}"sv, ReparseTag);

	return false;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("flink.fill.reparse.buffer")
{
	const auto BufferSize = 100;
	block_ptr<REPARSE_DATA_BUFFER> const Buffer(BufferSize);

	{
		unsigned char const ExpectedData[]
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

		REQUIRE(std::ranges::equal(ExpectedData, std::span(std::bit_cast<unsigned char const*>(Buffer.data()), std::size(ExpectedData))));
	}

	{
		unsigned char const ExpectedData[]
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

		REQUIRE(std::ranges::equal(ExpectedData, std::span(std::bit_cast<unsigned char const*>(Buffer.data()), std::size(ExpectedData))));
	}
}

TEST_CASE("reparse_tag_to_string")
{
	static const struct
	{
		DWORD Tag;
		bool Known;
		string_view Str;
	}
	Tests[]
	{
		// Unknown
		{ 0,                           false, L":00000000"sv },
		{ 1,                           false, L":00000001"sv },
		{ 0xFFFFFFFF,                  false, L":FFFFFFFF"sv },
		// MS
		{ IO_REPARSE_TAG_HSM,          true,  L"HSM"sv },
		{ IO_REPARSE_TAG_SIS,          true,  L"SIS"sv },
		// Non-MS
		{ IO_REPARSE_TAG_MOONWALK_HSM, true,  L"MOONWALK_HSM"sv },
		{ IO_REPARSE_TAG_NIPPON_HSM,   true,  L"NIPPON_HSM"sv },
	};

	string Str;
	for (const auto& i: Tests)
	{
		REQUIRE(reparse_tag_to_string(i.Tag, Str, true) == i.Known);
		REQUIRE(Str == i.Str);
	}
}
#endif

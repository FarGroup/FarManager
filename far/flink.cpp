﻿/*
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

// Self:
#include "flink.hpp"

// Internal:
#include "imports.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "dirmix.hpp"
#include "treelist.hpp"
#include "elevation.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "stddlg.hpp"

// Platform:
#include "platform.fs.hpp"
#include "platform.security.hpp"

// Common:
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

bool CreateVolumeMountPoint(const string& TargetVolume, const string& Object)
{
	string VolumeName;
	return os::fs::GetVolumeNameForVolumeMountPoint(TargetVolume, VolumeName) && SetVolumeMountPoint(Object.c_str(), VolumeName.c_str());
}

static bool FillREPARSE_DATA_BUFFER(REPARSE_DATA_BUFFER* rdb, const string& PrintName, const string& SubstituteName)
{
	switch (rdb->ReparseTag)
	{
	// IO_REPARSE_TAG_MOUNT_POINT and IO_REPARSE_TAG_SYMLINK buffers are filled differently:
	// different order of print and substitute names and additional zero bytes in IO_REPARSE_TAG_MOUNT_POINT.
	// No particular reason to do that, but Windows for some reason does and we just mimic its approach to make
	// reparse points created by Far indistinguishable from created by, say, mklink.

	case IO_REPARSE_TAG_MOUNT_POINT:
		{
			const size_t SubstituteNameOffset = 0;
			const size_t SubstituteNameLength = SubstituteName.size() * sizeof(wchar_t);
			const size_t PrintNameOffset = SubstituteNameLength + 1 * sizeof(wchar_t);
			const size_t PrintNameLength = PrintName.size() * sizeof(wchar_t);
			const size_t ReparseDataLength = offsetof(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) - REPARSE_DATA_BUFFER_HEADER_SIZE + PrintNameOffset + PrintNameLength + 1 * sizeof(wchar_t);

			if (ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE > MAXIMUM_REPARSE_DATA_BUFFER_SIZE)
				return false;

			rdb->ReparseDataLength = static_cast<USHORT>(ReparseDataLength);
			rdb->Reserved = 0;

			auto& Buffer = rdb->MountPointReparseBuffer;
			Buffer.SubstituteNameOffset = static_cast<USHORT>(SubstituteNameOffset);
			Buffer.SubstituteNameLength = static_cast<USHORT>(SubstituteNameLength);
			Buffer.PrintNameOffset = static_cast<USHORT>(PrintNameOffset);
			Buffer.PrintNameLength = static_cast<USHORT>(PrintNameLength);
			*std::copy(ALL_CONST_RANGE(SubstituteName), Buffer.PathBuffer + SubstituteNameOffset / sizeof(wchar_t)) = 0;
			*std::copy(ALL_CONST_RANGE(PrintName), Buffer.PathBuffer + PrintNameOffset / sizeof(wchar_t)) = 0;
			return true;
		}

	case IO_REPARSE_TAG_SYMLINK:
		{
			const size_t PrintNameOffset = 0;
			const size_t PrintNameLength = PrintName.size() * sizeof(wchar_t);
			const size_t SubstituteNameOffset = PrintNameLength;
			const size_t SubstituteNameLength = SubstituteName.size() * sizeof(wchar_t);
			const size_t ReparseDataLength = offsetof(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) - REPARSE_DATA_BUFFER_HEADER_SIZE + SubstituteNameOffset + SubstituteNameLength;

			if (ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE > MAXIMUM_REPARSE_DATA_BUFFER_SIZE)
				return false;

			rdb->ReparseDataLength = static_cast<USHORT>(ReparseDataLength);
			rdb->Reserved = 0;

			auto& Buffer = rdb->SymbolicLinkReparseBuffer;
			Buffer.SubstituteNameOffset = static_cast<USHORT>(SubstituteNameOffset);
			Buffer.SubstituteNameLength = static_cast<USHORT>(SubstituteNameLength);
			Buffer.PrintNameOffset = static_cast<USHORT>(PrintNameOffset);
			Buffer.PrintNameLength = static_cast<USHORT>(PrintNameLength);
			std::copy(ALL_CONST_RANGE(SubstituteName), Buffer.PathBuffer + SubstituteNameOffset / sizeof(wchar_t));
			std::copy(ALL_CONST_RANGE(PrintName), Buffer.PathBuffer + PrintNameOffset / sizeof(wchar_t));
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

static bool SetREPARSE_DATA_BUFFER(const string& Object, REPARSE_DATA_BUFFER* rdb)
{
	if (!IsReparseTagValid(rdb->ReparseTag))
		return false;

	SCOPED_ACTION(os::security::privilege){ SE_CREATE_SYMBOLIC_LINK_NAME };

	const auto Attributes = os::fs::get_file_attributes(Object);
	if(Attributes&FILE_ATTRIBUTE_READONLY)
	{
		os::fs::set_file_attributes(Object, Attributes&~FILE_ATTRIBUTE_READONLY);
	}

	SCOPE_EXIT
	{
		if (Attributes&FILE_ATTRIBUTE_READONLY)
			os::fs::set_file_attributes(Object, Attributes);
	};

	const auto SetBuffer = [&](bool ForceElevation)
	{
		const os::fs::file fObject(Object, GetDesiredAccessForReparsePointChange(), 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr, ForceElevation);
		return fObject && fObject.IoControl(FSCTL_SET_REPARSE_POINT, rdb, rdb->ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE, nullptr, 0);
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

bool CreateReparsePoint(const string& Target, const string& Object,ReparsePointTypes Type)
{
	bool Result=false;

	{
		switch (Type)
		{
			case RP_HARDLINK:
				break;
			case RP_EXACTCOPY:
				Result=DuplicateReparsePoint(Target,Object);
				break;
			case RP_SYMLINK:
			case RP_SYMLINKFILE:
			case RP_SYMLINKDIR:
				{
					os::fs::file_status const ObjectStatus(Object);
					if(Type == RP_SYMLINK)
					{
						Type = os::fs::is_directory(Target)? RP_SYMLINKDIR : RP_SYMLINKFILE;
					}
					if (imports.CreateSymbolicLinkW && !os::fs::exists(ObjectStatus))
					{
						Result=os::fs::CreateSymbolicLink(Object,Target,Type==RP_SYMLINKDIR?SYMBOLIC_LINK_FLAG_DIRECTORY:0);
					}
					else
					{
						const auto ObjectCreated = Type==RP_SYMLINKDIR?
							os::fs::is_directory(ObjectStatus) || os::fs::create_directory(Object) :
							os::fs::is_file(ObjectStatus) || os::fs::file(Object, 0, 0, nullptr, CREATE_NEW);

						if (ObjectCreated)
						{
							const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

							rdb->ReparseTag=IO_REPARSE_TAG_SYMLINK;
							const auto& strPrintName = Target;
							auto strSubstituteName = Target;

							if (IsAbsolutePath(Target))
							{
								strSubstituteName = KernelPath(NTPath(strSubstituteName));
								rdb->SymbolicLinkReparseBuffer.Flags=0;
							}
							else
							{
								rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
							}

							if (FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName))
							{
								Result=SetREPARSE_DATA_BUFFER(Object,rdb.get());
							}
							else
							{
								SetLastError(ERROR_INSUFFICIENT_BUFFER);
							}
						}
					}
				}
				break;
			case RP_JUNCTION:
			case RP_VOLMOUNT:
			{
				const auto strPrintName = ConvertNameToFull(Target);
				const auto strSubstituteName = KernelPath(NTPath(strPrintName));
				const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
				rdb->ReparseTag=IO_REPARSE_TAG_MOUNT_POINT;

				if (FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName))
				{
					Result=SetREPARSE_DATA_BUFFER(Object,rdb.get());
				}
				else
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
				}
			}
			break;
		}
	}

	return Result;
}

static bool GetREPARSE_DATA_BUFFER(const string& Object, REPARSE_DATA_BUFFER* rdb)
{
	const auto FileAttr = os::fs::get_file_attributes(Object);
	if (FileAttr == INVALID_FILE_ATTRIBUTES || !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
		return false;

	const os::fs::file fObject(Object, 0, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!fObject)
		return false;

	return fObject.IoControl(FSCTL_GET_REPARSE_POINT, nullptr, 0, rdb, MAXIMUM_REPARSE_DATA_BUFFER_SIZE) && IsReparseTagValid(rdb->ReparseTag);
}

bool DeleteReparsePoint(const string& Object)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!GetREPARSE_DATA_BUFFER(Object, rdb.get()))
		return false;

	const os::fs::file fObject(Object, GetDesiredAccessForReparsePointChange(), 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!fObject)
		return false;

	REPARSE_GUID_DATA_BUFFER rgdb{rdb->ReparseTag};
	return fObject.IoControl(FSCTL_DELETE_REPARSE_POINT, &rgdb, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, nullptr, 0);
}

bool GetReparsePointInfo(const string& Object, string& DestBuffer, LPDWORD ReparseTag)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!GetREPARSE_DATA_BUFFER(Object, rdb.get()))
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

std::optional<size_t> GetNumberOfLinks(const string& Name)
{
	const os::fs::file File(Name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
	if (!File)
		return {};

	BY_HANDLE_FILE_INFORMATION bhfi;
	if (!File.GetInformation(bhfi))
		return {};

	return bhfi.nNumberOfLinks;
}

bool MkHardLink(const string& ExistingName,const string& NewName, bool Silent)
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

bool EnumStreams(const string& FileName, unsigned long long& StreamsSize, size_t& StreamsCount)
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

bool DelSubstDrive(const string& DeviceName)
{
	string strTargetPath;
	return os::fs::QueryDosDevice(DeviceName, strTargetPath) &&
		DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE, DeviceName.c_str(), strTargetPath.c_str()) != FALSE;
}

bool GetSubstName(int DriveType,const string& DeviceName, string &strTargetPath)
{
	bool Ret=false;
	/*
	+ Обработка в зависимости от Global->Opt->SubstNameRule
	битовая маска:
	0 - если установлен, то опрашивать сменные диски
	1 - если установлен, то опрашивать все остальные
	*/
	const auto DriveRemovable = DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_CDROM;

	if (DriveType==DRIVE_NOT_INIT || (((Global->Opt->SubstNameRule & 1) || !DriveRemovable) && ((Global->Opt->SubstNameRule & 2) || DriveRemovable)))
	{
		const auto Type = ParsePath(DeviceName);
		if (Type == root_type::drive_letter)
		{
			string Name;
			if (os::fs::QueryDosDevice(DeviceName, Name))
			{
				if (starts_with(Name, L"\\??\\UNC\\"sv))
				{
					strTargetPath = concat(L"\\\\"sv, string_view(Name).substr(8));
					Ret = true;
				}
				else if (starts_with(Name, L"\\??\\"sv))
				{
					strTargetPath.assign(Name, 4, string::npos); // gcc 7.3-8.1 bug: npos required. TODO: Remove after we move to 8.2 or later
					Ret=true;
				}
			}
		}
	}

	return Ret;
}

bool GetVHDInfo(const string& DeviceName, string &strVolumePath, VIRTUAL_STORAGE_TYPE* StorageType)
{
	const auto IsDosDevice = DeviceName.size() == 2 && ends_with(DeviceName, L':');
	const os::fs::file Device(IsDosDevice? os::fs::get_unc_drive(DeviceName.front()) : DeviceName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
	if (!Device)
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
		if (Device.GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES, Size, StorageDependencyInfo.get(), &Size))
			break;
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			return false;
	}

	if (!StorageDependencyInfo->NumberEntries)
		return false;

	if(StorageType)
		*StorageType = StorageDependencyInfo->Version2Entries[0].VirtualStorageType;

	// trick: ConvertNameToReal also converts \\?\{GUID} to drive letter, if possible.
	strVolumePath = ConvertNameToReal(concat(StorageDependencyInfo->Version2Entries[0].HostVolumeName, StorageDependencyInfo->Version2Entries[0].DependentVolumeRelativePath));

	return true;
}

string GetPathRoot(string_view const Path)
{
	return ExtractPathRoot(ConvertNameToReal(Path));
}

bool ModifyReparsePoint(const string& Object,const string& NewData)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!GetREPARSE_DATA_BUFFER(Object, rdb.get()))
		return false;

	switch (rdb->ReparseTag)
	{
	case IO_REPARSE_TAG_MOUNT_POINT:
		{
			const auto strPrintName = ConvertNameToFull(NewData);
			const auto strSubstituteName = KernelPath(NTPath(strPrintName));
			if (!FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName))
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}
		}
		break;

	case IO_REPARSE_TAG_SYMLINK:
		{
			const auto& strPrintName = NewData;
			auto strSubstituteName = NewData;

			if (IsAbsolutePath(NewData))
			{
				strSubstituteName = KernelPath(NTPath(strSubstituteName));
				rdb->SymbolicLinkReparseBuffer.Flags=0;
			}
			else
			{
				rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
			}

			if (!FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName))
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}
		}
		break;

	default:
		return false;
	}

	return SetREPARSE_DATA_BUFFER(Object,rdb.get());
}

bool DuplicateReparsePoint(const string& Src,const string& Dst)
{
	const block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	return GetREPARSE_DATA_BUFFER(Src, rdb.get()) && SetREPARSE_DATA_BUFFER(Dst, rdb.get());
}

void NormalizeSymlinkName(string &strLinkName)
{
	if (!starts_with(strLinkName, L"\\??\\"sv))
		return;

	strLinkName[1] = L'\\';
	if (ParsePath(strLinkName) != root_type::unc_drive_letter)
		return;

	strLinkName.erase(0, 4);
}

// Кусок для создания SymLink для каталогов.
int MkSymLink(const string& Target, const string& LinkName, ReparsePointTypes LinkType, bool Silent, bool HoldTarget)
{
	if (!Target.empty() && !LinkName.empty())
	{
		string strFullTarget;
		// выделим имя
		auto strSelOnlyName = Target;
		DeleteEndSlash(strSelOnlyName);
		const auto SlashPos = FindLastSlash(strSelOnlyName);

		const auto symlink = LinkType==RP_SYMLINK || LinkType==RP_SYMLINKFILE || LinkType==RP_SYMLINKDIR;

		if (Target[1] == L':' && (!Target[2] || (IsSlash(Target[2]) && !Target[3]))) // C: или C:/
		{
//      if(Flags&FCOPY_VOLMOUNT)
			{
				strFullTarget = Target;
				AddEndSlash(strFullTarget);
			}
			/*
			  Вот здесь - ну очень умное поведение!
			  Т.е. если в качестве SelName передали "C:", то в этом куске происходит
			  коррекция типа линка - с symlink`а на volmount
			*/
			LinkType=RP_VOLMOUNT;
		}
		else
			strFullTarget = ConvertNameToFull(Target);

		auto strFullLink = ConvertNameToFull(LinkName);

		if (IsSlash(strFullLink.back()))
		{
			if (LinkType != RP_VOLMOUNT)
			{
				const auto SelName = SlashPos != string::npos? string_view(strSelOnlyName).substr(SlashPos + 1) : string_view(strSelOnlyName);
				append(strFullLink, SelName);
			}
			else
			{
				append(strFullLink, L"Disk_"sv, Target.front());
			}
		}

		if (LinkType==RP_VOLMOUNT)
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
				bool CreateDir=true;

				if (LinkType==RP_EXACTCOPY)
				{
					// в этом случае создается или каталог, или пустой файл
					if (os::fs::is_file(strFullTarget))
						CreateDir=false;
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

					return 0;
				}
			}

		if (LinkType!=RP_VOLMOUNT)
		{
			if (CreateReparsePoint(HoldTarget && symlink ? Target : strFullTarget, strFullLink, LinkType))
			{
				return 1;
			}
			else
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

				return 0;
			}
		}
		else
		{
			if (CreateVolumeMountPoint(strFullTarget,strFullLink))
			{
				return 1;
			}
			else
			{
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

				return 0;
			}
		}
	}

	return 2;
}

static string reparse_tag_to_string(DWORD ReparseTag)
{
	switch (ReparseTag)
	{
	case IO_REPARSE_TAG_MOUNT_POINT:                 return msg(lng::MListJunction);
	case IO_REPARSE_TAG_SYMLINK:                     return msg(lng::MListSymlink);
	case IO_REPARSE_TAG_HSM:                         return L"HSM"s;
	case IO_REPARSE_TAG_HSM2:                        return L"HSM2"s;
	case IO_REPARSE_TAG_SIS:                         return L"SIS"s;
	case IO_REPARSE_TAG_WIM:                         return L"WIM"s;
	case IO_REPARSE_TAG_CSV:                         return L"CSV"s;
	case IO_REPARSE_TAG_DFS:                         return L"DFS"s;
	case IO_REPARSE_TAG_DFSR:                        return L"DFSR"s;
	case IO_REPARSE_TAG_DEDUP:                       return L"DEDUP"s;
	case IO_REPARSE_TAG_NFS:                         return L"NFS"s;
	case IO_REPARSE_TAG_FILE_PLACEHOLDER:            return L"FILE PLACEHOLDER"s;
	case IO_REPARSE_TAG_WOF:                         return L"WOF"s;
	case IO_REPARSE_TAG_WCI:                         return L"WCI"s;
	case IO_REPARSE_TAG_WCI_1:                       return L"WCI 1"s;
	case IO_REPARSE_TAG_GLOBAL_REPARSE:              return L"GLOBAL_REPARSE"s;
	case IO_REPARSE_TAG_CLOUD:                       return L"CLOUD"s;
	case IO_REPARSE_TAG_CLOUD_1:                     return L"CLOUD 1"s;
	case IO_REPARSE_TAG_CLOUD_2:                     return L"CLOUD 2"s;
	case IO_REPARSE_TAG_CLOUD_3:                     return L"CLOUD 3"s;
	case IO_REPARSE_TAG_CLOUD_4:                     return L"CLOUD 4"s;
	case IO_REPARSE_TAG_CLOUD_5:                     return L"CLOUD 5"s;
	case IO_REPARSE_TAG_CLOUD_6:                     return L"CLOUD 6"s;
	case IO_REPARSE_TAG_CLOUD_7:                     return L"CLOUD 7"s;
	case IO_REPARSE_TAG_CLOUD_8:                     return L"CLOUD 8"s;
	case IO_REPARSE_TAG_CLOUD_9:                     return L"CLOUD 9"s;
	case IO_REPARSE_TAG_CLOUD_A:                     return L"CLOUD A"s;
	case IO_REPARSE_TAG_CLOUD_B:                     return L"CLOUD B"s;
	case IO_REPARSE_TAG_CLOUD_C:                     return L"CLOUD C"s;
	case IO_REPARSE_TAG_CLOUD_D:                     return L"CLOUD D"s;
	case IO_REPARSE_TAG_CLOUD_E:                     return L"CLOUD E"s;
	case IO_REPARSE_TAG_CLOUD_F:                     return L"CLOUD F"s;
	case IO_REPARSE_TAG_APPEXECLINK:                 return L"APPEXECLINK"s;
	case IO_REPARSE_TAG_PROJFS:                      return L"PROJFS"s;
	case IO_REPARSE_TAG_STORAGE_SYNC:                return L"STORAGE SYNC"s;
	case IO_REPARSE_TAG_WCI_TOMBSTONE:               return L"WCI TOMBSTONE"s;
	case IO_REPARSE_TAG_UNHANDLED:                   return L"UNHANDLED"s;
	case IO_REPARSE_TAG_ONEDRIVE:                    return L"ONEDRIVE"s;
	case IO_REPARSE_TAG_PROJFS_TOMBSTONE:            return L"PROJFS TOMBSTONE"s;
	case IO_REPARSE_TAG_AF_UNIX:                     return L"AF UNIX"s;
	case IO_REPARSE_TAG_LX_SYMLINK:                  return L"LX SYMLINK"s;
	case IO_REPARSE_TAG_LX_FIFO:                     return L"LX FIFO"s;
	case IO_REPARSE_TAG_LX_CHR:                      return L"LX CHR"s;
	case IO_REPARSE_TAG_LX_BLK:                      return L"LX BLK"s;
	case IO_REPARSE_TAG_DRIVE_EXTENDER:              return L"DRIVE EXTENDER"s;
	case IO_REPARSE_TAG_FILTER_MANAGER:              return L"FILTER MANAGER"s;
	case IO_REPARSE_TAG_IIS_CACHE:                   return L"IIS CACHE"s;
	case IO_REPARSE_TAG_APPXSTRM:                    return L"APPXSTRM"s;
	case IO_REPARSE_TAG_DFM:                         return L"DFM"s;
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

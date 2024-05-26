/*
cvtname.cpp

Функций для преобразования имен файлов/путей.
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
#include "cvtname.hpp"

// Internal:
#include "flink.hpp"
#include "pathmix.hpp"
#include "network.hpp"
#include "imports.hpp"
#include "strmix.hpp"
#include "elevation.hpp"
#include "string_utils.hpp"
#include "drivemix.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/enum_substrings.hpp"

// External:

//----------------------------------------------------------------------------

static void MixToFullPath(string& strPath)
{
	//Skip all path to root (with slash if exists)
	size_t DirOffset = 0;
	ParsePath(strPath, &DirOffset);

	//Process "." and ".." if exists
	for (size_t Pos = DirOffset; Pos < strPath.size();)
	{
		//fragment "."
		if (strPath[Pos] != L'.' || (Pos && !path::is_separator(strPath[Pos - 1])))
		{
			++Pos;
			continue;
		}

		//fragment "." at the end
		if(strPath.size() == Pos + 1)
		{
			strPath.resize(Pos);
			// don't change x:\ to x:
			if (strPath[Pos - 2] != L':')
			{
				strPath.pop_back();
			}
			continue;
		}

		switch (strPath[Pos + 1])
		{
			//fragment ".\"
		case L'\\':
			//fragment "./"
		case L'/':
			strPath.erase(Pos, 2);
			continue;

			//fragment "..\" or "../" or ".." at the end
		case L'.':
			if (Pos + 2 == strPath.size() || path::is_separator(strPath[Pos + 2]))
			{
				//Calculate subdir name offset
				size_t n = strPath.find_last_of(path::separators, Pos-2);
				n = (n == string::npos || n < DirOffset) ? DirOffset : n+1;

				//fragment "..\" or "../"
				if (Pos + 2 < strPath.size())
				{
					strPath.erase(n, Pos + 3 - n);
				}
					//fragment ".." at the end
				else
				{
					strPath.resize(n);
				}

				Pos = n;
				continue;
			}
			break;
		}

		++Pos;
	}
}

static void MixToFullPath(const string_view stPath, string& Dest, const string_view stCurrentDir)
{
	string strDest;
	string_view pstCurrentDir;
	bool blIgnore = false;
	size_t PathDirOffset = 0;
	const auto PathType = ParsePath(stPath, &PathDirOffset);
	size_t PathOffset = PathDirOffset;

	switch (PathType)
	{
	case root_type::unknown:
		if (HasPathPrefix(stPath)) // \\?\<ANY_UNKNOWN_FORMAT>
		{
			blIgnore = true;
		}
		else if (!stPath.empty() && path::is_separator(stPath.front())) //"\" or "\abc"
		{
			++PathOffset;
			if (!stCurrentDir.empty())
			{
				size_t CurDirDirOffset = 0;
				if (ParsePath(stCurrentDir, &CurDirDirOffset) != root_type::unknown)
				{
					strDest = stCurrentDir.substr(0, CurDirDirOffset);
				}
			}
		}
		else //"abc" or whatever
		{
			pstCurrentDir = stCurrentDir;
		}
		break;

	case root_type::drive_letter: //"C:" or "C:abc"
		if(stPath.size() > 2 && path::is_separator(stPath[2]))
		{
			PathOffset = 0;
		}
		else
		{
			const auto Drive = os::fs::drive::get_device_path(stPath[0]);
			const auto Value = os::env::get(L'=' + Drive);

			strDest = !Value.empty()?
				Value :
				upper(stPath[0]) == upper(stCurrentDir[0])?
					stCurrentDir :
					Drive;

			AddEndSlash(strDest);
		}
		break;

	case root_type::remote: //"\\abc"
		PathOffset = 0;
		break;

	case root_type::win32nt_drive_letter: //"\\?\whatever"
	case root_type::unc_remote:
	case root_type::volume:
	case root_type::unknown_rootlike:
		blIgnore=true;
		PathOffset = 0;
		break;
	}

	if (!pstCurrentDir.empty())
	{
		append(strDest, pstCurrentDir);
		AddEndSlash(strDest);
	}

	append(strDest, stPath.substr(PathOffset));

	if (!blIgnore && !HasPathPrefix(strDest))
		MixToFullPath(strDest);

	Dest = std::move(strDest);
}

string ConvertNameToFull(string_view const Object)
{
	string strDest;
	MixToFullPath(Object, strDest, os::fs::get_current_directory());
	return strDest;
}

std::optional<wchar_t> get_volume_drive(string_view const VolumePath)
{
	const auto SrcVolumeName = extract_root_directory(VolumePath);

	if (imports.GetVolumePathNamesForVolumeNameW)
	{
		string VolumePathNames;
		if (os::fs::GetVolumePathNamesForVolumeName(SrcVolumeName, VolumePathNames))
		{
			for (const auto& i: enum_substrings(VolumePathNames))
			{
				if (IsRootPath(i))
					return upper(i.front());
			}
		}

		return {};
	}

	string VolumeName;
	const os::fs::enum_drives Enumerator(os::fs::get_logical_drives());

	const auto ItemIterator = std::ranges::find_if(Enumerator, [&](const wchar_t i)
	{
		return os::fs::GetVolumeNameForVolumeMountPoint(os::fs::drive::get_win32nt_root_directory(i), VolumeName) && equal_icase(VolumeName, SrcVolumeName);
	});

	if (ItemIterator != Enumerator.cend())
		return *ItemIterator;

	return {};
}

static void ReplaceVolumeNameWithDriveLetter(string& Path)
{
	size_t DirectoryOffset;

	if (ParsePath(Path, &DirectoryOffset) != root_type::volume)
		return;

	const auto DriveLetter = get_volume_drive(Path);
	if (!DriveLetter)
		return;

	Path.replace(0, DirectoryOffset, os::fs::drive::get_root_directory(*DriveLetter));
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point.
  Note that Src can be partially non-existent.
*/
string ConvertNameToReal(string_view const Object)
{
	const auto PathPrefix = ExtractPathPrefix(Object);

	SCOPED_ACTION(elevation::suppress);

	// Получим сначала полный путь до объекта обычным способом
	const auto FullPath = ConvertNameToFull(Object);

	string Path = FullPath;
	os::fs::file File;

	while (!File.Open(Path, 0, os::fs::file_share_all, nullptr, OPEN_EXISTING))
	{
		if (IsRootPath(Path))
			break;

		Path = ExtractFilePath(Path);
	}

	if (!File)
		return FullPath;

	string FinalFilePath;
	const auto Result = File.GetFinalPathName(FinalFilePath);
	File.Close();

	if (!Result || FinalFilePath.empty())
		return FullPath;

	// append non-existent path part (if present)
	DeleteEndSlash(Path);

	if (FullPath.size() > Path.size() + 1)
		path::append(FinalFilePath, string_view(FullPath).substr(Path.size() + 1));

	ReplaceVolumeNameWithDriveLetter(FinalFilePath);

	// not needed or already there
	if (PathPrefix.empty() || HasPathPrefix(FinalFilePath))
		return FinalFilePath;

	if (PathPrefix.size() == 8) // \\?\UNC\...
	{
		// network -> network
		if (FinalFilePath.starts_with(L"\\\\"sv))
			return PathPrefix + string_view(FinalFilePath).substr(2);

		// network -> local
		return PathPrefix.substr(0, 2) + FinalFilePath;
	}

	// local -> network
	if (FinalFilePath.starts_with(L"\\\\"sv))
		return L"\\\\?\\UNC\\"sv + string_view(FinalFilePath).substr(2);

	// local -> local
	return PathPrefix + FinalFilePath;
}

static string ConvertName(string_view const Object, bool(*Mutator)(string_view, string&))
{
	string strDest;

	if (Mutator(Object, strDest))
		return strDest;

	strDest = Object;

	if (HasPathPrefix(Object) || !Mutator(nt_path(Object), strDest))
		return string(Object);

	switch (ParsePath(strDest))
	{
	case root_type::win32nt_drive_letter:
		strDest.erase(0, 4); // \\?\X:\path -> X:\path
		break;

	case root_type::unc_remote:
		strDest.erase(2, 6); // \\?\UNC\server -> \\server
		break;

	default:
		// should never happen
		break;
	}

	return strDest;
}

string ConvertNameToShort(string_view const Object)
{
	return ConvertName(Object, os::fs::GetShortPathName);
}

string ConvertNameToLong(string_view const Object)
{
	return ConvertName(Object, os::fs::GetLongPathName);
}

string ConvertNameToUNC(string_view const Object)
{
	auto strFileName = ConvertNameToFull(Object);
	// Посмотрим на тип файловой системы
	string strFileSystemName;
	// применяем WNetGetUniversalName для чего угодно, только не для Novell`а
	if (os::fs::GetVolumeInformation(GetPathRoot(strFileName), nullptr, nullptr, nullptr, nullptr, &strFileSystemName) && equal_icase(strFileSystemName, L"NWFS"sv) && strFileName.size() > 1 && strFileName[1] == L':')
	{
		// BugZ#449 - Неверная работа CtrlAltF с ресурсами Novell DS
		// Здесь, если не получилось получить UniversalName и если это
		// мапленный диск - получаем как для меню выбора дисков
		if (string strTemp; DriveLocalToRemoteName(true, strFileName, strTemp))
		{
			const auto SlashPos = FindSlash(strFileName);
			if (SlashPos != string::npos)
				path::append(strTemp, string_view(strFileName).substr(SlashPos + 1));

			strFileName = std::move(strTemp);
		}
	}
	else
	{
		DWORD uniSize = 1024;
		block_ptr<UNIVERSAL_NAME_INFO> uni(uniSize);
		switch (WNetGetUniversalName(strFileName.c_str(), UNIVERSAL_NAME_INFO_LEVEL, uni.data(), &uniSize))
		{
		case NO_ERROR:
			strFileName = uni->lpUniversalName;
			break;

		case ERROR_MORE_DATA:
			uni.reset(uniSize);
			if (WNetGetUniversalName(strFileName.c_str(), UNIVERSAL_NAME_INFO_LEVEL, uni.data(), &uniSize) == NO_ERROR)
				strFileName = uni->lpUniversalName;
			break;
		}
	}

	return ConvertNameToReal(strFileName);
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR
void PrepareDiskPath(string &strPath, bool CheckFullPath)
{
	if (strPath.size() <= 1 || (strPath[1] != L':' && (!path::is_separator(strPath[0]) || !path::is_separator(strPath[1]))))
		return;

	// elevation not required during cosmetic operation
	SCOPED_ACTION(elevation::suppress);

	path::inplace::normalize_separators(strPath);
	const auto DoubleSlash = strPath[1] == L'\\';
	remove_duplicates(strPath, L'\\');
	if(DoubleSlash)
	{
		strPath.insert(0, 1, L'\\');
	}

	if (CheckFullPath)
	{
		strPath = ConvertNameToFull(strPath);

		size_t DirOffset = 0;
		switch (ParsePath(strPath, &DirOffset))
		{
		case root_type::drive_letter:
			strPath[0] = upper(strPath[0]);
			break;

		case root_type::win32nt_drive_letter:
			strPath[4] = upper(strPath[4]);
			break;

		default:
			break;
		}

		const auto StartPos = DirOffset;

		if (StartPos >= strPath.size())
			return;

		string TmpStr;
		TmpStr.reserve(strPath.size());
		size_t LastPos = StartPos;
		const auto EndsWithSlash = path::is_separator(strPath.back());

		for (size_t i = StartPos; i <= strPath.size(); ++i)
		{
			if (!((i < strPath.size() && path::is_separator(strPath[i])) || (i == strPath.size() && !EndsWithSlash)))
				continue;

			os::fs::find_data fd;
			if (os::fs::get_find_data(string_view(strPath).substr(0, i), fd))
			{
				strPath.replace(LastPos, i - LastPos, fd.FileName);
				i += fd.FileName.size() - (i - LastPos);
			}

			if (i != strPath.size())
			{
				LastPos = i + 1;
			}
		}
	}
}

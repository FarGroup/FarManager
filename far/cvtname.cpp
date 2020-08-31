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
		if (strPath[Pos] != L'.' || (Pos && !IsSlash(strPath[Pos - 1])))
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
			if (Pos + 2 == strPath.size() || IsSlash(strPath[Pos + 2]))
			{
				//Calculate subdir name offset
				size_t n = strPath.find_last_of(L"\\/"sv, Pos-2);
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
		else if (!stPath.empty() && IsSlash(stPath.front())) //"\" or "\abc"
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
		if(stPath.size() > 2 && IsSlash(stPath[2]))
		{
			PathOffset = 0;
		}
		else
		{
			const auto Drive = os::fs::get_drive(stPath[0]);
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

	case root_type::unc_drive_letter: //"\\?\whatever"
	case root_type::unc_remote:
	case root_type::volume:
	case root_type::pipe:
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
	MixToFullPath(Object, strDest, os::fs::GetCurrentDirectory());
	return strDest;
}

// try to replace volume GUID (if present) with drive letter
// used by ConvertNameToReal() only
static string TryConvertVolumeGuidToDrivePath(string_view const Path)
{
	size_t DirectoryOffset;

	if (ParsePath(Path, &DirectoryOffset) != root_type::volume)
		return string(Path);

	if (imports.GetVolumePathNamesForVolumeNameW)
	{
		string VolumePathNames;
		if (os::fs::GetVolumePathNamesForVolumeName(ExtractPathRoot(Path), VolumePathNames))
		{
			for (const auto& i: enum_substrings(VolumePathNames.c_str()))
			{
				if (IsRootPath(i))
					return concat(i, Path.substr(DirectoryOffset));
			}
		}

		return string(Path);
	}

	string strVolumeGuid;
	const os::fs::enum_drives Enumerator(os::fs::get_logical_drives());

	if (const auto ItemIterator = std::find_if(ALL_CONST_RANGE(Enumerator), [&](const wchar_t i)
	{
		return os::fs::GetVolumeNameForVolumeMountPoint(os::fs::get_root_directory(i), strVolumeGuid) &&
			starts_with(Path, string_view(strVolumeGuid).substr(0, DirectoryOffset));
	}); ItemIterator != Enumerator.cend())
		return concat(os::fs::get_root_directory(*ItemIterator), Path.substr(DirectoryOffset));

	return string(Path);
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point.
  Note that Src can be partially non-existent.
*/
string ConvertNameToReal(string_view const Object)
{
	SCOPED_ACTION(elevation::suppress);

	// Получим сначала полный путь до объекта обычным способом
	const auto FullPath = ConvertNameToFull(Object);
	auto strDest = FullPath;

	string Path = FullPath;
	os::fs::file File;

	for (;;)
	{
		if (File.Open(Path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
			break;

		if (IsRootPath(Path))
			break;

		Path = ExtractFilePath(Path);
	}

	if (!File)
		return strDest;

	string FinalFilePath;
	const auto Result = File.GetFinalPathName(FinalFilePath);
	File.Close();

	//assert(!FinalFilePath.empty());

	if (!Result || FinalFilePath.empty())
		return strDest;

	// append non-existent path part (if present)
	DeleteEndSlash(Path);

	if (FullPath.size() > Path.size() + 1)
		path::append(FinalFilePath, string_view(FullPath).substr(Path.size() + 1));

	return TryConvertVolumeGuidToDrivePath(FinalFilePath);
}

static string ConvertName(string_view const Object, bool(*Mutator)(string_view, string&))
{
	string strDest;

	if (Mutator(Object, strDest))
		return strDest;

	strDest = Object;

	if (HasPathPrefix(Object) || !Mutator(NTPath(Object), strDest))
		return string(Object);

	switch (ParsePath(strDest))
	{
	case root_type::unc_drive_letter:
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
		string strTemp;
		if (DriveLocalToRemoteName(DRIVE_UNKNOWN, strFileName[0], strTemp))
		{
			const auto SlashPos = FindSlash(strFileName);
			if (SlashPos != string::npos)
				path::append(strTemp, string_view(strFileName).substr(SlashPos + 1));

			strFileName = strTemp;
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
	if (strPath.size() <= 1 || (strPath[1] != L':' && (!IsSlash(strPath[0]) || !IsSlash(strPath[1]))))
		return;

	// elevation not required during cosmetic operation
	SCOPED_ACTION(elevation::suppress);

	ReplaceSlashToBackslash(strPath);
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
		case root_type::unknown:
			if (HasPathPrefix(strPath))
				DirOffset = 4;
			break;

		case root_type::drive_letter:
			strPath[0] = upper(strPath[0]);
			break;

		case root_type::unc_drive_letter:
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
		const auto EndsWithSlash = IsSlash(strPath.back());

		for (size_t i = StartPos; i <= strPath.size(); ++i)
		{
			if (!((i < strPath.size() && IsSlash(strPath[i])) || (i == strPath.size() && !EndsWithSlash)))
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

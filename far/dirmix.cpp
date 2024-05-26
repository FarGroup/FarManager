/*
dirmix.cpp

Misc functions for working with directories
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
#include "dirmix.hpp"

// Internal:
#include "cvtname.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "pathmix.hpp"
#include "interf.hpp"
#include "elevation.hpp"
#include "network.hpp"
#include "string_utils.hpp"

// Platform:
#include "platform.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static auto make_curdir_name(wchar_t Drive)
{
	return far::format(L"={}:"sv, upper(Drive));
}

static auto env_get_current_dir(wchar_t Drive)
{
	return os::env::get(make_curdir_name(Drive));
}

static auto env_set_current_dir(wchar_t Drive, const string_view Value)
{
	return os::env::set(make_curdir_name(Drive), Value);
}

void set_drive_env_curdir(string_view const Directory)
{
	if (Directory.size() > 1 && Directory[1] == L':')
		env_set_current_dir(Directory[0], Directory);
}

bool FarChDir(string_view const NewDir)
{
	if (NewDir.empty())
		return false;

	string Directory;

	// если указана только буква диска, то путь возьмем из переменной
	if (NewDir.size() == 2 && NewDir[1] == L':')
	{
		Directory = env_get_current_dir(NewDir[0]);
		if (Directory.empty())
			Directory = NewDir;
	}
	else
	{
		Directory = ConvertNameToFull(NewDir);
	}

	AddEndSlash(Directory);
	path::inplace::normalize_separators(Directory);
	PrepareDiskPath(Directory, false); // resolving not needed, very slow

	const auto PathType = ParsePath(Directory);

	const auto IsNetworkPath = PathType == root_type::remote || PathType == root_type::unc_remote;

	if (os::fs::set_current_directory(Directory))
	{
		set_drive_env_curdir(Directory);
		return true;
	}

	const auto LastError = GetLastError();
	if (LastError != ERROR_PATH_NOT_FOUND && LastError != ERROR_ACCESS_DENIED && LastError != ERROR_LOGON_FAILURE)
		return false;

	{
		SCOPED_ACTION(os::last_error_guard);

		const auto IsDrive = PathType == root_type::drive_letter || PathType == root_type::win32nt_drive_letter;
		const auto IsNetworkDrive = IsDrive && os::fs::drive::is_standard_letter(Directory[0]) && GetSavedNetworkDrives()[os::fs::drive::get_number(Directory[0])];

		if (!IsNetworkDrive && !IsNetworkPath)
			return false;
	}

	ConnectToNetworkResource(Directory);

	if (os::fs::set_current_directory(Directory))
	{
		set_drive_env_curdir(Directory);
		return true;
	}

	return false;
}

/*
  Функция TestFolder возвращает одно состояний тестируемого каталога:

    TSTFLD_NOTEMPTY   (2) - не пусто
    TSTFLD_EMPTY      (1) - пусто
    TSTFLD_NOTFOUND   (0) - нет такого
    TSTFLD_NOTACCESS (-1) - нет доступа
    TSTFLD_ERROR     (-2) - ошибка (кривые параметры или не хватило памяти для выделения промежуточных буферов)
*/
int TestFolder(string_view const Path)
{
	if (Path.empty())
		return TSTFLD_ERROR;

	// первая проверка - че-нить считать можем?
	if (os::fs::is_not_empty_directory(Path))
		return TSTFLD_NOTEMPTY;

	const auto ErrorState = os::last_error();
	const auto LastError = ErrorState.Win32Error;
	if (LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES)
		return TSTFLD_EMPTY;

	if (LastError == ERROR_PATH_NOT_FOUND)
		return TSTFLD_NOTFOUND;

	// собственно... не факт, что диск не читаем, т.к. на чистом диске в корне нету даже "."
	// поэтому посмотрим на Root
	const auto Root = GetPathRoot(Path);
	if (Root == Path)
	{
		// проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
		if (os::fs::exists(Root))
		{
			if (LastError == ERROR_ACCESS_DENIED)
				return TSTFLD_NOTACCESS;

			return TSTFLD_EMPTY;
		}
	}

	if (!os::fs::exists(Path))
	{
		return TSTFLD_NOTFOUND;
	}

	{
		SCOPED_ACTION(elevation::suppress);
		if (os::fs::is_file(Path))
			return TSTFLD_ERROR;
	}
	return TSTFLD_NOTACCESS;
}

bool CutToExistingParent(string_view& Path)
{
	for (auto PathView = Path; ;)
	{
		if (!CutToParent(PathView))
			return false;

		if (!os::fs::exists(PathView))
			continue;

		Path.remove_suffix(Path.size() - PathView.size());
		return true;
	}
}

bool CutToExistingParent(string& Path)
{
	string_view PathView = Path;
	if (!CutToExistingParent(PathView))
		return false;

	Path.resize(PathView.size());
	return true;
}

bool TryParentFolder(string& Path)
{
	const auto ErrorState = os::last_error();
	if (Message(MSG_WARNING, ErrorState,
		msg(lng::MError),
		{
			Path,
			msg(lng::MNeedNearPath)
		},
		{ lng::MHYes, lng::MHNo }) != message_result::first_button)
		return false;

	return CutToExistingParent(Path);
}

bool CreatePath(string_view const InputPath, bool const AddToTreeCache)
{
	const auto Path = ConvertNameToFull(InputPath);

	size_t DirOffset = 0;
	ParsePath(Path, &DirOffset);

	for (const auto i: std::views::iota(DirOffset, Path.size() + 1))
	{
		if (i != Path.size() && !path::is_separator(Path[i]))
			continue;

		const auto Part = string_view(Path).substr(0, i);

		if (os::fs::is_directory(Part))
			continue;

		if (!os::fs::create_directory(Part))
			return false;

		if (AddToTreeCache)
			TreeList::AddTreeName(Part);
	}

	return true;
}

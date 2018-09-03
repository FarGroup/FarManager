﻿/*
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

#include "dirmix.hpp"

#include "cvtname.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "elevation.hpp"
#include "network.hpp"
#include "string_utils.hpp"

#include "platform.env.hpp"
#include "platform.fs.hpp"

#include "format.hpp"

static auto make_curdir_name(wchar_t Drive)
{
	return format(L"={0}:", upper(Drive));
}

static auto env_get_current_dir(wchar_t Drive)
{
	return os::env::get(make_curdir_name(Drive));
}

static auto env_set_current_dir(wchar_t Drive, const string_view Value)
{
	return os::env::set(make_curdir_name(Drive), Value);
}

bool FarChDir(string_view const NewDir, bool ChangeDir)
{
	if (NewDir.empty())
		return false;

	bool rc = false;
	string strCurDir;

	bool IsNetworkDrive = false;

	// если указана только буква диска, то путь возьмем из переменной
	if (NewDir.size() == 2 && NewDir[1]==L':')
	{
		strCurDir = env_get_current_dir(NewDir[0]);
		if (strCurDir.empty())
		{
			assign(strCurDir, NewDir);
			AddEndSlash(strCurDir);
			ReplaceSlashToBackslash(strCurDir);
		}

		if (ChangeDir)
		{
			rc=os::fs::SetCurrentDirectory(strCurDir);
		}

		if (!rc && GetLastError() == ERROR_PATH_NOT_FOUND)
		{
			IsNetworkDrive = os::fs::is_standard_drive_letter(NewDir[0]) && GetSavedNetworkDrives()[os::fs::get_drive_number(NewDir[0])];
		}
	}
	else
	{
		if (ChangeDir)
		{
			strCurDir = ConvertNameToFull(NewDir);
			ReplaceSlashToBackslash(strCurDir);
			AddEndSlash(strCurDir);
			PrepareDiskPath(strCurDir,false); // resolving not needed, very slow
			rc=os::fs::SetCurrentDirectory(strCurDir);
		}
	}

	if (!rc && (IsNetworkDrive || GetLastError() == ERROR_LOGON_FAILURE))
	{
		ConnectToNetworkResource(strCurDir);
		rc = os::fs::SetCurrentDirectory(strCurDir);
	}

	if (rc || !ChangeDir)
	{
		if (ChangeDir)
			strCurDir = os::fs::GetCurrentDirectory();

		if (strCurDir.size() > 1 && strCurDir[1]==L':')
		{
			env_set_current_dir(strCurDir[0], strCurDir);
		}
	}

	return rc;
}

/*
  Функция TestFolder возвращает одно состояний тестируемого каталога:

    TSTFLD_NOTEMPTY   (2) - не пусто
    TSTFLD_EMPTY      (1) - пусто
    TSTFLD_NOTFOUND   (0) - нет такого
    TSTFLD_NOTACCESS (-1) - нет доступа
    TSTFLD_ERROR     (-2) - ошибка (кривые параметры или не хватило памяти для выделения промежуточных буферов)
*/
int TestFolder(const string& Path)
{
	if (Path.empty())
		return TSTFLD_ERROR;

	// первая проверка - че-нить считать можем?
	const auto Find = os::fs::enum_files(path::join(Path, L'*'));
	if (Find.begin() != Find.end())
	{
		return TSTFLD_NOTEMPTY;
	}

	const auto ErrorState = error_state::fetch();
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

/*
   Проверка пути или хост-файла на существование
   Если идет проверка пути (TryClosest=true), то будет
   предпринята попытка найти ближайший путь. Результат попытки
   возвращается в переданном TestPath.
*/
bool CheckShortcutFolder(string& TestPath, bool TryClosest, bool Silent)
{
	if (os::fs::exists(TestPath))
		return true;

	SetLastError(ERROR_PATH_NOT_FOUND);
	const auto ErrorState = error_state::fetch();

	auto Target = TestPath;
	TruncPathStr(Target, ScrX - 16);

	if (!TryClosest)
	{
		if (!Silent)
		{
			Message(MSG_WARNING, ErrorState,
				msg(lng::MError),
				{
					Target
				},
				{ lng::MOk });
		}
		return false;
	}

	// попытка найти!
	if (Silent || Message(MSG_WARNING, ErrorState,
		msg(lng::MError),
		{
			Target,
			msg(lng::MNeedNearPath)
		},
		{ lng::MHYes, lng::MHNo }) == Message::first_button)
	{
		auto TestPathTemp = TestPath;
		for (;;)
		{
			if (!CutToParent(TestPathTemp))
				break;

			if (os::fs::exists(TestPathTemp))
			{
				TestPath = TestPathTemp;
				return true;
			}
		}
	}

	return false;
}

void CreatePath(const string &InputPath, bool Simple)
{
	const auto Path = ConvertNameToFull(InputPath);
	size_t DirOffset = 0;
	ParsePath(Path, &DirOffset);
	string Part;
	Part.reserve(Path.size());
	for (size_t i = DirOffset; i <= Path.size(); ++i)
	{
		if (i == Path.size() || IsSlash(Path[i]))
		{
			Part.assign(Path, 0, i);
			if (!os::fs::exists(Part))
			{
				if(os::fs::create_directory(Part) && !Simple)
					TreeList::AddTreeName(Part);
			}
		}
	}
}

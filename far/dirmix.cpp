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

#include "headers.hpp"
#pragma hdrstop

#include "dirmix.hpp"
#include "cvtname.hpp"
#include "message.hpp"
#include "language.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "elevation.hpp"

BOOL FarChDir(const string& NewDir, BOOL ChangeDir)
{
	if (NewDir.empty())
		return FALSE;

	bool rc = false;
	string Drive(L"=A:");
	string strCurDir;

	// если указана только буква диска, то путь возьмем из переменной
	if (NewDir.size() == 2 && NewDir[1]==L':')
	{
		Drive[1] = ToUpper(NewDir[0]);

		if (!os::env::get_variable(Drive, strCurDir))
		{
			strCurDir = NewDir;
			AddEndSlash(strCurDir);
			ReplaceSlashToBackslash(strCurDir);
		}

		if (ChangeDir)
		{
			rc=os::SetCurrentDirectory(strCurDir);
		}
	}
	else
	{
		if (ChangeDir)
		{
			strCurDir = NewDir;

			if (strCurDir == L"\\")
				strCurDir = os::GetCurrentDirectory(); // здесь берем корень

			ReplaceSlashToBackslash(strCurDir);
			ConvertNameToFull(NewDir,strCurDir);
			PrepareDiskPath(strCurDir,false); // resolving not needed, very slow
			rc=os::SetCurrentDirectory(strCurDir);
		}
	}

	if (rc || !ChangeDir)
	{
		if (ChangeDir)
			strCurDir = os::GetCurrentDirectory();

		if (strCurDir.size() > 1 && strCurDir[1]==L':')
		{
			Drive[1] = ToUpper(strCurDir[0]);
			os::env::set_variable(Drive, strCurDir);
		}
	}

	return rc;
}

/*
  ‘ункци€ TestFolder возвращает одно состо€ний тестируемого каталога:

    TSTFLD_NOTEMPTY   (2) - не пусто
    TSTFLD_EMPTY      (1) - пусто
    TSTFLD_NOTFOUND   (0) - нет такого
    TSTFLD_NOTACCESS (-1) - нет доступа
    TSTFLD_ERROR     (-2) - ошибка (кривые параметры или не хватило пам€ти дл€ выделени€ промежуточных буферов)
*/
int TestFolder(const string& Path)
{
	if (Path.empty())
		return TSTFLD_ERROR;

	string strFindPath = Path;
	// сообразим маску дл€ поиска.
	AddEndSlash(strFindPath);
	strFindPath += L"*";

	// перва€ проверка - че-нить считать можем?
	os::fs::enum_file Find(strFindPath);
	if (Find.begin() != Find.end())
	{
		return TSTFLD_NOTEMPTY;
	}

	Global->CatchError();
	DWORD LastError = Global->CaughtError();
	if (LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES)
		return TSTFLD_EMPTY;

	if (LastError == ERROR_PATH_NOT_FOUND)
		return TSTFLD_NOTFOUND;

	// собственно... не факт, что диск не читаем, т.к. на чистом диске в корне нету даже "."
	// поэтому посмотрим на Root
	GetPathRoot(Path,strFindPath);

	if (strFindPath == Path)
	{
		// проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
		if (os::fs::exists(strFindPath))
		{
			if (LastError == ERROR_ACCESS_DENIED)
				return TSTFLD_NOTACCESS;

			return TSTFLD_EMPTY;
		}
	}

	strFindPath = Path;

	if (!os::fs::exists(strFindPath))
	{
		return TSTFLD_NOTFOUND;
	}

	{
		SCOPED_ACTION(elevation::suppress);
		if (os::fs::is_file(strFindPath))
			return TSTFLD_ERROR;
	}
	return TSTFLD_NOTACCESS;
}

/*
   ѕроверка пути или хост-файла на существование
   ≈сли идет проверка пути (TryClosest=true), то будет
   предприн€та попытка найти ближайший путь. –езультат попытки
   возвращаетс€ в переданном TestPath.
*/
bool CheckShortcutFolder(string& pTestPath, bool TryClosest, bool Silent)
{
	bool Result = os::fs::exists(pTestPath);

	if (!Result)
	{
		SetLastError(ERROR_PATH_NOT_FOUND);
		Global->CatchError();

		string strTarget = pTestPath;
		TruncPathStr(strTarget, ScrX-16);

		if (!TryClosest)
		{
			if (!Silent)
				Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), strTarget.data(), MSG(MOk));
		}
		else // попытка найти!
		{
			if (Silent || !Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), strTarget.data(), MSG(MNeedNearPath), MSG(MHYes),MSG(MHNo)))
			{
				string strTestPathTemp = pTestPath;
				for (;;)
				{
					if (!CutToParent(strTestPathTemp))
						break;

					if (os::fs::exists(strTestPathTemp))
					{
						pTestPath = strTestPathTemp;
						Result = true;
						break;
					}
				}
			}
		}
	}

	return Result;
}

void CreatePath(const string &InputPath, bool Simple)
{
	string Path(InputPath);
	ConvertNameToFull(InputPath, Path);
	size_t DirOffset = 0;
	ParsePath(Path, &DirOffset);
	string Part;
	Part.reserve(Path.size());
	for (size_t i = DirOffset; i <= Path.size(); ++i)
	{
		if (i == Path.size() || IsSlash(Path[i]))
		{
			Part = Path.substr(0, i);
			if (!os::fs::exists(Part))
			{
				if(os::CreateDirectory(Part, nullptr) && !Simple)
					TreeList::AddTreeName(Part);
			}
		}
	}
}

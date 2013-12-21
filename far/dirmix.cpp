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
#include "lasterror.hpp"
#include "flink.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "treelist.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "elevation.hpp"

BOOL FarChDir(const string& NewDir, BOOL ChangeDir)
{
	if (NewDir.empty())
		return FALSE;

	BOOL rc=FALSE;
	string Drive(L"=A:");
	string strCurDir;

	// если указана только буква диска, то путь возьмем из переменной
	if (NewDir.size() == 2 && NewDir[1]==L':')
	{
		Drive[1] = Upper(NewDir[0]);

		if (!api::GetEnvironmentVariable(Drive, strCurDir))
		{
			strCurDir = NewDir;
			AddEndSlash(strCurDir);
			ReplaceSlashToBSlash(strCurDir);
		}

		//*CurDir=toupper(*CurDir); бред!
		if (ChangeDir)
		{
			rc=api::SetCurrentDirectory(strCurDir);
		}
	}
	else
	{
		if (ChangeDir)
		{
			strCurDir = NewDir;

			if (strCurDir == L"\\")
				api::GetCurrentDirectory(strCurDir); // здесь берем корень

			ReplaceSlashToBSlash(strCurDir);
			ConvertNameToFull(NewDir,strCurDir);
			PrepareDiskPath(strCurDir,false); // resolving not needed, very slow
			rc=api::SetCurrentDirectory(strCurDir);
		}
	}

	if (rc || !ChangeDir)
	{
		if ((!ChangeDir || api::GetCurrentDirectory(strCurDir)) &&
		        strCurDir.size() > 1 && strCurDir[1]==L':')
		{
			Drive[1] = Upper(strCurDir[0]);
			api::SetEnvironmentVariable(Drive, strCurDir);
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
    TSTFLD_ERROR     (-2) - ошибка (кривые параметры или нехватило памяти для выделения промежуточных буферов)
*/
int TestFolder(const string& Path)
{
	if (Path.empty())
		return TSTFLD_ERROR;

	string strFindPath = Path;
	// сообразим маску для поиска.
	AddEndSlash(strFindPath);
	strFindPath += L"*";

	// первая проверка - че-нить считать можем?
	api::FindFile Find(strFindPath);
	auto ItemIterator = Find.begin();
	if (ItemIterator != Find.end())
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
		if (api::GetFileAttributes(strFindPath)!=INVALID_FILE_ATTRIBUTES)
		{
			if (LastError == ERROR_ACCESS_DENIED)
				return TSTFLD_NOTACCESS;

			return TSTFLD_EMPTY;
		}
	}

	strFindPath = Path;

	if (CheckShortcutFolder(&strFindPath,FALSE,TRUE))
	{
		if (strFindPath != Path)
			return TSTFLD_NOTFOUND;
	}

	{
		DisableElevation de;

		DWORD Attr=api::GetFileAttributes(strFindPath);

		if (Attr!=INVALID_FILE_ATTRIBUTES && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
			return TSTFLD_ERROR;
	}
	return TSTFLD_NOTACCESS;
}

/*
   Проверка пути или хост-файла на существование
   Если идет проверка пути (IsHostFile=FALSE), то будет
   предпринята попытка найти ближайший путь. Результат попытки
   возвращается в переданном TestPath.

   Return: 0 - бЯда.
           1 - ОБИ!,
          -1 - Почти что ОБИ, но ProcessPluginEvent вернул TRUE
   TestPath может быть пустым, тогда просто исполним ProcessPluginEvent()

*/
int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent)
{
	if (pTestPath && !pTestPath->empty() && api::GetFileAttributes(*pTestPath) == INVALID_FILE_ATTRIBUTES)
	{
		int FoundPath=0;
		string strTarget = *pTestPath;
		TruncPathStr(strTarget, ScrX-16);

		if (IsHostFile)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);
			Global->CatchError();

			if (!Silent)
				Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), strTarget.data(), MSG(MOk));
		}
		else // попытка найти!
		{
			SetLastError(ERROR_PATH_NOT_FOUND);
			Global->CatchError();

			if (Silent || !Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), strTarget.data(), MSG(MNeedNearPath), MSG(MHYes),MSG(MHNo)))
			{
				string strTestPathTemp = *pTestPath;

				for (;;)
				{
					if (!CutToSlash(strTestPathTemp,true))
						break;

					if (api::GetFileAttributes(strTestPathTemp) != INVALID_FILE_ATTRIBUTES)
					{
						int ChkFld=TestFolder(strTestPathTemp);

						if (ChkFld == TSTFLD_EMPTY || ChkFld == TSTFLD_NOTEMPTY || ChkFld == TSTFLD_NOTACCESS)
						{
							if (!(pTestPath->size() > 1 && pTestPath->at(0) == L'\\' && pTestPath->at(1) == L'\\' && strTestPathTemp.size() == 1))
							{
								*pTestPath = strTestPathTemp;

								if (pTestPath->size() == 2) // для случая "C:", иначе попадем в текущий каталог диска C:
									AddEndSlash(*pTestPath);

								FoundPath=1;
							}

							break;
						}
					}
				}
			}
		}

		if (!FoundPath)
			return 0;
	}

	if (Global->CtrlObject->Cp()->ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		return -1;

	return 1;
}

void CreatePath(const string &Path, bool Simple)
{
	size_t DirOffset = 0;
	ParsePath(Path, &DirOffset);
	string Part;
	Part.reserve(Path.size());
	for (size_t i = DirOffset + (IsSlash(Path[DirOffset])? 1 : 0); i <= Path.size(); ++i)
	{
		if (i == Path.size() || IsSlash(Path[i]))
		{
			Part = Path.substr(0, i);
			if (api::GetFileAttributes(Part) == INVALID_FILE_ATTRIBUTES)
			{
				if (!Simple && Global->Opt->CreateUppercaseFolders && !IsCaseMixed(Part)) //BUGBUG
					Upper(Part);

				if(api::CreateDirectory(Part, nullptr) && !Simple)
					TreeList::AddTreeName(Part);
			}
		}
	}
}

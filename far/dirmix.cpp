/*
dirmix.cpp

Misc functions for working with directories
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
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

BOOL FarChDir(const wchar_t *NewDir, BOOL ChangeDir)
{
  if(!NewDir || *NewDir == 0)
    return FALSE;

  BOOL rc=FALSE;
  wchar_t Drive[4]=L"=A:";

  string strCurDir;

  if(*NewDir && NewDir[1]==L':' && NewDir[2]==0)// если указана только
  {                                                     // буква диска, то путь
    Drive[1]=Upper(*NewDir);                          // возьмем из переменной

    if ( !apiGetEnvironmentVariable (Drive, strCurDir) )
    {
      strCurDir = NewDir;
      AddEndSlash(strCurDir);
      ReplaceSlashToBSlash(strCurDir);
    }
    //*CurDir=toupper(*CurDir); бред!
    if(ChangeDir)
    {
      rc=apiSetCurrentDirectory(strCurDir);
    }
  }
  else
  {
    if(ChangeDir)
    {
    strCurDir = NewDir;

    if(!StrCmp(strCurDir,L"\\"))
      apiGetCurrentDirectory(strCurDir); // здесь берем корень

    ReplaceSlashToBSlash(strCurDir);
		ConvertNameToFull(NewDir,strCurDir);
    PrepareDiskPath(strCurDir,FALSE); // TRUE ???
    rc=apiSetCurrentDirectory(strCurDir);
    }
  }

  if(rc || !ChangeDir)
  {
    if ((!ChangeDir || apiGetCurrentDirectory(strCurDir)) &&
      strCurDir.At(0) && strCurDir.At(1)==L':')
    {
      Drive[1]=Upper(strCurDir.At(0));
			SetEnvironmentVariable(Drive,strCurDir);
    }
  }
  return rc;
}

/*
  Функция CheckFolder возвращает одно состояний тестируемого каталога:

    CHKFLD_NOTFOUND   (2) - нет такого
    CHKFLD_NOTEMPTY   (1) - не пусто
    CHKFLD_EMPTY      (0) - пусто
    CHKFLD_NOTACCESS (-1) - нет доступа
    CHKFLD_ERROR     (-2) - ошибка (параметры - дерьмо или нехватило памяти для выделения промежуточных буферов)
*/
int CheckFolder(const wchar_t *Path)
{
  if(!(Path && *Path)) // проверка на вшивость
    return CHKFLD_ERROR;

  HANDLE FindHandle;
  FAR_FIND_DATA_EX fdata;
  int Done=FALSE;

  string strFindPath = Path;

  // сообразим маску для поиска.
  AddEndSlash(strFindPath);

	strFindPath += L"*";

  // первая проверка - че-нить считать можем?
  if((FindHandle=apiFindFirstFile(strFindPath,&fdata)) == INVALID_HANDLE_VALUE)
  {
    GuardLastError lstError;
    if(lstError.Get() == ERROR_FILE_NOT_FOUND)
      return CHKFLD_EMPTY;

    // собственно... не факт, что диск не читаем, т.к. на чистом диске в корне нету даже "."
    // поэтому посмотрим на Root
    GetPathRoot(Path,strFindPath);

    if(!StrCmp(Path,strFindPath))
    {
      // проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
      if(apiGetFileAttributes(strFindPath)!=INVALID_FILE_ATTRIBUTES)
      {
        if(lstError.Get() == ERROR_ACCESS_DENIED)
          return CHKFLD_NOTACCESS;
        return CHKFLD_EMPTY;
      }
    }

    strFindPath = Path;

    if(CheckShortcutFolder(&strFindPath,FALSE,TRUE))
    {
      if(StrCmp(Path,strFindPath))
        return CHKFLD_NOTFOUND;
    }

    return CHKFLD_NOTACCESS;
  }

  // Ок. Что-то есть. Попробуем ответить на вопрос "путой каталог?"
  while(!Done)
  {
    if (fdata.strFileName.At(0) == L'.' && (fdata.strFileName.At(1) == 0 || (fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0)))
      ; // игнорируем "." и ".."
    else
    {
      // что-то есть, отличное от "." и ".." - каталог не пуст
      apiFindClose(FindHandle);
      return CHKFLD_NOTEMPTY;
    }
    Done=!apiFindNextFile(FindHandle,&fdata);
  }

  // однозначно каталог пуст
  apiFindClose(FindHandle);
  return CHKFLD_EMPTY;
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
  if( pTestPath && !pTestPath->IsEmpty() && apiGetFileAttributes(*pTestPath) == INVALID_FILE_ATTRIBUTES)
  {
    int FoundPath=0;

    string strTarget = *pTestPath;

    TruncPathStr(strTarget, ScrX-16);

    if(IsHostFile)
    {
      SetLastError(ERROR_FILE_NOT_FOUND);
      if(!Silent)
        Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG (MError), strTarget, MSG (MOk));
    }
    else // попытка найти!
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      if(Silent || Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG (MError), strTarget, MSG (MNeedNearPath), MSG(MHYes),MSG(MHNo)) == 0)
      {
        string strTestPathTemp = *pTestPath;

				for(;;)
        {
          if (!CutToSlash(strTestPathTemp,true))
            break;

          if(apiGetFileAttributes(strTestPathTemp) != INVALID_FILE_ATTRIBUTES)
          {
            int ChkFld=CheckFolder(strTestPathTemp);
            if(ChkFld > CHKFLD_ERROR && ChkFld < CHKFLD_NOTFOUND)
            {
              if(!(pTestPath->At(0) == L'\\' && pTestPath->At(1) == L'\\' && strTestPathTemp.At(1) == 0))
              {
                *pTestPath = strTestPathTemp;

                if( pTestPath->GetLength() == 2) // для случая "C:", иначе попадем в текущий каталог диска C:
                  AddEndSlash(*pTestPath);
                FoundPath=1;
              }
              break;
            }
          }
        }
      }
    }
    if(!FoundPath)
      return 0;
  }
  if(CtrlObject->Cp()->ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return -1;
  return 1;
}

void CreatePath(string &strPath)
{
  wchar_t *ChPtr = strPath.GetBuffer ();
  wchar_t *DirPart = ChPtr;

  BOOL bEnd = FALSE;

	for(;;)
  {
		if ( (*ChPtr == 0) || IsSlash(*ChPtr) )
    {
      if ( *ChPtr == 0 )
        bEnd = TRUE;

      *ChPtr = 0;

			if ( Opt.CreateUppercaseFolders && !IsCaseMixed(DirPart) && apiGetFileAttributes(strPath) == INVALID_FILE_ATTRIBUTES) //BUGBUG
				CharUpper(DirPart);

			if ( apiCreateDirectory(strPath, NULL) )
        TreeList::AddTreeName(strPath);

      if ( bEnd )
        break;

      *ChPtr = L'\\';
      DirPart = ChPtr+1;
    }

    ChPtr++;
  }
	strPath.ReleaseBuffer();
}

/*
cvtname.cpp

Функций для преобразования имен файлов/путей.
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

#include "fn.hpp"
#include "flink.hpp"
#include "syslog.hpp"

int ConvertNameToFull (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strSrc = lpwszSrc; //копирование в другую переменную на случай dest == src
	lpwszSrc = strSrc;

	const wchar_t *lpwszName = PointToName(lpwszSrc);

	if ( (lpwszName == lpwszSrc) &&
				!TestParentFolderName(lpwszName) && !TestCurrentFolderName(lpwszName))
	{
		apiGetCurrentDirectory(strDest);
		AddEndSlash(strDest);

		strDest += lpwszSrc;

		return (int)strDest.GetLength ();
	}

	if ( PathMayBeAbsolute(lpwszSrc) )
	{
		if ( *lpwszName &&
				(*lpwszName != L'.' || (lpwszName[1] != 0 && (lpwszName[1] != L'.' || lpwszName[2] != 0)) ) &&
				(wcsstr (lpwszSrc, L"\\..\\") == NULL && wcsstr (lpwszSrc, L"\\.\\") == NULL) )
		{
			strDest = lpwszSrc;

			return (int)strDest.GetLength ();
		}
	}
	return (int)apiGetFullPathName(lpwszSrc,strDest);
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point
*/
int ConvertNameToReal (const wchar_t *Src, string &strDest, bool Internal)
{
  string strTempDest;
  BOOL IsAddEndSlash=FALSE; // =TRUE, если слеш добавляли самостоятельно
                            // в конце мы его того... удавим.

  // Получим сначала полный путь до объекта обычным способом
  int Ret=ConvertNameToFull(Src, strTempDest);
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog(L"ConvertNameToFull('%s') -> '%s'",Src,(const wchar_t*)strTempDest));

  wchar_t *TempDest;
  /* $ 14.06.2003 IS
     Для нелокальных дисков даже и не пытаемся анализировать симлинки
  */
  // также ничего не делаем для нелокальных дисков, т.к. для них невозможно узнать
  // корректную информацию про объект, на который указывает симлинк (т.е. невозможно
  // "разыменовать симлинк")
  if (IsLocalDrive(strTempDest))
  {
    DWORD FileAttr;

		if((FileAttr=apiGetFileAttributes(strTempDest)) != INVALID_FILE_ATTRIBUTES)
    {
      AddEndSlash(strTempDest);
      IsAddEndSlash=TRUE;
    }

    TempDest = strTempDest.GetBuffer();
    wchar_t *Ptr, Chr;

    Ptr = TempDest + strTempDest.GetLength();

    const wchar_t *CtrlChar = TempDest;

    if (strTempDest.GetLength() > 2 && TempDest[0]==L'\\' && TempDest[1]==L'\\')
      CtrlChar = wcschr(TempDest+2, L'\\');

    // обычный цикл прохода имени от корня
    while(CtrlChar)
    {
			while(Ptr > TempDest && !IsSlash(*Ptr))
        --Ptr;
      /* $ 07.01.2003 IS
         - ошибка: зачем-то обрабатывали путь "буква:" - он равен
           текущему каталогу на диске "буква", что ведет к
           непредсказуемым результатам
      */
      // Если имя UNC, то работаем до имени сервера, не дальше...
      if(*Ptr != L'\\' || Ptr == CtrlChar
        // если дошли до "буква:", то тоже остановимся
        || *(Ptr-1)==L':')
        break;
      /* IS $ */

      Chr=*Ptr;
      *Ptr=0;
			FileAttr=apiGetFileAttributes(TempDest);
      // О! Это наш клиент - одна из "компонент" пути - симлинк
      if(FileAttr != INVALID_FILE_ATTRIBUTES && (FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        string strTempDest2;
        // Получим инфу симлинке
        if(GetReparsePointInfo(TempDest, strTempDest2))
        {
          // Убираем \\??\ из пути симлинка
          if(!StrCmpN(strTempDest2,L"\\??\\",4))
            strTempDest2.LShift(4);
          // для случая монтированного диска (не имеющего букву)...
          if(!StrCmpNI(strTempDest2, L"Volume{", 7))
          {
            string strJuncRoot;
            // получим либо букву диска, либо...
            GetPathRootOne(strTempDest2, strJuncRoot);
            // ...но в любом случае пишем полностью.
            // (поправка - если букву не получили - вернём точку монтирования)
            strTempDest2 = (strJuncRoot.At(1)==L':'||!Internal)?strJuncRoot:TempDest;
          }
          DeleteEndSlash(strTempDest2);
          // Длина пути симлинка
          size_t temp2Length = strTempDest2.GetLength();
          // Буфер симлинка
          wchar_t* TempDest2 = strTempDest2.GetBuffer();
          // Получаем длину левой и правой частей пути
          size_t leftLength = StrLength(TempDest);
          size_t rightLength = StrLength(Ptr + 1); // Измеряем длину пути начиная со следующего симовла после курсора
          // Восстановим символ
          *Ptr=Chr;
          // Если путь симлинка больше левой части пути, увеличиваем буфер
          if (leftLength < temp2Length)
          {
            // Выделяем новый буфер
            TempDest = strTempDest.GetBuffer(strTempDest.GetLength() + temp2Length - leftLength + (IsAddEndSlash?2:1));
          }
          // Так как мы производили манипуляции с левой частью пути изменяем указатель на
          // текущую позицию курсора в пути
          Ptr = TempDest + temp2Length - 1;
          // Перемещаем правую часть пути на нужное место, только если левая чать отличается по
          // размеру от пути симлинка
          if (leftLength != temp2Length)
          {
            // Копируемый буфер включает сам буфер, начальный '/', конечный '/' (если он есть) и '\0'
            wmemmove(TempDest + temp2Length, TempDest + leftLength, rightLength + (IsAddEndSlash ? 3 : 2));
          }
          // Копируем путь к симлинку вначало пути
          wmemcpy(TempDest, TempDest2, temp2Length);
          // Обновляем ссылку на маркер завершения прохождения по пути
          CtrlChar = TempDest;
          if (StrLength(TempDest) > 2 && TempDest[0] == L'\\' && TempDest[1] == L'\\')
          {
						CtrlChar=FirstSlash(TempDest+2);
          }
          // Устанавливаем длину возвращаемой строки
          Ret = StrLength(TempDest);
          // Релизим буфер для установления корректной длины данных. Если этого не делать, то при
          // увеличенни буфера могут скопироваться не все данные
          strTempDest.ReleaseBuffer(Ret);
          // Переходим к следующему шагу
          continue;
        }
      }
      *Ptr=Chr;
      --Ptr;
    }
  }

  // Если не просили - удалим.
  if(IsAddEndSlash)
  {
    if (DeleteEndSlash(strTempDest))
    {
      --Ret;
    }
  }

  strDest = strTempDest;

  return Ret;
}

void ConvertNameToShort(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;

	int nSize = GetShortPathNameW (strCopy, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (nSize);

		GetShortPathNameW (strCopy, lpwszDest, nSize);

		strDest.ReleaseBuffer ();
	}
	else
		strDest = strCopy;

	strDest.Upper ();
}

void ConvertNameToLong(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;

	int nSize = GetLongPathNameW (strCopy, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (nSize);

		GetLongPathNameW (strCopy, lpwszDest, nSize);

		strDest.ReleaseBuffer ();
	}
	else
		strDest = strCopy;
}

string &DriveLocalToRemoteName(int DriveType,wchar_t Letter,string &strDest)
{
  int NetPathShown=FALSE, IsOK=FALSE;
  wchar_t LocalName[8]=L" :\0\0\0";
  string strRemoteName;

  *LocalName=Letter;
  strDest=L"";

  if(DriveType == DRIVE_UNKNOWN)
  {
    LocalName[2]=L'\\';
    DriveType = FAR_GetDriveType(LocalName);
    LocalName[2]=0;
  }

  if (IsDriveTypeRemote(DriveType))
  {
    DWORD res = apiWNetGetConnection(LocalName,strRemoteName);
    if (res == NO_ERROR || res == ERROR_CONNECTION_UNAVAIL)
    {
      NetPathShown=TRUE;
      IsOK=TRUE;
    }
  }

  if (!NetPathShown)
    if (GetSubstName(DriveType,LocalName,strRemoteName))
      IsOK=TRUE;

  if(IsOK)
    strDest = strRemoteName;

  return strDest;
}

void ConvertNameToUNC(string &strFileName)
{
	ConvertNameToFull(strFileName,strFileName);
	// Посмотрим на тип файловой системы
	string strFileSystemName;
	string strTemp;
	GetPathRoot(strFileName,strTemp);

	if(!apiGetVolumeInformation (strTemp,NULL,NULL,NULL,NULL,&strFileSystemName))
		strFileSystemName=L"";

	DWORD uniSize = 1024;
	UNIVERSAL_NAME_INFOW *uni=(UNIVERSAL_NAME_INFOW*)xf_malloc(uniSize);

	// применяем WNetGetUniversalName для чего угодно, только не для Novell`а
	if (StrCmpI(strFileSystemName,L"NWFS"))
	{
		DWORD dwRet=WNetGetUniversalNameW(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize);
		switch(dwRet)
		{
		case NO_ERROR:
			strFileName = uni->lpUniversalName;
			break;
		case ERROR_MORE_DATA:
			uni=(UNIVERSAL_NAME_INFOW*)xf_realloc(uni,uniSize);
			if(WNetGetUniversalNameW(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize)==NO_ERROR)
				strFileName = uni->lpUniversalName;
			break;
		}
	}
	else if(strFileName.At(1) == L':')
	{
		// BugZ#449 - Неверная работа CtrlAltF с ресурсами Novell DS
		// Здесь, если не получилось получить UniversalName и если это
		// мапленный диск - получаем как для меню выбора дисков

		if(!DriveLocalToRemoteName(DRIVE_UNKNOWN,strFileName.At(0),strTemp).IsEmpty())
		{
			const wchar_t *NamePtr=FirstSlash(strFileName);
			if(NamePtr != NULL)
			{
				AddEndSlash(strTemp);
				strTemp += &NamePtr[1];
			}
			strFileName = strTemp;
		}
	}
	xf_free(uni);
	ConvertNameToReal(strFileName,strFileName);
}

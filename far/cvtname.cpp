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

#include "plugin.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"


int ConvertNameToFull (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strSrc = lpwszSrc; //копирование в другую переменную на случай dest == src
	lpwszSrc = strSrc;

	const wchar_t *lpwszName = PointToName(lpwszSrc);

	if ( (lpwszName == lpwszSrc) &&
				(lpwszName[0] != L'.' || lpwszName[1] != 0) )
	{
		FarGetCurDir(strDest);
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

	int nLength = GetFullPathNameW (lpwszSrc, 0, NULL, NULL);
	wchar_t *lpwszDest = strDest.GetBuffer (nLength);
	GetFullPathNameW (lpwszSrc, nLength, lpwszDest, NULL);

	// это когда ввели в масдае cd //host/share
	// а масдай выдал на гора c:\\host\share

	if ( lpwszSrc[0] == L'/' &&
				lpwszSrc[1] == L'/' &&
				lpwszDest[1] == L':' &&
				lpwszDest[3] == L'\\' )
				wmemmove (lpwszDest, lpwszDest+2, wcslen (lpwszDest+2)+1);

	strDest.ReleaseBuffer ();

	return (int)strDest.GetLength ();
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point в Win2K
  Если OS ниже, то вызывается обычный ConvertNameToFull()
*/
int WINAPI ConvertNameToReal (const wchar_t *Src, string &strDest, bool Internal)
{
  string strTempDest;
  BOOL IsAddEndSlash=FALSE; // =TRUE, если слеш добавляли самостоятельно
                            // в конце мы его того... удавим.

  // Получим сначала полный путь до объекта обычным способом
  int Ret=ConvertNameToFull(Src, strTempDest);
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog(L"ConvertNameToFull('%S') -> '%S'",Src,(const wchar_t*)strTempDest));

  wchar_t *TempDest;
  /* $ 14.06.2003 IS
     Для нелокальных дисков даже и не пытаемся анализировать симлинки
  */
  // остальное касается Win2K, т.к. в виндах ниже рангом нету некоторых
  // функций, позволяющих узнать истинное имя линка.
  // также ничего не делаем для нелокальных дисков, т.к. для них невозможно узнать
  // корректную информацию про объект, на который указывает симлинк (т.е. невозможно
  // "разыменовать симлинк")
  if (IsLocalDrive(strTempDest) &&
      WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    DWORD FileAttr;

    if((FileAttr=GetFileAttributesW(strTempDest)) != INVALID_FILE_ATTRIBUTES)
    {
      AddEndSlash(strTempDest);
      IsAddEndSlash=TRUE;
    }

    TempDest = strTempDest.GetBuffer();
    wchar_t *Ptr, Chr;

    Ptr = TempDest+StrLength(TempDest);

    const wchar_t *CtrlChar = TempDest;

    if (StrLength(TempDest) > 2 && TempDest[0]==L'\\' && TempDest[1]==L'\\')
      CtrlChar = wcschr(TempDest+2, L'\\');

    // обычный цикл прохода имени от корня
    while(CtrlChar)
    {
      while(Ptr > TempDest && *Ptr != L'\\')
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
      FileAttr=GetFileAttributesW(TempDest);
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
          // Буфер симлинка
          wchar_t* TempDest2 = strTempDest2.GetBuffer();
          // Длина пути симлинка
          size_t tempLength = StrLength(TempDest2);
          // Получаем длину левой и правой частей пути
          size_t leftLength = StrLength(TempDest);
          size_t rightLength = StrLength(Ptr + 1); // Измеряем длину пути начиная со следующего симовла после курсора
          // Восстановим символ
          *Ptr=Chr;
          // Если путь симлинка больше левой части пути, увеличиваем буфер
          if (leftLength < tempLength)
          {
            // Выделяем новый буфер
            TempDest = strTempDest.GetBuffer((int)(strTempDest.GetLength() + tempLength - leftLength + (IsAddEndSlash?2:1)));
          }
          // Так как мы производили манипуляции с левой частью пути изменяем указатель на
          // текущую позицию курсора в пути
          Ptr = TempDest + tempLength - 1;
          // Перемещаем правую часть пути на нужное место, только если левая чать отличается по
          // размеру от пути симлинка
          if (leftLength != tempLength)
          {
            // Копируемый буфер включает сам буфер, начальный '/', конечный '/' (если он есть) и '\0'
            wmemmove(TempDest + tempLength, TempDest + leftLength, rightLength + (IsAddEndSlash ? 3 : 2));
          }
          // Копируем путь к симлинку вначало пути
          wmemcpy(TempDest, TempDest2, tempLength);
          // Обновляем ссылку на маркер завершения прохождения по пути
          CtrlChar = TempDest;
          if (StrLength(TempDest) > 2 && TempDest[0] == L'\\' && TempDest[1] == L'\\')
          {
            CtrlChar = wcschr(TempDest + 2, L'\\');
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

int WINAPI OldConvertNameToReal(const wchar_t *Src, string &strDest)
{
	return ConvertNameToReal(Src,strDest,false);
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

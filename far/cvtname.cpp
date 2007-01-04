/*
cvtname.cpp

Функций для преобразования имен файлов/путей.

*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"


/* $ 02.07.2001 IS
   Получение длинного имени на основе известного короткого. Медленно, зато с
   гарантией.
   src     - указатель на короткое имя
   dest    - сюда помещать длинное имя
   maxsize - размер dest. В dest будет скопировано не больше (maxsize-1)
             символов
   Возвращается число скопированных символов или 0. Если размер dest
   недостаточен, то возвращается требуемый размер.
   Примечание: разрешено перекрытие src и dest
*/


DWORD RawConvertShortNameToLongNameW(const wchar_t *src, string &strDest)
{
  if(!src)
     return 0;

  if(!*src)
  {
     strDest=L"";
     return 1;
  }

  DWORD SrcSize=wcslen(src);

  string strBuffSrc;
  int nLength = GetFullPathNameW (src, 0, NULL, NULL)+1;
  wchar_t *lpwszBuffSrc = strBuffSrc.GetBuffer (nLength);
  GetFullPathNameW (src, nLength, lpwszBuffSrc, NULL);

  if(SrcSize == 3 && lpwszBuffSrc[1] == L':' && (lpwszBuffSrc[2] == L'\\' || lpwszBuffSrc[2] == L'/'))
  {
    strDest = strBuffSrc;
    strDest.Upper ();
    return strDest.GetLength();
  }


  DWORD DestSize=0, FinalSize=0, AddSize;
  BOOL Error=FALSE;

  wchar_t *Src, *Dest=0, *DestBuf=NULL,
       *SrcBuf=(wchar_t *)xf_malloc((SrcSize+1)*sizeof(wchar_t));

  while(SrcBuf)
  {
     wcscpy(SrcBuf, lpwszBuffSrc);
     Src=SrcBuf;

     WIN32_FIND_DATAW wfd;
     HANDLE hFile;

     wchar_t *Slash, *Dots=wcschr(Src, L':');

     if(Dots)
     {
       ++Dots;
       if(L'\\'==*Dots) ++Dots;
       wchar_t tmp=*Dots;
       *Dots=0;
       AddSize=wcslen(Src);
       FinalSize=AddSize;
       DestBuf=(wchar_t *)xf_malloc((AddSize+64)*sizeof(wchar_t));
       if(DestBuf)
       {
         DestSize=AddSize+64;
         Dest=DestBuf;
       }
       else
       {
         Error=TRUE;
         FinalSize=0;
         break;
       }
       wcscpy(Dest, Src);
       Dest+=AddSize;

       *Dots=tmp;
       Src=Dots; // +1 ??? зачем ???
     }
     else if (Src[0]==L'\\' && Src[1]==L'\\')
     {
       Dots=Src+2;
       while(*Dots && L'\\'!=*Dots) ++Dots;
       if(L'\\'==*Dots)
         ++Dots;
       else
       {
          strDest = strBuffSrc;
          if(SrcBuf) xf_free(SrcBuf);
          return strDest.GetLength();
       }
       while(*Dots && L'\\'!=*Dots) ++Dots;
       if(L'\\'==*Dots) ++Dots;
       wchar_t tmp=*Dots;
       *Dots=0;
       AddSize=wcslen(Src);
       FinalSize=AddSize;
       DestBuf=(wchar_t *)xf_malloc((AddSize+64)*sizeof(wchar_t));
       if(DestBuf)
       {
         DestSize=AddSize+64;
         Dest=DestBuf;
       }
       else
       {
         Error=TRUE;
         FinalSize=0;
         break;
       }
       wcscpy(Dest, Src);
       Dest+=AddSize;

       *Dots=tmp;
       Src=Dots;
     }

     /* $ 03.12.2001 DJ
        если ничего не осталось - не пытаемся найти пустую строку
     */
     while(!Error && *Src)   /* DJ $ */
     {
       Slash=wcschr(Src, L'\\');
       if(Slash) *Slash=0;
       hFile=FindFirstFileW(SrcBuf, &wfd);
       if(hFile!=INVALID_HANDLE_VALUE)
       {
         FindClose(hFile);
         AddSize=wcslen(wfd.cFileName);
         FinalSize+=AddSize;
         if(FinalSize>=DestSize)
         {
           DestBuf=(wchar_t *)xf_realloc(DestBuf, (FinalSize+64)*sizeof(wchar_t));
           if(DestBuf)
           {
             DestSize+=64;
             Dest=DestBuf+FinalSize-AddSize;
           }
           else
           {
             Error=TRUE;
             FinalSize=0;
             break;
           }
         }
         wcscpy(Dest, wfd.cFileName);
         Dest+=AddSize;
         if(Slash)
         {
           *Dest=*Slash=L'\\';
           ++Dest;
           /* $ 03.12.2001 DJ
              если после слэша ничего нету - надо добавить '\0'
           */
           *Dest = L'\0';
           /* DJ $ */
           ++FinalSize;
           ++Slash;
           Slash=wcschr(Src=Slash, L'\\');
         }
         else
           break;
       }
       else
       {
         Error=TRUE;
         break;
       }
     }
     break;
  }

  if(!Error)
    strDest = DestBuf;

  if(SrcBuf)  xf_free(SrcBuf);
  if(DestBuf) xf_free(DestBuf);

  return FinalSize;
}

int ConvertNameToFullW (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strSrc = lpwszSrc; //копирование в другую переменную на случай dest == src

	lpwszSrc = strSrc;

    int Result = wcslen (lpwszSrc);

    const wchar_t *lpwszName = PointToNameW (lpwszSrc);

    if ( (lpwszName == lpwszSrc) &&
         (lpwszName[0] != L'.' || lpwszName[1] != 0) )
    {
        FarGetCurDirW (strDest);
        AddEndSlashW (strDest);

        strDest += lpwszSrc;

        return strDest.GetLength ();
    }

    if ( PathMayBeAbsoluteW (lpwszSrc) )
    {
        if ( *lpwszName &&
            (*lpwszName != L'.' || lpwszName[1] != 0 && (lpwszName[1] != L'.' || lpwszName[2] != 0) ) &&
            (wcsstr (lpwszSrc, L"\\..\\") == NULL && wcsstr (lpwszSrc, L"\\.\\") == NULL) )
        {
            strDest = lpwszSrc;

            return strDest.GetLength ();
        }
    }

    int nLength = GetFullPathNameW (lpwszSrc, 0, NULL, NULL)+1;

    wchar_t *lpwszDest = strDest.GetBuffer (nLength);
    GetFullPathNameW (lpwszSrc, nLength, lpwszDest, NULL);


    // это когда ввели в масдае cd //host/share
    // а масдай выдал на гора c:\\host\share

    if ( lpwszSrc[0] == L'/' &&
         lpwszSrc[1] == L'/' &&
         lpwszDest[1] == L':' &&
         lpwszDest[3] == L'\\' )
         memmove (lpwszDest, lpwszDest+2, (wcslen (lpwszDest+2)+1)*sizeof (wchar_t));

    strDest.ReleaseBuffer (nLength);

    return strDest.GetLength ();
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point в Win2K
  Если OS ниже, то вызывается обычный ConvertNameToFull()
*/
int WINAPI ConvertNameToRealW (const wchar_t *Src, string &strDest)
{
  string strTempDest;
  BOOL IsAddEndSlash=FALSE; // =TRUE, если слеш добавляли самостоятельно
                            // в конце мы его того... удавим.

  // Получим сначала полный путь до объекта обычным способом
  int Ret=ConvertNameToFullW(Src, strTempDest);
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog("ConvertNameToFull('%S') -> '%S'",Src,(const wchar_t*)strTempDest));

  wchar_t *TempDest;
  /* $ 14.06.2003 IS
     Для нелокальных дисков даже и не пытаемся анализировать симлинки
  */
  // остальное касается Win2K, т.к. в виндах ниже рангом нету некоторых
  // функций, позволяющих узнать истинное имя линка.
  // также ничего не делаем для нелокальных дисков, т.к. для них невозможно узнать
  // корректную информацию про объект, на который указывает симлинк (т.е. невозможно
  // "разыменовать симлинк")
  if (IsLocalDriveW(strTempDest) &&
      WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    DWORD FileAttr;

    if((FileAttr=GetFileAttributesW(strTempDest)) != -1 && (FileAttr&FILE_ATTRIBUTE_DIRECTORY))
    {
      AddEndSlashW (strTempDest);
      IsAddEndSlash=TRUE;
    }

    TempDest = strTempDest.GetBuffer (2048); //BUGBUGBUG!!!!
    wchar_t *Ptr, Chr;

    Ptr = TempDest+wcslen(TempDest);

    const wchar_t *CtrlChar = TempDest;

    if (wcslen(TempDest) > 2 && TempDest[0]==L'\\' && TempDest[1]==L'\\')
      CtrlChar= wcschr(TempDest+2, L'\\');

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
      if(FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        string strTempDest2;

//        if(CheckParseJunction(TempDest,sizeof(TempDest)))
        {
          // Получим инфу симлинке
          if(GetJunctionPointInfoW(TempDest, strTempDest2))
          {
            strTempDest.LShift (4); //???
            // для случая монтированного диска (не имеющего букву)...
            if(!wcsncmp(strTempDest2,L"Volume{",7))
            {
              string strJuncRoot;
              // получим либо букву диска, либо...
              GetPathRootOneW(strTempDest2, strJuncRoot);
              // ...но в любом случае пишем полностью.
              strTempDest2 = strJuncRoot;
            }

            *Ptr=Chr; // восстановим символ
            DeleteEndSlashW(strTempDest2);
            strTempDest2 = Ptr;
            wcscpy(TempDest,strTempDest2); //BUGBUG
            Ret=wcslen(TempDest);
            // ВСЕ. Реальный путь у нас в кармане...
            break;
          }
        }
      }
      *Ptr=Chr;
      --Ptr;
    }

    strTempDest.ReleaseBuffer ();
  }

  TempDest = strTempDest.GetBuffer ();

  if(IsAddEndSlash) // если не просили - удалим.
    TempDest[wcslen(TempDest)-1]=0;

  strTempDest.ReleaseBuffer ();

  strDest = strTempDest;

  return Ret;
}


void ConvertNameToShortW(const wchar_t *Src, string &strDest)
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

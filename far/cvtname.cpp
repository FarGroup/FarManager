/*
cvtname.cpp

Функций для преобразования имен файлов/путей.

*/

/* Revision: 1.08 14.06.2003 $ */

/*
Modify:
  14.06.2003 IS
    ! ConvertNameToReal - для нелокальных дисков даже и не пытаемся анализировать
      симлинки, т.к. это все равно бесполезно
  05.03.2003 SVS
    + немного логов
    ! наработки по вопросу о символических связях
  25.02.2003 SVS
    ! "free/malloc/realloc -> xf_*" - что-то в прошлый раз пропустил.
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  07.01.2003 IS
    - ошибка в ConvertNameToReal: зачем-то обрабатывали путь "буква:" - он
      равен текущему каталогу на диске "буква", что ведет к непредсказуемым
      результатам
  21.06.2002 VVM
    ! При поиске линков учтем UNC пути и не работаем с именем сервера как с диском
  28.05.2002 SVS
    ! применим функцию  IsLocalPath()
    ! Номер ревизии приведен в порядок (кто-то когда то забыл подправить)
  22.03.2002 SVS
    ! Выделение в качестве самостоятельного модуля
    ! Функции CharBufferToSmallWarn, RawConvertShortNameToLongName,
      ConvertNameToFull, ConvertNameToReal, ConvertNameToShort перехали
      из mix.cpp в cvtname.cpp (выделение в отдельный бомонд ;-))
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"

/* $ 01.11.2000 OT
  Исправление логики. Теперь функция должна в обязательном порядке
  получить размер буфера и выдать длину полученного имени файла.
  Если размер буфера мал, то копирование не происходит
*/
void CharBufferTooSmallWarn(int BufSize, int FileNameSize)
{
  char Buf2 [80];
  sprintf (Buf2,MSG(MBuffSizeTooSmall_2), FileNameSize, BufSize);
  Message(MSG_WARNING,1,MSG(MError),MSG(MBuffSizeTooSmall_1),Buf2,MSG(MOk));
}

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
DWORD RawConvertShortNameToLongName(const char *src, char *dest, DWORD maxsize)
{
  if(!src || !dest)
     return 0;

  if(!*src)
  {
     *dest=0;
     return 1;
  }

  DWORD SrcSize=strlen(src);

  if(SrcSize == 3 && src[1] == ':' && (src[2] == '\\' || src[2] == '/'))
  {
    SrcSize=Min((DWORD)SrcSize,(DWORD)maxsize);
    memmove(dest,src,SrcSize);
    dest[SrcSize]=0;
    *dest=toupper(*dest);
    return SrcSize;
  }


  DWORD DestSize=0, FinalSize=0, AddSize;
  BOOL Error=FALSE;

  char *Src, *Dest, *DestBuf=NULL,
       *SrcBuf=(char *)xf_malloc(SrcSize+1);

  while(SrcBuf)
  {
     strcpy(SrcBuf, src);
     Src=SrcBuf;

     WIN32_FIND_DATA wfd;
     HANDLE hFile;

     char *Slash, *Dots=strchr(Src, ':');

     if(Dots)
     {
       ++Dots;
       if('\\'==*Dots) ++Dots;
       char tmp=*Dots;
       *Dots=0;
       AddSize=strlen(Src);
       FinalSize=AddSize;
       DestBuf=(char *)xf_malloc(AddSize+64);
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
       strcpy(Dest, Src);
       Dest+=AddSize;

       *Dots=tmp;
       Src=Dots; // +1 ??? зачем ???
     }

     /* $ 03.12.2001 DJ
        если ничего не осталось - не пытаемся найти пустую строку
     */
     while(!Error && *Src)   /* DJ $ */
     {
       Slash=strchr(Src, '\\');
       if(Slash) *Slash=0;
       hFile=FindFirstFile(SrcBuf, &wfd);
       if(hFile!=INVALID_HANDLE_VALUE)
       {
         FindClose(hFile);
         AddSize=strlen(wfd.cFileName);
         FinalSize+=AddSize;
         if(FinalSize>=DestSize)
         {
           DestBuf=(char *)xf_realloc(DestBuf, FinalSize+64);
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
         strcpy(Dest, wfd.cFileName);
         Dest+=AddSize;
         if(Slash)
         {
           *Dest=*Slash='\\';
           ++Dest;
           /* $ 03.12.2001 DJ
              если после слэша ничего нету - надо добавить '\0'
           */
           *Dest = '\0';
           /* DJ $ */
           ++FinalSize;
           ++Slash;
           Slash=strchr(Src=Slash, '\\');
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
  {
    if(FinalSize<maxsize)
       strcpy(dest, DestBuf);
    else
    {
       *dest=0;
       ++FinalSize;
    }
  }

  if(SrcBuf)  xf_free(SrcBuf);
  if(DestBuf) xf_free(DestBuf);

  return FinalSize;
}
/* IS $ */

int ConvertNameToFull(const char *Src,char *Dest, int DestSize)
{
  int Result = 0;
//  char *FullName = (char *) xf_malloc (DestSize);
//  char *AnsiName = (char *) xf_malloc (DestSize);
  char *FullName = (char *) alloca (DestSize);
  char *AnsiName = (char *) alloca (DestSize);
  *FullName = 0;
  *AnsiName = 0;

//  char FullName[NM],AnsiName[NM],
  char *NamePtr=PointToName(const_cast<char *>(Src));
  Result+=strlen(Src);

  if (NamePtr==Src && (NamePtr[0]!='.' || NamePtr[1]!=0))
  {
    Result+=FarGetCurDir(DestSize,FullName);
    Result+=AddEndSlash(FullName);
    if (Result < DestSize)
    {
      strncat(FullName,Src,Result);
      strncpy(Dest,FullName,Result);
      Dest [Result] = '\0';
    }
    else
    {
      CharBufferTooSmallWarn(DestSize,Result+1);
    }
    return Result;
  }

  if (PathMayBeAbsolute(Src)) //  (isalpha(Src[0]) && Src[1]==':' || Src[0]=='\\' && Src[1]=='\\') //????
  {
    if (*NamePtr &&
        (*NamePtr!='.' || NamePtr[1]!=0 && (NamePtr[1]!='.' || NamePtr[2]!=0)) &&
        (strstr(Src,"\\..\\")==NULL && strstr(Src,"\\.\\")==NULL)
       )
    {
      if (Dest!=Src)
        strcpy(Dest,Src);
      return Result;
    }
  }

  SetFileApisToANSI();
  OemToChar(Src,AnsiName);
  /* $ 08.11.2000 SVS
     Вместо DestSize использовался sizeof(FullName)...
  */
  if(GetFullPathName(AnsiName,DestSize,FullName,&NamePtr))
    CharToOem(FullName,Dest);
  else
    strcpy(Dest,Src);

  // это когда ввели в масдае cd //host/share
  // а масдай выдал на гора c:\\host\share
  if(Src[0] == '/' && Src[1] == '/' && Dest[1] == ':' && Dest[3] == '\\')
    memmove(Dest,Dest+2,strlen(Dest+2)+1);

  /* SVS $*/
  SetFileApisToOEM();

  return Result;
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point в Win2K
  Если OS ниже, то вызывается обычный ConvertNameToFull()
*/
int WINAPI ConvertNameToReal(const char *Src,char *Dest, int DestSize)
{
  _SVS(CleverSysLog Clev("ConvertNameToReal()"));
  _SVS(SysLog("Params: Src='%s'",Src));
  char TempDest[2048];
  BOOL IsAddEndSlash=FALSE; // =TRUE, если слеш добавляли самостоятельно
                            // в конце мы его того... удавим.

  // Получим сначала полный путь до объекта обычным способом
  int Ret=ConvertNameToFull(Src,TempDest,sizeof(TempDest));
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog("ConvertNameToFull('%s') -> '%s'",Src,TempDest));

  /* $ 14.06.2003 IS
     Для нелокальных дисков даже и не пытаемся анализировать симлинки
  */
  // остальное касается Win2K, т.к. в виндах ниже рангом нету некоторых
  // функций, позволяющих узнать истинное имя линка.
  // также ничего не делаем для нелокальных дисков, т.к. для них невозможно узнать
  // корректную информацию про объект, на который указывает симлинк (т.е. невозможно
  // "разыменовать симлинк")
  if (IsLocalDrive(TempDest) &&
      WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  /* IS $ */
  {
    _SVS(CleverSysLog Clev("VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5"));
    DWORD FileAttr;
    char *Ptr, Chr;

    Ptr=TempDest+strlen(TempDest);

    _SVS(SysLog("%d FileAttr=0x%08X",__LINE__,GetFileAttributes(TempDest)));
    // немного интелектуальности не помешает - корректную инфу мы
    // можем получить только если каталог будет завершен слешем!
    if((FileAttr=GetFileAttributes(TempDest)) != -1 && (FileAttr&FILE_ATTRIBUTE_DIRECTORY))
    {
      if(Ptr[-1] != '\\')
      {
        AddEndSlash(TempDest);
        IsAddEndSlash=TRUE;
        ++Ptr;
      }
    }

    /* $ 21.06.2002 VVM
      ! Учтем UNC пути */
    char *CtrlChar = TempDest;
    if (strlen(TempDest) > 2 && TempDest[0]=='\\' && TempDest[1]=='\\')
      CtrlChar= strchr(TempDest+2, '\\');
    /* VVM $ */
    // обычный цикл прохода имени от корня
    while(CtrlChar)
    {
      while(Ptr > TempDest && *Ptr != '\\')
        --Ptr;
      /* $ 07.01.2003 IS
         - ошибка: зачем-то обрабатывали путь "буква:" - он равен
           текущему каталогу на диске "буква", что ведет к
           непредсказуемым результатам
      */
      // Если имя UNC, то работаем до имени сервера, не дальше...
      if(*Ptr != '\\' || Ptr == CtrlChar
        // если дошли до "буква:", то тоже остановимся
        || *(Ptr-1)==':')
        break;
      /* IS $ */

      Chr=*Ptr;
      *Ptr=0;
      FileAttr=GetFileAttributes(TempDest);
      _SVS(SysLog("%d FileAttr=0x%08X ('%s')",__LINE__,FileAttr,TempDest));

      // О! Это наш клиент - одна из "компонент" пути - симлинк
      if(FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        char TempDest2[1024];

//        if(CheckParseJunction(TempDest,sizeof(TempDest)))
        {
          _SVS(SysLog("%d Parse Junction",__LINE__));
          // Получим инфу симлинке
          if(GetJunctionPointInfo(TempDest,TempDest2,sizeof(TempDest2)))
          {
            // для случая монтированного диска (не имеющего букву)...
            if(!strncmp(TempDest2+4,"Volume{",7))
            {
              char JuncRoot[NM];
              JuncRoot[0]=JuncRoot[1]=0;
              // получим либо букву диска, либо...
              GetPathRootOne(TempDest2+4,JuncRoot);
              // ...но в любом случае пишем полностью.
              strcpy(TempDest2+4,JuncRoot);
            }
            // небольшая метаморфоза с именем, дабы удалить ведущие "\\?\"
            // но для "Volume{" начало всегда будет корректным!
            memmove(TempDest2,TempDest2+4,strlen(TempDest2+4)+1);
            *Ptr=Chr; // восстановим символ
            DeleteEndSlash(TempDest2);
            strcat(TempDest2,Ptr);
            strcpy(TempDest,TempDest2);
            Ret=strlen(TempDest);
            // ВСЕ. Реальный путь у нас в кармане...
            break;
          }
        }
      }
      *Ptr=Chr;
      --Ptr;
    }
  }
  if(IsAddEndSlash) // если не просили - удалим.
    TempDest[strlen(TempDest)-1]=0;

  if(Dest && DestSize)
    strncpy(Dest,TempDest,DestSize-1);
  _SVS(SysLog("return Dest='%s'",Dest));
  return Ret;
}

void ConvertNameToShort(const char *Src,char *Dest)
{
  char ShortName[NM],AnsiName[NM];
  SetFileApisToANSI();
  OemToChar(Src,AnsiName);
  if (GetShortPathName(AnsiName,ShortName,sizeof(ShortName)))
    CharToOem(ShortName,Dest);
  else
    strcpy(Dest,Src);
  SetFileApisToOEM();
  LocalUpperBuf(Dest,strlen(Dest));
}

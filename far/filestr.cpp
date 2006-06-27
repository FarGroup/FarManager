/*
filestr.cpp

Класс GetFileString

*/

/* Revision: 1.07 25.05.2006 $ */

/*
Modify:
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  26.02.2002 SVS
    ! Удалим тестовый кусок.
  25.01.2002 SVS
    + Задаваемый размер FBufSize для буферизации чтения.
      Явление возможно временное - тестеры покажут.
    - Явная бага - размер StrLength увеличили, перераспределили память и
      если запрос памяти окончился неудачно, то StrLength так и осталась
      в новом значении.
  06.05.2001 DJ
    ! перетрях #include
  20.02.2001 SVS
    ! Заголовки - к общему виду!
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "filestr.hpp"

GetFileString::GetFileString(FILE *SrcFile)
{
  /* $ 13.07.2000 SVS
     Т.к. в последствии память перераспределяется через realloc, то
     конструкция Str=new char[1024]; не применима...
  */
  wStr = NULL;
  Str=(char*)xf_malloc(1024);
  /* SVS $ */
  StrLength=1024;
  GetFileString::SrcFile=SrcFile;

  ReadPos=ReadSize=0;
}


GetFileString::~GetFileString()
{
  /* $ 13.07.2000 SVS
     используем free
  */
  xf_free(Str);
  xf_free(wStr);
  /* SVS $ */
}


int GetFileString::GetStringW(wchar_t **DestStr, int CodePage, int &Length)
{
    char *Str;

    int nExitCode = GetString(&Str, Length);

    wStr = (wchar_t*)xf_realloc (wStr, (Length+1)*sizeof (wchar_t));
    wmemset (wStr, 0, Length+1);

    MultiByteToWideChar (CodePage, 0, Str, Length, wStr, Length);

    *DestStr = wStr;

    return nExitCode;
}

int GetFileString::GetString(char **DestStr,int &Length)
{
  Length=0;
  int CurLength=0,ExitCode=1;
  while (1)
  {
    if (ReadPos>=ReadSize)
    {
      if ((ReadSize=fread(ReadBuf,1,sizeof(ReadBuf),SrcFile))==0)
      {
        if (CurLength==0)
          ExitCode=0;
        break;
      }
      ReadPos=0;
    }
    int Ch=ReadBuf[ReadPos];
    if (Ch!='\n' && CurLength>0 && Str[CurLength-1]=='\r')
      break;
    ReadPos++;
    if (CurLength>=StrLength-1)
    {
      char *NewStr=(char *)xf_realloc(Str,StrLength+1024);
      if (NewStr==NULL)
        return(-1);
      Str=NewStr;
      StrLength+=1024;
    }
    Str[CurLength++]=Ch;
    if (Ch=='\n')
      break;
  }
  Str[CurLength]=0;
  *DestStr=Str;
  Length=CurLength;
  return(ExitCode);
}

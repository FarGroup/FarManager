/*
filestr.cpp

Класс GetFileString

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif
#if !defined(__NEW_H)
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif

#ifndef __GETFILESTRING_HPP__
#include "filestr.hpp"
#endif

GetFileString::GetFileString(FILE *SrcFile)
{
  Str=new char[1024];
  StrLength=1024;
  GetFileString::SrcFile=SrcFile;
  ReadPos=ReadSize=0;
}


GetFileString::~GetFileString()
{
  delete Str;
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
      StrLength+=1024;
      char *NewStr=(char *)realloc(Str,StrLength);
      if (NewStr==NULL)
        return(-1);
      Str=NewStr;
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


/*
plognmn.cpp

class PreserveLongName

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
#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __STDIO_H
#include <stdio.h>
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __PRESERVELONGNAME_HPP__
#include "plognmn.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif

PreserveLongName::PreserveLongName(char *ShortName,int Preserve)
{
  PreserveLongName::Preserve=Preserve;
  if (Preserve)
  {
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    FindHandle=FindFirstFile(ShortName,&FindData);
    FindClose(FindHandle);
    if (FindHandle==INVALID_HANDLE_VALUE)
      *SaveLongName=0;
    else
      strcpy(SaveLongName,FindData.cFileName);
    strcpy(SaveShortName,ShortName);
  }
}


PreserveLongName::~PreserveLongName()
{
  if (Preserve && GetFileAttributes(SaveShortName)!=0xFFFFFFFF)
  {
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    FindHandle=FindFirstFile(SaveShortName,&FindData);
    FindClose(FindHandle);
    if (FindHandle==INVALID_HANDLE_VALUE ||
        strcmp(SaveLongName,FindData.cFileName)!=0)
    {
      char NewName[NM];
      strcpy(NewName,SaveShortName);
      strcpy(PointToName(NewName),SaveLongName);
      rename(SaveShortName,NewName);
    }
  }
}

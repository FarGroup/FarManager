/*
plognmn.cpp

class PreserveLongName

*/

/* Revision: 1.01 20.02.2001 $ */

/*
Modify:
  20.02.2001 SVS
    ! Заголовки - к общему виду!
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"

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

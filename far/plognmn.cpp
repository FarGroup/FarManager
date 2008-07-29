/*
plognmn.cpp

class PreserveLongName

*/

#include "headers.hpp"
#pragma hdrstop

#include "plognmn.hpp"
#include "fn.hpp"

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
  if (Preserve && GetFileAttributes(SaveShortName)!=INVALID_FILE_ATTRIBUTES)
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

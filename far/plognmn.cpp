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

		if (!GetFileWin32FindData(ShortName,&FindData))
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

		if (!GetFileWin32FindData(SaveShortName,&FindData) ||
		        strcmp(SaveLongName,FindData.cFileName)!=0)
		{
			char NewName[NM];
			strcpy(NewName,SaveShortName);
			strcpy(PointToName(NewName),SaveLongName);
			rename(SaveShortName,NewName);
		}
	}
}

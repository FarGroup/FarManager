/*
plognmn.cpp

class PreserveLongName

*/

#include "headers.hpp"
#pragma hdrstop

#include "plognmn.hpp"
#include "fn.hpp"

PreserveLongNameW::PreserveLongNameW(const wchar_t *ShortName,int Preserve)
{
	PreserveLongNameW::Preserve=Preserve;
	if (Preserve)
	{
		FAR_FIND_DATA_EX FindData;

		if ( apiGetFindDataEx(ShortName, &FindData) )
			strSaveLongName = FindData.strFileName;
		else
			strSaveLongName = L"";

		strSaveShortName = ShortName;
	}
}


PreserveLongNameW::~PreserveLongNameW()
{
	if (Preserve && GetFileAttributesW(strSaveShortName)!=0xFFFFFFFF)
	{
		FAR_FIND_DATA_EX FindData;

		if ( !apiGetFindDataEx (strSaveShortName, &FindData) || wcscmp(strSaveLongName,FindData.strFileName)!=0)
		{
			string strNewName;

			strNewName = strSaveShortName;

			wchar_t *lpwszNewName = strNewName.GetBuffer ();

			lpwszNewName = wcsrchr (lpwszNewName, '\\'); //BUGBUG

			if ( lpwszNewName )
				*lpwszNewName = 0;

			strNewName.ReleaseBuffer ();

			strNewName += "\\";
			strNewName += strSaveLongName;

			MoveFileW (strSaveShortName, strNewName);
		}
	}
}

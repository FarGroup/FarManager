#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI FP_GetRegKey(const char *Key,const char *ValueName,DWORD Default)
{
	int   data;
	DWORD Type,
	      Size = sizeof(data);
	HKEY  hKey = FP_OpenRegKey(Key);

	if(hKey)
	{
		if(RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE*)&data,&Size) != ERROR_SUCCESS)
			data = Default;

		RegCloseKey(hKey);
	}
	else
		data = Default;

	return data;
}

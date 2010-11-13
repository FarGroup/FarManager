#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_DeleteRegKeyAll(LPCSTR hParentKey,LPCSTR Key)
{
	char name[ FAR_MAX_REG ];

	if(!Key || !Key[0])
		return FALSE;

	if(!hParentKey || !hParentKey[0])
		return FP_DeleteRegKeyAll(HKEY_CURRENT_USER,Key);
	else
	{
		sprintf(name,"%s" SLASH_STR "%s",hParentKey,Key);
		return FP_DeleteRegKeyAll(HKEY_CURRENT_USER,name);
	}
}

BOOL WINAPI FP_DeleteRegKeyAll(LPCSTR Key)
{
	char  name[ FAR_MAX_REG ];
	CHK_INITED

	if(!Key || !Key[0])
		return FALSE;

	sprintf(name,"%s" SLASH_STR "%s",FP_PluginRootKey,Key);
	return FP_DeleteRegKeyAll(HKEY_CURRENT_USER,name);
}

BOOL WINAPI FP_DeleteRegKeyAll(HKEY hParentKey, LPCSTR szKey)
{
	//PROC(( "FP_DeleteRegKeyAll","%s",szKey ))
	char     szSubKey[ FAR_MAX_REG ];
	HKEY     hKey = NULL;
	LONG     nRes;
	FILETIME tTime;
	DWORD    nSize;

	do
	{
		nRes = RegOpenKeyEx(hParentKey, szKey, 0, KEY_ENUMERATE_SUB_KEYS|KEY_READ|KEY_WRITE, &hKey);
		SetLastError(nRes); //Log(( "open [%s] rc: %s",szKey,FIO_ERROR ));

		if(nRes != ERROR_SUCCESS) break;

		for(nSize = sizeof(szSubKey); 1; nSize = sizeof(szSubKey))
		{
			szSubKey[0] = 0;
			nRes = RegEnumKeyEx(hKey,0,szSubKey, &nSize, 0, NULL, NULL, &tTime);
			SetLastError(nRes); //Log(( "enum [%s] rc: %s",szSubKey,FIO_ERROR ));

			if(nRes == ERROR_NO_MORE_ITEMS)
			{
				nRes = ERROR_SUCCESS;
				break;
			}

			if(nRes != ERROR_SUCCESS)
				break;

			if(!FP_DeleteRegKeyAll(hKey,szSubKey))
			{
				nRes = GetLastError();
				break;
			}
		}

		if(nRes == ERROR_SUCCESS)
		{
			nRes = RegDeleteKey(hParentKey, szKey);
			SetLastError(nRes); //Log(( "delete rc: %s", FIO_ERROR ));
		}
	}
	while(0);

	if(hKey)
		RegCloseKey(hKey);

	return nRes == ERROR_SUCCESS;
}

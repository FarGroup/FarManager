#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_CopyRegKeyAll(HKEY hTargParent, LPCSTR szTargKey, HKEY hSrcParent, LPCSTR szSrcKey)
{
	TCHAR *szSubKeyName = NULL;
	TCHAR *szValueName = NULL;
	BYTE  *pData = NULL;
	HKEY   hSrcKey = NULL, hTargKey = NULL;
	LONG   nRes;

	do
	{
		nRes = RegCreateKey(hTargParent, szTargKey, &hTargKey);

		if(nRes != ERROR_SUCCESS) break;

		nRes = RegOpenKeyEx(hSrcParent, szSrcKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_READ, &hSrcKey);

		if(nRes != ERROR_SUCCESS) break;

		DWORD nMaxKeyNameLen, nMaxValueNameLen, nMaxValueLen;
		RegQueryInfoKey(hSrcKey, NULL, NULL, NULL, NULL,
		                &nMaxKeyNameLen, NULL, NULL,
		                &nMaxValueNameLen, &nMaxValueLen, NULL, NULL);
		nMaxKeyNameLen++;
		nMaxValueNameLen++;
		szSubKeyName = new TCHAR[nMaxKeyNameLen];
		DWORD nIndex = 0, nSize = nMaxKeyNameLen;
		FILETIME tTime;

		while(RegEnumKeyEx(hSrcKey, nIndex++, szSubKeyName, &nSize, 0, NULL, NULL, &tTime) == ERROR_SUCCESS)
		{
			LONG nRes = FP_CopyRegKeyAll(hTargKey, szSubKeyName, hSrcKey, szSubKeyName);

			if(nRes != ERROR_SUCCESS)
				break;

			nSize = nMaxKeyNameLen;
		}

		if(nRes != ERROR_SUCCESS) break;

		szValueName = new TCHAR[nMaxValueNameLen];
		pData       = new BYTE[nMaxValueLen];
		DWORD nType, nDataSize;
		nIndex    = 0;
		nSize     = nMaxValueNameLen;
		nDataSize = nMaxValueLen;

		while(RegEnumValue(hSrcKey, nIndex++, szValueName, &nSize, NULL, NULL, pData, &nDataSize) == ERROR_SUCCESS)
		{
			nRes = RegQueryValueEx(hSrcKey, szValueName, NULL, &nType, pData, &nDataSize);

			if(nRes != ERROR_SUCCESS)
				break;

			nRes = RegSetValueEx(hTargKey, szValueName, 0, nType, pData, nDataSize);

			if(nRes != ERROR_SUCCESS)
				break;

			nSize = nMaxValueNameLen;
			nDataSize = nMaxValueLen;
		}

		if(nRes != ERROR_SUCCESS) break;

		nRes = ERROR_SUCCESS;
	}
	while(0);

	if(szSubKeyName) delete[] szSubKeyName;

	if(szValueName)  delete[] szValueName;

	if(pData)        delete[] pData;

	if(hSrcKey)      RegCloseKey(hSrcKey);

	if(hTargKey)     RegCloseKey(hTargKey);

	return nRes == ERROR_SUCCESS;
}

#include "FarRegistry.hpp"

int RegQueryStringArray (
		HKEY hKey,
		const TCHAR *lpPrefix,
		TCHAR*** Strings
		)
{
	TCHAR *lpValueName = StrCreate (260);
	TCHAR **pValueData;

	int i = 0;
	dword dwType = REG_SZ;
	dword dwSize;

	while ( true )
	{
		FSF.sprintf (lpValueName, _T("%s%d"), lpPrefix, i);

		if ( RegQueryValueEx (
				hKey,
				lpValueName,
				NULL,
				&dwType,
				NULL,
				&dwSize
				) == ERROR_SUCCESS )
		{
			if ( Strings )
			{
				pValueData = *Strings;
				pValueData[i] = StrCreate (dwSize+1);

				RegQueryValueEx (
					hKey,
					lpValueName,
					NULL,
					&dwType,
					(PBYTE)pValueData[i],
					&dwSize
					);
			}

			i++;
		}
		else
			break;
	}

	StrFree (lpValueName);

	return i;
}


void RegSaveStringArray (
		HKEY hKey,
		const TCHAR *lpPrefix,
		TCHAR **Strings,
		int nStringCount
		)
{
	TCHAR *lpValueName = StrCreate (260);

	dword dwSize;

	if ( Strings )
	{
		for (int i = 0; i < nStringCount; i++)
		{
			dwSize = StrLength(Strings[i])+1;

			FSF.sprintf (lpValueName, _T("%s%d"), lpPrefix, i);

			RegSetValueEx (
					hKey,
					lpValueName,
					0,
					REG_SZ,
					(PBYTE)Strings[i],
					dwSize
					);
		}
	}

	StrFree (lpValueName);
}


TCHAR *RegQueryStringValueEx (
		HKEY hKey,
		const TCHAR *lpValueName,
		TCHAR *lpCurrentValue /* = NULL */
		)
{
	dword dwSize = 0;

	TCHAR *lpResultValue;

	if ( (RegQueryValueEx (
			hKey,
			lpValueName,
			NULL,
			NULL,
			NULL,
			&dwSize
			) == ERROR_SUCCESS) )
	{
		StrFree (lpCurrentValue);
		lpResultValue = StrCreate (dwSize+1);

		RegQueryValueEx (
				hKey,
				lpValueName,
				NULL,
				NULL,
				(PBYTE)lpResultValue,
				&dwSize
				);

		return lpResultValue;
	}

	return lpCurrentValue;
}

void RegQueryLongValue (
		HKEY hKey,
		const TCHAR *lpValueName,
		void *pValueData
		)
{
	dword dwResult;
	dword dwSize = 4;
	dword dwType = REG_DWORD;

	if ( RegQueryValueEx (
			hKey,
			lpValueName,
			NULL,
			&dwType,
			(PBYTE)&dwResult,
			&dwSize
			) == ERROR_SUCCESS )
	{
		*(PDWORD)pValueData = dwResult;
	}
}

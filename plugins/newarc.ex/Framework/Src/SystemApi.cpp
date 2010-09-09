#include "SystemApi.hpp"

DWORD apiGetEnvironmentVariable(const TCHAR *lpName, string &strBuffer)
{
	string strName = lpName;

	int nSize = GetEnvironmentVariable(strName, NULL, 0);

	if ( nSize )
	{
		TCHAR *lpwszBuffer = strBuffer.GetBuffer (nSize);

		nSize = GetEnvironmentVariable(strName, lpwszBuffer, nSize);

		strBuffer.ReleaseBuffer ();
	}

	return nSize;
}


string& apiRegQueryStringValue(
		HKEY hKey,
		const TCHAR *lpValueName,
		string& strDefaultValue
		)
{
	DWORD dwSize = 0;

	if ( (RegQueryValueEx (
			hKey,
			lpValueName,
			NULL,
			NULL,
			NULL,
			&dwSize
			) == ERROR_SUCCESS) )
	{
		TCHAR *pBuffer = strDefaultValue.GetBuffer(dwSize+1);

		RegQueryValueEx (
				hKey,
				lpValueName,
				NULL,
				NULL,
				(PBYTE)pBuffer,
				&dwSize
				);

		strDefaultValue.ReleaseBuffer();
	}

	return strDefaultValue;
}

DWORD apiExpandEnvironmentStrings(const TCHAR *src, string &strDest)
{
	string strSrc = src;

	DWORD length = ExpandEnvironmentStrings(strSrc, NULL, 0);

	if ( length )
	{
		TCHAR* lpwszDest = strDest.GetBuffer (length);

		ExpandEnvironmentStrings(strSrc, lpwszDest, length);

		strDest.ReleaseBuffer ();

		length = (DWORD)strDest.GetLength ();
	}

	return length;
}

DWORD apiGetFullPathName(const TCHAR* lpFileName, string& strResult)
{
	DWORD dwSize = GetFullPathName(lpFileName, 0, NULL, NULL);

	TCHAR* pBuffer = strResult.GetBuffer(dwSize+1);

	GetFullPathName(lpFileName, dwSize+1, pBuffer, NULL);

	strResult.ReleaseBuffer();

	return dwSize;
}

void apiCreateDirectoryEx(const TCHAR* lpDirectory)
{
	TCHAR* lpCopy = StrDuplicate(lpDirectory);

	for (TCHAR* c = lpCopy; *c; c++)
	{
		if ( *c != _T(' ') )
		{
			for(; *c; c++)
			{
				if ( *c == _T('\\') )
				{
					*c = 0;
					CreateDirectory(lpCopy, NULL);
					*c = _T('\\');
				}
			}

			CreateDirectory(lpCopy, NULL);

			break;
		}
	}

	free (lpCopy);
}


void apiCreateDirectoryForFile(const TCHAR* lpFileName)
{
	string strNameCopy = lpFileName;

	CutToSlash(strNameCopy);
	apiCreateDirectoryEx(strNameCopy);
}

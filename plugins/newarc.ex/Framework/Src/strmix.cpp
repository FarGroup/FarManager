#include "strmix.hpp"


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


void CutTo (string& str, TCHAR symbol, bool bInclude)
{
	TCHAR *pBuffer = str.GetBuffer();

	CutTo(pBuffer, symbol, bInclude);

	str.ReleaseBuffer();
}

void CutToSlash(string &str, bool bInclude)
{
	CutTo(str, _T('\\'), bInclude);
}

void AddEndSlash(string& str)
{
	if ( !str.IsEmpty() && (str[str.GetLength()-1] != _T('\\')) )
		str += _T('\\');
}


wchar_t* AnsiToUnicode(const char* lpSrc, int CodePage)
{
	if ( !lpSrc )
		return NULL;

	int nLength = MultiByteToWideChar(CodePage, 0, lpSrc, -1, NULL, 0);

	wchar_t* lpResult = (wchar_t*)malloc((nLength+1)*sizeof(wchar_t));

	MultiByteToWideChar(CodePage, 0, lpSrc, -1, lpResult, nLength);

	return lpResult;
}

char* UnicodeToAnsi(const wchar_t* lpSrc, int CodePage)
{
	if ( !lpSrc )
		return NULL;

	int nLength = WideCharToMultiByte(CodePage, 0, lpSrc, -1, NULL, 0, NULL, NULL);

	char* lpResult = (char*)malloc(nLength+1);

	WideCharToMultiByte(CodePage, 0, lpSrc, -1, lpResult, nLength, NULL, NULL);

	return lpResult;
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

void ConvertSlashes(string& strStr)
{
	TCHAR* pBuffer = strStr.GetBuffer();

	while ( *pBuffer )
	{
		if ( *pBuffer == _T('/') )
			*pBuffer = _T('\\');

		pBuffer++;
	}

	strStr.ReleaseBuffer();
}

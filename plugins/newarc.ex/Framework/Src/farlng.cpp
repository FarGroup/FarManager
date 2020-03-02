#include "FarPluginBase.hpp"

/*
inline bool IsEOL(char c)
{
	return c == '\n' || c == '\r';
}

inline bool IsEmpty (char c)
{
	return c == ' ' || c == '\t';
}
*/

#define SkipEmpty(s, last) \
	while ( (s != last) && IsEmpty(*s) ) \
		s++;

bool GetLngHlfParam (
		char* s,
		char* last,
		char** qb,
		char** qe,
		char* ParamName
		)
{
	SkipEmpty(s, last);

	if ( s == last )
		return false;

	if ( *s++ != '.' )
		return false;

	if ( !strncmp(s, ParamName, strlen(ParamName)) )
	{
		s += strlen(ParamName);

		SkipEmpty(s, last);

		if ( (s != last) && (*s == '=') )
		{
			*qb = s+1;
			*qe = last;

			return true;
		}
	}

	return false;
}


char* FindStrEnd (
		char* s,
		char* last
		)
{
	while ( (s != last) && !IsEOL(*s) ) s++;

	if ( (s != last) || IsEOL(*s) )
		--s;

	return s;
}

char* FindNextStr (
		char* s,
		char* last
		)
{
	if ( s != last )
		s++;

	while ( (s != last) && IsEOL(*s) ) s++;

	return s;
}

bool FindQuotedSubstr (
		char* b,
		char* e,
		char** pqb,
		char** pqe
		)
{
	while ( (b != e) && IsEmpty (*b) ) b++;
	while ( (b != e) && IsEmpty (*e) ) e--;

	if ( *b != '"' )
		return false;

	if ( e != b )
	{
		if ( *e == '"' )
			e--;

		b++;
	}

	*pqb = b;
	*pqe = e;

	return true;
}


void TranslateMessage (
		const TCHAR* lpMessage,
		TCHAR* lpResultMessage
		)
{
	int j = 0;

	for (size_t i = 0; i < _tcslen(lpMessage); i++)
	{
		if ( lpMessage[i] == _T('\\') )
		{
			switch (lpMessage[i+1]) {

			case _T('t'):
				lpResultMessage[j] = _T('\t');
				break;

			case _T('n'):
				lpResultMessage[j] = _T('\n');
				break;

			case _T('r'):
				lpResultMessage[j] = _T('\r');
				break;

			case _T('b'):
				lpResultMessage[j] = _T('\b');
				break;

			case _T('\\'):
				lpResultMessage[j] = _T('\\');
				break;

			case _T('\"'):
				lpResultMessage[j] = _T('"');
				break;

			default:
				goto g;
			}

			j++;
			i++;
			continue;
		}
g:

		lpResultMessage[j] = lpMessage[i];
		j++;
	}

	lpResultMessage[j] = _T('\0');

}

bool ScanMsgsBuf(
		char* buf,
		char* eob,
		const char* lpLanguage,
		TCHAR** pMsgs = NULL,
		int* pStringsCount = NULL
		)
{
	bool bLanguageFound = (lpLanguage == NULL);
	bool bResult = true;

	char* b = buf, *e;
	char *qb, *qe;

	int nCount = 0;

	while ( (b < eob) && bResult )
	{
		e = FindStrEnd(b, eob);

		if ( FindQuotedSubstr(b, e, &qb, &qe) )
		{
			if ( pMsgs )
			{
				INT_PTR nLength = qe-qb;

				char* pSrc = new char[nLength+1];
				memcpy(pSrc, qb, nLength);
				pSrc[nLength] = 0;

#ifdef UNICODE
				TCHAR* pStr = AnsiToUnicode(pSrc, CP_OEMCP);
#else
				TCHAR* pStr = StrDuplicate(pSrc);
#endif

				TCHAR* pDest = new TCHAR[_tcslen(pStr)+1];

				TranslateMessage (pStr, pDest);

				pMsgs[nCount] = pDest;

				delete [] pSrc;
				free(pStr);
			};

			nCount++;
		}
		else
		{
			if ( !bLanguageFound )
			{
				if ( GetLngHlfParam(b, e, &qb, &qe, "Language") )
				{
					bLanguageFound = true;

					int nLength = qe-qb;

					char* lpFileLanguage = new char[nLength+1];
					memcpy(lpFileLanguage, qb, nLength);
					lpFileLanguage[nLength] = 0;

					char* p = strchr(lpFileLanguage, ',');

					if ( p )
						*p = '\0';

					if ( strcmp(lpLanguage, lpFileLanguage) )
						bResult = false;

					delete [] lpFileLanguage;
				}
			}
		}

		b = FindNextStr(e, eob);
    };

    if ( pStringsCount )
    	*pStringsCount = nCount;

    return bResult;
}

bool LoadLanguageFile (
		const TCHAR* lpFileName,
		const TCHAR* lpLanguage,
		TCHAR**& pStrings,
		int& nStringsCount
		)
{
	bool bResult = false;

	HANDLE hFile = CreateFile (
			lpFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD dwFileSize = GetFileSize (hFile, NULL);
		DWORD dwRead = 0;

		char* lpBuffer = new char[dwFileSize];
		memset(lpBuffer, 0, dwFileSize);

		char* lpEndOfBuffer;

		ReadFile (hFile, lpBuffer, dwFileSize, &dwRead, NULL);

		lpEndOfBuffer = (char*)((DWORD_PTR)lpBuffer+dwRead-1);

		OEM_NAME_CREATE_CONST(lpLanguage);			

		if ( ScanMsgsBuf (
				lpBuffer,
				lpEndOfBuffer,
				lpLanguageA,
				NULL, //
				&nStringsCount
				) ) // correct language found!
		{
			pStrings = new TCHAR*[nStringsCount];
			memset (pStrings, 0, nStringsCount*sizeof(TCHAR*));

			ScanMsgsBuf (
					lpBuffer,
					lpEndOfBuffer,
					NULL,  // we already know, this file has a correct language
					pStrings,
					NULL
					);

			bResult = true;
		}

		OEM_NAME_DELETE_CONST(lpLanguage);

		delete [] lpBuffer;

		CloseHandle (hFile);
	}

	return bResult;
}

bool SearchAndLoadLanguageFile(
		const TCHAR* lpPath,
		const TCHAR* lpLanguage,
		TCHAR**& pStrings,
		int& nStringsCount
		)
{
	return FALSE; //BUGBUG

	bool bResult = false;

	TCHAR* lpMask = StrDuplicate(lpPath);
	CutToSlash (lpMask);

	_tcscat (lpMask, _T("*.lng"));

	WIN32_FIND_DATA FindData;

	HANDLE hSearch = FindFirstFile (
			lpMask,
			(WIN32_FIND_DATA*)&FindData
			);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do {

			CutToSlash (lpMask);
			_tcscat (lpMask, FindData.cFileName);

			pStrings = NULL;

			bResult = LoadLanguageFile(
					lpMask,
					lpLanguage,
					pStrings,
					nStringsCount
					);

		} while ( !bResult && FindNextFile (hSearch, (WIN32_FIND_DATA*)&FindData) );

		FindClose (hSearch);
	}

	StrFree(lpMask);

	return bResult;
}

void FinalizeLanguageStrings (
		TCHAR **pStrings,
		int nStringsCount
		)
{
	if ( pStrings && nStringsCount )
	{
		for (int i = 0; i < nStringsCount; i++)
			delete pStrings[i];

		delete pStrings;

		pStrings = NULL;
	}
}

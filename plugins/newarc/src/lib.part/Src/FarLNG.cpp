#include <Rtl.Base.h>

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

/*
void CutTo (char *s, char symbol, bool bInclude)
{
	for (int i = strlen (s)-1; i >= 0; i--)
		if ( s[i] == symbol )
		{
			bInclude?s[i] = 0:s[i+1] = 0;
			break;
		}

}


void CutToSlash (char *s)
{
	CutTo (s, '\\', false);
}
*/

bool GetLngHlfParam (
		char *s,
		char *last,
		char **qb,
		char **qe,
		const char *ParamName
		)
{
	SkipEmpty (s, last);

	if ( s == last )
		return false;

	if ( *s++ != '.' )
		return false;

	if ( !strncmp (s, ParamName, strlen (ParamName)) )
	{
		s += strlen (ParamName);

		SkipEmpty (s, last);

		if ( (s != last) && (*s == '=') )
		{
			*qb = s+1;
			*qe = last;

			return true;
		}
	}

	return false;
}


char *FindStrEnd (
		char *s,
		char *last
		)
{
	while ( (s != last) && !IsEOL(*s) ) s++;

	if ( (s != last) || IsEOL(*s) )
		--s;

	return s;
}

char *FindNextStr (
		char *s,
		char *last
		)
{
	if ( s != last )
		s++;

	while ( (s != last) && IsEOL(*s) ) s++;

	return s;
}

bool FindQuotedSubstr (
		char *b,
		char *e,
		char **pqb,
		char **pqe
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
		const char *lpMessage,
		char *lpResultMessage
		)
{
	int j = 0;

	for (size_t i = 0; i < strlen (lpMessage); i++)
	{
		if ( lpMessage[i] == '\\' )
		{
			switch (lpMessage[i+1]) {

			case 't':
				lpResultMessage[j] = '\t';
				break;

			case 'n':
				lpResultMessage[j] = '\n';
				break;

			case 'r':
				lpResultMessage[j] = '\r';
				break;

			case 'b':
				lpResultMessage[j] = '\b';
				break;

			case '\\':
				lpResultMessage[j] = '\\';
				break;

			case '\"':
				lpResultMessage[j] = '"';
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

	lpResultMessage[j] = '\0';

}

bool ScanMsgsBuf(
		char *buf,
		char *eob,
		const char *lpLanguage,
		char **pMsgs = NULL,
		int *pStringsCount = NULL
		)
{
	bool bLanguageFound = (lpLanguage == NULL);
	bool bResult = true;

	char *lpFileLanguage = (char*)malloc (260);

	char *b = buf, *e;
	char *qb, *qe;

	int nCount = 0;

	while ( (b != eob) && bResult )
	{
		e = FindStrEnd(b, eob);

		if ( FindQuotedSubstr(b, e, &qb, &qe) )
		{
			if ( pMsgs )
			{
				INT_PTR nLength = qe-qb;

				char *pnew = (char*)malloc (nLength+2);
				memset(pnew, 0, nLength+2);

				char *pnew2 = (char*)malloc (nLength+2);

				strncpy(pnew, qb, nLength+1);

				TranslateMessage (pnew, pnew2);

				pMsgs[nCount] = pnew2;

				free (pnew);
			};

			nCount++;
		}
		else
		{
			if ( !bLanguageFound )
			{
				if ( GetLngHlfParam (b, e, &qb, &qe, "Language") )
				{
					bLanguageFound = true;

					strncpy (lpFileLanguage, qb, qe-qb+1);

					char *p = strchr (lpFileLanguage, ',');

					if ( p )
						*p = '\0';

					if ( strcmp (lpLanguage, lpFileLanguage) )
						bResult = false;
				}
			}
		}

		b = FindNextStr(e, eob);
    };

    free (lpFileLanguage);

    if ( pStringsCount )
    	*pStringsCount = nCount;

    return bResult;
}

bool LoadLanguageFile (
		const char *lpFileName,
		const char *lpLanguage,
		char **&pStrings,
		int &nStringsCount
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

		char *lpBuffer = (char*)malloc (dwFileSize);
		char *lpEndOfBuffer;

		ReadFile (hFile, lpBuffer, dwFileSize, &dwRead, NULL);

		lpEndOfBuffer = (char*)((DWORD_PTR)lpBuffer+dwRead-1);

		if ( ScanMsgsBuf (
				lpBuffer,
				lpEndOfBuffer,
				lpLanguage,
				NULL, //
				&nStringsCount
				) ) // correct language found!
		{
			pStrings = (char**)malloc (nStringsCount*sizeof(char*));
			memset (pStrings, 0, nStringsCount*sizeof(char*));

			ScanMsgsBuf (
					lpBuffer,
					lpEndOfBuffer,
					NULL,  // we already know, this file has a correct language
					pStrings,
					NULL
					);

			bResult = true;
		}

		free (lpBuffer);

		CloseHandle (hFile);
	}

	return bResult;
}

bool SearchAndLoadLanguageFile (
		const char *lpPath,
		const char *lpLanguage,
		char **&pStrings,
		int &nStringsCount
		)
{
	bool bResult = false;

	char *lpMask = (char*)malloc(260);

	strcpy (lpMask, lpPath);

	CutToSlash (lpMask);

	strcat (lpMask, "*.lng");

	WIN32_FIND_DATA FindData;

	HANDLE hSearch = FindFirstFile (
			lpMask,
			(WIN32_FIND_DATA*)&FindData
			);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do {

			CutToSlash (lpMask);
			strcat (lpMask, (char*)&FindData.cFileName);

			pStrings = NULL;

			bResult = LoadLanguageFile (
					lpMask,
					lpLanguage,
					pStrings,
					nStringsCount
					);

		} while ( !bResult && FindNextFile (hSearch, (WIN32_FIND_DATA*)&FindData) );

		FindClose (hSearch);
	}

	free (lpMask);

	return bResult;
}

void FinalizeLanguageStrings (
		char **pStrings,
		int nStringsCount
		)
{
	if ( pStrings && nStringsCount )
	{
		for (int i = 0; i < nStringsCount; i++)
			free (pStrings[i]);

		free (pStrings);
		pStrings = NULL;
	}
}

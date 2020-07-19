/*
Copyright (c) 2005 WARP ItSelf & Alex Yaroslavsky
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//BUG: /* comments don't work!

#include "lng.common.h"

#define VERSION "v1.0 rc3"

void FixQuotes (char *lpStr)
{
	char *lpStart = lpStr;
	char *lpEnd = lpStr+strlen(lpStr)-1;
	char *lpTmp = (char *)malloc(strlen(lpStr)*2);
	char *lpPtr = lpTmp;

	memset (lpTmp, 0, strlen(lpStr)*2);

	while ( *lpStart && *lpStart != '"' )
		lpStart++;

	while ( lpEnd > lpStart && *lpEnd != '"' )
		lpEnd--;

	if ( lpEnd == lpStart )
	{
		strcpy (lpTmp, lpStart);
		strcat (lpTmp, "\"");
		strcpy (lpStr, lpTmp);

		free (lpTmp);

		return;
	}

	*(lpPtr++) = '"';
	lpStart++;

	while ( lpStart < lpEnd )
	{
		if ( *lpStart == '"' && *(lpStart-1) != '\\' )
			*(lpPtr++) = '\\';

		*(lpPtr++) = *(lpStart++);
	}

	*lpPtr = '"';

	strcpy (lpStr, lpTmp);

	free (lpTmp);
}

bool ReadFromBufferEx (
		char *&lpStart,
		char **lpParam,
		bool bSkipWhite = false,
		bool bSkipCmt = false
		)
{
	char *lpEnd = lpStart;
	bool bWhite;
	bool bCmt;

	*lpParam = NULL;

	while (*lpStart)
	{
		bWhite = true;
		bCmt = false;

		while ( *lpEnd && *lpEnd != '\r' && *lpEnd != '\n' )
		{
			if ( !bCmt && bWhite && *lpEnd == '/' && *(lpEnd+1) == '/' )
				bCmt = true;

			if ( bWhite && *lpEnd != ' ' && *lpEnd != '\t' )
				bWhite = false;

			lpEnd++;
		}

		if ( !((bSkipWhite && bWhite) || (bSkipCmt && bCmt)) )
		{
			*lpParam = (char*)malloc (lpEnd-lpStart+1);

			memset (*lpParam, 0, lpEnd-lpStart+1);
			memcpy (*lpParam, lpStart, lpEnd-lpStart);
		}

		lpStart = lpEnd;

		if ( (*(lpStart+1) == '\r' || *(lpStart+1) == '\n') && *(lpStart+1) != *lpStart )
			lpStart++;

		lpStart++;
		lpEnd = lpStart;

		if ( !((bSkipWhite && bWhite) || (bSkipCmt && bCmt)) )
			return true;
	}

	return false;
}

bool ReadLanguage (
		char *&lpRealStart,
		char **lpLngName,
		char **lpLngDesc
		)
{
	char *lpStart = lpRealStart;
	char *lpParam = NULL;
	bool bResult = false;

	*lpLngName = (char*)malloc(100);
	*lpLngDesc = (char*)malloc(100);

	strcpy (*lpLngName, "none");
	strcpy (*lpLngDesc, "none");

	if ( ReadFromBufferEx (
			lpStart,
			&lpParam,
			true,
			true
			) && strstr (lpParam,".Language") )
	{
		strcpy (*lpLngName, strstr (lpParam, "=")+1);
		strcpy (*lpLngDesc, "\"");

		strcat (*lpLngDesc, strstr (*lpLngName, ",")+1);
		strcat (*lpLngDesc, "\"");

		char *lpComa = strchr(*lpLngName, ',');

		if ( lpComa )
			*lpComa = 0;

		lpRealStart = lpStart;

		while ( *lpRealStart=='\r' || *lpRealStart=='\n' )
			lpRealStart++;

		bResult = true;
	}

	if ( lpParam )
		free (lpParam);

	return bResult;
}

struct LanguageEntry {
	char *lpBuffer;
	char *lpStart;
	char *lpString;
	char *lpOldStart;
};

int main (int argc, const char* argv[])
{
	printf (".LNG Convertor "VERSION"\n");
	printf ("Copyright (C) 2005-2006 WARP ItSelf & Alex Yaroslavsky\n\n");

	if ( argc < 5 )
	{
		printf ("Usage: convertor feed_file hpp_file num_of_lng lng_file1 lng_file2 ...\r\n");
		return 0;
	}

	HANDLE hFeedFile = CreateFile (
			argv[1],
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
			);

	if ( hFeedFile == INVALID_HANDLE_VALUE )
	{
		printf ("ERROR: Can't create the feed file, exiting.\r\n");
		return 0;
	}


	HANDLE hHFile = CreateFile (
			argv[2],
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
			);

	if ( hHFile == INVALID_HANDLE_VALUE )
	{
		printf ("ERROR: Can't open the header file, exiting.\r\n");
		return 0;
	}

	DWORD dwLangs = atol(argv[3]);

	if ( !dwLangs )
	{
		printf ("ERROR: Zero language files to process, exiting.\r\n");
		return 0;
	}

	DWORD dwRead;
	DWORD dwSize = GetFileSize (hHFile, NULL);

	char *pHBuffer = (char*)malloc (dwSize+1);

	memset (pHBuffer, 0, dwSize+1);

	ReadFile (hHFile, pHBuffer, dwSize, &dwRead, NULL);

	CloseHandle (hHFile);

	char *lpStart = pHBuffer;

	LanguageEntry *pLangEntries = (LanguageEntry*)malloc (dwLangs*sizeof (LanguageEntry));

	memset (pLangEntries, 0, dwLangs*sizeof (LanguageEntry));

	HANDLE hFile;

	for (int i = 0; i < dwLangs; i++)
	{
		hFile = CreateFile (
				argv[4+i],
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
				);

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			dwSize = GetFileSize (hFile, NULL);

			pLangEntries[i].lpBuffer = (char*)malloc (dwSize+1);
			pLangEntries[i].lpStart = pLangEntries[i].lpBuffer;

			memset (pLangEntries[i].lpBuffer, 0, dwSize+1);

			ReadFile (hFile, pLangEntries[i].lpBuffer, dwSize, &dwRead, NULL);

			CloseHandle (hFile);
		}
		else
			printf ("WARNING: Can't open the language file \"%s\", skiping\r\n", argv[4+i]);
	}

	char *lpTmp = (char*)malloc (2048);
	char *lpString;

	sprintf (lpTmp, "#hpp file name\r\n%s\r\n#number of languages\r\n%s\r\n", argv[2], argv[3]);

	WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

	for (int i = 0; i < dwLangs; i++)
	{
		if ( pLangEntries[i].lpBuffer ) //or we skipping
		{
			char *lpLngName, *lpLngDesc;

			sprintf (lpTmp, "#id:%d language file name, language name, language description\r\n%s", i, argv[4+i]);

			WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

			ReadLanguage (pLangEntries[i].lpStart, &lpLngName, &lpLngDesc);

			sprintf (lpTmp," %s %s\r\n", lpLngName, lpLngDesc);
			WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

			free (lpLngName);
			free (lpLngDesc);
		}
	}

	sprintf(lpTmp,"\r\n#head of the hpp file\r\n");

	WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

	while ( ReadFromBufferEx (lpStart, &lpString) )
	{
		if ( !strncmp (lpString,"enum",4) )
		{
			free(lpString);

			char *lpOldStart = lpStart;

			ReadFromBufferEx (lpStart, &lpString);

			if ( strstr(lpString, "{") )
				lpOldStart = lpStart;

			free(lpString);

			while ( ReadFromBufferEx (lpStart, &lpString,true,true) && !strstr (lpString, "};") )
				free(lpString);

			char *lpEnd = lpStart-2;

			while ( *lpEnd != '\r' && *lpEnd != '\n')
				lpEnd--;

			while ( *lpEnd == '\r' || *lpEnd == '\n')
				lpEnd--;

			while ( *lpEnd != '\r' && *lpEnd != '\n')
				lpEnd--;

			*(lpEnd+1)=0;


			sprintf (lpTmp, "\r\n#tail of the hpp file\r\n");

			WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

			while ( ReadFromBufferEx (lpStart, &lpString) )
			{
				sprintf (lpTmp, "htail:%s\r\n", lpString);

				WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

				free(lpString);
			}

			lpStart = lpOldStart;

			break;
		}
		else
		{
			sprintf(lpTmp, "hhead:%s\r\n", lpString);
			WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);
		}

		free(lpString);
	}

	char *lpOldStart = lpStart;

	while ( true )
	{
		WriteFile (hFeedFile, "\r\n", 2, &dwRead, NULL);

		lpOldStart = lpStart;

		while ( ReadFromBufferEx (lpStart, &lpString) )
		{
			TrimStart (lpString);

			if ( strstr (lpString, "//") != lpString ) // "//" at the beginning at the string -> copy, else it will be discarded
			{
				free(lpString);

				lpStart = lpOldStart;

			break;
			}
			else
			{
				sprintf (lpTmp, "h:%s\r\n", lpString);
				WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);
			}

			free(lpString);

			lpOldStart = lpStart;
		}

		if ( ReadFromBufferEx (lpStart, &lpString) )
		{
			char *lpComment = strstr (lpString, "//");

			if ( lpComment )
				*lpComment = 0;

			char *lpComa = strchr (lpString, ',');

			if ( lpComa )
				*lpComa = 0;

			Trim (lpString);

			if ( !*lpString )
				continue;

			sprintf (lpTmp, "%s\r\n", lpString);

			WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

			free(lpString);

			while ( true )
			{
				bool bSame=true;

				for (int i = 0; i < dwLangs; i++)
				{
					if ( pLangEntries[i].lpBuffer ) // or we skipping, no file read
					{
						pLangEntries[i].lpOldStart = pLangEntries[i].lpStart;
						bSame = bSame && ReadFromBufferEx (pLangEntries[i].lpStart, &pLangEntries[i].lpString);
					}
				}

				for (int i = 1; bSame && i < dwLangs; i++)
				{
					if ( pLangEntries[i].lpBuffer ) // or we skipping, no file read
					{
						if ( !pLangEntries[i].lpString ||
								(pLangEntries[i].lpString[0] == '"') ||
								strcmp (pLangEntries[0].lpString,pLangEntries[i].lpString) )
							bSame=false;
					}
				}

				if ( !bSame )
				{
					for (int i = 0; i < dwLangs; i++)
					{
						pLangEntries[i].lpStart = pLangEntries[i].lpOldStart;

						if ( pLangEntries[i].lpString )
							free(pLangEntries[i].lpString);
					}

					break;
				}

				if ( *pLangEntries[0].lpString )
				{
					sprintf (lpTmp, "l:%s\r\n", pLangEntries[0].lpString);
					WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);
				}

				for (int i = 0; i < dwLangs; i++)
					free (pLangEntries[i].lpString);
			}

			for (int i = 0; i < dwLangs; i++)
			{
				if ( pLangEntries[i].lpBuffer ) // or we skipping, no file read
				{
					while ( ReadFromBufferEx (
							pLangEntries[i].lpStart,
							&pLangEntries[i].lpString
							) && pLangEntries[i].lpString[0] != '"' )
					{
						sprintf(lpTmp,"ls:%s\r\n",pLangEntries[i].lpString);

						WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

						free(pLangEntries[i].lpString);
					}

					strcpy(lpTmp,pLangEntries[i].lpString);

					FixQuotes(lpTmp);

					strcat(lpTmp,"\r\n");

					WriteFile (hFeedFile, lpTmp, strlen (lpTmp), &dwRead, NULL);

					free(pLangEntries[i].lpString);
				}
			}
		}
		else
			break;
	}

	CloseHandle(hFeedFile);

	free(pHBuffer);

	for (int i = 0; i < dwLangs; i++)
		free(pLangEntries[i].lpBuffer);

	return 0;
}

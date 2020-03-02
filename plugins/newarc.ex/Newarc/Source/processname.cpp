#define PF_FLAG_QUOTE_SPACES		1 //Q
#define PF_FLAG_QUOTE_ALL			2 //q
#define PF_FLAG_USE_BACKSLASH		4 //S
#define PF_FLAG_DIR_NAME_AS_MASK	8 //M
#define PF_FLAG_DIR_NAME_AS_NAME	16 //N
#define PF_FLAG_NAME_ONLY			32 //W
#define PF_FLAG_PATH_ONLY			64 //P
#define PF_FLAG_ANSI_CHARSET		128 //A
#define PF_FLAG_UTF8_CHARSET		256 //8
#define PF_FLAG_UTF16_CHARSET		512 //U

#define QUERY_AND_SET_PARAM_FLAG(c, flag) \
	case c: \
		dwFlags |= flag; \
		break;


const TCHAR *GetFlags(const TCHAR *p, DWORD &dwFlags)
{
	dwFlags = 0;

	while ( *p && (*p != _T(' ')) && (*p != _T('}')) )
	{
		switch ( *p )
		{
			QUERY_AND_SET_PARAM_FLAG (_T('Q'), PF_FLAG_QUOTE_SPACES);
			QUERY_AND_SET_PARAM_FLAG (_T('q'), PF_FLAG_QUOTE_ALL);
			QUERY_AND_SET_PARAM_FLAG (_T('S'), PF_FLAG_USE_BACKSLASH);
			QUERY_AND_SET_PARAM_FLAG (_T('M'), PF_FLAG_DIR_NAME_AS_MASK);
			QUERY_AND_SET_PARAM_FLAG (_T('N'), PF_FLAG_DIR_NAME_AS_NAME);
			QUERY_AND_SET_PARAM_FLAG (_T('W'), PF_FLAG_NAME_ONLY);
			QUERY_AND_SET_PARAM_FLAG (_T('P'), PF_FLAG_PATH_ONLY);
			QUERY_AND_SET_PARAM_FLAG (_T('A'), PF_FLAG_ANSI_CHARSET);
			QUERY_AND_SET_PARAM_FLAG (_T('8'), PF_FLAG_UTF8_CHARSET);
			QUERY_AND_SET_PARAM_FLAG (_T('U'), PF_FLAG_UTF16_CHARSET);
		}

		p++;
	}

	return p;
}

#define QUERY_AND_SET_PARAM(str) \
	p++; \
	p = GetFlags (p, dwFlags); \
	if ( str && *str ) \
	{\
		_tcscat (lpResult, str); \
		n += StrLength(str); \
		bEmpty = false; \
	};\
	break;

struct ParamStruct {

	string strArchiveName;
	string strShortArchiveName;
	string strPassword;
	string strAdditionalCommandLine;

	string strTempPath;
	string strPathInArchive;
	string strListFileName;
};

void ProcessName (
		const TCHAR *lpFileName,
		string& strResult,
		int dwFlags,
		bool bForList
		)
{
	strResult = lpFileName;

	if ( OptionIsOn(dwFlags, PF_FLAG_PATH_ONLY) )
		CutToSlash(strResult);

	if ( bForList )
	{
		if ( OptionIsOn(dwFlags, PF_FLAG_NAME_ONLY) )
			if ( !OptionIsOn(dwFlags, PF_FLAG_PATH_ONLY) )
				strResult = FSF.PointToName(lpFileName);

		if ( OptionIsOn(dwFlags, PF_FLAG_USE_BACKSLASH) )
		{
			TCHAR* pBuffer = strResult.GetBuffer();

			for (unsigned int i = 0; i < strResult.GetLength(); i++)
			{
				if ( pBuffer[i] == _T('\\') )
					pBuffer[i] = _T('/');
			}

			strResult.ReleaseBuffer();
		}

		if ( OptionIsOn(dwFlags, PF_FLAG_QUOTE_SPACES) )
			farQuoteSpaceOnly(strResult);

		if ( OptionIsOn(dwFlags, PF_FLAG_QUOTE_ALL) )
			Quote(strResult); //NOT TESTED!
	}
}

void WriteLine(HANDLE hFile, const TCHAR* lpLine, DWORD dwFlags)
{
	DWORD dwWritten;
	string strProcessed;

	ProcessName(
			lpLine,
			strProcessed,
			dwFlags,
			true
			);

	//надо еще UTF-8 добавить и конвертацию OEM/ANSI

	if ( OptionIsOn(dwFlags, PF_FLAG_UTF16_CHARSET) )
	{
		const wchar_t* lpwszCRLF = L"\r\n";

#ifdef UNICODE
		const wchar_t* lpBuffer = strProcessed.GetString();
#else
		wchar_t* lpBuffer = AnsiToUnicode(strProcessed);
#endif

		WriteFile (hFile, lpBuffer, wcslen(lpBuffer)*sizeof(wchar_t), &dwWritten, NULL);
		WriteFile (hFile, lpwszCRLF, 2*sizeof(wchar_t), &dwWritten, NULL);

#ifndef UNICODE
		free(lpBuffer);
#endif
	}
	else
	{
		char* lpBuffer = nullptr;
		const char* lpCRLF = "\r\n";

		if ( OptionIsOn(dwFlags, PF_FLAG_UTF8_CHARSET) )
		{
#ifdef UNICODE
			lpBuffer = UnicodeToUTF8(strProcessed);
#else
			lpBuffer = AnsiToUnicode(strProcessed);
#endif
		}
		else
		{
#ifdef UNICODE
			lpBuffer = UnicodeToAnsi(strProcessed);
#else
			lpBuffer = StrDuplicate(strProcessed);
#endif
			if ( OptionIsOn(dwFlags, PF_FLAG_ANSI_CHARSET) )
				OemToCharA(lpBuffer, lpBuffer);
		}

		WriteFile (hFile, lpBuffer, strlen(lpBuffer), &dwWritten, NULL);
		WriteFile (hFile, lpCRLF, 2, &dwWritten, NULL);

		free(lpBuffer);
	}
}

void CreateListFile(
		const ArchiveItemArray& items,
		const TCHAR *lpListFileName,
		int dwFlags
		)
{
	//FarPanelInfo info;

	HANDLE hListFile = CreateFile (
			lpListFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
			);

	if ( hListFile != INVALID_HANDLE_VALUE )
	{
		WORD wSignature = 0xFEFF;
		DWORD dwWritten;

		if ( OptionIsOn(dwFlags, PF_FLAG_UTF16_CHARSET) )
			WriteFile(hListFile, &wSignature, 2, &dwWritten, NULL);

		for (unsigned int i = 0; i < items.count(); i++)
		{
			const ArchiveItem *item = &items[i];

			if ( (item->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( (dwFlags & PF_FLAG_DIR_NAME_AS_MASK) == PF_FLAG_DIR_NAME_AS_MASK )
				{
					string strFileName = item->lpFileName;
					AddEndSlash(strFileName);
					strFileName += _T("*.*");

					WriteLine(hListFile, strFileName, dwFlags);
				}
			}

			WriteLine(hListFile, item->lpFileName, dwFlags);
		}

		CloseHandle (hListFile);
	}
}


#define PE_SUCCESS		0
#define PE_MORE_FILES	1

int ParseString(
		const ArchiveItemArray& items,
		const TCHAR *lpString,
		string &strResult,
		ParamStruct *pParam,
		int &pStartItemNumber
		)
{
	const TCHAR *p = (TCHAR*)lpString;

	int bOnlyIfExists = 0;

	DWORD dwFlags;

	bool bHaveList = false;
	bool bHaveAdditionalOptions = false;

	bool bEmpty = false;

	int n = 0;
	int nSavedPos = 0;

	TCHAR* lpResult = strResult.GetBuffer(512); //BUGBUG
	string strProcessedName;

	int nResult = PE_SUCCESS;

	while ( *p )
	{
		switch ( *p )
		{

		case _T('{'):

        	bOnlyIfExists++;
			p++;

			bEmpty = true;
			nSavedPos = n-1;
			break;

		case _T('}'):

			bOnlyIfExists--;
			p++;

			if ( bEmpty )
			{
				lpResult[nSavedPos] = _T('\0');
				n = nSavedPos;
			}

			break;

		case _T('%'):

			if ( *(p+1) && (*(p+1) == _T('%')) )
			{
				p += 2;

				switch ( *p )
				{

				case _T('A'):
					p++;
					p = GetFlags (p, dwFlags);

					if ( !pParam->strArchiveName.IsEmpty() )
					{
						ProcessName (
								pParam->strArchiveName,
								strProcessedName,
								dwFlags,
								false
								);

						_tcscat (lpResult, strProcessedName);
						n += strProcessedName.GetLength();
						bEmpty = false;
					};
					break;

				case _T('a'):

					p++;
					p = GetFlags(p, dwFlags);

					if ( !pParam->strShortArchiveName.IsEmpty() )
					{
						ProcessName (
								pParam->strShortArchiveName,
								strProcessedName,
								dwFlags,
								false
								);

						_tcscat (lpResult, strProcessedName);
						n += strProcessedName.GetLength();
						bEmpty = false;
					};

					break;

				case _T('W'):
					QUERY_AND_SET_PARAM(pParam->strTempPath);

				case _T('P'):
					QUERY_AND_SET_PARAM(pParam->strPassword);

				case _T('R'):
					QUERY_AND_SET_PARAM(pParam->strPathInArchive);

				case _T('S'):
					bHaveAdditionalOptions = true;
					QUERY_AND_SET_PARAM(pParam->strAdditionalCommandLine);

				case _T('L'):
				case _T('l'):

					p++;
					p = GetFlags (p, dwFlags);

					if ( !bHaveList && !pParam->strListFileName.IsEmpty() )
					{
						bHaveList = true;

						CreateListFile (items, pParam->strListFileName, dwFlags);

						_tcscat (lpResult, pParam->strListFileName);
						n += StrLength (pParam->strListFileName);
						bEmpty = false;
					};

					break;

				case _T('f'):

					p++;
					p = GetFlags (p, dwFlags);

					if ( items[pStartItemNumber].lpFileName )
					{
						ProcessName (
								items[pStartItemNumber].lpFileName,
								strProcessedName,
								dwFlags,
								true
								);

						_tcscat (lpResult, strProcessedName);
						n += strProcessedName.GetLength();

						bEmpty = false;

						pStartItemNumber++;

						if ( pStartItemNumber != items.count() )
							nResult = PE_MORE_FILES;
					};

					break;

				default:
					p++;
					break;
				}
			}
			else
			{
				lpResult[n] = *p;
				lpResult[n+1] = _T('\0');

				n++;
				p++;
			}

			break;

		default:

			lpResult[n] = *p;
			lpResult[n+1] = _T('\0');

			n++;
			p++;
		}
	}

	if ( !pParam->strAdditionalCommandLine.IsEmpty() &&
		 !bHaveAdditionalOptions )
	{
		_tcscat (lpResult, _T(" "));
		_tcscat (lpResult, pParam->strAdditionalCommandLine);
	}

	strResult.ReleaseBuffer();

	return nResult;
}

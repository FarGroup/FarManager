#include "rar.h"

RarModule::RarModule (
		const TCHAR *lpFileName
		)
{
	string strModuleName = Info.ModuleName;

	CutToSlash(strModuleName);

	strModuleName += _T("unrar.dll");

	m_hModule = LoadLibraryEx (
			strModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnOpenArchiveEx = (RAROPENARCHIVEEX)GetProcAddress (m_hModule, "RAROpenArchiveEx");
		m_pfnCloseArchive = (RARCLOSEARCHIVE)GetProcAddress (m_hModule, "RARCloseArchive");
		m_pfnReadHeader = (RARREADHEADER)GetProcAddress (m_hModule, "RARReadHeader");
		m_pfnReadHeaderEx = (RARREADHEADEREX)GetProcAddress (m_hModule, "RARReadHeaderEx");
		m_pfnProcessFile = (RARPROCESSFILE)GetProcAddress (m_hModule, "RARProcessFile");
		m_pfnProcessFileW = (RARPROCESSFILEW)GetProcAddress (m_hModule, "RARProcessFileW");
		m_pfnSetCallback = (RARSETCALLBACK)GetProcAddress (m_hModule, "RARSetCallback");
		m_pfnSetProcessDataProc = (RARSETPROCESSDATAPROC)GetProcAddress (m_hModule, "RARSetProcessDataProc");
		m_pfnGetDllVersion = (RARGETDLLVERSION)GetProcAddress (m_hModule, "RARGetDllVersion");
		m_pfnSetPassword = (RARSETPASSWORD)GetProcAddress (m_hModule, "RARSetPassword");
	}
}

RarModule::~RarModule ()
{
	FreeLibrary (m_hModule);
}


RarArchive::RarArchive (RarModule* pModule, const TCHAR* lpFileName)
{
	m_pModule = pModule;
	m_strFileName = lpFileName;

	m_bAborted = false;
}

RarArchive::~RarArchive ()
{
}

bool RarArchive::OpenArchive (
		int nOpMode,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_pfnCallback = pfnCallback;
	m_hCallback = hCallback;

	RAROpenArchiveDataEx arcData;

	memset (&arcData, 0, sizeof (arcData));

	if ( nOpMode == OM_LIST )
		m_nOpMode = RAR_OM_LIST;
	else
		m_nOpMode = RAR_OM_EXTRACT;

	UNICODE_NAME(arcData.ArcName) = (TCHAR*)m_strFileName.GetString();
	arcData.OpenMode = m_nOpMode;

	m_hArchive = m_pModule->m_pfnOpenArchiveEx (&arcData);

	if ( arcData.OpenResult == 0 )
	{
		m_pModule->m_pfnSetCallback (m_hArchive, (UNRARCALLBACK)RarCallback, (LONG)this);
		return true;
	}

	return false;
}

void RarArchive::CloseArchive ()
{
	m_pModule->m_pfnCloseArchive (m_hArchive);
}

int RarArchive::GetArchiveItem (
		ArchiveItem *pItem
		)
{
	RARHeaderDataEx *fileHeader = (RARHeaderDataEx*)malloc(sizeof(RARHeaderDataEx));

	int nResult = m_pModule->m_pfnReadHeaderEx (m_hArchive, fileHeader);

	if ( !nResult )
	{
		memset(pItem, 0, sizeof(ArchiveItem));

		pItem->lpFileName = StrDuplicate(UNICODE_NAME(fileHeader->FileName));

		pItem->dwFileAttributes = (BYTE)(fileHeader->FileAttr >> 16); //бред!

		if ( (fileHeader->Flags & 0x04) == 0x04 )
			pItem->dwFlags |= AIF_CRYPTED;

		FILETIME lFileTime;

		DosDateTimeToFileTime (HIWORD(fileHeader->FileTime), LOWORD(fileHeader->FileTime), &lFileTime);
		LocalFileTimeToFileTime (&lFileTime, &pItem->ftLastWriteTime);

		pItem->nFileSize = ((__int64)fileHeader->UnpSizeHigh << 32)+(__int64)fileHeader->UnpSize;

		m_pModule->m_pfnProcessFile (m_hArchive, RAR_SKIP, NULL, NULL);

		free(fileHeader);

		return E_SUCCESS;
	}

	free(fileHeader);

	if ( nResult == ERAR_END_ARCHIVE )
		return E_EOF;

	return E_BROKEN;
}

bool RarArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	StrFree(pItem->lpFileName);
	StrFree(pItem->lpAlternateFileName);

	return true;
}


int __stdcall RarArchive::RarCallback (
		int nMsg,
		LONG UserData,
		int nParam1,
		int nParam2
		)
{
	RarArchive* pArchive = (RarArchive*)UserData; //and what about x64?

	if ( nMsg == UCM_NEEDPASSWORD )
	{
		ArchivePassword Password;

		Password.lpBuffer = StrCreate (260);
		Password.dwBufferSize = 260;

		if ( pArchive->Callback (AM_NEED_PASSWORD, (pArchive->m_nOpMode == RAR_OM_LIST)?PASSWORD_LIST:PASSWORD_FILE, (LONG_PTR)&Password) );
		{
#ifdef UNICODE
			char *lpPassword = UnicodeToAnsi(Password.lpBuffer);

			lstrcpynA((char*)nParam1, lpPassword, nParam2);

			free(lpPassword);
#else
			lstrcpyn ((char*)nParam1, Password.lpBuffer, nParam2);
#endif
		}

		StrFree (Password.lpBuffer);
	}

	if ( nMsg == UCM_PROCESSDATA )
	{
		if ( !pArchive->Callback (AM_PROCESS_DATA, nParam1, (LONG_PTR)nParam2) )
		{
			pArchive->m_bAborted = true; // а нужно ли...
			return -1;
		}
	}

	return 1;
}


bool RarArchive::Test(
		const ArchiveItem *pItems,
		int nItemsNumber
		)
{
/*	int nResult = 0;

	RARHeaderData fileHeader;

	while ( nResult == 0 )
	{
		nResult = m_pModule->m_pfnReadHeader (m_hArchive, &fileHeader);

		if ( nResult == 0 )
		{
			PluginPanelItem Item;

			strcpy (Item.FindData.cFileName, fileHeader.FileName);

			m_pfnCallback (AM_START_EXTRACT_FILE, (dword)&Item, (dword)"123");

        	m_pModule->m_pfnProcessFile (m_hArchive, RAR_TEST, NULL, NULL);

		}
	}*/

	return true;
}


bool RarArchive::Extract (
		const ArchiveItem *pItems,
		int nItemsNumber,
		const TCHAR *lpDestPath,
		const TCHAR *lpCurrentFolder
		)
{
	int nProcessed = 0;
	int nResult = 0;

	RARHeaderDataEx *fileHeader = (RARHeaderDataEx *)malloc (sizeof(RARHeaderDataEx));

	bool bFound;

	string strDestName;

	Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	while ( nResult == 0 )
	{
		nResult = m_pModule->m_pfnReadHeaderEx (m_hArchive, fileHeader);

		if ( nResult == 0 )
		{
			bFound = false;

			for (int i = 0; i < nItemsNumber; i++)
			{
				if ( !_tcscmp(pItems[i].lpFileName, UNICODE_NAME(fileHeader->FileName)) )
				{
					strDestName = lpDestPath;

					AddEndSlash (strDestName);

					const TCHAR *lpName = pItems[i].lpFileName;

					if ( *lpCurrentFolder /*&& !strncmp (
							lpName,
							lpCurrentFolder,
							strlen (lpCurrentFolder)
							)*/ )
						lpName += _tcslen(lpCurrentFolder); //BUGBUG

					if ( *lpName == _T('\\') )
						lpName++;

					strDestName += lpName;

					ProcessFileStruct pfs;

					pfs.lpDestName = strDestName;
					pfs.pItem = &pItems[i];

					Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

#ifdef UNICODE
					if ( m_pModule->m_pfnProcessFileW (m_hArchive, RAR_EXTRACT, NULL, strDestName) == 0 )
						nProcessed++;
#else
					wchar_t *lpDestNameW = AnsiToUnicode(strDestName);

					if ( m_pModule->m_pfnProcessFileW (m_hArchive, RAR_EXTRACT, NULL, lpDestNameW) == 0 )
						nProcessed++;

					free   (lpDestNameW);
#endif

					if ( m_bAborted || (nProcessed == nItemsNumber) )
						goto l_1;

					bFound = true;

					break;
				}
			}

			if ( !bFound )
				m_pModule->m_pfnProcessFile (m_hArchive, RAR_SKIP, NULL, NULL);
		}
	}

l_1:

	m_bAborted = false;

	free(fileHeader);

	return (bool)nProcessed;
}


LONG_PTR RarArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}

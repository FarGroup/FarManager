#include <FarPluginBase.h>
#ifdef _DEBUG
#include <debug.h>
#endif
#include "rar.class.h"
#include "dll.hpp"

RarModule::RarModule (
		const char *lpFileName
		)
{
	/*
	m_hModule = LoadLibraryEx (
			"unrar.dll",
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);
	*/

	char *lpModuleName = StrDuplicate(Info.ModuleName, 260);

	CutToSlash(lpModuleName);

	strcat (lpModuleName, "unrar.dll");

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	StrFree (lpModuleName);

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

RarArchive::RarArchive (RarModule *pModule, const char *lpFileName)
{
	m_pModule = pModule;

	CreateClassThunk (RarArchive, RarCallback, m_pfnRarCallback);
	CreateClassThunk (RarArchive, RarProcessDataProc, m_pfnRarProcessDataProc);

	m_lpFileName = StrDuplicate (lpFileName);

	m_bAborted = false;
}

RarArchive::~RarArchive ()
{
	StrFree (m_lpFileName);
	free (m_pfnRarCallback);
	free (m_pfnRarProcessDataProc);
}

bool __stdcall RarArchive::pOpenArchive (
		int nOpMode,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_pfnCallback = pfnCallback;

	RAROpenArchiveDataEx arcData;

	memset (&arcData, 0, sizeof (arcData));

	if ( nOpMode == OM_LIST )
		nOpMode = RAR_OM_LIST;
	else
		nOpMode = RAR_OM_EXTRACT;

	arcData.ArcName = m_lpFileName;
	arcData.OpenMode = nOpMode;

	m_hArchive = m_pModule->m_pfnOpenArchiveEx (&arcData);

	if ( arcData.OpenResult == 0 )
	{
		if ( (arcData.Flags & 0x0080) == 0x0080 )
		{
			ArchivePassword Password;

			Password.lpBuffer = StrCreate (260);
			Password.dwBufferSize = 260;

			if ( m_pfnCallback (AM_NEED_PASSWORD, PASSWORD_LIST, (int)&Password) )
				m_pModule->m_pfnSetPassword (m_hArchive, Password.lpBuffer);

			StrFree (Password.lpBuffer);
		}

	//	m_pModule->m_pfnSetCallback (m_hArchive, (UNRARCALLBACK)m_pfnRarCallback, NULL);
		m_pModule->m_pfnSetProcessDataProc (m_hArchive, (PROCESSDATAPROC)m_pfnRarProcessDataProc);

		return true;
	}

	return false;
}

void __stdcall RarArchive::pCloseArchive ()
{
	m_pModule->m_pfnCloseArchive (m_hArchive);
}

int __stdcall RarArchive::pGetArchiveItem (
		ArchiveItemInfo *pItem
		)
{
	RARHeaderDataEx fileHeader;

	int nResult = m_pModule->m_pfnReadHeaderEx (m_hArchive, &fileHeader);

	if ( !nResult )
	{
		memset (pItem, 0, sizeof (PluginPanelItem));

		strcpy ((char*)pItem->pi.FindData.cFileName, fileHeader.FileName);

		pItem->pi.FindData.dwFileAttributes = fileHeader.FileAttr;

		if ( (fileHeader.Flags & 0x04) == 0x04 )
			pItem->dwFlags |= AIF_CRYPTED;

		FILETIME lFileTime;

		DosDateTimeToFileTime (HIWORD(fileHeader.FileTime), LOWORD(fileHeader.FileTime), &lFileTime);
		LocalFileTimeToFileTime (&lFileTime, &pItem->pi.FindData.ftLastWriteTime);

		pItem->pi.FindData.nFileSizeHigh = fileHeader.UnpSizeHigh;
		pItem->pi.FindData.nFileSizeLow = fileHeader.UnpSize;

		m_pModule->m_pfnProcessFile (m_hArchive, RAR_SKIP, NULL, NULL);

		return E_SUCCESS;
	}

	if ( nResult == ERAR_END_ARCHIVE )
		return E_EOF;

	return E_BROKEN;
}

int __stdcall RarArchive::RarCallback (
		int nMsg,
		void *pParam,
		int nParam1,
		int nParam2
		)
{
/*	if ( nMsg == UCM_NEEDPASSWORD )
	{
		char *lpPassword = (char*)m_pfnCallback (AM_NEED_PASSWORD, 0, 0);

		if ( lpPassword )
			m_pModule->m_pfnSetPassword (m_hArchive, lpPassword);
	}*/

	return 1;
}

int __stdcall RarArchive::RarProcessDataProc (
		unsigned char *Addr,
		int Size
		)
{
	int nResult = m_pfnCallback (AM_PROCESS_DATA, (dword)Addr, (dword)Size);

	if ( nResult == 0 )
	{
		m_bAborted = true;
		//nResult = -1;
	}

	return nResult;
}

bool __stdcall RarArchive::pTest (
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	int nResult = 0;

	RARHeaderData fileHeader;

	while ( nResult == 0 )
	{
		nResult = m_pModule->m_pfnReadHeader (m_hArchive, &fileHeader);

		if ( nResult == 0 )
		{
			PluginPanelItem Item;

			strcpy (Item.FindData.cFileName, fileHeader.FileName);

			if ( (fileHeader.Flags & 0x04) == 0x04 )
			{
				ArchivePassword Password;

				Password.lpBuffer = StrCreate (260);
				Password.dwBufferSize = 260;

				if ( m_pfnCallback (AM_NEED_PASSWORD, PASSWORD_FILE, (int)&Password) )
					m_pModule->m_pfnSetPassword (m_hArchive, Password.lpBuffer);

				StrFree (Password.lpBuffer);
			}


			m_pfnCallback (AM_START_EXTRACT_FILE, (dword)&Item, (dword)"123");

        	#ifdef _DEBUG
        	int nError =
        	#endif

        	m_pModule->m_pfnProcessFile (m_hArchive, RAR_TEST, NULL, NULL);

			#ifdef _DEBUG
			if ( nError )
				__debug ("error - %d", nError);
			#endif
		}
	}

	return true;
}


bool __stdcall RarArchive::pExtract (
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	int nProcessed = 0;
	int nResult = 0;

	RARHeaderDataEx fileHeader;

	bool bFound;

	char *lpDestName = StrCreate (260);

	while ( nResult == 0 )
	{
		nResult = m_pModule->m_pfnReadHeaderEx (m_hArchive, &fileHeader);

		if ( nResult == 0 )
		{
			bFound = false;

			for (int i = 0; i < nItemsNumber; i++)
			{
				if ( !strcmp (pItems[i].FindData.cFileName, fileHeader.FileName) )
				{
					strcpy (lpDestName, lpDestPath);

					FSF.AddEndSlash (lpDestName);

					char *lpName = pItems[i].FindData.cFileName;

					if ( *lpCurrentFolder /*&& !strncmp (
							lpName,
							lpCurrentFolder,
							strlen (lpCurrentFolder)
							)*/ )
						lpName += strlen (lpCurrentFolder);

					if ( *lpName == '\\' )
						lpName++;

					strcat (lpDestName, lpName);

					if ( (fileHeader.Flags & 0x04) == 0x04 )
					{
						ArchivePassword Password;

						Password.lpBuffer = StrCreate (260);
						Password.dwBufferSize = 260;

						if ( m_pfnCallback (AM_NEED_PASSWORD, PASSWORD_FILE, (int)&Password) )
							m_pModule->m_pfnSetPassword (m_hArchive, Password.lpBuffer);

						StrFree (Password.lpBuffer);
					}


					m_pfnCallback (AM_START_EXTRACT_FILE, (dword)&pItems[i], (dword)lpDestName);

					wchar_t *lpDestNameW = (wchar_t*)malloc (2*260);

					MultiByteToWideChar (CP_OEMCP, 0, lpDestName, 260, lpDestNameW, 260);

					if ( m_pModule->m_pfnProcessFileW (m_hArchive, RAR_EXTRACT, NULL, lpDestNameW) == 0 )
						nProcessed++;

					free   (lpDestNameW);

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


	StrFree (lpDestName);

	return (bool)nProcessed;
}

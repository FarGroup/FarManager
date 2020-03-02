#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "rar.class.h"
#include "dll.hpp"

RarModule::RarModule (
		const char *lpFileName
		)
{
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

LONG_PTR __stdcall RarCallbackThunk(
		int nMsg,
		void *pParam,
		int nParam1,
		int nParam2
		)
{
	RarArchive *p = (RarArchive*)THUNK_MAGIC;
	return p->RarCallback(nMsg, pParam, nParam1, nParam2);
}


RarArchive::RarArchive (RarModule *pModule, const char *lpFileName)
{
	m_pModule = pModule;

	m_pfnRarCallback = CreateThunkFastEx (this, (void *)RarCallbackThunk);
	m_lpFileName = StrDuplicate (lpFileName);

	m_bAborted = false;
}

RarArchive::~RarArchive ()
{
	StrFree (m_lpFileName);
	ReleaseThunkEx (m_pfnRarCallback);
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
		m_nOpMode = RAR_OM_LIST;
	else
		m_nOpMode = RAR_OM_EXTRACT;

	arcData.ArcName = m_lpFileName;
	arcData.OpenMode = m_nOpMode;

	m_hArchive = m_pModule->m_pfnOpenArchiveEx (&arcData);

	if ( arcData.OpenResult == 0 )
	{
		m_pModule->m_pfnSetCallback (m_hArchive, (UNRARCALLBACK)m_pfnRarCallback, 0);
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
	RARHeaderDataEx *fileHeader = (RARHeaderDataEx *)malloc (sizeof(RARHeaderDataEx));

	int nResult = m_pModule->m_pfnReadHeaderEx (m_hArchive, fileHeader);

	if ( !nResult )
	{
		memset (pItem, 0, sizeof (PluginPanelItem));

		strcpy ((char*)pItem->pi.FindData.cFileName, fileHeader->FileName);

		pItem->pi.FindData.dwFileAttributes = (BYTE)(fileHeader->FileAttr >> 16); //бред!

		if ( (fileHeader->Flags & 0x04) == 0x04 )
			pItem->dwFlags |= AIF_CRYPTED;

		FILETIME lFileTime;

		DosDateTimeToFileTime (HIWORD(fileHeader->FileTime), LOWORD(fileHeader->FileTime), &lFileTime);
		LocalFileTimeToFileTime (&lFileTime, &pItem->pi.FindData.ftLastWriteTime);

		pItem->pi.FindData.nFileSizeHigh = fileHeader->UnpSizeHigh;
		pItem->pi.FindData.nFileSizeLow = fileHeader->UnpSize;

		m_pModule->m_pfnProcessFile (m_hArchive, RAR_SKIP, NULL, NULL);

		free(fileHeader);

		return E_SUCCESS;
	}

	free(fileHeader);

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
	if ( nMsg == UCM_NEEDPASSWORD )
	{
		ArchivePassword Password;

		Password.lpBuffer = StrCreate (260);
		Password.dwBufferSize = 260;

		if ( Callback (AM_NEED_PASSWORD, (m_nOpMode == RAR_OM_LIST)?PASSWORD_LIST:PASSWORD_FILE, (LONG_PTR)&Password) );
		{
			lstrcpyn ((char*)nParam1, Password.lpBuffer, nParam2);
		}

		StrFree (Password.lpBuffer);
	}

	if ( nMsg == UCM_PROCESSDATA )
	{
		if ( !Callback (AM_PROCESS_DATA, nParam1, (LONG_PTR)nParam2) )
		{
			m_bAborted = true; // а нужно ли...
			return -1;
		}
	}

	return 1;
}


bool __stdcall RarArchive::pTest (
		PluginPanelItem *pItems,
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


bool __stdcall RarArchive::pExtract (
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	int nProcessed = 0;
	int nResult = 0;

	RARHeaderDataEx *fileHeader = (RARHeaderDataEx *)malloc (sizeof(RARHeaderDataEx));

	bool bFound;

	char *lpDestName = StrCreate (260);

	Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	while ( nResult == 0 )
	{
		nResult = m_pModule->m_pfnReadHeaderEx (m_hArchive, fileHeader);

		if ( nResult == 0 )
		{
			bFound = false;

			for (int i = 0; i < nItemsNumber; i++)
			{
				if ( !strcmp (pItems[i].FindData.cFileName, fileHeader->FileName) )
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

					ProcessFileStruct pfs;

					pfs.lpDestFileName = lpDestName;
					pfs.pItem = &pItems[i];

					Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

					wchar_t *lpDestNameW = (wchar_t *)malloc (2*260);

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

	free(fileHeader);

	StrFree (lpDestName);

	return (bool)nProcessed;
}


LONG_PTR RarArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (nMsg, nParam1, nParam2);

	return FALSE;
}

#include <FarPluginBase.hpp>
#include "zip.class.h"
#include <debug.h>

void ConvertSlashes (char *lpFileName)
{
	for (size_t i = 0; i < strlen (lpFileName); i++)
		if ( lpFileName[i] == '/' )
			lpFileName[i] = '\\';
}

ZipModule::ZipModule (
		const char *lpFileName
		)
{
	/*
	m_hModule = LoadLibraryEx (
			"zlib.dll",
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);
	*/

	char *lpModuleName = StrDuplicate(Info.ModuleName, 260);

	CutToSlash(lpModuleName);

	strcat (lpModuleName, "zlib.dll");

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	StrFree (lpModuleName);

	if ( m_hModule )
	{
		m_pfnUnzOpen = (UNZOPEN)GetProcAddress (m_hModule, "unzOpen");
		m_pfnUnzOpen2 = (UNZOPEN2)GetProcAddress (m_hModule, "unzOpen2");
		m_pfnUnzClose = (UNZCLOSE)GetProcAddress (m_hModule, "unzClose");
		m_pfnUnzGoToFirstFile = (UNZGOTOFIRSTFILE)GetProcAddress (m_hModule, "unzGoToFirstFile");
		m_pfnUnzGoToNextFile = (UNZGOTONEXTFILE)GetProcAddress (m_hModule, "unzGoToNextFile");
		m_pfnUnzGetCurrentFileInfo = (UNZGETCURRENTFILEINFO)GetProcAddress (m_hModule, "unzGetCurrentFileInfo");
		m_pfnUnzGetGlobalInfo = (UNZGETGLOBALINFO)GetProcAddress (m_hModule, "unzGetGlobalInfo");

		m_pfnUnzOpenCurrentFile = (UNZOPENCURRENTFILE)GetProcAddress (m_hModule, "unzOpenCurrentFile");
		m_pfnUnzOpenCurrentFilePassword = (UNZOPENCURRENTFILEPASSWORD)GetProcAddress (m_hModule, "unzOpenCurrentFilePassword");
		m_pfnUnzCloseCurrentFile = (UNZCLOSECURRENTFILE)GetProcAddress (m_hModule, "unzCloseCurrentFile");
		m_pfnUnzReadCurrentFile = (UNZREADCURRENTFILE)GetProcAddress (m_hModule, "unzReadCurrentFile");
	}
}

ZipModule::~ZipModule ()
{
	FreeLibrary (m_hModule);
}

ZipArchive::ZipArchive (ZipModule *pModule, const char *lpFileName)
{
	m_pModule = pModule;

	m_lpFileName = StrDuplicate (lpFileName);
}

ZipArchive::~ZipArchive ()
{
	StrFree (m_lpFileName);
}

bool __stdcall ZipArchive::pOpenArchive (
		int nOpMode,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_pfnCallback = pfnCallback;

	m_hFile = m_pModule->m_pfnUnzOpen (m_lpFileName);

	if ( m_hFile )
	{
		m_bFirst = true;
		return true;
	}

	return false;
}

void __stdcall ZipArchive::pCloseArchive ()
{
	m_pModule->m_pfnUnzClose (m_hFile);
}

int __stdcall ZipArchive::pGetArchiveItem (
		ArchiveItemInfo *pItem
		)
{
	int nResult;
	unz_file_info fileInfo;

	if ( m_bFirst )
	{
		nResult = m_pModule->m_pfnUnzGoToFirstFile (m_hFile);
		m_bFirst = false;
	}
	else
	    nResult = m_pModule->m_pfnUnzGoToNextFile (m_hFile);

	if ( nResult == UNZ_OK )
	{
		nResult = m_pModule->m_pfnUnzGetCurrentFileInfo (
					m_hFile,
					&fileInfo,
					pItem->pi.FindData.cFileName,
					260,
					NULL,
					0,
					NULL,
					0
					);

		if ( nResult == UNZ_OK )
		{
			ConvertSlashes (pItem->pi.FindData.cFileName);

			pItem->pi.FindData.dwFileAttributes = fileInfo.external_fa;
			pItem->pi.FindData.nFileSizeLow = fileInfo.uncompressed_size;

			FILETIME lFileTime;

			DosDateTimeToFileTime (HIWORD(fileInfo.dosDate), LOWORD(fileInfo.dosDate), &lFileTime);
			LocalFileTimeToFileTime (&lFileTime, &pItem->pi.FindData.ftLastWriteTime);

			return E_SUCCESS;
		}
	}

	if ( nResult == UNZ_END_OF_LIST_OF_FILE )
		return E_EOF;

	return E_BROKEN;
}

void CreateDirectory(char *FullPath) //$ 16.05.2002 AA
{
//	if( !FileExists (FullPath) )
	{
	    for (char *c = FullPath; *c; c++)
		{
      if(*c!=' ')
      {
        for(; *c; c++)
          if(*c=='\\')
          {
            *c=0;
            CreateDirectory(FullPath, NULL);
            *c='\\';
          }
        CreateDirectory(FullPath, NULL);
        break;
      }
		}
    }
}


void CreateDirs (const char *lpFileName)
{
	char *lpNameCopy = StrDuplicate (lpFileName);

	CutToSlash (lpNameCopy);

	CreateDirectory (lpNameCopy);

	StrFree (lpNameCopy);
}


bool __stdcall ZipArchive::pExtract (
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	int nProcessed = 0;
	int nResult = UNZ_OK;
	unz_file_info fileInfo;

	char *lpPassword = NULL;
	char *lpDestName = StrCreate (260);
	char *lpFileName = StrCreate (260);
	char *lpRealName = StrCreate (260);

	bool bAborted = false;

//	HANDLE hScreen = Info.SaveScreen (0, 0, -1, -1);

	while ( nResult == UNZ_OK )
	{
		if ( m_bFirst )
		{
			nResult = m_pModule->m_pfnUnzGoToFirstFile (m_hFile);
			m_bFirst = false;

			if ( nResult == UNZ_OK )
				Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);
		}
		else
		    nResult = m_pModule->m_pfnUnzGoToNextFile (m_hFile);

		if ( nResult == UNZ_OK )
		{
			char *lpPassword = NULL;

			nResult = m_pModule->m_pfnUnzGetCurrentFileInfo (
						m_hFile,
						&fileInfo,
						lpRealName,
						260,
						NULL,
						0,
						NULL,
						0
						);

			if ( fileInfo.flag & 1 )
			{
				ArchivePassword Password;

				Password.lpBuffer = StrCreate (260);
				Password.dwBufferSize = 260;

				if ( Callback (AM_NEED_PASSWORD, PASSWORD_FILE, (int)&Password) )
					lpPassword = StrDuplicate (Password.lpBuffer);

				StrFree (Password.lpBuffer);
			}

			strcpy (lpFileName, lpRealName);
			ConvertSlashes (lpFileName);

			if ( nResult == UNZ_OK )
			{
				for (int i = 0; i < nItemsNumber; i++)
				{
					if ( !strcmp (pItems[i].FindData.cFileName, lpFileName) )
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

						CreateDirs (lpDestName);

						HANDLE hFile = CreateFile (
								lpDestName,
								GENERIC_WRITE,
								FILE_SHARE_READ,
								NULL,
								CREATE_ALWAYS,
								0,
								NULL
								);

						if ( hFile != INVALID_HANDLE_VALUE )
						{
							if ( lpPassword )
								nResult = m_pModule->m_pfnUnzOpenCurrentFilePassword (
										m_hFile,
										lpPassword
										);
							else
								nResult = m_pModule->m_pfnUnzOpenCurrentFile (
										m_hFile
										);

							if ( nResult == UNZ_OK )
							{
								int nRead;
								DWORD dwWritten;
								char *pBuffer = (char *)malloc (128*1024);

								do {
									nRead = m_pModule->m_pfnUnzReadCurrentFile (
											m_hFile,
											(voidp)pBuffer,
											128*1024
											);

									if ( nRead > 0 )
									{
										WriteFile(hFile, pBuffer, nRead, &dwWritten, NULL);

										if ( !Callback (AM_PROCESS_DATA, 0, (LONG_PTR)nRead) )
										{
											bAborted = true;
											break;
										}
									}

								} while (nRead > 0);

								free(pBuffer);

								m_pModule->m_pfnUnzCloseCurrentFile (m_hFile);

								nProcessed++;
							}

							CloseHandle (hFile);
						}

						if ( (nProcessed == nItemsNumber) || bAborted )
							goto l_1;

						break;
					}
				}
			}
		}
	}

l_1:

	StrFree (lpPassword);
	StrFree (lpRealName);
	StrFree (lpDestName);
	StrFree (lpFileName);

//	Info.RestoreScreen (hScreen);

	return (bool)nProcessed;
}

LONG_PTR ZipArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (nMsg, nParam1, nParam2);

	return FALSE;
}

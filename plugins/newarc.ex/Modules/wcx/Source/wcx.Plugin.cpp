#include "wcx.h"

WcxPlugin::WcxPlugin(const GUID& uid)
{
	m_uid = uid;
	m_pFormatInfo = NULL;

	m_pfnOpenArchive = nullptr;
	m_pfnOpenArchiveW = nullptr;

	m_pfnReadHeader = nullptr;
	m_pfnReadHeaderEx = nullptr;
	m_pfnReadHeaderExW = nullptr;

	m_pfnProcessFile = nullptr;
	m_pfnProcessFileW = nullptr;

	m_pfnPackFiles = nullptr;
	m_pfnPackFilesW = nullptr;
	m_pfnDeleteFiles = nullptr;
	m_pfnDeleteFilesW = nullptr;

	m_pfnSetChangeVolProc = nullptr;
	m_pfnSetChangeVolProcW = nullptr;
	m_pfnSetProcessDataProc = nullptr;
	m_pfnSetProcessDataProcW = nullptr;

	m_pfnCanYouHandleThisFile = nullptr;
	m_pfnCanYouHandleThisFileW = nullptr;
	
	m_pfnCloseArchive = nullptr;
	m_pfnConfigurePacker = nullptr;
	m_pfnGetPackerCaps = nullptr;
	m_pfnPackSetDefaultParams = nullptr;
}

bool WcxPlugin::Load(const TCHAR* lpModuleName)
{
	m_strModuleName = lpModuleName;

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnOpenArchive = (PLUGINOPENARCHIVE)GetProcAddress(m_hModule,"OpenArchive");
		m_pfnCloseArchive = (PLUGINCLOSEARCHIVE)GetProcAddress(m_hModule,"CloseArchive");
		m_pfnReadHeader = (PLUGINREADHEADER)GetProcAddress(m_hModule,"ReadHeader");
		m_pfnProcessFile = (PLUGINPROCESSFILE)GetProcAddress(m_hModule,"ProcessFile");
		m_pfnPackFiles = (PLUGINPACKFILES)GetProcAddress(m_hModule,"PackFiles");
		m_pfnDeleteFiles = (PLUGINDELETEFILES)GetProcAddress(m_hModule,"DeleteFiles");
		m_pfnSetChangeVolProc = (PLUGINSETCHANGEVOLPROC)GetProcAddress(m_hModule,"SetChangeVolProc");
		m_pfnSetProcessDataProc = (PLUGINSETPROCESSDATAPROC)GetProcAddress(m_hModule,"SetProcessDataProc");
		m_pfnConfigurePacker = (PLUGINCONFIGUREPACKER)GetProcAddress(m_hModule,"ConfigurePacker");
		m_pfnGetPackerCaps = (PLUGINGETPACKERCAPS)GetProcAddress(m_hModule,"GetPackerCaps");
		m_pfnCanYouHandleThisFile = (PLUGINCANYOUHANDLETHISFILE)GetProcAddress(m_hModule,"CanYouHandleThisFile");
		m_pfnPackSetDefaultParams = (PLUGINPACKSETDEFAULTPARAMS)GetProcAddress(m_hModule,"PackSetDefaultParams");
		m_pfnReadHeaderEx = (PLUGINREADHEADEREX)GetProcAddress(m_hModule, "ReadHeaderEx");

		m_pfnOpenArchiveW = (PLUGINOPENARCHIVEW)GetProcAddress(m_hModule, "OpenArchiveW");
		m_pfnReadHeaderExW = (PLUGINREADHEADEREXW)GetProcAddress(m_hModule, "ReadHeaderExW");
		m_pfnProcessFileW = (PLUGINPROCESSFILEW)GetProcAddress(m_hModule, "ProcessFileW");
		m_pfnPackFilesW = (PLUGINPACKFILESW)GetProcAddress(m_hModule, "PackFilesW");
		m_pfnDeleteFilesW = (PLUGINDELETEFILESW)GetProcAddress(m_hModule, "DeleteFilesW");
		m_pfnSetChangeVolProcW = (PLUGINSETCHANGEVOLPROCW)GetProcAddress(m_hModule, "SetChangeVolProcW");
		m_pfnSetProcessDataProcW = (PLUGINSETPROCESSDATAPROCW)GetProcAddress(m_hModule, "SetProcessDataProcW");
		m_pfnCanYouHandleThisFileW = (PLUGINCANYOUHANDLETHISFILEW)GetProcAddress(m_hModule, "CanYouHandleThisFileW");

		if ( (m_pfnOpenArchive && m_pfnCloseArchive && m_pfnReadHeader && m_pfnProcessFile) ||
			 (m_pfnOpenArchiveW && m_pfnCloseArchive && m_pfnReadHeaderExW && m_pfnProcessFileW) )
		{
			if (m_pfnPackSetDefaultParams)
			{
				PackDefaultParamStruct dps;

				dps.size = sizeof(dps);
				dps.PluginInterfaceVersionLow = 20; //hmm
				dps.PluginInterfaceVersionHi = 2;
				strcpy(dps.DefaultIniName,"");

				m_pfnPackSetDefaultParams(&dps);
			}

			m_pFormatInfo = new ArchiveFormatInfo;
			memset(m_pFormatInfo, 0, sizeof(ArchiveFormatInfo));

			m_pFormatInfo->uid = CreateFormatUID(m_uid, FSF.PointToName(lpModuleName));
			
			string strName = FSF.PointToName(lpModuleName);
			CutTo(strName, _T('.'), true);

			string strExtention = strName;

			strName += _T(" [wcx]");

			m_pFormatInfo->lpName = StrDuplicate(strName);
			m_pFormatInfo->lpDefaultExtention = StrDuplicate(strExtention);

			m_pFormatInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;

			//emulate real TC
			if ( m_pfnGetPackerCaps )
			{
				int caps = m_pfnGetPackerCaps();

				if ( (caps & PK_CAPS_NEW) == PK_CAPS_NEW )
					m_pFormatInfo->dwFlags |= AFF_SUPPORT_INTERNAL_CREATE;

				if ( (caps & PK_CAPS_MODIFY) == PK_CAPS_MODIFY )
					m_pFormatInfo->dwFlags |= AFF_SUPPORT_INTERNAL_ADD;

				if ( (caps & PK_CAPS_DELETE) == PK_CAPS_DELETE )
					m_pFormatInfo->dwFlags |= AFF_SUPPORT_INTERNAL_DELETE;
					
				if ( (caps & PK_CAPS_OPTIONS) == PK_CAPS_OPTIONS )
					m_pFormatInfo->dwFlags |= AFF_SUPPORT_CONFIG_GENERAL;
			}

			return true;
		}
	}

	return false;
}

const GUID& WcxPlugin::GetUID()
{
	return m_uid;
}

const TCHAR* WcxPlugin::GetModuleName()
{
	return m_strModuleName;
}

const ArchiveFormatInfo* WcxPlugin::GetFormats()
{
	return m_pFormatInfo;
}

unsigned int WcxPlugin::GetNumberOfFormats()
{
	return 1;
}

WcxPlugin::~WcxPlugin()
{
	if ( m_pFormatInfo )
	{
		StrFree((void*)m_pFormatInfo->lpName);
		StrFree((void*)m_pFormatInfo->lpDefaultExtention);

		delete m_pFormatInfo;
	}

	FreeLibrary (m_hModule);
}


__int64 WcxPlugin::CreateFileList(const ArchiveItem* pItems, int nItemsNumber, int nOperation, void** pResult)
{
	bool bUnicode = false;

	if ( (nOperation == ARCHIVE_OPERATION_DELETE) && m_pfnDeleteFilesW )
		bUnicode = true;

	if ( (nOperation == ARCHIVE_OPERATION_ADD) && m_pfnPackFilesW )
		bUnicode = true;

	wchar_t* pListW = NULL;
	char* pList = NULL;

	if ( bUnicode )
		pListW = (wchar_t*)malloc(0);
	else
		pList = (char*)malloc(0);

	DWORD dwSize = 0;
	DWORD dwOffset = 0;

	__int64 nTotalSize = 0;

	for (int i = 0; i < nItemsNumber; i++)
	{
		string strFullName = pItems[i].lpFileName;

		if ( (pItems[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
			AddEndSlash(strFullName);

		nTotalSize += pItems[i].nFileSize;

		UNIVERSAL_ANSI_NAME_CREATE_EX(strFullName, lpFileName);

		if ( bUnicode )
		{
			size_t nLength = wcslen(lpFileNameW)+1;

			dwSize += (nLength+2)*sizeof(wchar_t);

			pListW = (wchar_t*)realloc(pListW, dwSize);

			memcpy(&pListW[dwOffset], lpFileNameW, nLength*sizeof(wchar_t));

			dwOffset += nLength;
		}
		else
		{
			size_t nLength = strlen(lpFileNameA)+1;

			dwSize += (nLength+2);

			pList = (char*)realloc(pList, dwSize);

			memcpy(&pList[dwOffset], lpFileNameA, nLength);

			dwOffset += nLength;
		}

		UNIVERSAL_NAME_DELETE(lpFileName);
	}

	if ( bUnicode )
	{
		pListW[dwOffset] = 0;
		*pResult = (void*)pListW;
	}
	else
	{
		pList[dwOffset] = 0;
		*pResult = (void*)pList;
	}

	return nTotalSize;
}


HANDLE WcxPlugin::OpenArchive(const TCHAR* lpFileName, int nOpMode)
{
	AnsiGuard guard;

	HANDLE hResult = NULL;

	UNIVERSAL_ANSI_NAME_CREATE(lpFileName);

	if ( m_pfnOpenArchiveW )
	{
		tOpenArchiveDataW OpenArchiveData;

		memset (&OpenArchiveData, 0, sizeof (OpenArchiveData));

		OpenArchiveData.ArcName = (wchar_t*)lpFileNameW;
		OpenArchiveData.OpenMode = nOpMode;

		hResult = m_pfnOpenArchiveW(&OpenArchiveData);
	}
	else

	if ( m_pfnOpenArchive )
	{
		tOpenArchiveData OpenArchiveData;

		memset (&OpenArchiveData, 0, sizeof (OpenArchiveData));

		OpenArchiveData.ArcName = (char*)lpFileNameA;
		OpenArchiveData.OpenMode = nOpMode;

		hResult = m_pfnOpenArchive (&OpenArchiveData);
	}

	UNIVERSAL_NAME_DELETE(lpFileName);

	return hResult;
}

void WcxPlugin::CloseArchive(HANDLE hArchive)
{
	AnsiGuard guard;

	if ( m_pfnCloseArchive )
		m_pfnCloseArchive(hArchive);
}

void WcxPlugin::SetCallbacks(
			HANDLE hArchive,
			tProcessDataProcW pfnProcessDataProcW,
			tProcessDataProc pfnProcessDataProc,
			tChangeVolProcW pfnSetChangeVolProcW,
			tChangeVolProc pfnSetChangeVolProc
			)
{
	if ( !hArchive )
		hArchive = INVALID_HANDLE_VALUE;

	if ( m_pfnSetProcessDataProcW ) 
		m_pfnSetProcessDataProcW(hArchive, pfnProcessDataProcW);

	else

	if ( m_pfnSetProcessDataProc )
		m_pfnSetProcessDataProc(hArchive, pfnProcessDataProc);

	if ( m_pfnSetChangeVolProcW )
		m_pfnSetChangeVolProcW(hArchive, pfnSetChangeVolProcW);
	else

	if ( m_pfnSetChangeVolProc )
		m_pfnSetChangeVolProc(hArchive, pfnSetChangeVolProc);
		
}

int WcxPlugin::ConvertResult(int nResult)
{
	switch ( nResult )
	{
		case 0:
			return E_SUCCESS;
		case E_END_ARCHIVE:
			return E_EOF;
		case E_BAD_ARCHIVE:
			return E_BROKEN;
		case E_BAD_DATA:
			return E_UNEXPECTED_EOF;
		case E_EREAD:
			return E_READ_ERROR;
	}

	return E_UNEXPECTED_EOF;
}


int WcxPlugin::GetArchiveItem(HANDLE hArchive, ArchiveItem* pItem)
{
	AnsiGuard guard;

	int nResult = 0;

	if ( m_pfnReadHeaderExW ) //unicode first
	{
		tHeaderDataExW HeaderData;

		memset(&HeaderData, 0, sizeof(HeaderData));

		nResult = m_pfnReadHeaderExW(hArchive, &HeaderData);

		if ( !nResult )
		{
#ifdef UNICODE
			pItem->lpFileName = StrDuplicate(HeaderData.FileName);
#else
			pItem->lpFileName = UnicodeToAnsi(HeaderData.FileName);
#endif
			pItem->dwCRC32 = HeaderData.FileCRC;
			pItem->dwFileAttributes = HeaderData.FileAttr;
			pItem->nFileSize = ((__int64)HeaderData.UnpSizeHigh << 32)+(__int64)HeaderData.UnpSize;
			pItem->nPackSize = ((__int64)HeaderData.PackSizeHigh << 32)+(__int64)HeaderData.PackSize;

			FILETIME filetime;
			DosDateTimeToFileTime ((WORD)((DWORD)HeaderData.FileTime >> 16), (WORD)HeaderData.FileTime, &filetime);
			LocalFileTimeToFileTime (&filetime, &pItem->ftLastWriteTime);
	
			//???
			pItem->ftCreationTime = pItem->ftLastAccessTime = pItem->ftLastWriteTime;
			pItem->dwCRC32 = HeaderData.FileCRC;

			return E_SUCCESS;
		}
	}
	else

	if ( m_pfnReadHeaderEx ) //Ex now
	{
		tHeaderDataEx HeaderData;

		memset(&HeaderData, 0, sizeof(HeaderData));

		nResult = m_pfnReadHeaderEx(hArchive, &HeaderData);

		if ( !nResult )
		{
			CharToOemA(HeaderData.FileName, HeaderData.FileName);

#ifdef UNICODE
			pItem->lpFileName = AnsiToUnicode(HeaderData.FileName);
#else
			pItem->lpFileName = StrDuplicate(HeaderData.FileName);
#endif
			pItem->dwCRC32 = HeaderData.FileCRC;
			pItem->dwFileAttributes = HeaderData.FileAttr;
			pItem->nFileSize = ((__int64)HeaderData.UnpSizeHigh << 32)+(__int64)HeaderData.UnpSize;
			pItem->nPackSize = ((__int64)HeaderData.PackSizeHigh << 32)+(__int64)HeaderData.PackSize;

			FILETIME filetime;
			DosDateTimeToFileTime ((WORD)((DWORD)HeaderData.FileTime >> 16), (WORD)HeaderData.FileTime, &filetime);
			LocalFileTimeToFileTime (&filetime, &pItem->ftLastWriteTime);
	
	        //???
			pItem->ftCreationTime = pItem->ftLastAccessTime = pItem->ftLastWriteTime;
			pItem->dwCRC32 = HeaderData.FileCRC;

			memcpy(pItem, pItem, sizeof(ArchiveItem));

			return E_SUCCESS;
		}
	}
	else

	if ( m_pfnReadHeader ) //fallback
	{
		tHeaderData HeaderData;
		memset (&HeaderData, 0, sizeof (HeaderData));

		nResult = m_pfnReadHeader(hArchive, &HeaderData);

		if ( !nResult )
		{
			CharToOemA(HeaderData.FileName, HeaderData.FileName);

#ifdef UNICODE
			pItem->lpFileName = AnsiToUnicode(HeaderData.FileName);
#else
			pItem->lpFileName = StrDuplicate(HeaderData.FileName);
#endif

			pItem->dwFileAttributes = HeaderData.FileAttr;
			pItem->nFileSize = HeaderData.UnpSize;  //low only
			pItem->nPackSize = HeaderData.PackSize;

			FILETIME filetime;
			DosDateTimeToFileTime ((WORD)((DWORD)HeaderData.FileTime >> 16), (WORD)HeaderData.FileTime, &filetime);
			LocalFileTimeToFileTime (&filetime, &pItem->ftLastWriteTime);

			//???
			pItem->ftCreationTime = pItem->ftLastAccessTime = pItem->ftLastWriteTime;
			pItem->dwCRC32 = HeaderData.FileCRC;

			memcpy(pItem, pItem, sizeof(ArchiveItem));

			return E_SUCCESS;
		}
	}

	return ConvertResult (nResult);

}

int WcxPlugin::ProcessFile(HANDLE hArchive, int OpMode, const TCHAR* lpDestPath, const TCHAR* lpDestName)
{
	AnsiGuard guard;

	int nResult = E_BROKEN; //??

	UNIVERSAL_ANSI_NAME_CREATE(lpDestName);

	if ( m_pfnProcessFileW ) //unicode first
		nResult = m_pfnProcessFileW(hArchive, OpMode, NULL, lpDestNameW);
	else

	if ( m_pfnProcessFile )
		nResult = m_pfnProcessFile(hArchive, OpMode, NULL, lpDestNameA);

	UNIVERSAL_NAME_DELETE(lpDestName);

	return ConvertResult(nResult);
}

WcxArchive* WcxPlugin::OpenCreateArchive(
		const GUID& uidFormat, 
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	return new WcxArchive(this, uidFormat, lpFileName, hCallback, pfnCallback, bCreate);
}


void WcxPlugin::CloseArchive(WcxArchive* pArchive)
{
	delete pArchive;
}

int WcxPlugin::QueryArchive(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result)
{
	AnsiGuard guard;
	bool bResult = false;

	UNIVERSAL_ANSI_NAME_CREATE(lpFileName);

	if ( m_pfnCanYouHandleThisFileW ) //unicode first
		 bResult = m_pfnCanYouHandleThisFileW(lpFileNameW);
	else

	if ( m_pfnCanYouHandleThisFile )
		bResult = m_pfnCanYouHandleThisFile(lpFileNameA);
	else
	{
		HANDLE hArchive = OpenArchive(lpFileName, PK_OM_LIST);

		if ( hArchive )
		{
			bResult = true;
			CloseArchive(hArchive);
		}
	}

	UNIVERSAL_NAME_DELETE(lpFileName);

	if ( bResult )
	{
		ArchiveQueryResult* pResult = new ArchiveQueryResult;

		pResult->uidPlugin = m_uid;
		pResult->uidFormat = m_pFormatInfo->uid;

		result.add(pResult);
	}

	return 1;
}


int WcxPlugin::DeleteFiles(const TCHAR* lpPackedFile, void* pNamesList)
{
	AnsiGuard guard;
	int nResult = E_BROKEN; //????

	UNIVERSAL_ANSI_NAME_CREATE(lpPackedFile);

	if ( m_pfnDeleteFilesW )
		nResult = m_pfnDeleteFilesW(lpPackedFileW, (const wchar_t*)pNamesList);
	else

	if ( m_pfnDeleteFiles )
		nResult = m_pfnDeleteFiles(lpPackedFileA, (const char*)pNamesList);

	UNIVERSAL_NAME_DELETE(lpPackedFile);

	return ConvertResult(nResult);
}

int WcxPlugin::PackFiles(
		const TCHAR* lpPackedFile, 
		const TCHAR* SubPath, 
		const TCHAR* SrcPath, 
		void* pNamesList,
		int Flags
		)
{
	//это бред, надо его будет в класс запихать (без этого не работает, нет). 
	string strPackedFile = lpPackedFile;
	CutToSlash(strPackedFile);
	SetCurrentDirectory(strPackedFile);

	AnsiGuard guard;

	int nResult = E_BROKEN; //????

	UNIVERSAL_ANSI_NAME_CREATE(lpPackedFile);
	UNIVERSAL_ANSI_NAME_CREATE(SubPath);
	UNIVERSAL_ANSI_NAME_CREATE(SrcPath);


	if ( m_pfnPackFilesW )
		nResult = m_pfnPackFilesW(lpPackedFileW, *SubPathW?SubPathW:0, SrcPathW, (const wchar_t*)pNamesList, PK_PACK_SAVE_PATHS);
	else

	if ( m_pfnPackFiles )
		nResult = m_pfnPackFiles(lpPackedFileA, *SubPathA?SubPathA:0, SrcPathA, (const char*)pNamesList, PK_PACK_SAVE_PATHS);

	UNIVERSAL_NAME_DELETE(SrcPath);
	UNIVERSAL_NAME_DELETE(SubPath);
	UNIVERSAL_NAME_DELETE(lpPackedFile);

	return ConvertResult(nResult);
}

int WcxPlugin::GetPackerCaps()
{
	AnsiGuard guard;

	if ( m_pfnGetPackerCaps )
		return m_pfnGetPackerCaps();

	return 0;
}


void WcxPlugin::ConfigurePacker()
{
	AnsiGuard guard;

	HWND hwnd = (HWND)Info.AdvControl(Info.ModuleNumber, ACTL_GETFARHWND, 0);

	if ( m_pfnConfigurePacker )
		m_pfnConfigurePacker(hwnd, m_hModule);
}

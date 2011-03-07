#include "observer.h"

ObserverPlugin::ObserverPlugin(const GUID& uid)
{
	m_pFormatInfo = NULL;
	m_uid = uid;
}

bool ObserverPlugin::Load(const TCHAR* lpModuleName)
{
	m_strModuleName = lpModuleName;
		
	m_hModule = LoadLibraryEx(
			lpModuleName, 
			0, 
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnLoadSubModule = (LoadSubModuleFunc)GetProcAddress(m_hModule, "LoadSubModule");
		m_pfnOpenStorage = (OpenStorageFunc)GetProcAddress(m_hModule, "OpenStorage");
		m_pfnCloseStorage = (CloseStorageFunc)GetProcAddress(m_hModule, "CloseStorage");
		m_pfnGetStorageItem = (GetItemFunc)GetProcAddress(m_hModule, "GetStorageItem");
		m_pfnExtract = (ExtractFunc)GetProcAddress(m_hModule, "ExtractItem");

		if ( m_pfnOpenStorage && m_pfnCloseStorage && m_pfnGetStorageItem )
		{
			m_pFormatInfo = new ArchiveFormatInfo;
			memset(m_pFormatInfo, 0, sizeof(ArchiveFormatInfo));

			m_pFormatInfo->uid = CreateFormatUID(m_uid, FSF.PointToName(lpModuleName)); 
			m_pFormatInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_NEED_EXTERNAL_NOTIFICATIONS;

			string strName = FSF.PointToName(lpModuleName);
			strName += _T(" [observer]");

			m_pFormatInfo->lpName = StrDuplicate(strName);
			m_pFormatInfo->lpDefaultExtention = StrDuplicate(strName); //BUGBUG

			return true;
		}
	}

	return false;
}

const GUID& ObserverPlugin::GetUID()
{
	return m_uid;
}

const TCHAR* ObserverPlugin::GetModuleName()
{
	return m_strModuleName;
}

const ArchiveFormatInfo* ObserverPlugin::GetFormats()
{
	return m_pFormatInfo;
}

unsigned int ObserverPlugin::GetNumberOfFormats()
{
	return 1;
}


int ObserverPlugin::QueryArchives(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result)
{
	StorageGeneralInfo info;

	INT_PTR* hArchive = OpenStorage(lpFileName, &info);

	if ( hArchive )
	{
		CloseStorage(hArchive);

		ArchiveQueryResult* pResult = new ArchiveQueryResult;

		pResult->uidFormat = m_pFormatInfo->uid;
		pResult->uidPlugin = m_uid;

		result.add(pResult);

		return 1;
	}

	return 0;
}

ObserverArchive* ObserverPlugin::OpenArchive(
		const GUID& uid, 
		const TCHAR* lpFileName, 
		HANDLE hCallback, 
		ARCHIVECALLBACK pfnCallback
		)
{
	return new ObserverArchive(this, uid, lpFileName, hCallback, pfnCallback);
}

void ObserverPlugin::CloseArchive(ObserverArchive* pArchive)
{
	delete pArchive;
}


INT_PTR* ObserverPlugin::OpenStorage(const TCHAR* lpFileName, StorageGeneralInfo* pInfo)
{

#ifdef UNICODE
	const wchar_t* lpName = lpFileName;
#else
	wchar_t* lpName = AnsiToUnicode(lpFileName);
#endif

	INT_PTR* hResult = NULL;

	if ( m_pfnOpenStorage )
	{
		INT_PTR* hArchive;

		if ( m_pfnOpenStorage(lpName, &hArchive, pInfo) == TRUE )
			hResult = hArchive;
	}

#ifdef UNICODE
#else
	free(lpName);
#endif


	return hResult;
}

void ObserverPlugin::CloseStorage(INT_PTR* hArchive)
{
	if ( m_pfnCloseStorage )
		m_pfnCloseStorage(hArchive);
}


ObserverPlugin::~ObserverPlugin()
{
	if ( m_pFormatInfo )
	{
		StrFree((void*)m_pFormatInfo->lpName);
		StrFree((void*)m_pFormatInfo->lpDefaultExtention);

		delete m_pFormatInfo;
	}

	if ( m_hModule )
		FreeLibrary(m_hModule);
}

int ObserverPlugin::GetStorageItem(INT_PTR* hArchive, int nIndex, ArchiveItem* pItem, unsigned int& uNumberOfFiles)
{
	int nResult = E_BROKEN;

	if ( m_pfnGetStorageItem )
	{
		WIN32_FIND_DATAW fdata;
		wchar_t wszTempBuffer[4096];

		nResult = m_pfnGetStorageItem(hArchive, nIndex, &fdata, wszTempBuffer, sizeof(wszTempBuffer));

		if ( nResult == GET_ITEM_NOMOREITEMS )
			return E_EOF;

		if ( nResult == GET_ITEM_OK )
		{
			memset(pItem, 0, sizeof(ArchiveItem));

#ifdef UNICODE
			pItem->lpFileName = StrDuplicate(wszTempBuffer);
#else
			pItem->lpFileName = UnicodeToAnsi(wszTempBuffer); 
#endif
			pItem->dwFileAttributes = fdata.dwFileAttributes;
			pItem->nFileSize = ((__int64)fdata.nFileSizeHigh << 32)+(__int64)fdata.nFileSizeLow;
				
			memcpy(&pItem->ftCreationTime, &fdata.ftCreationTime, sizeof(FILETIME));
			memcpy(&pItem->ftLastAccessTime, &fdata.ftLastAccessTime, sizeof(FILETIME));
			memcpy(&pItem->ftLastWriteTime, &fdata.ftLastWriteTime, sizeof(FILETIME));
			
		    pItem->UserData = nIndex+1; 

			if ( !OptionIsOn(pItem->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
				uNumberOfFiles++;

		    nResult = E_SUCCESS;
		}
	}

	return nResult;
}



int ObserverPlugin::ExtractItem(
		INT_PTR* hArchive, 
		int nIndex,
		const TCHAR* lpDestPath, 
		ExtractProcessCallbacks* pCallbacks 
		)
{
	ExtractOperationParams params;

	memset(&params, 0, sizeof(params));
	memcpy(&params.callbacks, pCallbacks, sizeof(ExtractProcessCallbacks));

#ifdef UNICODE
	const wchar_t* lpDestPathCopy = lpDestPath;
#else
	wchar_t* lpDestPathCopy = AnsiToUnicode(lpDestPath);
#endif

	params.destFilePath = lpDestPathCopy;
	params.flags = 0;
	params.item = nIndex;

	int nResult = E_BROKEN; //TO DO REAL RESULT

	if ( m_pfnExtract )
		nResult = m_pfnExtract(hArchive, params); 

#ifdef UNICODE
#else
	free(lpDestPathCopy);
#endif

	return ConvertResult(nResult);
}

int ObserverPlugin::ConvertResult(int nResult)
{
	//BUGBUG
	return nResult;
}


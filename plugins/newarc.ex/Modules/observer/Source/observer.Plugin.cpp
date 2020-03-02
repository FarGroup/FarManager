#include "observer.h"

ObserverPlugin::ObserverPlugin(const GUID& uid)
{
	m_pFormatInfo = NULL;
	m_uid = uid;

	m_pfnLoadSubModule = nullptr;
	m_pfnUnloadSubModule = nullptr;

	m_pfnOpenStorage = nullptr;
	m_pfnCloseStorage = nullptr;
	m_pfnGetStorageItem = nullptr;
	m_pfnExtract = nullptr;
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
		m_pfnUnloadSubModule = (UnloadSubModuleFunc)GetProcAddress(m_hModule, "UnloadSubModule");
		
		if ( m_pfnLoadSubModule && m_pfnUnloadSubModule ) //в обсервере так
		{
			ModuleLoadParameters params;
			memset(&params, 0, sizeof(params));

			if ( m_pfnLoadSubModule(&params) )
			{
				m_pfnOpenStorage = params.OpenStorage;
				m_pfnCloseStorage = params.CloseStorage;
				m_pfnGetStorageItem = params.GetItem;
				m_pfnExtract = params.ExtractItem;

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

	HANDLE hArchive = OpenStorage(lpFileName, &info);

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


HANDLE ObserverPlugin::OpenStorage(const TCHAR* lpFileName, StorageGeneralInfo* pInfo)
{

#ifdef UNICODE
	const wchar_t* lpName = lpFileName;
#else
	wchar_t* lpName = AnsiToUnicode(lpFileName);
#endif

	HANDLE hResult = NULL;

	if ( m_pfnOpenStorage )
	{
		HANDLE hArchive;
		StorageOpenParams params;

		params.FilePath = lpFileName;
		params.Password = nullptr; //oops

		if ( m_pfnOpenStorage(params, &hArchive, pInfo) == TRUE )
			hResult = hArchive;
	}

#ifdef UNICODE
#else
	free(lpName);
#endif


	return hResult;
}

void ObserverPlugin::CloseStorage(HANDLE hArchive)
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

	if ( m_pfnUnloadSubModule )
		m_pfnUnloadSubModule();

	if ( m_hModule )
		FreeLibrary(m_hModule);
}

int ObserverPlugin::GetStorageItem(HANDLE hArchive, int nIndex, ArchiveItem* pItem, unsigned int& uNumberOfFiles)
{
	int nResult = E_BROKEN;

	if ( m_pfnGetStorageItem )
	{
		StorageItemInfo item;
		//WIN32_FIND_DATAW fdata;
		//wchar_t wszTempBuffer[4096];

		nResult = m_pfnGetStorageItem(hArchive, nIndex, &item);
		//fdata, wszTempBuffer, sizeof(wszTempBuffer));

		if ( nResult == GET_ITEM_NOMOREITEMS )
			return E_EOF;

		if ( nResult == GET_ITEM_OK )
		{
			memset(pItem, 0, sizeof(ArchiveItem));

#ifdef UNICODE
			pItem->lpFileName = StrDuplicate(item.Path);
#else
			pItem->lpFileName = UnicodeToAnsi(item.Path); 
#endif
			pItem->dwFileAttributes = item.Attributes;
			pItem->nFileSize = item.Size;
				
			memcpy(&pItem->ftCreationTime, &item.CreationTime, sizeof(FILETIME));
			//memcpy(&pItem->ftLastAccessTime, &fdata.ftLastAccessTime, sizeof(FILETIME));
			memcpy(&pItem->ftLastWriteTime, &item.ModificationTime, sizeof(FILETIME));
			
		    pItem->UserData = nIndex+1; 

			if ( !OptionIsOn(pItem->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
				uNumberOfFiles++;

		    nResult = E_SUCCESS;
		}
	}

	return nResult;
}



int ObserverPlugin::ExtractItem(
		HANDLE hArchive, 
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


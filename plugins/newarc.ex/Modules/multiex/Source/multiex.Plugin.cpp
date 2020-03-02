#include "MultiEx.h"

MultiExPlugin::MultiExPlugin(const GUID& uid)
{
	m_uid = uid;

	m_pfnMpGetInterfaceVersion = nullptr;
	m_pfnMpGetPluginInfo = nullptr;
	m_pfnMpGetFormatCount = nullptr;
	m_pfnMpGetFormatInfo = nullptr;
	m_pfnMpGetOptions = nullptr;
	m_pfnMpSetOption = nullptr;
	m_pfnMpOpenArchive = nullptr;
	m_pfnMpOpenArchiveBindStream = nullptr;
	m_pfnMpCloseArchive = nullptr;
	m_pfnMpIndexCount = nullptr;
	m_pfnMpIndexedInfo = nullptr;
	m_pfnMpFindInfo = nullptr;
	m_pfnMpFindFirstFile = nullptr;
	m_pfnMpFindNextFile = nullptr;
	m_pfnMpFindClose = nullptr;
	m_pfnMpIsFileAnArchive = nullptr;
	m_pfnMpIsStreamAnArchive = nullptr;
	m_pfnMpExportFileByNameToFile = nullptr;
	m_pfnMpExportFileByIndexToFile = nullptr;
	m_pfnMpExportFileByNameToStream = nullptr;
	m_pfnMpExportFileByIndexToStream = nullptr;
	m_pfnMpImportFileFromFile = nullptr;
	m_pfnMpImportFileFromStream = nullptr;
	m_pfnMpRemoveFileByName = nullptr;
	m_pfnMpRemoveFileByIndex = nullptr;
	m_pfnMpGetLastError = nullptr;
	m_pfnMpGetErrorText = nullptr;
}

bool MultiExPlugin::Load(const TCHAR* lpModuleName)
{
	m_strModuleName = lpModuleName;

	m_hModule = LoadLibraryEx(
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnMpGetInterfaceVersion = (MPGETINTERFACEVERSION)GetProcAddress(m_hModule, "mpGetInterfaceVersion");
		m_pfnMpGetPluginInfo = (MPGETPLUGININFO)GetProcAddress(m_hModule, "mpGetPluginInfo");
		m_pfnMpGetFormatCount = (MPGETFORMATCOUNT)GetProcAddress(m_hModule, "mpGetFormatCount");
		m_pfnMpGetFormatInfo = (MPGETFORMATINFO)GetProcAddress(m_hModule, "mpGetFormatInfo");
		m_pfnMpGetOptions = (MPGETOPTIONS)GetProcAddress(m_hModule, "mpGetOptions");
		m_pfnMpSetOption = (MPSETOPTION)GetProcAddress(m_hModule, "mpSetOptions");
		m_pfnMpOpenArchive = (MPOPENARCHIVE)GetProcAddress(m_hModule, "mpOpenArchive");
		m_pfnMpOpenArchiveBindStream = (MPOPENARCHIVEBINDSTREAM)GetProcAddress(m_hModule, "mpOpenArchiveBindStream");
		m_pfnMpCloseArchive = (MPCLOSEARCHIVE)GetProcAddress(m_hModule, "mpCloseArchive");
		m_pfnMpIndexCount = (MPINDEXCOUNT)GetProcAddress(m_hModule, "mpIndexCount");
		m_pfnMpIndexedInfo = (MPINDEXEDINFO)GetProcAddress(m_hModule, "mpIndexedInfo");
		m_pfnMpFindInfo = (MPFINDINFO)GetProcAddress(m_hModule, "mpFindInfo");
		m_pfnMpFindFirstFile = (MPFINDFIRSTFILE)GetProcAddress(m_hModule, "mpFindFirstFile");
		m_pfnMpFindNextFile = (MPFINDNEXTFILE)GetProcAddress(m_hModule, "mpFindNextFile");
		m_pfnMpFindClose = (MPFINDCLOSE)GetProcAddress(m_hModule, "mpFindClose");
		m_pfnMpIsFileAnArchive = (MPISFILEANARCHIVE)GetProcAddress(m_hModule, "mpIsFileAnArchive");
		m_pfnMpIsStreamAnArchive = (MPISSTREAMANARCHIVE)GetProcAddress(m_hModule, "mpIsStreamAnArchive");
		m_pfnMpExportFileByNameToFile = (MPEXPORTFILEBYNAMETOFILE)GetProcAddress(m_hModule, "mpExportFileByNameToFile");
		m_pfnMpExportFileByIndexToFile = (MPEXPORTFILEBYINDEXTOFILE)GetProcAddress(m_hModule, "mpExportFileByIndexToFile");
		m_pfnMpExportFileByNameToStream = (MPEXPORTFILEBYNAMETOSTREAM)GetProcAddress(m_hModule, "mpExportFileByNameToStream");
		m_pfnMpExportFileByIndexToStream = (MPEXPORTFILEBYINDEXTOSTREAM)GetProcAddress(m_hModule, "mpExportFileByIndexToStream");
		m_pfnMpImportFileFromFile = (MPIMPORTFILEFROMFILE)GetProcAddress(m_hModule, "mpImportFileFromFile");
		m_pfnMpImportFileFromStream = (MPIMPORTFILEFROMSTREAM)GetProcAddress(m_hModule, "mpImportFileFromStream");
		m_pfnMpRemoveFileByName = (MPREMOVEFILEBYNAME)GetProcAddress(m_hModule, "mpRemoveFileByName");
		m_pfnMpRemoveFileByIndex = (MPREMOVEFILEBYINDEX)GetProcAddress(m_hModule, "mpRemoveFileByIndex");
		m_pfnMpGetLastError = (MPGETLASTERROR)GetProcAddress(m_hModule, "mpGetLastError");
		m_pfnMpGetErrorText = (MPGETERRORTEXT)GetProcAddress(m_hModule, "mpGetErrorText");

		if ( m_pfnMpGetFormatCount )
		{
			/*MpPluginInfo pinfo;
			memset(&pinfo, 0, sizeof(pinfo));

			pinfo.Size = sizeof(pinfo);

			m_pfnMpGetPluginInfo(pinfo);

			DWORD mj, mn;
			m_pfnMpGetInterfaceVersion(mj, mn);
			*/
			DWORD dwFormats = m_pfnMpGetFormatCount();

			MpFormatInfo finfo;
			finfo.Size = sizeof(finfo);


			for (int i = 0; i < dwFormats; i++)
			{
				if ( m_pfnMpGetFormatInfo(i, finfo) )
				{
					ArchiveFormatInfo* info = m_pFormatInfo.add();


#ifdef UNICODE
					info->lpName = AnsiToUnicode(finfo.GameName);
					info->lpDefaultExtention = AnsiToUnicode(finfo.FileMask);
#else
					info->lpName = StrDuplicate(finfo.GameName);
					info->lpDefaultExtention = StrDuplicate(finfo.FileMask);
#endif

					info->uid = CreateFormatUID(m_uid, info->lpName);
					info->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_NEED_EXTERNAL_NOTIFICATIONS;
				}
			}
		}

		return true;
	}

	return false;
}

const GUID& MultiExPlugin::GetUID()
{
	return m_uid;
}

const ArchiveFormatInfo* MultiExPlugin::GetFormats()
{
	return m_pFormatInfo.data();
}

unsigned int MultiExPlugin::GetNumberOfFormats()
{
	return m_pFormatInfo.count();
}

const TCHAR* MultiExPlugin::GetModuleName()
{
	return m_strModuleName;
}

MultiExPlugin::~MultiExPlugin()
{
	for (int i = 0; i < m_pFormatInfo.count(); i++)
	{
		StrFree((void*)m_pFormatInfo[i].lpName);
		StrFree((void*)m_pFormatInfo[i].lpDefaultExtention);
	}

	FreeLibrary (m_hModule);
}


int MultiExPlugin::QueryArchives(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result)
{
	AnsiGuard guard;
	AnsiString strFileName(lpFileName, CP_ACP);

	for (int i = 0; i < m_pFormatInfo.count(); i++)
	{
		if ( m_pfnMpIsFileAnArchive(i, strFileName) )
		{
			ArchiveQueryResult* pResult = new ArchiveQueryResult;

			pResult->uidPlugin = m_uid;
			pResult->uidFormat = m_pFormatInfo[i].uid;

			result.add(pResult);
		}				
	}
	
	return 0;
}


int MultiExPlugin::QueryArchive(const GUID& uidFormat, const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result)
{
 	AnsiGuard guard;
	AnsiString strFileName(lpFileName, CP_ACP);

	int index = GetFormatIndex(uidFormat);

	if ( index != -1 )
	{
		if ( m_pfnMpIsFileAnArchive(index, strFileName) )
		{
			ArchiveQueryResult* pResult = new ArchiveQueryResult;

			pResult->uidPlugin = m_uid;
			pResult->uidFormat = uidFormat;

			result.add(pResult);
		}
	}

	return 0;
}

MultiExArchive* MultiExPlugin::OpenCreateArchive(
		const GUID& uidFormat, 
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	if ( bCreate )
		return NULL; //теоретически оно как-бы да, но проверить не на чем

	return new MultiExArchive(this, uidFormat, lpFileName, hCallback, pfnCallback);
}


void MultiExPlugin::CloseArchive(MultiExArchive* pArchive)
{
	delete pArchive;
} 

int MultiExPlugin::OpenArchive(int nFormatIndex, const TCHAR* lpFileName)
{
	AnsiGuard guard;
	int nResult;

	ANSI_NAME_CREATE(lpFileName);

	if ( !m_pfnMpOpenArchive(nResult, nFormatIndex, lpFileNameA, OPENFLAG_OPENALWAYS) )
		nResult = 0;

	ANSI_NAME_DELETE(lpFileName);

	return nResult;
}

void MultiExPlugin::CloseArchive(int hArchive)
{
	AnsiGuard guard;

	m_pfnMpCloseArchive(hArchive);
}


int MultiExPlugin::GetArchiveItem(int hArchive, int& hSearch, ArchiveItem* pItem)
{
	AnsiGuard guard;

	bool bLast = false;

	if ( hSearch == NULL )
		hSearch = m_pfnMpFindFirstFile(hArchive, "*");
	else
		bLast = !m_pfnMpFindNextFile(hSearch);

	if ( hSearch == 0 )
		return E_BROKEN;

	AnsiString strFileSize = m_pfnMpFindInfo(hSearch, "FileSize");

	pItem->nFileSize = atoi(strFileSize);

#ifdef UNICODE
	pItem->lpFileName = AnsiToUnicode(m_pfnMpFindInfo(hSearch, "FileName"));
#else
	pItem->lpFileName = StrDuplicate(m_pfnMpFindInfo(hSearch, "FileName"));
#endif

	if ( bLast )
	{
		m_pfnMpFindClose(hSearch);
		hSearch = 0;

		return E_EOF;
	}

	return E_SUCCESS;
}

void MultiExPlugin::FreeArchiveItem(int hArchive, ArchiveItem* pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);
}

bool MultiExPlugin::Delete(int hArchive, const ArchiveItem* pItem, int nItemsNumber)
{
	return false; 
}

bool MultiExPlugin::Extract(
		int hArchive, 
		const ArchiveItem* pItem, 
		int nItemsNumber, 
		const TCHAR* lpDestDiskPath, 
		const TCHAR* lpPathInArchive
		)
{
	AnsiGuard guard;

	AnsiString strFileName;
	AnsiString strDestFile;

	for (int i = 0; i < nItemsNumber; i++)
	{
		strFileName.SetData(pItem[i].lpFileName, CP_ACP);

		strDestFile.SetData(lpDestDiskPath, CP_ACP);
		strDestFile += strFileName;

		m_pfnMpExportFileByNameToFile(hArchive, strFileName, strDestFile);
	}

	return true;
}

bool MultiExPlugin::AddFiles(
		int hArchive,
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpSourceDiskFile,
		const TCHAR* lpPathInArchive
		)
{
	return false; 
}

int MultiExPlugin::GetFormatIndex(const GUID& uidFormat)
{
	for (int i = 0; i < m_pFormatInfo.count(); i++)
		if ( m_pFormatInfo[i].uid == uidFormat )
			return i;

	return -1;
}
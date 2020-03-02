#include "observer.h"

ObserverArchive::ObserverArchive(
		ObserverPlugin* pPlugin, 
		const GUID& uid,
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_uid = uid;

	m_pPlugin = pPlugin;
	m_strFileName = lpFileName;

	m_hCallback = hCallback;
	m_pfnCallback = pfnCallback;

	m_bOpened = false;

	m_bArchiveInfoAdded = false;

	m_nIndex = 0;
	m_uNumberOfFiles = 0;
}

const GUID& ObserverArchive::GetUID()
{
	return m_uid;
}

ObserverArchive::~ObserverArchive()
{
	for (unsigned int i = 0; i < m_pArchiveInfo.count(); i++)
	{
		StrFree((void*)m_pArchiveInfo[i].lpName);
		StrFree((void*)m_pArchiveInfo[i].lpValue);
	}

	Close();
}

bool ObserverArchive::Open()
{
	if ( !m_bOpened )
	{
		memset(&m_Info, 0, sizeof(StorageGeneralInfo));

		HANDLE hArchive = m_pPlugin->OpenStorage(m_strFileName, &m_Info);

		if ( hArchive )
		{
			m_hArchive = hArchive;
			m_bOpened = true;
		}
	}

	return m_bOpened;
}

void ObserverArchive::Close()
{
	if ( m_bOpened )
	{
		m_pPlugin->CloseStorage(m_hArchive);
		m_bOpened = false;
		m_hArchive = NULL;
	}
}

bool ObserverArchive::StartOperation(int nOperation, bool bInternal)
{
	if ( !bInternal )
		Close();
	else
	{
		if ( !Open() )
			return false;
	}

	return true;
}

bool ObserverArchive::EndOperation(int nOperation, bool bInternal)
{
	return true;
}
	
int ObserverArchive::GetArchiveItem(ArchiveItem *pItem)
{
	int nResult = m_pPlugin->GetStorageItem(m_hArchive, m_nIndex++, pItem, m_uNumberOfFiles);

	if ( nResult == E_EOF )
	{
		if ( !m_bArchiveInfoAdded )
		{
			ArchiveInfoItem* item;
			string strValue;
				
			if ( *m_Info.Format )
			{
				item = m_pArchiveInfo.add();
				item->lpName = StrDuplicate(_T("Format"));
#ifdef UNICODE
				item->lpValue = StrDuplicate(m_Info.Format);
#else
				char* lpTemp = UnicodeToAnsi(m_Info.Format);
				item->lpValue = StrDuplicate(lpTemp);
				free(lpTemp);
#endif
			}

			if ( *m_Info.Compression )
			{
				item = m_pArchiveInfo.add();
				item->lpName = StrDuplicate(_T("Compression"));
#ifdef UNICODE
				item->lpValue = StrDuplicate(m_Info.Compression);
#else
				char* lpTemp = UnicodeToAnsi(m_Info.Compression);
				item->lpValue = StrDuplicate(lpTemp);
				free(lpTemp);
#endif
			}

			if ( *m_Info.Comment )
			{
				item = m_pArchiveInfo.add();
				item->lpName = StrDuplicate(_T("Comment"));
#ifdef UNICODE
				item->lpValue = StrDuplicate(m_Info.Comment);
#else
				char* lpTemp = UnicodeToAnsi(m_Info.Comment);
				item->lpValue = StrDuplicate(lpTemp);
				free(lpTemp);
#endif
			}

			FILETIME time;
			SYSTEMTIME stime;

			FileTimeToLocalFileTime(&m_Info.Created, &time);
			FileTimeToSystemTime(&time, &stime);

			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Creation time"));

			strValue.Format(_T("%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d"), stime.wDay, stime.wMonth, stime.wYear, stime.wHour, stime.wMinute, stime.wSecond);
			item->lpValue = StrDuplicate(strValue);

			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Number of files"));

			strValue.Format(_T("%d"), m_uNumberOfFiles);
			item->lpValue = StrDuplicate(strValue);

			m_bArchiveInfoAdded = true;
		}
	}

	return nResult;
}

bool ObserverArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);

	return true;
}

int ObserverArchive::GetResult(int nSuccessCount, int nItemsNumber, bool bUserAbort)
{
	if ( bUserAbort )
		return RESULT_CANCEL;

	if ( nSuccessCount > nItemsNumber )
		__debug(_T("LOGIC ERROR, PLEASE REPORT"));

	if ( nSuccessCount == 0 )
		return RESULT_ERROR;

	if ( nSuccessCount < nItemsNumber )
		return RESULT_PARTIAL;

	return RESULT_SUCCESS;
}

int ObserverArchive::Extract(
		const ArchiveItem *pItems, 
		int nItemsNumber, 
		const TCHAR *lpDestPath, 
		const TCHAR *lpCurrentFolder
		)
{
	int nSuccessCount = 0;
	bool bUserAbort = false;

	ExtractProcessCallbacks callbacks;

	ProgressContextEx pc;

	pc.hArchive = this; ///AAAA!!!
	pc.ctx.nTotalProcessedBytes = 0;
	pc.ctx.nProcessedFileBytes = 0;

	m_uProcessedBytes = 0;

	callbacks.FileProgress = (ExtractProgressFunc)OnExtractProgress;
	callbacks.signalContext = &pc;

	Callback(AM_START_OPERATION, OPERATION_EXTRACT, NULL);

	for (int i = 0; i < nItemsNumber; i++)
	{
		ProcessFileStruct PF;

		string strDestName;

		strDestName = lpDestPath;  //make AUTO
		strDestName += pItems[i].lpFileName+_tcslen(lpCurrentFolder);

		PF.lpDestFileName = strDestName;
		PF.pItem = &pItems[i];

		int nOverwrite = Callback(AM_PROCESS_FILE, 0, (LONG_PTR)&PF);

		if ( nOverwrite == PROCESS_OVERWRITE )
		{
			if ( pItems[i].UserData )
			{
				if ( (pItems[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
					apiCreateDirectoryEx(strDestName);
				else
				{
					apiCreateDirectoryForFile(strDestName);

					int nResult = m_pPlugin->ExtractItem(m_hArchive, pItems[i].UserData-1, strDestName, &callbacks);
					
					if ( nResult == SER_SUCCESS)
						nSuccessCount++;

					if ( nResult == SER_USERABORT )
						bUserAbort = true;
				}
			}
		}

		if ( nOverwrite == PROCESS_CANCEL )
			bUserAbort = true;

		if ( bUserAbort )
			break;
	}		
		
	return GetResult(nSuccessCount, nItemsNumber, bUserAbort);
}

int ObserverArchive::GetArchiveInfo(const ArchiveInfoItem** pItems)
{
	if ( m_pArchiveInfo.count() )
	{
		*pItems = m_pArchiveInfo.data();
		return m_pArchiveInfo.count();
	}

	return 0;
}


LONG_PTR ObserverArchive::Callback(int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback(m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}

//

int __stdcall ObserverArchive::OnExtractProgress(ProgressContextEx* pContext, unsigned __int64 uProcessedBytes)
{
	ObserverArchive* pArchive = (ObserverArchive*)pContext->hArchive;

	ProcessDataStruct PD;

	memset(&PD, 0, sizeof(ProcessDataStruct));

	PD.uProcessedSize = uProcessedBytes;//-pArchive->m_uProcessedBytes;

	pArchive->m_uProcessedBytes += PD.uProcessedSize;

	if ( !pArchive->Callback(AM_PROCESS_DATA, 0, (LONG_PTR)&PD) )
		return FALSE;

	return TRUE;
}



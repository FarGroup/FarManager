#include "wcx.h"

WcxArchive::WcxArchive(
		WcxPlugin* pPlugin, 
		const GUID& uid, 
		const TCHAR *lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	//m_bCreate = bCreate;

	m_uid = uid;
	m_pPlugin = pPlugin;

	m_strFileName = lpFileName;
	m_hArchive = NULL;

	m_hCallback = hCallback;
	m_pfnCallback = pfnCallback;

	bProcessDataProc = false;


//Ex thunks doesn't work under VC10 DEBUG, no x64 support anyway, so...
//#ifdef _DEBUG
	CreateClassThunk(WcxArchive, OnProcessData, m_pfnProcessData);
	CreateClassThunk(WcxArchive, OnProcessDataW, m_pfnProcessDataW);
	CreateClassThunk(WcxArchive, OnChangeVol, m_pfnChangeVol);
	CreateClassThunk(WcxArchive, OnChangeVolW, m_pfnChangeVolW);
//#else
//	m_pfnProcessData = CreateThunkFastEx(this, (void*)OnProcessDataThunk);
//	m_pfnSetChangeVol = CreateThunkFastEx(this, (void*)OnChangeVolThunk);
//	m_pfnProcessDataW = CreateThunkFastEx(this, (void*)OnProcessDataThunkW);
//	m_pfnSetChangeVolW = CreateThunkFastEx(this, (void*)OnChangeVolThunkW);
//#endif
}

const GUID& WcxArchive::GetUID()
{
	return m_uid;
}

WcxArchive::~WcxArchive()
{
	ReleaseThunkEx (m_pfnProcessData);
	ReleaseThunkEx (m_pfnChangeVol);
	ReleaseThunkEx (m_pfnProcessDataW);
	ReleaseThunkEx (m_pfnChangeVolW);
}


bool WcxArchive::StartOperation(int nOperation, bool bInternal)
{
	if ( !bInternal )
		return true;

	m_hArchive = NULL;

	if ( nOperation == OPERATION_LIST )
		m_hArchive = m_pPlugin->OpenArchive(m_strFileName, PK_OM_LIST);
	else

	if ( nOperation == OPERATION_EXTRACT )
		m_hArchive = m_pPlugin->OpenArchive(m_strFileName, PK_EXTRACT);
	else

	if ( nOperation == OPERATION_TEST )
		m_hArchive = m_pPlugin->OpenArchive(m_strFileName, PK_TEST);

	m_pPlugin->SetCallbacks(
			m_hArchive, 
			(tProcessDataProcW)m_pfnProcessDataW, 
			(tProcessDataProc)m_pfnProcessData, 
			(tChangeVolProcW)m_pfnChangeVolW, 
			(tChangeVolProc)m_pfnChangeVol
			);


	return (m_hArchive != NULL) || (nOperation == OPERATION_ADD) || (nOperation == OPERATION_DELETE);
}

bool WcxArchive::EndOperation(int nOperation, bool bInternal)
{
	if ( !bInternal )
		return true;

	if ( m_hArchive )
		m_pPlugin->CloseArchive(m_hArchive);

	return true;
}

int WcxArchive::GetArchiveItem(ArchiveItem* pItem)
{
	int nResult = m_pPlugin->GetArchiveItem(m_hArchive, pItem);

	if ( nResult == E_SUCCESS )
		m_pPlugin->ProcessFile(m_hArchive, PK_SKIP, NULL, NULL);

	return nResult;
}

void WcxArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);
}



int WcxArchive::GetResult(int nItemsNumber)
{
	if ( m_bUserAbort )
		return RESULT_CANCEL;

	if ( m_nSuccessCount > nItemsNumber )
		__debug(_T("LOGIC ERROR, PLEASE REPORT"));

	if ( m_nSuccessCount == 0 )
		return RESULT_ERROR;

	if ( m_nSuccessCount < nItemsNumber )
		return RESULT_PARTIAL;

	return RESULT_SUCCESS;
}


int WcxArchive::Extract(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpDestPath,
		const TCHAR* lpCurrentFolder
		)
{
	m_nSuccessCount = 0;
	m_bUserAbort = false;

	bool bFound;

	string strCurrentFileName;

	Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	while ( true )
	{
		int nProcessResult = 0;

		ArchiveItem item; //???

		if ( m_pPlugin->GetArchiveItem(m_hArchive, &item) != 0 )  //do not free this item!!!
			break; 

		strCurrentFileName = item.lpFileName;

		bFound = false;

		for (int i = 0; i < nItemsNumber; i++)
		{
			if ( !_tcscmp(pItems[i].lpFileName, strCurrentFileName) )
			{
				string strDestName = lpDestPath;

				AddEndSlash(strDestName);

				const TCHAR* lpName = strCurrentFileName;

				if ( *lpCurrentFolder /*&& !strncmp (
						lpCurrentFolder,
						strlen (lpCurrentFolder)
						)*/ )
					lpName += _tcslen(lpCurrentFolder);

				if ( *lpName == _T('\\') )
					lpName++;

				strDestName += lpName;

				ProcessFileStruct pfs;

				pfs.pItem = &pItems[i];
				pfs.lpDestFileName = strDestName;

				int nOverwrite = Callback(AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

				if ( nOverwrite == PROCESS_CANCEL )
				{
					m_bUserAbort = true;
					goto l_1;
				}

				if ( nOverwrite == PROCESS_OVERWRITE )
				{
					bProcessDataProc = true;
		
					if ( (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
					{
						apiCreateDirectoryEx(strDestName);
						nProcessResult = m_pPlugin->ProcessFile(m_hArchive, PK_SKIP, NULL, NULL);
					}
					else
					{
						apiCreateDirectoryForFile(strDestName);
						nProcessResult = m_pPlugin->ProcessFile(m_hArchive, PK_EXTRACT, NULL, strDestName);
					}

					bProcessDataProc = false;
				}	

				bFound = true;
				break;
			}
		}

		if ( !bFound )
			nProcessResult = m_pPlugin->ProcessFile(m_hArchive, PK_SKIP, NULL, NULL);

		if ( nProcessResult == E_SUCCESS )
			m_nSuccessCount++;

	}

l_1:

	return GetResult(nItemsNumber);
}

LONG_PTR WcxArchive::Callback(int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}


int WcxArchive::Delete(const ArchiveItem *pItems, int nItemsNumber)
{
	void* list = NULL;

	__int64 nTotalSize = m_pPlugin->CreateFileList(pItems, nItemsNumber, ARCHIVE_OPERATION_DELETE, &list);

	StartOperationStruct SO;

	SO.uTotalFiles = nItemsNumber;
	SO.uTotalSize = nTotalSize;
	SO.dwFlags = OS_FLAG_TOTALFILES|OS_FLAG_TOTALSIZE;

	Callback(AM_START_OPERATION, OPERATION_DELETE, (LONG_PTR)&SO);
	//Callback(AM_PROCESS_FILE, 0, 0);

	int nResult = (m_pPlugin->DeleteFiles(m_strFileName, list) == E_SUCCESS)?RESULT_SUCCESS:RESULT_ERROR; //rave!!!

	free(list);

	return nResult;
}

int WcxArchive::AddFiles(
		const ArchiveItem *pItems, 
		int nItemsNumber, 
		const TCHAR *lpSourcePath, 
		const TCHAR *lpCurrentPath
		)
{
	void* list = NULL;

	__int64 nTotalSize = m_pPlugin->CreateFileList(pItems, nItemsNumber, ARCHIVE_OPERATION_ADD, &list);
	
	StartOperationStruct SO;

	SO.uTotalFiles = nItemsNumber;
	SO.uTotalSize = nTotalSize;
	SO.dwFlags = OS_FLAG_TOTALFILES|OS_FLAG_TOTALSIZE;

	Callback(AM_START_OPERATION, OPERATION_ADD, (LONG_PTR)&SO);

	int nResult = (m_pPlugin->PackFiles(
			m_strFileName, 
			lpCurrentPath, 
			lpSourcePath, 
			list,
			0
			) == E_SUCCESS)?RESULT_SUCCESS:RESULT_ERROR;


	free(list);

	return nResult;
}



int WcxArchive::OnProcessData(const char* FileName, int Size)
{
	if ( m_pfnCallback /*&& bProcessDataProc*/ )
	{
	/*	ArchiveItem item;

		memset(&item, 0, sizeof(ArchiveItem));

#ifdef UNICODE
		item.lpFileName = AnsiToUnicode(FileName);
#else
		item.lpFileName = (TCHAR*)FileName;
#endif

		ProcessFileStruct pfs;
		memset(&pfs, 0, sizeof(ProcessFileStruct));

		pfs.pItem = &item;

		if ( (Size < 0) && (Size > -1000) ) //percents
		{
			//чушь собачья
			//Size = (__x/100)*(-Size);
			//__x -= Size;
		}

		m_pfnCallback (m_hCallback, AM_PROCESS_FILE, 0, (int)&pfs);

		int nResult = (int)m_pfnCallback (m_hCallback, AM_PROCESS_DATA, 0, (LONG_PTR)Size);

		free((void*)item.lpFileName);

		return nResult;*/
	}

	return 1;
}

int WcxArchive::OnChangeVol(const char* ArcName, int Mode)
{
	return 1;
}

int WcxArchive::OnProcessDataW(const wchar_t* FileName, int Size)
{
	if ( m_pfnCallback /*&& bProcessDataProc*/ )  //???
	{
	/*	ProcessDataStruct DS;
		bool bResult = true;

		if ( (Size <= -1) && (Size >= -100) ) //first percent
		{
			DS.nMode = PROGRESS_PERCENTS;
			DS.cPercents = (char)(-Size);
			DS.cTotalPercents = -1;
		}
		else

		if ( (Size <= -1000) && (Size >= -1100) ) //second percent
		{
			DS.nMode = PROGRESS_PERCENTS;
			DS.cPercents = -1;
			DS.cTotalPercents = (char)(-Size-1000);
		}
		else

		if ( Size > 0 )
		{
			DS.nMode = PROGRESS_PROCESSED_DIFF;
			DS.uProcessedDiff = (unsigned __int64)Size;
		}
		else
			bResult = false;

		if ( bResult )
			return (int)m_pfnCallback (m_hCallback, AM_PROCESS_DATA, 0, (LONG_PTR)&DS);  */
	}

	return 1;
}

int WcxArchive::OnChangeVolW(const wchar_t* ArcName, int Mode)
{
	return 1;
}

//thunks, NEVER USER BREAKPOINTS HERE, OR YOU WILL GO MAD!
//And yes, no optimize!

//callbacks
#pragma optimize("", off)

int __stdcall WcxArchive::OnChangeVolThunk(const char* ArcName, int Mode)
{
	WcxArchive *p = (WcxArchive*)THUNK_MAGIC;
	return p->OnChangeVol(ArcName, Mode);
}

int __stdcall WcxArchive::OnProcessDataThunk(const char* FileName, int Size)
{
	WcxArchive *p = (WcxArchive*)THUNK_MAGIC;
	return p->OnProcessData(FileName, Size);
}

int __stdcall WcxArchive::OnChangeVolThunkW(const wchar_t *ArcName, int Mode)
{
	WcxArchive *p = (WcxArchive*)THUNK_MAGIC;
	return p->OnChangeVolW(ArcName, Mode);
}


int __stdcall WcxArchive::OnProcessDataThunkW(const wchar_t *FileName, int Size)
{
	WcxArchive *p = (WcxArchive*)THUNK_MAGIC;
	return p->OnProcessDataW(FileName, Size);
}

#pragma optimize("", on)

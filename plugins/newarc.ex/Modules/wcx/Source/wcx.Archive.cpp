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



void CreateDirs(const TCHAR *lpFileName)
{
	string strNameCopy = lpFileName;

	CutToSlash(strNameCopy);
	apiCreateDirectoryEx(strNameCopy);
}



bool WcxArchive::Extract(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpDestPath,
		const TCHAR* lpCurrentFolder
		)
{
	int nProcessed = 0;
	int nResult = 0;

	bool bFound;

	string strCurrentFileName;

	Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	while ( /*m_hArchive &&*/ nResult == 0 )
	{
		ArchiveItem item;

		nResult = m_pPlugin->GetArchiveItem(m_hArchive, &item); //do not free this item!!!

		if ( nResult == 0 )
		{
			strCurrentFileName = item.lpFileName;

			bFound = false;

			for (int i = 0; i < nItemsNumber; i++)
			{
				if ( !_tcscmp (pItems[i].lpFileName, strCurrentFileName) )
				{
					string strDestName = lpDestPath;

					AddEndSlash (strDestName);

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

					int nOverwrite = Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

					if ( nOverwrite == RESULT_CANCEL )
						goto l_1;

					if ( nOverwrite == RESULT_OVERWRITE )
					{
						int nProcessResult = 0;

						bProcessDataProc = true;
		
						if ( (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
						{
							apiCreateDirectoryEx(strDestName);
							nProcessResult = m_pPlugin->ProcessFile (m_hArchive, PK_SKIP, NULL, NULL);
						}
						else
						{
							CreateDirs(strDestName);
							nProcessResult = m_pPlugin->ProcessFile (m_hArchive, PK_EXTRACT, NULL, strDestName);
						}

						bProcessDataProc = false;

						if ( nProcessResult == E_SUCCESS )
							nProcessed++;

						if ( /*m_bAborted ||*/ (nProcessResult != E_SUCCESS) || (nProcessed == nItemsNumber) )
							goto l_1;
					}	

					bFound = true;

					break;
				}
			}

			if ( !bFound )
				m_pPlugin->ProcessFile (m_hArchive, PK_SKIP, NULL, NULL);
		}
	}

l_1:

	return nProcessed!=0;
}

LONG_PTR WcxArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}


bool WcxArchive::Delete(const ArchiveItem *pItems, int nItemsNumber)
{
	void* list = NULL;

	__int64 nTotalSize = m_pPlugin->CreateFileList(pItems, nItemsNumber, ARCHIVE_OPERATION_DELETE, &list);

	StartOperationStruct SO;

	SO.uTotalFiles = nItemsNumber;
	SO.uTotalSize = nTotalSize;
	SO.dwFlags = OS_FLAG_TOTALFILES|OS_FLAG_TOTALSIZE;

	Callback(AM_START_OPERATION, OPERATION_DELETE, (LONG_PTR)&SO);
	//Callback(AM_PROCESS_FILE, 0, 0);

	bool bResult = m_pPlugin->DeleteFiles(m_strFileName, list) == E_SUCCESS; //rave!!!

	free(list);

	return bResult;
}

bool WcxArchive::AddFiles(
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

	bool bResult = m_pPlugin->PackFiles(
			m_strFileName, 
			lpCurrentPath, 
			lpSourcePath, 
			list,
			0
			) == E_SUCCESS;


	free(list);

	return bResult;
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
	if ( m_pfnCallback /*&& bProcessDataProc*/ )
	{
		/*ProcessDataStruct DS;

		DS.uProcessedSize = Size;

		if ( (Size < 0) && (Size > -1000) ) //percents
		{
			int nRealSize = 0;//(__x*abs(Size))/100;

			return (int)m_pfnCallback (m_hCallback, AM_PROCESS_DATA, 0, (LONG_PTR)&DS);
		}

		if ( Size >= 0 )
		{
			return (int)m_pfnCallback (m_hCallback, AM_PROCESS_DATA, 0, (LONG_PTR)&DS);
		}*/
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

#include "ace.h"

// {76BD43C8-484E-497D-8C29-4BBD2373416E}
MY_DEFINE_GUID(CLSID_PluginACE, 0x76bd43c8, 0x484e, 0x497d, 0x8c, 0x29, 0x4b, 0xbd, 0x23, 0x73, 0x41, 0x6e);

extern "C" const GUID CLSID_FormatACE;

AcePlugin::AcePlugin()
{
	m_pFormatInfo = NULL;

	m_pfnInitDll = nullptr;
	m_pfnReadArchiveData = nullptr;
	m_pfnList = nullptr;
	m_pfnTest = nullptr;
	m_pfnExtract = nullptr;
	m_pfnAdd = nullptr;
	m_pfnDelete = nullptr;
};

const GUID& AcePlugin::GetUID()
{
	return CLSID_PluginACE;
}

const TCHAR* AcePlugin::GetModuleName()
{
	return m_strModuleName;
}

unsigned int AcePlugin::GetNumberOfFormats()
{
	return 1;
}

const ArchiveFormatInfo* AcePlugin::GetFormats()
{
	return m_pFormatInfo;
}

bool AcePlugin::Load(const TCHAR* lpModuleName)
{
	m_strModuleName = lpModuleName;

	m_hModule = LoadLibraryEx (
			m_strModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnInitDll = (ACEINITDLL)GetProcAddress (m_hModule, "ACEInitDll");
		m_pfnReadArchiveData = (ACEREADARCHIVEDATA)GetProcAddress (m_hModule, "ACEReadArchiveData");
		m_pfnList = (ACELIST)GetProcAddress (m_hModule, "ACEList");
		m_pfnTest = (ACETEST)GetProcAddress (m_hModule, "ACETest");
		m_pfnExtract = (ACEEXTRACT)GetProcAddress (m_hModule, "ACEExtract");

		m_pfnAdd = (ACEADD)GetProcAddress (m_hModule, "ACEAdd");
		m_pfnDelete = (ACEDELETE)GetProcAddress (m_hModule, "ACEDelete");

		bool bUpdate = false;

		if ( m_pfnAdd || m_pfnDelete )
			bUpdate = true;

		if ( Initialize() )
		{
			m_pFormatInfo = new ArchiveFormatInfo;
			memset(m_pFormatInfo, 0, sizeof(m_pFormatInfo));

			m_pFormatInfo->uid = CLSID_FormatACE;
			m_pFormatInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;

			if ( bUpdate )
				m_pFormatInfo->dwFlags |= (AFF_SUPPORT_INTERNAL_DELETE|AFF_SUPPORT_INTERNAL_ADD|AFF_SUPPORT_INTERNAL_CREATE);

			m_pFormatInfo->lpName = _T("ACE Archive");
			m_pFormatInfo->lpDefaultExtention = _T("ace");
		
			return true;
		}
	}

	return false;
}

AcePlugin::~AcePlugin()
{
	if ( m_pFormatInfo )
		delete m_pFormatInfo;

	if ( m_hModule )
		FreeModule(m_hModule);
}

int __stdcall AcePlugin::InfoProc(pACEInfoCallbackProcStruc Info)
{
	AcePlugin* pPlugin = (AcePlugin*)Info->Global.GlobalData->Obj;
	return pPlugin->OnInfo(Info);
}

int __stdcall AcePlugin::StateProc(pACEStateCallbackProcStruc State)
{
	AcePlugin* pPlugin = (AcePlugin*)State->Archive.GlobalData->Obj;
	return pPlugin->OnState(State);
}

int __stdcall AcePlugin::RequestProc(pACERequestCallbackProcStruc Request)
{
	AcePlugin* pPlugin = (AcePlugin*)Request->Global.GlobalData->Obj;
	return pPlugin->OnRequest(Request);
}

int __stdcall AcePlugin::ErrorProc(pACEErrorCallbackProcStruc Error)
{
	AcePlugin *pPlugin = (AcePlugin*)Error->Global.GlobalData->Obj;
	return pPlugin->OnError(Error);
}

bool AcePlugin::Initialize()
{
	if ( m_pfnInitDll )
	{
		tACEInitDllStruc DllData;

		memset(&DllData, 0, sizeof(DllData));   // set all fields to zero

		DllData.GlobalData.Obj = (void*)this;

		DllData.GlobalData.MaxArchiveTestBytes = 0x2ffFF; // search for archive
														  // header in first 256k of file
		DllData.GlobalData.MaxFileBufSize = 0x2ffFF; // read/write buffer size
													 // is 256k

		DllData.GlobalData.Comment.BufSize = sizeof(m_CommentBuf);
		DllData.GlobalData.Comment.Buf = m_CommentBuf; // set comment bufffer
                                                       // to receive comments
                                                       // of archive and/or
                                                       // set comments

		char szTempPath[MAX_PATH];
		GetTempPathA(MAX_PATH, szTempPath);

		DllData.GlobalData.TempDir = szTempPath; // set temp dir

		// set callback function pointers
		DllData.GlobalData.InfoCallbackProc = InfoProc;
		DllData.GlobalData.ErrorCallbackProc = ErrorProc;
		DllData.GlobalData.RequestCallbackProc = RequestProc;
		DllData.GlobalData.StateCallbackProc = StateProc;

		return m_pfnInitDll(&DllData) == ACE_ERROR_NOERROR;
	}

	return false;
}

struct ACEHEADER
{
  WORD  CRC16;        // CRC16 over block
  WORD  HeaderSize;   // size of the block(from HeaderType)
  BYTE  HeaderType;   // archive header type is 0
  WORD  HeaderFlags;
  BYTE  Signature[7]; // '**ACE**'
  BYTE  VerExtract;   // version needed to extract archive
  BYTE  VerCreate;    // version used to create the archive
  BYTE  Host;         // HOST-OS for ACE used to create the archive
  BYTE  VolumeNum;    // which volume of a multi-volume-archive is it?
  DWORD AcrTime;      // date and time in MS-DOS format
  BYTE  Reserved[8];  // 8 bytes reserved for the future
};


//too simple
bool IsACE(const unsigned char* pBuffer, DWORD dwBufferSize)
{
	for (int i = 0; i < dwBufferSize-sizeof(ACEHEADER); i++)
	{
		ACEHEADER* pHeader = (ACEHEADER*)(pBuffer+i);

		if ( !memcmp(pHeader, "**ACE**", 7) )
		{
			if ( pHeader->HeaderType == 0 )
				return true;
		}
	}

	return false;
}

bool AcePlugin::QueryArchive(
		const TCHAR* lpFileName, 
		const unsigned char* pBuffer,
		DWORD dwBufferSize,
		ArchiveQueryResult* pResult)
{
	bool bResult = false;

	if ( IsACE(pBuffer, dwBufferSize) )
	{
/*	if ( m_pfnList )
	{
		OEM_NAME_CREATE_CONST(lpFileName);

		m_nMode = -1; //???
		m_bIsArchive = true;

		tACEListStruc List;

		memset(&List, 0, sizeof(List)); 

		List.Files.SourceDir = "";        // archive main directory is
                                      // base directory for FileList
		List.Files.FileList = "<>";  // set FileList
		List.Files.ExcludeList = "";        // no files to exclude
		List.Files.FullMatch = 0;         // also list files partially matching
                                      // (for instance: list DIR1\TEST.DAT
		if ( m_pfnList(lpFileNameA, &List) == ACE_ERROR_NOERROR )
		{
			if ( m_bIsArchive ) //oops, magic
			{*/
				pResult->uidFormat = m_pFormatInfo->uid;
				pResult->uidPlugin = CLSID_PluginACE;

				bResult = true;
/*			}
		}

		OEM_NAME_DELETE_CONST(lpFileName);
	}
*/
	}

	return bResult;
}

AceArchive* AcePlugin::OpenCreateArchive(
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	return new AceArchive(this, lpFileName, hCallback, pfnCallback, bCreate);
}

void AcePlugin::CloseArchive(AceArchive* pArchive)
{
	delete pArchive;
}

int __stdcall AcePlugin::AceListThread(void* pParam)
{
	AceArchiveHandle* pHandle = (AceArchiveHandle*)pParam;

	tACEListStruc List;

	memset(&List, 0, sizeof(List));     // set all fields to zero

	pHandle->pPlugin->m_nMode = OPERATION_LIST; //BUGBUG!!!

	List.Files.SourceDir = "";        // archive main directory is
                                      // base directory for FileList
	List.Files.FileList = "*";  // set FileList
	List.Files.ExcludeList = "";        // no files to exclude
	List.Files.FullMatch = 0;         // also list files partially matching
                                      // (for instance: list DIR1\TEST.DAT
                                      //  if FileList specifies TEST.DAT)
    pHandle->pPlugin->m_pfnList(pHandle->lpFileName, &List);

    StrFree((void*)pHandle->lpFileName);
    delete pHandle;

	ExitThread (0);
}

HANDLE AcePlugin::OpenArchive(const TCHAR* lpFileName, bool bList)
{
	AceArchiveHandle* pHandle = new AceArchiveHandle;

#ifdef UNICODE
	pHandle->lpFileName = UnicodeToAnsi(lpFileName);
#else
	pHandle->lpFileName = StrDuplicate(lpFileName);
#endif
	pHandle->pPlugin = this;
	pHandle->bList = bList;

	if ( bList )
	{
		DWORD dwThreadId;

		m_hListEvent = CreateEvent(NULL, false, false, NULL);
		m_hListEventComplete = CreateEvent(NULL, false, false, NULL);
		m_hListThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AceListThread, (void*)pHandle, 0, &dwThreadId);
	}

	return (HANDLE)pHandle;
}


void AcePlugin::CloseArchive(HANDLE hArchive)
{
	AceArchiveHandle* pHandle = (AceArchiveHandle*)hArchive;

	if ( pHandle->bList )
	{
		DWORD dwExitCode;

		do {
			if ( GetExitCodeThread (m_hListThread, &dwExitCode) &&
				 (dwExitCode == STILL_ACTIVE) )
		 		SetEvent (m_hListEvent);
			else
				break;
		} while ( true ); //???wait for thread??

		CloseHandle (m_hListThread);
		CloseHandle (m_hListEvent);
		CloseHandle (m_hListEventComplete);
	}

	free((void*)pHandle->lpFileName);
	delete pHandle;
}

int AcePlugin::GetArchiveItem(ArchiveItem* pItem)
{
	DWORD dwExitCode;

	if ( GetExitCodeThread (m_hListThread, &dwExitCode) &&
		 (dwExitCode == STILL_ACTIVE) )
	{
		m_pItem = pItem;

		SetEvent (m_hListEvent);
		Sleep (0); //leave this thread, now! 8-)

		WaitForSingleObject(m_hListEventComplete, INFINITE);

		return E_SUCCESS;
	}
	else
		return E_EOF;
}


void AcePlugin::FreeArchiveItem(ArchiveItem *pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);
}

int AcePlugin::OnInfo(pACEInfoCallbackProcStruc Info)
{
	return ACE_CALLBACK_RETURN_OK;
}

int AcePlugin::OnState(pACEStateCallbackProcStruc State)
{
	if ( State->StructureType == ACE_CALLBACK_TYPE_ARCHIVEDFILE )
	{

		if ( State->ArchivedFile.Operation == ACE_CALLBACK_OPERATION_ADD )
		{
			//MessageBox (0, "asd", State->ArchivedFile.FileData->SourceFileName, MB_OK);
		}


		if ( State->ArchivedFile.Code == ACE_CALLBACK_STATE_STARTFILE )
		{
			if ( m_nMode == OPERATION_LIST/*OM_LIST*/ )
			{
				WaitForSingleObject (m_hListEvent, INFINITE);

#ifdef UNICODE
				m_pItem->lpFileName = AnsiToUnicode(State->ArchivedFile.FileData->SourceFileName);
#else
				m_pItem->lpFileName = StrDuplicate(State->ArchivedFile.FileData->SourceFileName);
#endif

				m_pItem->nFileSize = State->ArchivedFile.FileData->Size;
				m_pItem->dwFileAttributes = State->ArchivedFile.FileData->Attributes;

				m_pItem->dwCRC32 = State->ArchivedFile.FileData->CRC32;
				m_pItem->nPackSize = State->ArchivedFile.FileData->CompressedSize;

				DWORD DateTime = State->ArchivedFile.FileData->Time;

				FILETIME time;

				DosDateTimeToFileTime (HIWORD(DateTime), LOWORD(DateTime), &time);
				LocalFileTimeToFileTime(&time, &m_pItem->ftLastWriteTime);

				SetEvent (m_hListEventComplete);
			}
			else
			{
				if ( (State->ArchivedFile.Operation == ACE_CALLBACK_OPERATION_EXTRACT) || 
					 (State->ArchivedFile.Operation == ACE_CALLBACK_OPERATION_ADD) )
					
				{
					//BUGBUG
					ArchiveItem item;

					memset (&item, 0, sizeof (item));
				
#ifdef UNICODE
					item.lpFileName = AnsiToUnicode(State->ArchivedFile.FileData->SourceFileName);
#else
					item.lpFileName = StrDuplicate(State->ArchivedFile.FileData->SourceFileName);
#endif
					item.nFileSize = State->ArchivedFile.FileData->Size;

					ProcessFileStruct pfs;

					pfs.pItem = &item;
					pfs.lpDestFileName = item.lpFileName;

					//Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

					StrFree((void*)item.lpFileName);
				}
			}
		}
	}


	if ( (State->StructureType == ACE_CALLBACK_TYPE_PROGRESS) &&
		 ((State->Progress.Operation == ACE_CALLBACK_OPERATION_EXTRACT) || 
		  (State->Progress.Operation == ACE_CALLBACK_OPERATION_ADD)) )
	{
		if ( m_dwLastProcessed == 0 )
		{
			StartOperationStruct os;

			os.uTotalFiles = 1;
			os.uTotalSize = State->Progress.ProgressData->TotalSize+State->Progress.ProgressData->TotalCompressedSize;
			os.dwFlags = OS_FLAG_TOTALFILES|OS_FLAG_TOTALSIZE;

			//Callback (AM_START_OPERATION, OPERATION_ADD, (LONG_PTR)&os);
			//Callback (AM_PROCESS_FILE, 0, 0);
		}

		//unsigned __int64 diff = State->Progress.ProgressData->TotalProcessedSize-m_dwLastProcessed;

		//if ( !Callback (AM_PROCESS_DATA, 0, (LONG_PTR)diff) ) //__int64 to int, but it's just a diff (change API?) BUGBUG!!!
		//	return ACE_CALLBACK_RETURN_CANCEL;

		m_dwLastProcessed = State->Progress.ProgressData->TotalProcessedSize;
	}

	return ACE_CALLBACK_RETURN_OK;
}

int AcePlugin::OnRequest(pACERequestCallbackProcStruc Request)
{
	return ACE_CALLBACK_RETURN_OK;
}

int AcePlugin::OnError(pACEErrorCallbackProcStruc Error)
{
	if ( (Error->StructureType == ACE_CALLBACK_TYPE_ARCHIVE) &&
		 (Error->Archive.Code == ACE_CALLBACK_ERROR_ISNOTANARCHIVE) &&
		 (m_nMode == -1) ) //QueryArchive
		m_bIsArchive = false;

	return ACE_CALLBACK_RETURN_CANCEL;
}

void __CreateFileList(const ArchiveItem* pItems, int nItemsNumber, int nFolderLength, void** pResult)
{       
	char* pBuffer = (char*)malloc(0);

	int newsize = 0;
	int size = 0;

	for (int i = 0; i < nItemsNumber; i++)
	{
#ifdef UNICODE
		char *name = UnicodeToAnsi(pItems[i].lpFileName);
#else
		const char *name = pItems[i].lpFileName;
#endif

		name = name+nFolderLength;

		newsize += strlen(name)+1;

		pBuffer = (char*)realloc(pBuffer, newsize);

		//memset (&extract.Files.FileList[size], 0, newsize-size-1);

		strcat (pBuffer, name);

		if ( i != (nItemsNumber-1) )
			pBuffer[newsize-1] = 0x0D;

		size = newsize;

#ifdef UNICODE
		free(name);
#endif
	}

	*pResult = pBuffer;
}



bool AcePlugin::Extract (
		HANDLE hArchive,
		const ArchiveItem *pItems,
		int nItemsNumber,
		const TCHAR *lpDestDiskPath,
		const TCHAR *lpPathInArchive
		)
{
	bool bResult = false;

	AceArchiveHandle* handle = (AceArchiveHandle*)hArchive;

	if ( m_pfnExtract )
	{
		sACEExtractStruc extract;
		DWORD size = 0;
		DWORD newsize = 0;

		memset (&extract, 0, sizeof (extract));

		__CreateFileList(pItems, nItemsNumber, StrLength(lpPathInArchive), (void**)&extract.Files.FileList);

#pragma message("ERROR!!!!");
		extract.Files.SourceDir = (char*)lpPathInArchive;
		extract.Files.FullMatch = TRUE;
		extract.DestinationDir = (char*)lpDestDiskPath;

		//if ( m_pfnCallback )
		//	m_pfnCallback (m_hCallback, AM_START_OPERATION, OPERATION_EXTRACT, 0);

		m_dwLastProcessed = 0;

		bool bResult = (m_pfnExtract(handle->lpFileName, &extract) == ACE_ERROR_NOERROR);

		free(extract.Files.FileList);
	}

	return bResult;
}

bool AcePlugin::AddFiles(
		HANDLE hArchive, 
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpSourceDiskPath,
		const TCHAR* lpPathInArchive
		)
{
	bool bResult = false;

	AceArchiveHandle* handle = (AceArchiveHandle*)hArchive;

	if ( m_pfnAdd )
	{
		tACEAddStruc add;

		m_dwLastProcessed = 0;

		memset (&add, 0, sizeof (add));

		__CreateFileList(pItems, nItemsNumber, 0, (void**)&add.Files.FileList);

		add.Files.ExcludeList = "";             // no files to exclude
		add.DestinationDir = ""; // archive directory to add to
		add.Files.SourceDir = (char*)lpSourceDiskPath;
		add.Files.FullMatch = TRUE;
		add.Files.RecurseSubDirs = true;
		add.EncryptPassword = "";             // do not encrypt files
		add.DecryptPassword = "";             // no encrypted files expected
		add.SFXName = ACE_SFX_NONE;

		add.CompressParams.Dictionary = 20;
		add.CompressParams.Level = ACE_LEVEL_BEST;
		add.CompressParams.V20Compression.DoUse = 1;

		bResult = (m_pfnAdd(handle->lpFileName, &add) == ACE_ERROR_NOERROR);

		free (add.Files.FileList);
	}

	return bResult;
}


bool AcePlugin::Delete (
		HANDLE hArchive,
		const ArchiveItem *pItems,
		int nItemsNumber
		)
{
	bool bResult;
	AceArchiveHandle* handle = (AceArchiveHandle*)hArchive;

	if ( m_pfnDelete )
	{
		tACEDeleteStruc del;

		memset (&del, 0, sizeof (del));

		__CreateFileList(pItems, nItemsNumber, 0, (void**)&del.Files.FileList);

		del.Files.ExcludeList = "";
		del.Files.FullMatch = false;
		del.Files.RecurseSubDirs = TRUE;
		del.Files.SourceDir = "";
		del.DecryptPassword  = ""; 

        bool bResult = m_pfnDelete(handle->lpFileName, &del);

		free (del.Files.FileList);
	}

	return bResult;
}


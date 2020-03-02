#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "Ace.class.h"
#include "ace.h"

MY_DEFINE_GUID (CLSID_FormatACE, 0x08183761, 0x87D3, 0x4C85, 0xBF, 0xB7, 0x7C, 0x5E, 0x8A, 0x7F, 0x81, 0xEA);

AceModule::AceModule ()
{
	char *lpModuleName = StrDuplicate(Info.ModuleName, 260);

	CutToSlash(lpModuleName);

	strcat (lpModuleName, "unacev2.dll");

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
		m_bSupportUpdate = false;
	else
	{
		CutToSlash(lpModuleName);

		strcat (lpModuleName, "acev2.dll");

		m_hModule = LoadLibraryEx (
				lpModuleName,
				NULL,
				LOAD_WITH_ALTERED_SEARCH_PATH
				);

		if ( m_hModule )
			m_bSupportUpdate = true;
	}

	if ( m_hModule )
	{
		m_pfnInitDll = (ACEINITDLL)GetProcAddress (m_hModule, "ACEInitDll");
		m_pfnReadArchiveData = (ACEREADARCHIVEDATA)GetProcAddress (m_hModule, "ACEReadArchiveData");
		m_pfnList = (ACELIST)GetProcAddress (m_hModule, "ACEList");
		m_pfnTest = (ACETEST)GetProcAddress (m_hModule, "ACETest");
		m_pfnExtract = (ACEEXTRACT)GetProcAddress (m_hModule, "ACEExtract");

		if ( m_bSupportUpdate )
		{
			m_pfnAdd = (ACEADD)GetProcAddress (m_hModule, "ACEAdd");
			m_pfnDelete = (ACEDELETE)GetProcAddress (m_hModule, "ACEDelete");
		}
	}

	StrFree (lpModuleName);
}

void AceModule::GetArchiveFormatInfo (ArchiveFormatInfo *pInfo)
{
	pInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;
	pInfo->uid = CLSID_FormatACE;

	if ( m_bSupportUpdate )
		pInfo->dwFlags |= (AFF_SUPPORT_INTERNAL_DELETE|AFF_SUPPORT_INTERNAL_ADD|AFF_SUPPORT_INTERNAL_CREATE);

	pInfo->lpName = "ACE Archive";
	pInfo->lpDefaultExtention = "ace";
}


AceModule::~AceModule ()
{
	FreeLibrary (m_hModule);
}

int __stdcall InfoProc (pACEInfoCallbackProcStruc Info)
{
	AceArchive *pArchive = (AceArchive*)Info->Global.GlobalData->Obj;
	return pArchive->OnInfo (Info);
}

int __stdcall StateProc (pACEStateCallbackProcStruc State)
{
	AceArchive *pArchive = (AceArchive*)State->Archive.GlobalData->Obj;
	return pArchive->OnState (State);
}

int __stdcall RequestProc (pACERequestCallbackProcStruc Request)
{
	AceArchive *pArchive = (AceArchive*)Request->Global.GlobalData->Obj;
	return pArchive->OnRequest (Request);
}

int __stdcall ErrorProc(pACEErrorCallbackProcStruc Error)
{
	AceArchive *pArchive = (AceArchive*)Error->Global.GlobalData->Obj;
	return pArchive->OnError (Error);
}

AceArchive::AceArchive (AceModule *pModule, const char *lpFileName, bool bNewFile)
{
	m_pModule = pModule;
	m_bNewFile = bNewFile;

	if ( m_pModule->m_pfnInitDll )
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
		GetTempPath (MAX_PATH, szTempPath);

		DllData.GlobalData.TempDir = szTempPath; // set temp dir

		// set callback function pointers
		DllData.GlobalData.InfoCallbackProc = InfoProc;
		DllData.GlobalData.ErrorCallbackProc = ErrorProc;
		DllData.GlobalData.RequestCallbackProc = RequestProc;
		DllData.GlobalData.StateCallbackProc = StateProc;

		m_pModule->m_pfnInitDll(&DllData);
	}

	m_lpFileName = StrDuplicate (lpFileName);
}

AceArchive::~AceArchive ()
{
	StrFree (m_lpFileName);
//	delete m_pModule;
}

bool AceArchive::IsArchive ()
{
	m_nMode = -1;
	m_bIsArchive = true;

	tACEListStruc List;

	memset(&List, 0, sizeof(List));     // set all fields to zero

	List.Files.SourceDir   = "";        // archive main directory is
                                      // base directory for FileList
	List.Files.FileList    = "<>";  // set FileList
	List.Files.ExcludeList = "";        // no files to exclude
	List.Files.FullMatch   = 0;         // also list files partially matching
                                      // (for instance: list DIR1\TEST.DAT
	m_pModule->m_pfnList (m_lpFileName, &List);

	return m_bIsArchive;
}

int __stdcall AceListThread (AceArchive *pArchive)
{
	tACEListStruc List;

	memset(&List, 0, sizeof(List));     // set all fields to zero

	List.Files.SourceDir   = "";        // archive main directory is
                                      // base directory for FileList
	List.Files.FileList    = "*";  // set FileList
	List.Files.ExcludeList = "";        // no files to exclude
	List.Files.FullMatch   = 0;         // also list files partially matching
                                      // (for instance: list DIR1\TEST.DAT
                                      //  if FileList specifies TEST.DAT)
    pArchive->m_pModule->m_pfnList (pArchive->m_lpFileName, &List);

	ExitThread (0);
}

bool __stdcall AceArchive::pOpenArchive (
		int nOpMode,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_nMode = nOpMode;
	m_pfnCallback = pfnCallback;

	if ( m_nMode == OM_LIST )
	{
		DWORD dwThreadId;

		m_hListEvent = CreateEvent (NULL, false, false, NULL);
		m_hListEventComplete = CreateEvent (NULL, false, false, NULL);
		m_hListThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)AceListThread, this, 0, &dwThreadId);
	}

	return true;
}

void __stdcall AceArchive::pCloseArchive ()
{
	if ( m_nMode == OM_LIST )
	{
		DWORD dwExitCode;

		do {
			if ( GetExitCodeThread (m_hListThread, &dwExitCode) &&
				 (dwExitCode == STILL_ACTIVE) )
				 SetEvent (m_hListEvent);
			else
				break;
		} while ( true );

		CloseHandle (m_hListThread);
		CloseHandle (m_hListEvent);
		CloseHandle (m_hListEventComplete);
	}
}

int __stdcall AceArchive::pGetArchiveItem (
		ArchiveItemInfo *pItem
		)
{
	DWORD dwExitCode;

	m_item = pItem;
	memset (m_item, 0, sizeof (ArchiveItemInfo));

	if ( GetExitCodeThread (m_hListThread, &dwExitCode) &&
		 (dwExitCode == STILL_ACTIVE) )
	{
		 SetEvent (m_hListEvent);
		 Sleep (0); //leave this thread, now! 8-)

		 WaitForSingleObject (m_hListEventComplete, INFINITE);

		 return E_SUCCESS;
	}
	else
		return E_EOF;
}

int __stdcall AceArchive::OnInfo (pACEInfoCallbackProcStruc Info)
{
	return ACE_CALLBACK_RETURN_OK;
}

int __stdcall AceArchive::OnState (pACEStateCallbackProcStruc State)
{
	if ( State->StructureType == ACE_CALLBACK_TYPE_ARCHIVEDFILE )
	{

		if ( State->ArchivedFile.Operation == ACE_CALLBACK_OPERATION_ADD )
		{
			//MessageBox (0, "asd", State->ArchivedFile.FileData->SourceFileName, MB_OK);

		}


		if ( State->ArchivedFile.Code == ACE_CALLBACK_STATE_STARTFILE )
		{
			if ( m_nMode == OM_LIST )
			{
				WaitForSingleObject (m_hListEvent, INFINITE);

				strcpy (m_item->pi.FindData.cFileName, State->ArchivedFile.FileData->SourceFileName);
				m_item->pi.FindData.nFileSizeLow = (DWORD)State->ArchivedFile.FileData->Size;
				m_item->pi.FindData.nFileSizeHigh = (DWORD)(State->ArchivedFile.FileData->Size >> 32);
				m_item->pi.FindData.dwFileAttributes = State->ArchivedFile.FileData->Attributes;

				m_item->pi.CRC32 = State->ArchivedFile.FileData->CRC32;
				m_item->pi.PackSize = (DWORD)State->ArchivedFile.FileData->CompressedSize;
				m_item->pi.PackSizeHigh = (DWORD)(State->ArchivedFile.FileData->CompressedSize >> 32);

				DWORD DateTime = State->ArchivedFile.FileData->Time;

				FILETIME time;

				DosDateTimeToFileTime (HIWORD(DateTime), LOWORD(DateTime), &time);
				LocalFileTimeToFileTime(&time, &m_item->pi.FindData.ftLastWriteTime);

				SetEvent (m_hListEventComplete);
			}
			else
			{
				if ( (State->ArchivedFile.Operation == ACE_CALLBACK_OPERATION_EXTRACT) || 
					 (State->ArchivedFile.Operation == ACE_CALLBACK_OPERATION_ADD) )
					
				{
					PluginPanelItem item;

					memset (&item, 0, sizeof (item));
					strcpy (item.FindData.cFileName, State->ArchivedFile.FileData->SourceFileName);

					item.FindData.nFileSizeLow = (DWORD)State->ArchivedFile.FileData->Size;
					item.FindData.nFileSizeHigh = (DWORD)(State->ArchivedFile.FileData->Size >> 32);

					ProcessFileStruct pfs;

					pfs.pItem = &item;
					pfs.lpDestFileName = (const char*)&item.FindData.cFileName;

					Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);
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
			OperationStructPlugin os;

			os.uTotalFiles = 1;
			os.uTotalSize = State->Progress.ProgressData->TotalSize+State->Progress.ProgressData->TotalCompressedSize;
			os.dwFlags = OS_FLAG_TOTALFILES|OS_FLAG_TOTALSIZE;

			Callback (AM_START_OPERATION, OPERATION_ADD, (LONG_PTR)&os);
			Callback (AM_PROCESS_FILE, 0, 0);
		}

		unsigned __int64 diff = State->Progress.ProgressData->TotalProcessedSize-m_dwLastProcessed;

		if ( !Callback (AM_PROCESS_DATA, 0, (LONG_PTR)diff) ) //__int64 to int, but it's just a diff (change API?) BUGBUG!!!
			return ACE_CALLBACK_RETURN_CANCEL;

		m_dwLastProcessed = State->Progress.ProgressData->TotalProcessedSize;
	}

	return ACE_CALLBACK_RETURN_OK;
}

int __stdcall AceArchive::OnRequest (pACERequestCallbackProcStruc Request)
{
	return ACE_CALLBACK_RETURN_OK;
}

int __stdcall AceArchive::OnError (pACEErrorCallbackProcStruc Error)
{
	if ( (Error->StructureType == ACE_CALLBACK_TYPE_ARCHIVE) &&
		 (Error->Archive.Code == ACE_CALLBACK_ERROR_ISNOTANARCHIVE) &&
		 (m_nMode == -1) ) //QueryArchive
		m_bIsArchive = false;

	return ACE_CALLBACK_RETURN_CANCEL;
}

bool __stdcall AceArchive::pExtract (
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	sACEExtractStruc extract;
	DWORD size = 0;
	DWORD newsize = 0;

	memset (&extract, 0, sizeof (extract));

	for (int i = 0; i < nItemsNumber; i++)
	{
		char *name = pItems[i].FindData.cFileName+StrLength(lpCurrentFolder);

		newsize += StrLength(name)+1;

		extract.Files.FileList = (char*)realloc (extract.Files.FileList, newsize);

		memset (&extract.Files.FileList[size], 0, newsize-size-1);

		strcat (extract.Files.FileList, name);

		if ( i != (nItemsNumber-1) )
			extract.Files.FileList[newsize-1] = 0x0D;

		size = newsize;
	}

	extract.Files.SourceDir = (char*)lpCurrentFolder;
	extract.Files.FullMatch = TRUE;
	extract.DestinationDir = (char*)lpDestPath;

	if ( m_pfnCallback )
		m_pfnCallback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	m_dwLastProcessed = 0;

	m_pModule->m_pfnExtract (m_lpFileName, &extract);

	free (extract.Files.FileList);

	return true;
}

bool __stdcall AceArchive::pAddFiles (
		const char *lpSourcePath,
		const char *lpCurrentPath,
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	m_nMode = OM_ADD; //BUGBUG!!!!

	tACEAddStruc add;
	DWORD size = 0;
	DWORD newsize = 0;

	m_dwLastProcessed = 0;

	memset (&add, 0, sizeof (add));

	char name[MAX_PATH];

	for (int i = 0; i < nItemsNumber; i++)
	{
		strcpy (name, pItems[i].FindData.cFileName);

		newsize += StrLength(name)+1;

		add.Files.FileList = (char*)realloc (add.Files.FileList, newsize);

		memset (&add.Files.FileList[size], 0, newsize-size-1);

		strcat (add.Files.FileList, name);

		if ( i != (nItemsNumber-1) )
			add.Files.FileList[newsize-1] = 0x0D;

		size = newsize;
	}

	add.Files.ExcludeList = "";             // no files to exclude
	add.DestinationDir = ""; // archive directory to add to
	add.Files.SourceDir = (char*)lpSourcePath;
	add.Files.FullMatch = TRUE;
	add.Files.RecurseSubDirs = true;
	add.EncryptPassword = "";             // do not encrypt files
	add.DecryptPassword = "";             // no encrypted files expected
	add.SFXName = ACE_SFX_NONE;

	add.CompressParams.Dictionary = 20;
	add.CompressParams.Level = ACE_LEVEL_BEST;
	add.CompressParams.V20Compression.DoUse = 1;

	m_pModule->m_pfnAdd (m_lpFileName, &add);

	free (add.Files.FileList);

	return true;
}

LONG_PTR __stdcall AceArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (nMsg, nParam1, nParam2);

	return FALSE;
}

bool __stdcall AceArchive::pDelete (
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	tACEDeleteStruc del;

	DWORD size = 0;
	DWORD newsize = 0;

	memset (&del, 0, sizeof (del));

	char name[MAX_PATH];

	for (int i = 0; i < nItemsNumber; i++)
	{
		strcpy (name, pItems[i].FindData.cFileName);

		newsize += StrLength(name)+1;

		del.Files.FileList = (char*)realloc (del.Files.FileList, newsize);

		memset (&del.Files.FileList[size], 0, newsize-size-1);

		strcat (del.Files.FileList, name);

		if ( i != (nItemsNumber-1) )
			del.Files.FileList[newsize-1] = 0x0D;

		size = newsize;
	}

	del.Files.ExcludeList = "";
	del.Files.FullMatch = false;
	del.Files.RecurseSubDirs = TRUE;
	del.Files.SourceDir = "";
	del.DecryptPassword  = ""; 


	m_pModule->m_pfnDelete (m_lpFileName, &del);

	free (del.Files.FileList);

	return true;
}
